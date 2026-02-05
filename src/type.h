/*
 * See LICENSE file for copyright and license details.
 */
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "wlr/util/box.h"
#include <getopt.h>
#include <libinput.h>
#include <limits.h>
#include <linux/input-event-codes.h>
#include <scenefx/render/fx_renderer/fx_renderer.h>
#include <scenefx/types/fx/blur_data.h>
#include <scenefx/types/fx/clipped_region.h>
#include <scenefx/types/fx/corner_location.h>
#include <scenefx/types/wlr_scene.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/backend/headless.h>
#include <wlr/backend/libinput.h>
#include <wlr/backend/multi.h>
#include <wlr/backend/wayland.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_alpha_modifier_v1.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_cursor_shape_v1.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_drm.h>
#include <wlr/types/wlr_drm_lease_v1.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_ext_data_control_v1.h>
#include <wlr/types/wlr_ext_image_capture_source_v1.h>
#include <wlr/types/wlr_ext_image_copy_capture_v1.h>
#include <wlr/types/wlr_fractional_scale_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_idle_inhibit_v1.h>
#include <wlr/types/wlr_idle_notify_v1.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_keyboard_group.h>
#include <wlr/types/wlr_keyboard_shortcuts_inhibit_v1.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/types/wlr_linux_drm_syncobj_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <wlr/types/wlr_output_power_management_v1.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_relative_pointer_v1.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_session_lock_v1.h>
#include <wlr/types/wlr_single_pixel_buffer_v1.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_switch.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/types/wlr_virtual_pointer_v1.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_activation_v1.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_foreign_registry.h>
#include <wlr/types/wlr_xdg_foreign_v1.h>
#include <wlr/types/wlr_xdg_foreign_v2.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include <wlr/util/region.h>
#include <wordexp.h>
#include <xkbcommon/xkbcommon.h>
#ifdef XWAYLAND
#include <X11/Xlib.h>
#include <wlr/xwayland.h>
#include <xcb/xcb_icccm.h>
#endif
#include "common/util.h"

/* macros */
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define GEZERO(A) ((A) >= 0 ? (A) : 0)
#define CLEANMASK(mask) (mask & ~WLR_MODIFIER_CAPS)
#define INSIDEMON(A)                                                           \
	(A->geom.x >= A->mon->m.x && A->geom.y >= A->mon->m.y &&                   \
	 A->geom.x + A->geom.width <= A->mon->m.x + A->mon->m.width &&             \
	 A->geom.y + A->geom.height <= A->mon->m.y + A->mon->m.height)
#define GEOMINSIDEMON(A, M)                                                    \
	(A->x >= M->m.x && A->y >= M->m.y &&                                       \
	 A->x + A->width <= M->m.x + M->m.width &&                                 \
	 A->y + A->height <= M->m.y + M->m.height)
#define ISTILED(A)                                                             \
	(A && !(A)->isfloating && !(A)->isminimized && !(A)->iskilling &&          \
	 !(A)->ismaximizescreen && !(A)->isfullscreen && !(A)->isunglobal)
#define ISSCROLLTILED(A)                                                       \
	(A && !(A)->isfloating && !(A)->isminimized && !(A)->iskilling &&          \
	 !(A)->isunglobal)
#define VISIBLEON(C, M)                                                        \
	((C) && (M) && (C)->mon == (M) && ((C)->tags & (M)->tagset[(M)->seltags]))
#define LENGTH(X) (sizeof X / sizeof X[0])
#define END(A) ((A) + LENGTH(A))
#define TAGMASK ((1 << LENGTH(tags)) - 1)
#define LISTEN(E, L, H) wl_signal_add((E), ((L)->notify = (H), (L)))
#define ISFULLSCREEN(A)                                                        \
	((A)->isfullscreen || (A)->ismaximizescreen ||                             \
	 (A)->overview_ismaximizescreenbak || (A)->overview_isfullscreenbak)
#define LISTEN_STATIC(E, H)                                                    \
	do {                                                                       \
		struct wl_listener *_l = ecalloc(1, sizeof(*_l));                      \
		_l->notify = (H);                                                      \
		wl_signal_add((E), _l);                                                \
	} while (0)

#define APPLY_INT_PROP(obj, rule, prop)                                        \
	if (rule->prop >= 0)                                                       \
	obj->prop = rule->prop

#define APPLY_FLOAT_PROP(obj, rule, prop)                                      \
	if (rule->prop > 0.0f)                                                     \
	obj->prop = rule->prop

#define APPLY_STRING_PROP(obj, rule, prop)                                     \
	if (rule->prop != NULL)                                                    \
	obj->prop = rule->prop

#define BAKED_POINTS_COUNT 256

/* enums */
enum { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT };

enum { VERTICAL, HORIZONTAL };
enum { SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT };
enum { CurNormal, CurPressed, CurMove, CurResize }; /* server.cursor */
enum { XDGShell, LayerShell, X11 };					/* client types */
enum { AxisUp, AxisDown, AxisLeft, AxisRight };		// 滚轮滚动的方向
enum {
	LyrBg,
	LyrBlur,
	LyrBottom,
	LyrTile,
	LyrTop,
	LyrFadeOut,
	LyrOverlay,
	LyrIMPopup, // text-input layer
	LyrBlock,
	NUM_LAYERS
}; /* server.scene  server.layers */
#ifdef XWAYLAND
enum {
	NetWMWindowTypeDialog,
	NetWMWindowTypeSplash,
	NetWMWindowTypeToolbar,
	NetWMWindowTypeUtility,
	NetLast
}; /* EWMH atoms */
#endif
enum { UP, DOWN, LEFT, RIGHT, UNDIR }; /* smartmovewin */
enum { NONE, OPEN, MOVE, CLOSE, TAG, FOCUS, OPAFADEIN, OPAFADEOUT };
enum { UNFOLD, FOLD, INVALIDFOLD };
enum { PREV, NEXT };
enum { STATE_UNSPECIFIED = 0, STATE_ENABLED, STATE_DISABLED };

enum tearing_mode {
	TEARING_DISABLED = 0,
	TEARING_ENABLED,
	TEARING_FULLSCREEN_ONLY,
};

enum seat_config_shortcuts_inhibit {
	SHORTCUTS_INHIBIT_DISABLE,
	SHORTCUTS_INHIBIT_ENABLE,
};

// 事件掩码枚举
enum print_event_type {
	PRINT_ACTIVE = 1 << 0,
	PRINT_TAG = 1 << 1,
	PRINT_LAYOUT = 1 << 2,
	PRINT_TITLE = 1 << 3,
	PRINT_APPID = 1 << 4,
	PRINT_LAYOUT_SYMBOL = 1 << 5,
	PRINT_FULLSCREEN = 1 << 6,
	PRINT_FLOATING = 1 << 7,
	PRINT_X = 1 << 8,
	PRINT_Y = 1 << 9,
	PRINT_WIDTH = 1 << 10,
	PRINT_HEIGHT = 1 << 11,
	PRINT_LAST_LAYER = 1 << 12,
	PRINT_KB_LAYOUT = 1 << 13,
	PRINT_KEYMODE = 1 << 14,
	PRINT_SCALEFACTOR = 1 << 15,
	PRINT_FRAME = 1 << 16,
	PRINT_ALL = (1 << 17) - 1 // 所有位都设为1
};

typedef struct Pertag Pertag;
typedef struct Monitor Monitor;
typedef struct Client Client;

struct dvec2 {
	double x, y;
};

struct ivec2 {
	int32_t x, y, width, height;
};

typedef struct {
	int32_t i;
	int32_t i2;
	float f;
	float f2;
	char *v;
	char *v2;
	char *v3;
	uint32_t ui;
	uint32_t ui2;
} Arg;

typedef struct {
	char *name;		 // keymode名称
	bool is_default; // 是否为默认模式
	bool is_common;	 // 是否为公共模式
} KeyMode;

typedef struct {
	uint32_t mod;
	uint32_t button;
	int32_t (*func)(const Arg *);
	const Arg arg;
} Button; // 鼠标按键

typedef struct {
	uint32_t mod;
	uint32_t dir;
	int32_t (*func)(const Arg *);
	const Arg arg;
} Axis;

typedef struct {
	struct wl_list link;
	struct wlr_input_device *wlr_device;
	struct libinput_device *libinput_device;
	struct wl_listener destroy_listener; // 用于监听设备销毁事件
	void *device_data;					 // 新增：指向设备特定数据（如 Switch）
} InputDevice;

typedef struct {
	struct wl_list link;
	struct wlr_switch *wlr_switch;
	struct wl_listener toggle;
	InputDevice *input_dev;
} Switch;

struct dwl_animation {
	bool should_animate;
	bool running;
	bool tagining;
	bool tagouted;
	bool tagouting;
	bool begin_fade_in;
	bool tag_from_rule;
	uint32_t time_started;
	uint32_t duration;
	struct wlr_box initial;
	struct wlr_box current;
	int32_t action;
};

struct dwl_opacity_animation {
	bool running;
	float current_opacity;
	float target_opacity;
	float initial_opacity;
	uint32_t time_started;
	uint32_t duration;
	float current_border_color[4];
	float target_border_color[4];
	float initial_border_color[4];
};

typedef struct {
	float width_scale;
	float height_scale;
	int32_t width;
	int32_t height;
	enum corner_location corner_location;
	bool should_scale;
} BufferData;

struct Client {
	/* Must keep these three elements in this order */
	uint32_t type; /* XDGShell or X11* */
	struct wlr_box geom, pending, float_geom, animainit_geom,
		overview_backup_geom, current,
		drag_begin_geom; /* layout-relative, includes border */
	Monitor *mon;
	struct wlr_scene_tree *scene;
	struct wlr_scene_rect *border; /* top, bottom, left, right */
	struct wlr_scene_shadow *shadow;
	struct wlr_scene_tree *scene_surface;
	struct wl_list link;
	struct wl_list flink;
	struct wl_list fadeout_link;
	union {
		struct wlr_xdg_surface *xdg;
		struct wlr_xwayland_surface *xwayland;
	} surface;
	struct wl_listener commit;
	struct wl_listener map;
	struct wl_listener maximize;
	struct wl_listener minimize;
	struct wl_listener unmap;
	struct wl_listener destroy;
	struct wl_listener set_title;
	struct wl_listener fullscreen;
#ifdef XWAYLAND
	struct wl_listener activate;
	struct wl_listener associate;
	struct wl_listener dissociate;
	struct wl_listener configure;
	struct wl_listener set_hints;
	struct wl_listener set_geometry;
#endif
	uint32_t bw;
	uint32_t tags, oldtags, mini_restore_tag;
	bool dirty;
	uint32_t configure_serial;
	struct wlr_foreign_toplevel_handle_v1 *foreign_toplevel;
	int32_t isfloating, isurgent, isfullscreen, isfakefullscreen,
		need_float_size_reduce, isminimized, isoverlay, isnosizehint,
		ignore_maximize, ignore_minimize;
	int32_t ismaximizescreen;
	int32_t overview_backup_bw;
	int32_t fullscreen_backup_x, fullscreen_backup_y, fullscreen_backup_w,
		fullscreen_backup_h;
	int32_t overview_isfullscreenbak, overview_ismaximizescreenbak,
		overview_isfloatingbak;

	struct wlr_xdg_toplevel_decoration_v1 *decoration;
	struct wl_listener foreign_activate_request;
	struct wl_listener foreign_fullscreen_request;
	struct wl_listener foreign_close_request;
	struct wl_listener foreign_destroy;
	struct wl_listener foreign_minimize_request;
	struct wl_listener foreign_maximize_request;
	struct wl_listener set_decoration_mode;
	struct wl_listener destroy_decoration;

	const char *animation_type_open;
	const char *animation_type_close;
	int32_t is_in_scratchpad;
	int32_t iscustomsize;
	int32_t iscustompos;
	int32_t is_scratchpad_show;
	int32_t isglobal;
	int32_t isnoborder;
	int32_t isnoshadow;
	int32_t isnoradius;
	int32_t isnoanimation;
	int32_t isopensilent;
	int32_t istagsilent;
	int32_t iskilling;
	int32_t istagswitching;
	int32_t isnamedscratchpad;
	bool is_pending_open_animation;
	bool is_restoring_from_ov;
	float scroller_proportion;
	float stack_proportion;
	float old_stack_proportion;
	bool need_output_flush;
	struct dwl_animation animation;
	struct dwl_opacity_animation opacity_animation;
	int32_t isterm, noswallow;
	int32_t allow_csd;
	int32_t force_maximize;
	pid_t pid;
	Client *swallowing, *swallowedby;
	bool is_clip_to_hide;
	bool drag_to_tile;
	bool scratchpad_switching_mon;
	bool fake_no_border;
	int32_t nofocus;
	int32_t nofadein;
	int32_t nofadeout;
	int32_t no_force_center;
	int32_t isunglobal;
	float focused_opacity;
	float unfocused_opacity;
	char oldmonname[128];
	int32_t noblur;
	double master_mfact_per, master_inner_per, stack_inner_per;
	double old_master_mfact_per, old_master_inner_per, old_stack_inner_per;
	double old_scroller_pproportion;
	bool ismaster;
	bool cursor_in_upper_half, cursor_in_left_half;
	bool isleftstack;
	int32_t tearing_hint;
	int32_t force_tearing;
	int32_t allow_shortcuts_inhibit;
	float scroller_proportion_single;
	bool isfocusing;
	struct Client *next_in_stack;
	struct Client *prev_in_stack;
};

typedef struct {
	struct wl_list link;
	struct wl_resource *resource;
	Monitor *mon;
} DwlIpcOutput;

typedef struct {
	uint32_t mod;
	xkb_keysym_t keysym;
	int32_t (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	struct wlr_keyboard_group *wlr_group;

	int32_t nsyms;
	const xkb_keysym_t *keysyms; /* invalid if nsyms == 0 */
	uint32_t mods;				 /* invalid if nsyms == 0 */
	uint32_t keycode;
	struct wl_event_source *key_repeat_source;

	struct wl_listener modifiers;
	struct wl_listener key;
	struct wl_listener destroy;
} KeyboardGroup;

typedef struct {
	struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor;
	struct wl_listener destroy;
	struct wl_list link;
} KeyboardShortcutsInhibitor;

typedef struct {
	/* Must keep these three elements in this order */
	uint32_t type; /* LayerShell */
	struct wlr_box geom, current, pending, animainit_geom;
	Monitor *mon;
	struct wlr_scene_tree *scene;
	struct wlr_scene_tree *popups;
	struct wlr_scene_shadow *shadow;
	struct wlr_scene_layer_surface_v1 *scene_layer;
	struct wl_list link;
	struct wl_list fadeout_link;
	int32_t mapped;
	struct wlr_layer_surface_v1 *layer_surface;

	struct wl_listener destroy;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener surface_commit;

	struct dwl_animation animation;
	bool dirty;
	int32_t noblur;
	int32_t noanim;
	int32_t noshadow;
	char *animation_type_open;
	char *animation_type_close;
	bool need_output_flush;
	bool being_unmapped;
} LayerSurface;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
	const char *name;
	uint32_t id;
} Layout;

struct Monitor {
	struct wl_list link;
	struct wlr_output *wlr_output;
	struct wlr_scene_output *scene_output;
	struct wlr_output_state pending;
	struct wl_listener frame;
	struct wl_listener destroy;
	struct wl_listener request_state;
	struct wl_listener destroy_lock_surface;
	struct wlr_session_lock_surface_v1 *lock_surface;
	struct wlr_box m;		  /* monitor area, layout-relative */
	struct wlr_box w;		  /* window area, layout-relative */
	struct wl_list layers[4]; /* LayerSurface::link */
	uint32_t seltags;
	uint32_t tagset[2];

	struct wl_list dwl_ipc_outputs;
	int32_t gappih; /* horizontal gap between windows */
	int32_t gappiv; /* vertical gap between windows */
	int32_t gappoh; /* horizontal outer gaps */
	int32_t gappov; /* vertical outer gaps */
	Pertag *pertag;
	Client *sel, *prevsel;
	int32_t isoverview;
	int32_t is_in_hotarea;
	int32_t asleep;
	uint32_t visible_clients;
	uint32_t visible_tiling_clients;
	uint32_t visible_scroll_tiling_clients;
	struct wlr_scene_optimized_blur *blur;
	char last_surface_ws_name[256];
	struct wlr_ext_workspace_group_handle_v1 *ext_group;
};

typedef struct {
	struct wlr_pointer_constraint_v1 *constraint;
	struct wl_listener destroy;
} PointerConstraint;

typedef struct {
	struct wlr_scene_tree *scene;

	struct wlr_session_lock_v1 *lock;
	struct wl_listener new_surface;
	struct wl_listener unlock;
	struct wl_listener destroy;
} SessionLock;

// 在你的头文件中（比如 server.h）
// 在你的头文件中（比如 server.h）
typedef struct {
	// 键盘模式相关
	KeyMode keymode;

	// 核心 Wayland/WLRoots 对象
	struct wl_display *dpy;
	struct wl_event_loop *event_loop;
	struct wlr_backend *backend;
	struct wlr_backend *headless_backend;
	struct wlr_scene *scene;
	struct wlr_scene_tree *layers[NUM_LAYERS];
	struct wlr_renderer *drw;
	struct wlr_allocator *alloc;
	struct wlr_compositor *compositor;

	// Shell 相关
	struct wlr_xdg_shell *xdg_shell;
	struct wlr_xdg_activation_v1 *activation;
	struct wlr_xdg_decoration_manager_v1 *xdg_decoration_mgr;
	struct wl_list clients; /* tiling order */
	struct wl_list fstack;	/* focus order */
	struct wl_list fadeout_clients;
	struct wl_list fadeout_layers;
	struct wlr_idle_notifier_v1 *idle_notifier;
	struct wlr_idle_inhibit_manager_v1 *idle_inhibit_mgr;
	struct wlr_layer_shell_v1 *layer_shell;
	struct wlr_output_manager_v1 *output_mgr;
	struct wlr_virtual_keyboard_manager_v1 *virtual_keyboard_mgr;
	struct wlr_keyboard_shortcuts_inhibit_manager_v1
		*keyboard_shortcuts_inhibit;
	struct wlr_virtual_pointer_manager_v1 *virtual_pointer_mgr;
	struct wlr_output_power_manager_v1 *power_mgr;
	struct wlr_pointer_gestures_v1 *pointer_gestures;
	struct wlr_drm_lease_v1_manager *drm_lease_manager;
	struct mango_print_status_manager *print_status_manager;

	// 光标和输入相关
	struct wlr_cursor *cursor;
	struct wlr_xcursor_manager *cursor_mgr;
	struct wlr_session *session;

	// 界面元素
	struct wlr_scene_rect *root_bg;
	struct wlr_session_lock_manager_v1 *session_lock_mgr;
	struct wlr_scene_rect *locked_bg;
	struct wlr_session_lock_v1 *cur_lock;
	struct wlr_scene_tree *drag_icon;
	struct wlr_cursor_shape_manager_v1 *cursor_shape_mgr;
	struct wlr_pointer_constraints_v1 *pointer_constraints;
	struct wlr_relative_pointer_manager_v1 *relative_pointer_mgr;
	struct wlr_pointer_constraint_v1 *active_constraint;

	// 座位和键盘
	struct wlr_seat *seat;
	KeyboardGroup *kb_group;
	struct wl_list inputdevices;
	struct wl_list keyboard_shortcut_inhibitors;

	// 布局相关
	struct wlr_output_layout *output_layout;
	struct wl_list mons;
	Monitor *selmon;
	struct wlr_box sgeom;

	// 动画曲线数据
	struct dvec2 *baked_points_move;
	struct dvec2 *baked_points_open;
	struct dvec2 *baked_points_tag;
	struct dvec2 *baked_points_close;
	struct dvec2 *baked_points_focus;
	struct dvec2 *baked_points_opafadein;
	struct dvec2 *baked_points_opafadeout;

	// 事件源
	struct wl_event_source *hide_source;

	// 光标状态
	struct {
		enum wp_cursor_shape_device_v1_shape shape;
		struct wlr_surface *surface;
		int32_t hotspot_x;
		int32_t hotspot_y;
	} last_cursor;

	// 各种状态变量
	pid_t child_pid;
	int32_t locked;
	uint32_t locked_mods;
	void *exclusive_focus;
	uint32_t cursor_mode;
	Client *grabc;
	int32_t rzcorner;
	int32_t grabcx, grabcy;
	int32_t drag_begin_cursorx, drag_begin_cursory;
	bool start_drag_window;
	int32_t last_apply_drag_time;

	int32_t enablegaps;
	int32_t axis_apply_time;
	int32_t axis_apply_dir;
	int32_t scroller_focus_lock;

	uint32_t swipe_fingers;
	double swipe_dx;
	double swipe_dy;

	bool render_border;
	uint32_t chvt_backup_tag;
	bool allow_frame_scheduling;
	char chvt_backup_selmon[32];

	bool cursor_hidden;
	bool tag_combo;

	// 配置路径
	const char *cli_config_path;
} Server;


/* function declarations */
void init_server(void);
void free_server(void);
void applybounds(
	Client *c,
	struct wlr_box *bbox); // 设置边界规则,能让一些窗口拥有比较适合的大小
void applyrules(Client *c); // 窗口规则应用,应用config.h中定义的窗口规则
void arrange(Monitor *m, bool want_animation,
					bool from_view); // 布局函数,让窗口俺平铺规则移动和重置大小
void arrangelayer(Monitor *m, struct wl_list *list,
						 struct wlr_box *usable_area, int32_t exclusive);
void arrangelayers(Monitor *m);
void handle_print_status(struct wl_listener *listener, void *data);
void axisnotify(struct wl_listener *listener,
					   void *data); // 滚轮事件处理
void buttonpress(struct wl_listener *listener,
						void *data); // 鼠标按键事件处理
int32_t ongesture(struct wlr_pointer_swipe_end_event *event);
void swipe_begin(struct wl_listener *listener, void *data);
void swipe_update(struct wl_listener *listener, void *data);
void swipe_end(struct wl_listener *listener, void *data);
void pinch_begin(struct wl_listener *listener, void *data);
void pinch_update(struct wl_listener *listener, void *data);
void pinch_end(struct wl_listener *listener, void *data);
void hold_begin(struct wl_listener *listener, void *data);
void hold_end(struct wl_listener *listener, void *data);
void checkidleinhibitor(struct wlr_surface *exclude);
void cleanup(void);										  // 退出清理
void cleanupmon(struct wl_listener *listener, void *data); // 退出清理
void closemon(Monitor *m);
void cleanuplisteners(void);
void toggle_hotarea(int32_t x_root, int32_t y_root); // 触发热区
void maplayersurfacenotify(struct wl_listener *listener, void *data);
void commitlayersurfacenotify(struct wl_listener *listener, void *data);
void commitnotify(struct wl_listener *listener, void *data);
void createdecoration(struct wl_listener *listener, void *data);
void createidleinhibitor(struct wl_listener *listener, void *data);
void createkeyboard(struct wlr_keyboard *keyboard);
void requestmonstate(struct wl_listener *listener, void *data);
void createlayersurface(struct wl_listener *listener, void *data);
void createlocksurface(struct wl_listener *listener, void *data);
void createmon(struct wl_listener *listener, void *data);
void createnotify(struct wl_listener *listener, void *data);
void createpointer(struct wlr_pointer *pointer);
void configure_pointer(struct libinput_device *device);
void destroyinputdevice(struct wl_listener *listener, void *data);
void createswitch(struct wlr_switch *switch_device);
void switch_toggle(struct wl_listener *listener, void *data);
void createpointerconstraint(struct wl_listener *listener, void *data);
void cursorconstrain(struct wlr_pointer_constraint_v1 *constraint);
void commitpopup(struct wl_listener *listener, void *data);
void createpopup(struct wl_listener *listener, void *data);
void cursorframe(struct wl_listener *listener, void *data);
void cursorwarptohint(void);
void destroydecoration(struct wl_listener *listener, void *data);
void destroydragicon(struct wl_listener *listener, void *data);
void destroyidleinhibitor(struct wl_listener *listener, void *data);
void destroylayernodenotify(struct wl_listener *listener, void *data);
void destroylock(SessionLock *lock, int32_t unlocked);
void destroylocksurface(struct wl_listener *listener, void *data);
void destroynotify(struct wl_listener *listener, void *data);
void destroypointerconstraint(struct wl_listener *listener, void *data);
void destroysessionlock(struct wl_listener *listener, void *data);
void destroykeyboardgroup(struct wl_listener *listener, void *data);
Monitor *dirtomon(enum wlr_direction dir);
void setcursorshape(struct wl_listener *listener, void *data);

void focusclient(Client *c, int32_t lift);

void setborder_color(Client *c);
Client *focustop(Monitor *m);
void fullscreennotify(struct wl_listener *listener, void *data);
void gpureset(struct wl_listener *listener, void *data);

int32_t keyrepeat(void *data);

void inputdevice(struct wl_listener *listener, void *data);
int32_t keybinding(uint32_t state, bool locked, uint32_t mods,
						  xkb_keysym_t sym, uint32_t keycode);
void keypress(struct wl_listener *listener, void *data);
void keypressmod(struct wl_listener *listener, void *data);
bool keypressglobal(struct wlr_surface *last_surface,
						   struct wlr_keyboard *keyboard,
						   struct wlr_keyboard_key_event *event, uint32_t mods,
						   xkb_keysym_t keysym, uint32_t keycode);
void locksession(struct wl_listener *listener, void *data);
void mapnotify(struct wl_listener *listener, void *data);
void maximizenotify(struct wl_listener *listener, void *data);
void minimizenotify(struct wl_listener *listener, void *data);
void motionabsolute(struct wl_listener *listener, void *data);
void motionnotify(uint32_t time, struct wlr_input_device *device,
						 double sx, double sy, double sx_unaccel,
						 double sy_unaccel);
void motionrelative(struct wl_listener *listener, void *data);

void reset_foreign_tolevel(Client *c);
void remove_foreign_topleve(Client *c);
void add_foreign_topleve(Client *c);
void exchange_two_client(Client *c1, Client *c2);
void outputmgrapply(struct wl_listener *listener, void *data);
void outputmgrapplyortest(struct wlr_output_configuration_v1 *config,
								 int32_t test);
void outputmgrtest(struct wl_listener *listener, void *data);
void pointerfocus(Client *c, struct wlr_surface *surface, double sx,
						 double sy, uint32_t time);
void printstatus(void);
void quitsignal(int32_t signo);
void powermgrsetmode(struct wl_listener *listener, void *data);
void rendermon(struct wl_listener *listener, void *data);
void requestdecorationmode(struct wl_listener *listener, void *data);
void requestdrmlease(struct wl_listener *listener, void *data);
void requeststartdrag(struct wl_listener *listener, void *data);
void resize(Client *c, struct wlr_box geo, int32_t interact);
void run(char *startup_cmd);
void setcursor(struct wl_listener *listener, void *data);
void setfloating(Client *c, int32_t floating);
void setfakefullscreen(Client *c, int32_t fakefullscreen);
void setfullscreen(Client *c, int32_t fullscreen);
void setmaximizescreen(Client *c, int32_t maximizescreen);
void reset_maximizescreen_size(Client *c);
void setgaps(int32_t oh, int32_t ov, int32_t ih, int32_t iv);

void setmon(Client *c, Monitor *m, uint32_t newtags, bool focus);
void setpsel(struct wl_listener *listener, void *data);
void setsel(struct wl_listener *listener, void *data);
void setup(void);
void startdrag(struct wl_listener *listener, void *data);

void unlocksession(struct wl_listener *listener, void *data);
void unmaplayersurfacenotify(struct wl_listener *listener, void *data);
void unmapnotify(struct wl_listener *listener, void *data);
void updatemons(struct wl_listener *listener, void *data);
void updatetitle(struct wl_listener *listener, void *data);
void urgent(struct wl_listener *listener, void *data);
void view(const Arg *arg, bool want_animation);

void handlesig(int32_t signo);
void
handle_keyboard_shortcuts_inhibit_new_inhibitor(struct wl_listener *listener,
												void *data);
void virtualkeyboard(struct wl_listener *listener, void *data);
void virtualpointer(struct wl_listener *listener, void *data);
void warp_cursor(const Client *c);
Monitor *xytomon(double x, double y);
void xytonode(double x, double y, struct wlr_surface **psurface,
					 Client **pc, LayerSurface **pl, double *nx, double *ny);
void clear_fullscreen_flag(Client *c);
pid_t getparentprocess(pid_t p);
int32_t isdescprocess(pid_t p, pid_t c);
Client *termforwin(Client *w);
void swallow(Client *c, Client *w);

void warp_cursor_to_selmon(Monitor *m);
uint32_t want_restore_fullscreen(Client *target_client);
void overview_restore(Client *c, const Arg *arg);
void overview_backup(Client *c);
void set_minimized(Client *c);

void show_scratchpad(Client *c);
void show_hide_client(Client *c);
void tag_client(const Arg *arg, Client *target_client);

struct wlr_box setclient_coordinate_center(Client *c, Monitor *m,
												  struct wlr_box geom,
												  int32_t offsetx,
												  int32_t offsety);
uint32_t get_tags_first_tag(uint32_t tags);

struct wlr_output_mode *
get_nearest_output_mode(struct wlr_output *output, int32_t width,
						int32_t height, float refresh);

void client_commit(Client *c);
void layer_commit(LayerSurface *l);
void apply_border(Client *c);
void client_set_opacity(Client *c, double opacity);
void init_baked_points(void);
void scene_buffer_apply_opacity(struct wlr_scene_buffer *buffer,
									   int32_t sx, int32_t sy, void *data);

Client *direction_select(const Arg *arg);
void view_in_mon(const Arg *arg, bool want_animation, Monitor *m,
						bool changefocus);

void buffer_set_effect(Client *c, BufferData buffer_data);
void snap_scene_buffer_apply_effect(struct wlr_scene_buffer *buffer,
										   int32_t sx, int32_t sy, void *data);
void client_set_pending_state(Client *c);
void layer_set_pending_state(LayerSurface *l);
void set_rect_size(struct wlr_scene_rect *rect, int32_t width,
						  int32_t height);
Client *center_tiled_select(Monitor *m);
void handlecursoractivity(void);
int32_t hidecursor(void *data);
bool check_hit_no_border(Client *c);
void reset_keyboard_layout(void);
void client_update_oldmonname_record(Client *c, Monitor *m);
void pending_kill_client(Client *c);
uint32_t get_tags_first_tag_num(uint32_t source_tags);
void set_layer_open_animaiton(LayerSurface *l, struct wlr_box geo);
void init_fadeout_layers(LayerSurface *l);
void layer_actual_size(LayerSurface *l, int32_t *width, int32_t *height);
void get_layer_target_geometry(LayerSurface *l,
									  struct wlr_box *target_box);
void scene_buffer_apply_effect(struct wlr_scene_buffer *buffer,
									  int32_t sx, int32_t sy, void *data);
double find_animation_curve_at(double t, int32_t type);

void apply_opacity_to_rect_nodes(Client *c, struct wlr_scene_node *node,
										double animation_passed);
enum corner_location set_client_corner_location(Client *c);
double all_output_frame_duration_ms();
struct wlr_scene_tree *
wlr_scene_tree_snapshot(struct wlr_scene_node *node,
						struct wlr_scene_tree *parent);
bool is_scroller_layout(Monitor *m);
bool is_centertile_layout(Monitor *m);
void create_output(struct wlr_backend *backend, void *data);
void get_layout_abbr(char *abbr, const char *full_name);
void apply_named_scratchpad(Client *target_client);
Client *get_client_by_id_or_title(const char *arg_id,
										 const char *arg_title);
bool switch_scratchpad_client_state(Client *c);
bool check_trackpad_disabled(struct wlr_pointer *pointer);
uint32_t get_tag_status(uint32_t tag, Monitor *m);
void enable_adaptive_sync(Monitor *m, struct wlr_output_state *state);
Client *get_next_stack_client(Client *c, bool reverse);
void set_float_malposition(Client *tc);
void set_size_per(Monitor *m, Client *c);
void resize_tile_client(Client *rec, bool isdrag, int32_t offsetx,
							   int32_t offsety, uint32_t time);
void refresh_monitors_workspaces_status(Monitor *m);
void init_client_properties(Client *c);
float *get_border_color(Client *c);
void clear_fullscreen_and_maximized_state(Monitor *m);
void request_fresh_all_monitors(void);
Client *find_client_by_direction(Client *tc, const Arg *arg,
										bool findfloating, bool ignore_align);
void exit_scroller_stack(Client *c);
Client *get_scroll_stack_head(Client *c);
bool client_only_in_one_tag(Client *c);
Client *get_focused_stack_client(Client *sc);
bool client_is_in_same_stack(Client *sc, Client *tc, Client *fc);

#include "config/preset.h"
struct Pertag {
	uint32_t curtag, prevtag;			/* current and previous tag */
	int32_t nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
	float mfacts[LENGTH(tags) + 1];		/* mfacts per tag */
	bool no_hide[LENGTH(tags) + 1];		/* no_hide per tag */
	bool no_render_border[LENGTH(tags) + 1]; /* no_render_border per tag */
	const Layout
		*ltidxs[LENGTH(tags) + 1]; /* matrix of tags and layouts indexes  */
};
