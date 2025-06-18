// SPDX-License-Identifier: GPL-2.0-only
/* Based on labwc (https://github.com/labwc/labwc) */

#include "ext-workspace.h"
#include "../common/array.h"
#include "../common/list.h"
#include "../common/mem.h"
#include "ext-workspace-internal.h"
#include "ext-workspace-v1-protocol.h"
#include "transaction-addon.h"
#include <assert.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>

/*
 *	.--------------------.
 *	|        TODO        |
 *	|--------------------|
 *	| - go through xml   |
 *	|   and verify impl  |
 *	| - assert pub API   |
 *	`--------------------´
 *
 */

/* Only used within an assert() */
#ifndef NDEBUG
#define EXT_WORKSPACE_V1_VERSION 1
#endif

/*
 * Set when creating a new workspace state so we
 * don't end up having to send the state twice.
 */
#define WS_STATE_INVALID 0xffffffff

struct ws_create_workspace_event {
	char *name;
	struct {
		struct wl_listener transaction_op_destroy;
	} on;
};

/* Workspace */
static void workspace_handle_destroy(struct wl_client *client,
									 struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void workspace_handle_activate(struct wl_client *client,
									  struct wl_resource *resource) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (!addon) {
		/* Workspace was destroyed from the compositor side */
		return;
	}
	struct dwl_ext_workspace *workspace = addon->data;
	dwl_transaction_op_add(addon->ctx, WS_PENDING_WS_ACTIVATE, workspace,
						   /*data*/ NULL);
}

static void workspace_handle_deactivate(struct wl_client *client,
										struct wl_resource *resource) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (!addon) {
		/* Workspace was destroyed from the compositor side */
		return;
	}
	struct dwl_ext_workspace *workspace = addon->data;
	dwl_transaction_op_add(addon->ctx, WS_PENDING_WS_DEACTIVATE, workspace,
						   /*data*/ NULL);
}

static void workspace_handle_assign(struct wl_client *client,
									struct wl_resource *resource,
									struct wl_resource *new_group_resource) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (!addon) {
		/* Workspace was destroyed from the compositor side */
		return;
	}
	struct dwl_ext_workspace *workspace = addon->data;
	struct dwl_wl_resource_addon *grp_addon =
		wl_resource_get_user_data(new_group_resource);
	if (!grp_addon) {
		/* Group was destroyed from the compositor side */
		return;
	}
	struct dwl_ext_workspace_group *new_grp = grp_addon->data;
	dwl_transaction_op_add(addon->ctx, WS_PENDING_WS_ASSIGN, workspace,
						   new_grp);
}

static void workspace_handle_remove(struct wl_client *client,
									struct wl_resource *resource) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (!addon) {
		/* Workspace was destroyed from the compositor side */
		return;
	}
	struct dwl_ext_workspace *workspace = addon->data;
	dwl_transaction_op_add(addon->ctx, WS_PENDING_WS_REMOVE, workspace,
						   /*data*/ NULL);
}

static const struct ext_workspace_handle_v1_interface workspace_impl = {
	.destroy = workspace_handle_destroy,
	.activate = workspace_handle_activate,
	.deactivate = workspace_handle_deactivate,
	.assign = workspace_handle_assign,
	.remove = workspace_handle_remove,
};

static void workspace_instance_resource_destroy(struct wl_resource *resource) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (addon) {
		dwl_resource_addon_destroy(addon);
		wl_resource_set_user_data(resource, NULL);
	}

	wl_list_remove(wl_resource_get_link(resource));
}

static struct wl_resource *
workspace_resource_create(struct dwl_ext_workspace *workspace,
						  struct wl_resource *manager_resource,
						  struct dwl_transaction_session_context *ctx) {
	struct wl_client *client = wl_resource_get_client(manager_resource);
	struct wl_resource *resource =
		wl_resource_create(client, &ext_workspace_handle_v1_interface,
						   wl_resource_get_version(manager_resource), 0);
	if (!resource) {
		wl_client_post_no_memory(client);
		return NULL;
	}

	struct dwl_wl_resource_addon *addon = dwl_resource_addon_create(ctx);
	addon->data = workspace;

	wl_resource_set_implementation(resource, &workspace_impl, addon,
								   workspace_instance_resource_destroy);

	wl_list_insert(&workspace->resources, wl_resource_get_link(resource));
	return resource;
}

/* Workspace internal helpers */
static void workspace_send_state(struct dwl_ext_workspace *workspace,
								 struct wl_resource *target) {
	if (target) {
		ext_workspace_handle_v1_send_state(target, workspace->state);
	} else {
		struct wl_resource *resource;
		wl_resource_for_each(resource, &workspace->resources) {
			ext_workspace_handle_v1_send_state(resource, workspace->state);
		}
	}
}

static void workspace_send_initial_state(struct dwl_ext_workspace *workspace,
										 struct wl_resource *resource) {
	ext_workspace_handle_v1_send_capabilities(resource,
											  workspace->capabilities);
	if (workspace->coordinates.size > 0) {
		ext_workspace_handle_v1_send_coordinates(resource,
												 &workspace->coordinates);
	}
	if (workspace->name) {
		ext_workspace_handle_v1_send_name(resource, workspace->name);
	}
	if (workspace->id) {
		ext_workspace_handle_v1_send_id(resource, workspace->id);
	}
}

static void workspace_set_state(struct dwl_ext_workspace *workspace,
								enum ext_workspace_handle_v1_state state,
								bool enabled) {
	if (!!(workspace->state_pending & state) == enabled) {
		return;
	}

	if (enabled) {
		workspace->state_pending |= state;
	} else {
		workspace->state_pending &= ~state;
	}
	ext_manager_schedule_done_event(workspace->manager);
}

/* Group */
static void
ws_create_workspace_handle_transaction_op_destroy(struct wl_listener *listener,
												  void *data) {
	struct ws_create_workspace_event *ev =
		wl_container_of(listener, ev, on.transaction_op_destroy);
	wl_list_remove(&ev->on.transaction_op_destroy.link);
	free(ev->name);
	free(ev);
}

static void group_handle_create_workspace(struct wl_client *client,
										  struct wl_resource *resource,
										  const char *name) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (!addon) {
		return;
	}

	struct dwl_ext_workspace_group *group = addon->data;
	struct ws_create_workspace_event *ev = znew(*ev);
	ev->name = xstrdup(name);

	struct dwl_transaction_op *transaction_op =
		dwl_transaction_op_add(addon->ctx, WS_PENDING_WS_CREATE, group, ev);

	ev->on.transaction_op_destroy.notify =
		ws_create_workspace_handle_transaction_op_destroy;
	wl_signal_add(&transaction_op->events.destroy,
				  &ev->on.transaction_op_destroy);
}

static void group_handle_destroy(struct wl_client *client,
								 struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct ext_workspace_group_handle_v1_interface group_impl = {
	.create_workspace = group_handle_create_workspace,
	.destroy = group_handle_destroy,
};

static void group_instance_resource_destroy(struct wl_resource *resource) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (addon) {
		dwl_resource_addon_destroy(addon);
		wl_resource_set_user_data(resource, NULL);
	}
	wl_list_remove(wl_resource_get_link(resource));
}

static struct wl_resource *
group_resource_create(struct dwl_ext_workspace_group *group,
					  struct wl_resource *manager_resource,
					  struct dwl_transaction_session_context *ctx) {
	struct wl_client *client = wl_resource_get_client(manager_resource);
	struct wl_resource *resource =
		wl_resource_create(client, &ext_workspace_group_handle_v1_interface,
						   wl_resource_get_version(manager_resource), 0);
	if (!resource) {
		wl_client_post_no_memory(client);
		return NULL;
	}

	struct dwl_wl_resource_addon *addon = dwl_resource_addon_create(ctx);
	addon->data = group;

	wl_resource_set_implementation(resource, &group_impl, addon,
								   group_instance_resource_destroy);

	wl_list_insert(&group->resources, wl_resource_get_link(resource));
	return resource;
}

/* Group internal helpers */
static void group_send_state(struct dwl_ext_workspace_group *group,
							 struct wl_resource *resource) {
	ext_workspace_group_handle_v1_send_capabilities(resource,
													group->capabilities);

	ext_group_output_send_initial_state(group, resource);
}

/* Manager itself */
static void manager_handle_commit(struct wl_client *client,
								  struct wl_resource *resource) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (!addon) {
		return;
	}

	struct dwl_ext_workspace *workspace;
	struct dwl_ext_workspace_group *group;
	struct dwl_transaction_op *trans_op, *trans_op_tmp;
	dwl_transaction_for_each_safe(trans_op, trans_op_tmp, addon->ctx) {
		switch (trans_op->change) {
		case WS_PENDING_WS_CREATE:
			group = trans_op->src;
			struct ws_create_workspace_event *ev = trans_op->data;
			wl_signal_emit_mutable(&group->events.create_workspace, ev->name);
			break;
		case WS_PENDING_WS_ACTIVATE:
			workspace = trans_op->src;
			wl_signal_emit_mutable(&workspace->events.activate, NULL);
			break;
		case WS_PENDING_WS_DEACTIVATE:
			workspace = trans_op->src;
			wl_signal_emit_mutable(&workspace->events.deactivate, NULL);
			break;
		case WS_PENDING_WS_REMOVE:
			workspace = trans_op->src;
			wl_signal_emit_mutable(&workspace->events.remove, NULL);
			break;
		case WS_PENDING_WS_ASSIGN:
			workspace = trans_op->src;
			wl_signal_emit_mutable(&workspace->events.assign, trans_op->data);
			break;
		default:
			wlr_log(WLR_ERROR, "Invalid transaction state: %u",
					trans_op->change);
		}

		dwl_transaction_op_destroy(trans_op);
	}
}

static void manager_handle_stop(struct wl_client *client,
								struct wl_resource *resource) {
	ext_workspace_manager_v1_send_finished(resource);
	wl_resource_destroy(resource);
}

static const struct ext_workspace_manager_v1_interface manager_impl = {
	.commit = manager_handle_commit,
	.stop = manager_handle_stop,
};

static void manager_instance_resource_destroy(struct wl_resource *resource) {
	struct dwl_wl_resource_addon *addon = wl_resource_get_user_data(resource);
	if (addon) {
		dwl_resource_addon_destroy(addon);
		wl_resource_set_user_data(resource, NULL);
	}
	wl_list_remove(wl_resource_get_link(resource));
}

static void manager_handle_bind(struct wl_client *client, void *data,
								uint32_t version, uint32_t id) {
	struct dwl_ext_workspace_manager *manager = data;
	struct wl_resource *manager_resource = wl_resource_create(
		client, &ext_workspace_manager_v1_interface, version, id);
	if (!manager_resource) {
		wl_client_post_no_memory(client);
		return;
	}

	struct dwl_wl_resource_addon *addon =
		dwl_resource_addon_create(/* session context*/ NULL);
	addon->data = manager;

	wl_resource_set_implementation(manager_resource, &manager_impl, addon,
								   manager_instance_resource_destroy);

	wl_list_insert(&manager->resources, wl_resource_get_link(manager_resource));

	struct dwl_ext_workspace *workspace;
	struct dwl_ext_workspace_group *group;
	wl_list_for_each(group, &manager->groups, link) {
		/* Create group resource */
		struct wl_resource *group_resource =
			group_resource_create(group, manager_resource, addon->ctx);
		ext_workspace_manager_v1_send_workspace_group(manager_resource,
													  group_resource);
		group_send_state(group, group_resource);
		wl_list_for_each(workspace, &manager->workspaces, link) {
			if (workspace->group != group) {
				continue;
			}
			/* Create workspace resource for the current group */
			struct wl_resource *workspace_resource = workspace_resource_create(
				workspace, manager_resource, addon->ctx);
			ext_workspace_manager_v1_send_workspace(manager_resource,
													workspace_resource);
			workspace_send_initial_state(workspace, workspace_resource);
			workspace_send_state(workspace, workspace_resource);
			ext_workspace_group_handle_v1_send_workspace_enter(
				group_resource, workspace_resource);
		}
	}

	/* Create workspace resource for workspaces not belonging to any group */
	wl_list_for_each(workspace, &manager->workspaces, link) {
		if (workspace->group) {
			continue;
		}
		struct wl_resource *workspace_resource =
			workspace_resource_create(workspace, manager_resource, addon->ctx);
		ext_workspace_manager_v1_send_workspace(manager_resource,
												workspace_resource);
		workspace_send_initial_state(workspace, workspace_resource);
		workspace_send_state(workspace, workspace_resource);
	}
	ext_workspace_manager_v1_send_done(manager_resource);
}

static void manager_handle_display_destroy(struct wl_listener *listener,
										   void *data) {
	struct dwl_ext_workspace_manager *manager =
		wl_container_of(listener, manager, on.display_destroy);

	struct dwl_ext_workspace_group *group, *tmp;
	wl_list_for_each_safe(group, tmp, &manager->groups, link) {
		dwl_ext_workspace_group_destroy(group);
	}

	struct dwl_ext_workspace *ws, *ws_tmp;
	wl_list_for_each_safe(ws, ws_tmp, &manager->workspaces, link) {
		dwl_ext_workspace_destroy(ws);
	}

	if (manager->idle_source) {
		wl_event_source_remove(manager->idle_source);
	}

	wl_list_remove(&manager->on.display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

/* Manager internal helpers */
static void manager_idle_send_done(void *data) {
	struct dwl_ext_workspace_manager *manager = data;

	struct dwl_ext_workspace *workspace;
	wl_list_for_each(workspace, &manager->workspaces, link) {
		if (workspace->state != workspace->state_pending) {
			workspace->state = workspace->state_pending;
			workspace_send_state(workspace, /*target*/ NULL);
		}
	}

	struct wl_resource *resource;
	wl_resource_for_each(resource, &manager->resources) {
		ext_workspace_manager_v1_send_done(resource);
	}
	manager->idle_source = NULL;
}

/* Internal API */
void ext_manager_schedule_done_event(
	struct dwl_ext_workspace_manager *manager) {
	if (manager->idle_source) {
		return;
	}
	if (!manager->event_loop) {
		return;
	}
	manager->idle_source = wl_event_loop_add_idle(
		manager->event_loop, manager_idle_send_done, manager);
}

static void send_group_workspace_event(struct dwl_ext_workspace_group *group,
									   struct dwl_ext_workspace *workspace,
									   void (*fn)(struct wl_resource *grp_res,
												  struct wl_resource *ws_res)) {
	struct dwl_wl_resource_addon *workspace_addon, *group_addon;
	struct wl_resource *workspace_resource, *group_resource;
	wl_resource_for_each(workspace_resource, &workspace->resources) {
		workspace_addon = wl_resource_get_user_data(workspace_resource);
		wl_resource_for_each(group_resource, &group->resources) {
			group_addon = wl_resource_get_user_data(group_resource);
			if (group_addon->ctx != workspace_addon->ctx) {
				continue;
			}
			fn(group_resource, workspace_resource);
			break;
		}
	}
}

/* Public API */
struct dwl_ext_workspace_manager *
dwl_ext_workspace_manager_create(struct wl_display *display, uint32_t caps,
								 uint32_t version) {
	assert(display);
	assert(version <= EXT_WORKSPACE_V1_VERSION);

	struct dwl_ext_workspace_manager *manager = znew(*manager);
	manager->global =
		wl_global_create(display, &ext_workspace_manager_v1_interface, version,
						 manager, manager_handle_bind);

	if (!manager->global) {
		free(manager);
		return NULL;
	}

	manager->caps = caps;
	manager->event_loop = wl_display_get_event_loop(display);

	manager->on.display_destroy.notify = manager_handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->on.display_destroy);

	wl_list_init(&manager->groups);
	wl_list_init(&manager->workspaces);
	wl_list_init(&manager->resources);
	return manager;
}

struct dwl_ext_workspace_group *
dwl_ext_workspace_group_create(struct dwl_ext_workspace_manager *manager) {
	assert(manager);

	struct dwl_ext_workspace_group *group = znew(*group);
	group->manager = manager;
	group->capabilities = manager->caps & WS_CAP_GRP_ALL;

	wl_list_init(&group->outputs);
	wl_list_init(&group->resources);
	wl_signal_init(&group->events.create_workspace);
	wl_signal_init(&group->events.destroy);

	wl_list_append(&manager->groups, &group->link);

	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp, &manager->resources) {
		struct dwl_wl_resource_addon *addon =
			wl_resource_get_user_data(resource);
		assert(addon && addon->ctx);
		struct wl_resource *group_resource =
			group_resource_create(group, resource, addon->ctx);
		ext_workspace_manager_v1_send_workspace_group(resource, group_resource);
		group_send_state(group, group_resource);
	}
	ext_manager_schedule_done_event(manager);

	return group;
}

void dwl_ext_workspace_group_destroy(struct dwl_ext_workspace_group *group) {
	assert(group);
	wl_signal_emit_mutable(&group->events.destroy, NULL);

	struct dwl_ext_workspace *workspace;
	wl_list_for_each(workspace, &group->manager->workspaces, link) {
		if (workspace->group == group) {
			send_group_workspace_event(
				group, workspace,
				ext_workspace_group_handle_v1_send_workspace_leave);
			workspace->group = NULL;
		}
	}

	struct wl_resource *resource, *res_tmp;
	wl_resource_for_each_safe(resource, res_tmp, &group->resources) {
		ext_workspace_group_handle_v1_send_removed(resource);
		struct dwl_wl_resource_addon *addon =
			wl_resource_get_user_data(resource);
		dwl_resource_addon_destroy(addon);
		wl_resource_set_user_data(resource, NULL);
		wl_list_remove(wl_resource_get_link(resource));
		wl_list_init(wl_resource_get_link(resource));
	}

	/* Cancel pending transaction ops involving this group */
	struct dwl_transaction_op *trans_op, *trans_op_tmp;
	wl_resource_for_each(resource, &group->manager->resources) {
		struct dwl_wl_resource_addon *addon =
			wl_resource_get_user_data(resource);
		dwl_transaction_for_each_safe(trans_op, trans_op_tmp, addon->ctx) {
			if (trans_op->src == group || trans_op->data == group) {
				dwl_transaction_op_destroy(trans_op);
			}
		}
	}

	ext_manager_schedule_done_event(group->manager);

	wl_list_remove(&group->link);
	free(group);
}

struct dwl_ext_workspace *
dwl_ext_workspace_create(struct dwl_ext_workspace_manager *manager,
						 const char *id) {
	assert(manager);

	struct dwl_ext_workspace *workspace = znew(*workspace);
	/*
	 * Ensures we are sending workspace->state_pending on the done event,
	 * regardless if the compositor has changed any state in between here
	 * and the scheduled done event or not.
	 *
	 * Without this we might have to send the state twice, first here and
	 * then again in the scheduled done event when there were any changes.
	 */
	workspace->state = WS_STATE_INVALID;
	workspace->capabilities = (manager->caps & WS_CAP_WS_ALL) >> 16;
	workspace->manager = manager;
	if (id) {
		workspace->id = xstrdup(id);
	}

	wl_list_init(&workspace->resources);
	wl_array_init(&workspace->coordinates);
	wl_signal_init(&workspace->events.activate);
	wl_signal_init(&workspace->events.deactivate);
	wl_signal_init(&workspace->events.remove);
	wl_signal_init(&workspace->events.assign);
	wl_signal_init(&workspace->events.destroy);

	wl_list_append(&manager->workspaces, &workspace->link);

	/* Notify clients */
	struct dwl_wl_resource_addon *manager_addon;
	struct wl_resource *manager_resource, *workspace_resource;
	wl_resource_for_each(manager_resource, &manager->resources) {
		manager_addon = wl_resource_get_user_data(manager_resource);
		workspace_resource = workspace_resource_create(
			workspace, manager_resource, manager_addon->ctx);
		ext_workspace_manager_v1_send_workspace(manager_resource,
												workspace_resource);
		workspace_send_initial_state(workspace, workspace_resource);
	}

	ext_manager_schedule_done_event(manager);

	return workspace;
}

void dwl_ext_workspace_assign_to_group(struct dwl_ext_workspace *workspace,
									   struct dwl_ext_workspace_group *group) {
	assert(workspace);

	if (workspace->group == group) {
		return;
	}

	if (workspace->group) {
		/* Send leave event for the old group */
		send_group_workspace_event(
			workspace->group, workspace,
			ext_workspace_group_handle_v1_send_workspace_leave);
		ext_manager_schedule_done_event(workspace->manager);
	}
	workspace->group = group;

	if (!group) {
		return;
	}

	/* Send enter event for the new group */
	send_group_workspace_event(
		group, workspace, ext_workspace_group_handle_v1_send_workspace_enter);
	ext_manager_schedule_done_event(workspace->manager);
}

void dwl_ext_workspace_set_name(struct dwl_ext_workspace *workspace,
								const char *name) {
	assert(workspace);
	assert(name);

	if (!workspace->name || strcmp(workspace->name, name)) {
		free(workspace->name);
		workspace->name = xstrdup(name);
		struct wl_resource *resource;
		wl_resource_for_each(resource, &workspace->resources) {
			ext_workspace_handle_v1_send_name(resource, workspace->name);
		}
	}
	ext_manager_schedule_done_event(workspace->manager);
}

void dwl_ext_workspace_set_active(struct dwl_ext_workspace *workspace,
								  bool enabled) {
	assert(workspace);
	workspace_set_state(workspace, EXT_WORKSPACE_HANDLE_V1_STATE_ACTIVE,
						enabled);
}

void dwl_ext_workspace_set_urgent(struct dwl_ext_workspace *workspace,
								  bool enabled) {
	assert(workspace);
	workspace_set_state(workspace, EXT_WORKSPACE_HANDLE_V1_STATE_URGENT,
						enabled);
}

void dwl_ext_workspace_set_hidden(struct dwl_ext_workspace *workspace,
								  bool enabled) {
	assert(workspace);
	workspace_set_state(workspace, EXT_WORKSPACE_HANDLE_V1_STATE_HIDDEN,
						enabled);
}

void dwl_ext_workspace_set_coordinates(struct dwl_ext_workspace *workspace,
									   struct wl_array *coordinates) {
	assert(workspace);
	assert(coordinates);

	wl_array_release(&workspace->coordinates);
	wl_array_init(&workspace->coordinates);
	wl_array_copy(&workspace->coordinates, coordinates);

	struct wl_resource *resource;
	wl_resource_for_each(resource, &workspace->resources) {
		ext_workspace_handle_v1_send_coordinates(resource,
												 &workspace->coordinates);
	}
	ext_manager_schedule_done_event(workspace->manager);
}

void dwl_ext_workspace_destroy(struct dwl_ext_workspace *workspace) {
	assert(workspace);

	wl_signal_emit_mutable(&workspace->events.destroy, NULL);

	if (workspace->group) {
		send_group_workspace_event(
			workspace->group, workspace,
			ext_workspace_group_handle_v1_send_workspace_leave);
	}

	struct wl_resource *ws_res, *ws_tmp;
	wl_resource_for_each_safe(ws_res, ws_tmp, &workspace->resources) {
		ext_workspace_handle_v1_send_removed(ws_res);
		struct dwl_wl_resource_addon *ws_addon =
			wl_resource_get_user_data(ws_res);
		dwl_resource_addon_destroy(ws_addon);
		wl_resource_set_user_data(ws_res, NULL);
		wl_list_remove(wl_resource_get_link(ws_res));
		wl_list_init(wl_resource_get_link(ws_res));
	}
	ext_manager_schedule_done_event(workspace->manager);

	/* Cancel pending transaction ops involving this workspace */
	struct wl_resource *resource;
	struct dwl_transaction_op *trans_op, *trans_op_tmp;
	wl_resource_for_each(resource, &workspace->manager->resources) {
		struct dwl_wl_resource_addon *addon =
			wl_resource_get_user_data(resource);
		dwl_transaction_for_each_safe(trans_op, trans_op_tmp, addon->ctx) {
			if (trans_op->src == workspace) {
				dwl_transaction_op_destroy(trans_op);
			}
		}
	}

	wl_list_remove(&workspace->link);
	wl_array_release(&workspace->coordinates);
	zfree(workspace->id);
	zfree(workspace->name);
	free(workspace);
}
