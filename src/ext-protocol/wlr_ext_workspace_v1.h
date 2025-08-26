// bash on: https://gitlab.freedesktop.org/tokyo4j/wlroots/-/tree/ext-workspace
// TODO: remove this file
// refer: https://gitlab.freedesktop.org/wlroots/wlroots/-/merge_requests/5115 

/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_EXT_WORKSPACE_V1_H
#define WLR_TYPES_WLR_EXT_WORKSPACE_V1_H

#include <wayland-server-core.h>

struct wlr_output;

enum wlr_ext_workspace_group_handle_v1_cap {
	WLR_EXT_WORKSPACE_GROUP_HANDLE_V1_CAP_CREATE_WORKSPACE = 1 << 0,
};

enum wlr_ext_workspace_handle_v1_cap {
	WLR_EXT_WORKSPACE_HANDLE_V1_CAP_ACTIVATE = 1 << 0,
	WLR_EXT_WORKSPACE_HANDLE_V1_CAP_DEACTIVATE = 1 << 1,
	WLR_EXT_WORKSPACE_HANDLE_V1_CAP_REMOVE = 1 << 2,
	WLR_EXT_WORKSPACE_HANDLE_V1_CAP_ASSIGN = 1 << 3,
};

struct wlr_ext_workspace_manager_v1 {
	struct wl_global *global;
	struct wl_list groups; // wlr_ext_workspace_group_handle_v1.link
	struct wl_list workspaces; // wlr_ext_workspace_handle_v1.link

	struct {
		struct wl_signal destroy;
	} events;

	struct wl_list clients; // wlr_ext_workspace_manager_client_v1.link
	struct wl_event_source *idle_source;
	struct wl_event_loop *event_loop;
	struct wl_listener display_destroy;
};

struct wlr_ext_workspace_group_handle_v1 {
	struct wlr_ext_workspace_manager_v1 *manager;
	uint32_t caps; // wlr_ext_workspace_group_handle_v1_cap
	struct {
		struct wl_signal create_workspace; // const char *
		struct wl_signal destroy;
	} events;

	struct wl_list link; // wlr_ext_workspace_manager_v1.groups

	struct wl_list outputs; // wlr_ext_workspace_v1_group_output.link
	struct wl_list clients; // wlr_ext_workspace_manager_client_v1.link
};

struct wlr_ext_workspace_handle_v1 {
	struct wlr_ext_workspace_manager_v1 *manager;
	struct wlr_ext_workspace_group_handle_v1 *group; // May be NULL
	char *id;
	char *name;
	struct wl_array coordinates;
	uint32_t caps; // wlr_ext_workspace_handle_v1_cap
	uint32_t state; // ext_workspace_handle_v1_state

	struct {
		struct wl_signal activate;
		struct wl_signal deactivate;
		struct wl_signal remove;
		struct wl_signal assign; // wlr_ext_workspace_group_handle_v1
		struct wl_signal destroy;
	} events;

	struct wl_list link; // wlr_ext_workspace_manager_v1.workspaces;

	struct wl_list clients;
};

struct wlr_ext_workspace_manager_v1 *wlr_ext_workspace_manager_v1_create(
	struct wl_display *display, uint32_t version);

struct wlr_ext_workspace_group_handle_v1 *wlr_ext_workspace_group_handle_v1_create(
	struct wlr_ext_workspace_manager_v1 *manager, uint32_t caps);
void wlr_ext_workspace_group_handle_v1_destroy(
	struct wlr_ext_workspace_group_handle_v1 *group);

void wlr_ext_workspace_group_handle_v1_output_enter(
	struct wlr_ext_workspace_group_handle_v1 *group, struct wlr_output *output);
void wlr_ext_workspace_group_handle_v1_output_leave(
	struct wlr_ext_workspace_group_handle_v1 *group, struct wlr_output *output);

struct wlr_ext_workspace_handle_v1 *wlr_ext_workspace_handle_v1_create(
	struct wlr_ext_workspace_manager_v1 *manager, const char *id, uint32_t caps);
void wlr_ext_workspace_handle_v1_destroy(struct wlr_ext_workspace_handle_v1 *workspace);

void wlr_ext_workspace_handle_v1_set_group(
	struct wlr_ext_workspace_handle_v1 *workspace,
	struct wlr_ext_workspace_group_handle_v1 *group);
void wlr_ext_workspace_handle_v1_set_name(
	struct wlr_ext_workspace_handle_v1 *workspace, const char *name);
void wlr_ext_workspace_handle_v1_set_coordinates(
	struct wlr_ext_workspace_handle_v1 *workspace, struct wl_array *coordinates);
void wlr_ext_workspace_handle_v1_set_active(
	struct wlr_ext_workspace_handle_v1 *workspace, bool enabled);
void wlr_ext_workspace_handle_v1_set_urgent(
	struct wlr_ext_workspace_handle_v1 *workspace, bool enabled);
void wlr_ext_workspace_handle_v1_set_hidden(
	struct wlr_ext_workspace_handle_v1 *workspace, bool enabled);

#endif
