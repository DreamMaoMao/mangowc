#include "../common/list.h"
#include "../common/mem.h"
#include "ext-workspace.h"

typedef struct Monitor Monitor;

struct workspace {
	struct wl_list link;
	unsigned int tag;
	Monitor *m;
	struct dwl_ext_workspace *ext_workspace;
	struct wl_listener activate;
	struct wl_listener deactivate;
	struct wl_listener assign;
	struct wl_listener remove;
};

struct dwl_ext_workspace_manager *ext_manager;
struct wl_list workspaces;

void workspaces_switch_to(struct workspace *target) {
	unsigned int tag;
	tag = 1 << (target->tag - 1);
	if (target->tag == 0) {
		toggleoverview(&(Arg){.i = -1});
		return;
	} else {
		view(&(Arg){.ui = tag}, true);
	}
}

/* ext workspace handlers */
static void handle_ext_workspace_activate(struct wl_listener *listener,
										  void *data) {
	struct workspace *workspace =
		wl_container_of(listener, workspace, activate);
	workspaces_switch_to(workspace);
	wlr_log(WLR_INFO, "ext activating workspace %d", workspace->tag);
}

static const char *get_name_from_tag(unsigned int tag) {
	static const char *names[] = {"overview", "1", "2", "3", "4",
								  "5",		  "6", "7", "8", "9"};
	return (tag < sizeof(names) / sizeof(names[0])) ? names[tag] : NULL;
}

void destroy_workspace(struct workspace *workspace) {
	wl_list_remove(&workspace->activate.link);
	dwl_ext_workspace_destroy(workspace->ext_workspace);
	wl_list_remove(&workspace->link);
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
	struct workspace *workspace, *tmp;
	wl_list_for_each_safe(workspace, tmp, &workspaces, link) {
		if (workspace->tag == tag && workspace->m == m) {
			destroy_workspace(workspace);
			return;
		}
	}
}

static void add_workspace_by_tag(int tag, Monitor *m) {
	const char *name = get_name_from_tag(tag);

	struct workspace *workspace = znew(*workspace);
	wl_list_append(&workspaces, &workspace->link);

	workspace->tag = tag;
	workspace->m = m;
	workspace->ext_workspace =
		dwl_ext_workspace_create(ext_manager, /*id*/ NULL);
	dwl_ext_workspace_assign_to_group(workspace->ext_workspace, m->ext_group);
	dwl_ext_workspace_set_name(workspace->ext_workspace, name);
	workspace->activate.notify = handle_ext_workspace_activate;
	wl_signal_add(&workspace->ext_workspace->events.activate,
				  &workspace->activate);
}

void workspaces_init() {

	ext_manager = dwl_ext_workspace_manager_create(dpy, WS_CAP_WS_ACTIVATE, 1);
	wl_list_init(&workspaces);
}
