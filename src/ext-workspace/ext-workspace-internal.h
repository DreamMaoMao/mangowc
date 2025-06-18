/* SPDX-License-Identifier: GPL-2.0-only */
/* Based on labwc (https://github.com/labwc/labwc) */

struct wl_resource;
struct dwl_ext_workspace_group;
struct dwl_ext_workspace_manager;

enum pending_ext_workspaces_change {
	/* group events */
	WS_PENDING_WS_CREATE = 1 << 0,

	/* ws events*/
	WS_PENDING_WS_ACTIVATE = 1 << 1,
	WS_PENDING_WS_DEACTIVATE = 1 << 2,
	WS_PENDING_WS_REMOVE = 1 << 3,
	WS_PENDING_WS_ASSIGN = 1 << 4,
};

void ext_group_output_send_initial_state(struct dwl_ext_workspace_group *group,
										 struct wl_resource *group_resource);

void ext_manager_schedule_done_event(struct dwl_ext_workspace_manager *manager);
