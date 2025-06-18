/* SPDX-License-Identifier: GPL-2.0-only */
/* Based on labwc (https://github.com/labwc/labwc) */

#include <stdbool.h>
#include <wayland-server-core.h>

struct wlr_output;

struct dwl_ext_workspace_manager {
	struct wl_global *global;
	struct wl_list groups;
	struct wl_list workspaces;
	uint32_t caps;
	struct wl_event_source *idle_source;
	struct wl_event_loop *event_loop;

	struct {
		struct wl_listener display_destroy;
	} on;

	struct wl_list resources;
};

struct dwl_ext_workspace_group {
	struct dwl_ext_workspace_manager *manager;
	uint32_t capabilities;
	struct {
		struct wl_signal create_workspace;
		struct wl_signal destroy;
	} events;

	struct wl_list link;
	struct wl_list outputs;
	struct wl_list resources;
};

struct dwl_ext_workspace {
	struct dwl_ext_workspace_manager *manager;
	struct dwl_ext_workspace_group *group;
	char *id;
	char *name;
	struct wl_array coordinates;
	uint32_t capabilities;
	uint32_t state;			/* enum dwl_ext_workspace_state */
	uint32_t state_pending; /* enum dwl_ext_workspace_state */

	struct {
		struct wl_signal activate;
		struct wl_signal deactivate;
		struct wl_signal remove;
		struct wl_signal assign;
		struct wl_signal destroy;
	} events;

	struct wl_list link;
	struct wl_list resources;
};

enum dwl_ext_workspace_caps {
	WS_CAP_NONE = 0,
	WS_CAP_GRP_ALL = 0x0000ffff,
	WS_CAP_WS_ALL = 0xffff0000,

	/* group caps */
	WS_CAP_GRP_WS_CREATE = 1 << 0,

	/* workspace caps */
	WS_CAP_WS_ACTIVATE = 1 << 16,
	WS_CAP_WS_DEACTIVATE = 1 << 17,
	WS_CAP_WS_REMOVE = 1 << 18,
	WS_CAP_WS_ASSIGN = 1 << 19,
};

struct dwl_ext_workspace_manager *
dwl_ext_workspace_manager_create(struct wl_display *display, uint32_t caps,
								 uint32_t version);

struct dwl_ext_workspace_group *
dwl_ext_workspace_group_create(struct dwl_ext_workspace_manager *manager);

void dwl_ext_workspace_group_output_enter(struct dwl_ext_workspace_group *group,
										  struct wlr_output *output);

void dwl_ext_workspace_group_output_leave(struct dwl_ext_workspace_group *group,
										  struct wlr_output *output);

void dwl_ext_workspace_group_destroy(struct dwl_ext_workspace_group *group);

/* Create a new workspace, id may be NULL */
struct dwl_ext_workspace *
dwl_ext_workspace_create(struct dwl_ext_workspace_manager *manager,
						 const char *id);

void dwl_ext_workspace_assign_to_group(struct dwl_ext_workspace *workspace,
									   struct dwl_ext_workspace_group *group);

void dwl_ext_workspace_set_name(struct dwl_ext_workspace *workspace,
								const char *name);

void dwl_ext_workspace_set_active(struct dwl_ext_workspace *workspace,
								  bool enabled);

void dwl_ext_workspace_set_urgent(struct dwl_ext_workspace *workspace,
								  bool enabled);

void dwl_ext_workspace_set_hidden(struct dwl_ext_workspace *workspace,
								  bool enabled);

void dwl_ext_workspace_set_coordinates(struct dwl_ext_workspace *workspace,
									   struct wl_array *coordinates);

void dwl_ext_workspace_destroy(struct dwl_ext_workspace *workspace);