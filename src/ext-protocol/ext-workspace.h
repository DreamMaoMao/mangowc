#include "wlr_ext_workspace_v1.h"

typedef struct Monitor Monitor;

struct workspace {
	struct wl_list link; // Link in global workspaces list
	unsigned int tag;	 // Numeric identifier (1-9, 0=overview)
	Monitor *m;			 // Associated monitor
	struct wlr_ext_workspace_handle_v1 *ext_workspace; // Protocol object
	/* Event listeners */
	struct wl_listener activate;
	struct wl_listener deactivate;
	struct wl_listener assign;
	struct wl_listener remove;
};

struct wlr_ext_workspace_manager_v1 *ext_manager;
struct wl_list workspaces;

void goto_workspace(struct workspace *target) {
	unsigned int tag;
	tag = 1 << (target->tag - 1);
	if (target->tag == 0) {
		toggleoverview(&(Arg){.i = -1});
		return;
	} else {
		view(&(Arg){.ui = tag}, true);
	}
}

static void handle_ext_workspace_activate(struct wl_listener *listener,
										  void *data) {
	struct workspace *workspace =
		wl_container_of(listener, workspace, activate);
	goto_workspace(workspace);
	wlr_log(WLR_INFO, "ext activating workspace %d", workspace->tag);
}

static const char *get_name_from_tag(unsigned int tag) {
	static const char *names[] = {"overview", "1", "2", "3", "4",
								  "5",		  "6", "7", "8", "9"};
	return (tag < sizeof(names) / sizeof(names[0])) ? names[tag] : NULL;
}

void destroy_workspace(struct workspace *workspace) {
	wl_list_remove(&workspace->activate.link);
	wlr_ext_workspace_handle_v1_destroy(workspace->ext_workspace);
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

	struct workspace *workspace = ecalloc(1, sizeof(*workspace));
	wl_list_append(&workspaces, &workspace->link);

	workspace->tag = tag;
	workspace->m = m;
	workspace->ext_workspace = wlr_ext_workspace_handle_v1_create(
		ext_manager, name, WLR_EXT_WORKSPACE_HANDLE_V1_CAP_ACTIVATE);
	wlr_ext_workspace_handle_v1_set_group(workspace->ext_workspace,
										  m->ext_group);
	wlr_ext_workspace_handle_v1_set_name(workspace->ext_workspace, name);
	workspace->activate.notify = handle_ext_workspace_activate;
	wl_signal_add(&workspace->ext_workspace->events.activate,
				  &workspace->activate);
}

unsigned int get_tag_status(unsigned int tag, Monitor *m) {
	Client *c;
	unsigned int status = 0;
	wl_list_for_each(c, &clients, link) {
		if (c->mon == m && c->tags & 1 << (tag - 1) & TAGMASK) {
			if (c->isurgent) {
				status = 2;
				break;
			}
			status = 1;
		}
	}
	return status;
}

unsigned int get_tags_first_tag_num(unsigned int source_tags) {
	unsigned int i, tag;
	tag = 0;

	if (!source_tags) {
		return selmon->pertag->curtag;
	}

	for (i = 0; !(tag & 1) && source_tags != 0 && i < LENGTH(tags); i++) {
		tag = source_tags >> i;
	}

	if (i == 1) {
		return 1;
	} else if (i > 9) {
		return 9;
	} else {
		return i;
	}
}

void dwl_ext_workspace_printstatus(Monitor *m) {
	struct workspace *w;
	unsigned int tag_status = 0;

	wl_list_for_each(w, &workspaces, link) {
		if (w && w->m == m) {

			tag_status = get_tag_status(w->tag, m);
			if (tag_status == 2) {
				wlr_ext_workspace_handle_v1_set_hidden(w->ext_workspace, false);
				wlr_ext_workspace_handle_v1_set_urgent(w->ext_workspace, true);
			} else if (tag_status == 1) {
				wlr_ext_workspace_handle_v1_set_urgent(w->ext_workspace, false);
				wlr_ext_workspace_handle_v1_set_hidden(w->ext_workspace, false);
			} else {
				wlr_ext_workspace_handle_v1_set_urgent(w->ext_workspace, false);
				wlr_ext_workspace_handle_v1_set_hidden(w->ext_workspace, true);
			}

			if ((m->tagset[m->seltags] & (1 << (w->tag - 1)) & TAGMASK) ||
				m->isoverview) {
				wlr_ext_workspace_handle_v1_set_hidden(w->ext_workspace, false);
				wlr_ext_workspace_handle_v1_set_active(w->ext_workspace, true);
			} else {
				wlr_ext_workspace_handle_v1_set_active(w->ext_workspace, false);
			}
		}
	}
}

void refresh_monitors_workspaces_status(Monitor *m) {
	int i;

	if (m->isoverview) {
		for (i = 1; i <= LENGTH(tags); i++) {
			remove_workspace_by_tag(i, m);
		}
		add_workspace_by_tag(0, m);
	} else {
		remove_workspace_by_tag(0, m);
		for (i = 1; i <= LENGTH(tags); i++) {
			add_workspace_by_tag(i, m);
		}
	}

	dwl_ext_workspace_printstatus(m);
}

void workspaces_init() {
	/* Create the global workspace manager with activation capability */
	ext_manager = wlr_ext_workspace_manager_v1_create(dpy, 1);
	/* Initialize the global workspaces list */
	wl_list_init(&workspaces);
}