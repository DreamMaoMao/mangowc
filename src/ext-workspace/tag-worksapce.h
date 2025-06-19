#include "../common/list.h"
#include "../common/mem.h"
#include "ext-workspace.h"

typedef struct Monitor Monitor;

struct workspace {
	struct wl_list link;

	char *name;
	unsigned int tag;
	struct wlr_scene_tree *tree;
	Monitor *m;

	struct dwl_ext_workspace *ext_workspace;
	struct {
		struct wl_listener activate;
		struct wl_listener deactivate;
		struct wl_listener assign;
		struct wl_listener remove;
	} on_ext;
};

struct dwl_ext_workspace_manager *ext_manager;
struct wl_list workspaces; /* struct workspace.link */

void workspaces_switch_to(struct workspace *target) {
	if (target == target->m->workspace_current) {
		return;
	}

	wlr_scene_node_set_enabled(&target->m->workspace_current->tree->node,
							   false);
	dwl_ext_workspace_set_active(target->m->workspace_current->ext_workspace,
								 false);

	unsigned int tag;
	tag = 1 << (target->tag - 1);
	if (target->tag == 0) {
		toggleoverview(&(Arg){.i = -1});
		return;
	} else {
		view(&(Arg){.ui = tag}, true);
	}

	wlr_scene_node_set_enabled(&target->tree->node, true);
	target->m->workspace_last = target->m->workspace_current;
	target->m->workspace_current = target;
	dwl_ext_workspace_set_active(target->ext_workspace, true);
}

/* ext workspace handlers */
static void handle_ext_workspace_activate(struct wl_listener *listener,
										  void *data) {
	struct workspace *workspace =
		wl_container_of(listener, workspace, on_ext.activate);
	workspaces_switch_to(workspace);
	wlr_log(WLR_INFO, "ext activating workspace %s", workspace->name);
}

static char *get_name_from_tag(unsigned int tag) {
	char *name = NULL;
	switch (tag) {
	case 0:
		name = "overview";
		break;
	case 1:
		name = "1";
		break;
	case 2:
		name = "2";
		break;
	case 3:
		name = "3";
		break;
	case 4:
		name = "4";
		break;
	case 5:
		name = "5";
		break;
	case 6:
		name = "6";
		break;
	case 7:
		name = "7";
		break;
	case 8:
		name = "8";
		break;
	case 9:
		name = "9";
		break;
	}
	return name;
}

void destroy_workspace(struct workspace *workspace) {
	// Clean up external workspace
	wl_list_remove(&workspace->on_ext.activate.link);
	dwl_ext_workspace_destroy(workspace->ext_workspace);

	// Remove from the list and free resources
	wl_list_remove(&workspace->link);
	wlr_scene_node_destroy(&workspace->tree->node);
	free(workspace->name);
	free(workspace);
}

void cleanup_workspaces_by_monitor(Monitor *m) {
	struct workspace *workspace, *tmp;
	wl_list_for_each_safe(workspace, tmp, &workspaces, link) {
		if (workspace->m == m) {
			destroy_workspace(workspace);
		}
	}
}

static void remove_workspace_by_tag(unsigned int tag, Monitor *m) {
	char *name = get_name_from_tag(tag);
	struct workspace *workspace, *tmp;
	wl_list_for_each_safe(workspace, tmp, &workspaces, link) {
		if (strcmp(workspace->name, name) == 0 && workspace->m == m) {
			// If this is the current workspace, we need to handle that
			if (m->workspace_current == workspace) {
				// Find another workspace to make current (maybe the overview?)
				struct workspace *new_current = NULL;
				if (!wl_list_empty(&workspaces)) {
					struct workspace *first =
						wl_container_of(workspaces.next, first, link);
					if (first != workspace) {
						new_current = first;
					} else if (workspaces.next->next != &workspaces) {
						new_current = wl_container_of(workspaces.next->next,
													  new_current, link);
					}
				}
				m->workspace_current = new_current;
				if (new_current) {
					wlr_scene_node_set_enabled(&new_current->tree->node, true);
					dwl_ext_workspace_set_active(new_current->ext_workspace,
												 true);
				}
			}

			destroy_workspace(workspace);
			return;
		}
	}
}

/* Internal API */
static void add_workspace_by_tag(int tag, Monitor *m) {
	char *name = get_name_from_tag(tag);
	struct workspace *workspace = znew(*workspace);
	workspace->name = xstrdup(name);
	workspace->tag = tag;
	workspace->m = m;
	workspace->tree = wlr_scene_tree_create(ws_tree);
	wl_list_append(&workspaces, &workspace->link);
	if (!m->workspace_current) {
		m->workspace_current = workspace;
	} else {
		wlr_scene_node_set_enabled(&workspace->tree->node, false);
	}

	bool active = m->workspace_current == workspace;

	workspace->ext_workspace =
		dwl_ext_workspace_create(ext_manager, /*id*/ NULL);
	dwl_ext_workspace_assign_to_group(workspace->ext_workspace, m->ext_group);
	dwl_ext_workspace_set_name(workspace->ext_workspace, name);
	dwl_ext_workspace_set_active(workspace->ext_workspace, active);

	workspace->on_ext.activate.notify = handle_ext_workspace_activate;
	wl_signal_add(&workspace->ext_workspace->events.activate,
				  &workspace->on_ext.activate);
}

void workspaces_init() {

	ext_manager = dwl_ext_workspace_manager_create(dpy, WS_CAP_WS_ACTIVATE, 1);
	wl_list_init(&workspaces);
}
