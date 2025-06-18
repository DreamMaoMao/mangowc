// SPDX-License-Identifier: GPL-2.0-only
/* Based on labwc (https://github.com/labwc/labwc) */

#include "../common/mem.h"
#include "ext-workspace-internal.h"
#include "ext-workspace-v1-protocol.h"
#include "ext-workspace.h"
#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>

struct group_output {
	struct wlr_output *wlr_output;
	struct dwl_ext_workspace_group *group;
	struct {
		struct wl_listener group_destroy;
		struct wl_listener output_bind;
		struct wl_listener output_destroy;
	} on;

	struct wl_list link;
};

/* Internal helpers */
static void group_output_send_event(
	struct wl_list *group_resources, struct wl_list *output_resources,
	void (*notifier)(struct wl_resource *group, struct wl_resource *output)) {
	struct wl_client *client;
	struct wl_resource *group_resource, *output_resource;
	wl_resource_for_each(group_resource, group_resources) {
		client = wl_resource_get_client(group_resource);
		wl_resource_for_each(output_resource, output_resources) {
			if (wl_resource_get_client(output_resource) == client) {
				notifier(group_resource, output_resource);
			}
		}
	}
}

static void group_output_destroy(struct group_output *group_output) {
	group_output_send_event(&group_output->group->resources,
							&group_output->wlr_output->resources,
							ext_workspace_group_handle_v1_send_output_leave);

	ext_manager_schedule_done_event(group_output->group->manager);

	wl_list_remove(&group_output->link);
	wl_list_remove(&group_output->on.group_destroy.link);
	wl_list_remove(&group_output->on.output_bind.link);
	wl_list_remove(&group_output->on.output_destroy.link);
	free(group_output);
}

/* Event handlers */
static void handle_output_bind(struct wl_listener *listener, void *data) {
	struct group_output *group_output =
		wl_container_of(listener, group_output, on.output_bind);

	struct wlr_output_event_bind *event = data;
	struct wl_client *client = wl_resource_get_client(event->resource);

	bool sent = false;
	struct wl_resource *group_resource;
	wl_resource_for_each(group_resource, &group_output->group->resources) {
		if (wl_resource_get_client(group_resource) == client) {
			ext_workspace_group_handle_v1_send_output_enter(group_resource,
															event->resource);
			sent = true;
		}
	}
	if (!sent) {
		return;
	}

	struct wl_resource *manager_resource;
	struct wl_list *manager_resources =
		&group_output->group->manager->resources;
	wl_resource_for_each(manager_resource, manager_resources) {
		if (wl_resource_get_client(manager_resource) == client) {
			ext_workspace_manager_v1_send_done(manager_resource);
		}
	}
}

static void handle_output_destroy(struct wl_listener *listener, void *data) {
	struct group_output *group_output =
		wl_container_of(listener, group_output, on.output_destroy);
	group_output_destroy(group_output);
}

static void handle_group_destroy(struct wl_listener *listener, void *data) {
	struct group_output *group_output =
		wl_container_of(listener, group_output, on.group_destroy);
	group_output_destroy(group_output);
}

/* Internal API*/
void ext_group_output_send_initial_state(struct dwl_ext_workspace_group *group,
										 struct wl_resource *group_resource) {
	struct group_output *group_output;
	struct wl_resource *output_resource;
	struct wl_client *client = wl_resource_get_client(group_resource);
	wl_list_for_each(group_output, &group->outputs, link) {
		wl_resource_for_each(output_resource,
							 &group_output->wlr_output->resources) {
			if (wl_resource_get_client(output_resource) == client) {
				ext_workspace_group_handle_v1_send_output_enter(
					group_resource, output_resource);
			}
		}
	}
}

/* Public API */
void dwl_ext_workspace_group_output_enter(struct dwl_ext_workspace_group *group,
										  struct wlr_output *wlr_output) {
	struct group_output *group_output;
	wl_list_for_each(group_output, &group->outputs, link) {
		if (group_output->wlr_output == wlr_output) {
			return;
		}
	}
	group_output = znew(*group_output);
	group_output->wlr_output = wlr_output;
	group_output->group = group;

	group_output->on.group_destroy.notify = handle_group_destroy;
	wl_signal_add(&group->events.destroy, &group_output->on.group_destroy);

	group_output->on.output_bind.notify = handle_output_bind;
	wl_signal_add(&wlr_output->events.bind, &group_output->on.output_bind);

	group_output->on.output_destroy.notify = handle_output_destroy;
	wl_signal_add(&wlr_output->events.destroy,
				  &group_output->on.output_destroy);

	wl_list_insert(&group->outputs, &group_output->link);

	group_output_send_event(&group_output->group->resources,
							&group_output->wlr_output->resources,
							ext_workspace_group_handle_v1_send_output_enter);

	ext_manager_schedule_done_event(group->manager);
}

void dwl_ext_workspace_group_output_leave(struct dwl_ext_workspace_group *group,
										  struct wlr_output *wlr_output) {
	struct group_output *tmp;
	struct group_output *group_output = NULL;
	wl_list_for_each(tmp, &group->outputs, link) {
		if (tmp->wlr_output == wlr_output) {
			group_output = tmp;
			break;
		}
	}
	if (!group_output) {
		wlr_log(WLR_ERROR, "output %s was never entered", wlr_output->name);
		return;
	}

	group_output_destroy(group_output);
}
