// TODO: remove this file in the future, replace all global variables with
// config.xxx

/* speedie's mango config */

#define COLOR(hex)                                                             \
	{                                                                          \
		((hex >> 24) & 0xFF) / 255.0f, ((hex >> 16) & 0xFF) / 255.0f,          \
			((hex >> 8) & 0xFF) / 255.0f, (hex & 0xFF) / 255.0f                \
	}

/* animation */
char *animation_type_open =
	"slide"; // Animation type for window open: slide or zoom
char *animation_type_close =
	"slide"; // Animation type for window close: slide or zoom
char *layer_animation_type_open =
	"slide"; // Animation type for layer open: slide or zoom
char *layer_animation_type_close =
	"slide";				  // Animation type for layer close: slide or zoom
int32_t animations = 1;		  // Enable window animations
int32_t layer_animations = 0; // Enable layer animations
int32_t tag_animation_direction = HORIZONTAL; // Tag animation direction
int32_t animation_fade_in = 1;				  // Enable animation fade in
int32_t animation_fade_out = 1;				  // Enable animation fade out
float zoom_initial_ratio = 0.3; // Initial window size ratio for zoom animation
float zoom_end_ratio = 0.8;		// End window size ratio for zoom animation
float fadein_begin_opacity = 0.5;  // Begin opac window ratio for animations
float fadeout_begin_opacity = 0.5; // Begin opac window ratio for animations
uint32_t animation_duration_move = 500;	 // Animation move speed
uint32_t animation_duration_open = 400;	 // Animation open speed
uint32_t animation_duration_tag = 300;	 // Animation tag speed
uint32_t animation_duration_close = 300; // Animation close speed
uint32_t animation_duration_focus = 0;	 // Animation focus opacity speed
// Animation curve for move
double animation_curve_move[4] = {0.46, 1.0, 0.29, 0.99};
// Animation curve for open
double animation_curve_open[4] = {0.46, 1.0, 0.29, 0.99};
// Animation curve for tag
double animation_curve_tag[4] = {0.46, 1.0, 0.29, 0.99};
// Animation curve for close
double animation_curve_close[4] = {0.46, 1.0, 0.29, 0.99};
// Animation curve for focus
double animation_curve_focus[4] = {0.46, 1.0, 0.29, 0.99};
// Animation curve for opacity fade in
double animation_curve_opafadein[4] = {0.46, 1.0, 0.29, 0.99};
// Animation curve for opacity fade out
double animation_curve_opafadeout[4] = {0.5, 0.5, 0.5, 0.5};

/* appearance */
uint32_t axis_bind_apply_timeout =
	100; // Timeout interval for mouse wheel binding actions
uint32_t focus_on_activate =
	1; // Auto-focus when receiving window activation request
uint32_t new_is_master = 1;	  // Insert new windows at the head
double default_mfact = 0.55f; // Master window proportion
uint32_t default_nmaster = 1; // Default number of master windows
int32_t center_master_overspread =
	0; // Whether to fill screen when center master
int32_t center_when_single_stack =
	1; // Whether to center when single stack window
/* logging */
int32_t log_level = WLR_ERROR;
uint32_t numlockon = 0; // Enable numlock
uint32_t capslock = 0;	// Enable capslock

uint32_t ov_tab_mode = 0;	// Alt-tab switch mode
uint32_t hotarea_size = 10; // Hot corner size in pixels (10x10)
uint32_t hotarea_corner = BOTTOM_LEFT;
uint32_t enable_hotarea = 1; // Enable mouse hot corner
int32_t smartgaps = 0; /* 1 means no outer gap when there is only one window */
int32_t sloppyfocus = 1; /* focus follows mouse */
uint32_t gappih = 5;	 /* horiz inner gap between windows */
uint32_t gappiv = 5;	 /* vert inner gap between windows */
uint32_t gappoh = 10;	 /* horiz outer gap between windows and screen edge */
uint32_t gappov = 10;	 /* vert outer gap between windows and screen edge */
float scratchpad_width_ratio = 0.8;
float scratchpad_height_ratio = 0.9;

int32_t scroller_structs = 20;
float scroller_default_proportion = 0.9;
float scroller_default_proportion_single = 1.0;
int32_t scroller_ignore_proportion_single = 1;
int32_t scroller_focus_center = 0;
int32_t scroller_prefer_center = 0;
int32_t focus_cross_monitor = 0;
int32_t focus_cross_tag = 0;
int32_t exchange_cross_monitor = 0;
int32_t scratchpad_cross_monitor = 0;
int32_t view_current_to_back = 0;
int32_t no_border_when_single = 0;
int32_t no_radius_when_single = 0;
int32_t snap_distance = 30;
int32_t enable_floating_snap = 0;
int32_t drag_tile_to_tile = 0;
uint32_t cursor_size = 24;
uint32_t cursor_hide_timeout = 0;

uint32_t swipe_min_threshold = 1;

int32_t idleinhibit_ignore_visible =
	0; /* 1 means idle inhibitors will disable idle tracking even if it's
		  surface isn't visible  */
uint32_t borderpx = 4; /* border pixel of windows */
float rootcolor[] = COLOR(0x323232ff);
float bordercolor[] = COLOR(0x444444ff);
float focuscolor[] = COLOR(0xc66b25ff);
float maximizescreencolor[] = COLOR(0x89aa61ff);
float urgentcolor[] = COLOR(0xad401fff);
float scratchpadcolor[] = COLOR(0x516c93ff);
float globalcolor[] = COLOR(0xb153a7ff);
float overlaycolor[] = COLOR(0x14a57cff);
// char *cursor_theme = "Bibata-Modern-Ice";

int32_t overviewgappi =
	5; /* Gap size between windows and edges in overview mode */
int32_t overviewgappo = 30; /* Gap size between windows in overview mode */

/* To conform the xdg-protocol, set the alpha to zero to restore the old
 * behavior */
float fullscreen_bg[] = {0.1, 0.1, 0.1, 1.0};

int32_t warpcursor = 1;
int32_t drag_corner = 3;
int32_t drag_warp_cursor = 1;
int32_t xwayland_persistence = 1; /* xwayland persistence */
int32_t syncobj_enable = 0;
int32_t allow_lock_transparent = 0;
double drag_tile_refresh_interval = 16.0;
double drag_floating_refresh_interval = 8.0;
int32_t allow_tearing = TEARING_DISABLED;
int32_t allow_shortcuts_inhibit = SHORTCUTS_INHIBIT_ENABLE;

/* keyboard */

/*
	only layout can modify after fisrt init
	other fields change will be ignored.
*/
char xkb_rules_rules[256];
char xkb_rules_model[256];
char xkb_rules_layout[256];
char xkb_rules_variant[256];
char xkb_rules_options[256];

/* keyboard */
static const struct xkb_rule_names xkb_fallback_rules = {
	.layout = "us",
	.variant = NULL,
	.model = NULL,
	.rules = NULL,
	.options = NULL,
};

static const struct xkb_rule_names xkb_default_rules = {
	.options = NULL,
};

struct xkb_rule_names xkb_rules = {
	/* can specify fields: rules, model, layout, variant, options */
	/* example:
	.options = "ctrl:nocaps",
	*/
	.rules = xkb_rules_rules,	  .model = xkb_rules_model,
	.layout = xkb_rules_layout,	  .variant = xkb_rules_variant,
	.options = xkb_rules_options,
};

int32_t repeat_rate = 25;
int32_t repeat_delay = 600;

/* Trackpad */
int32_t disable_trackpad = 0;
int32_t tap_to_click = 1;
int32_t tap_and_drag = 1;
int32_t drag_lock = 1;
int32_t mouse_natural_scrolling = 0;
int32_t trackpad_natural_scrolling = 0;
int32_t disable_while_typing = 1;
int32_t left_handed = 0;
int32_t middle_button_emulation = 0;
int32_t single_scratchpad = 1;
int32_t edge_scroller_pointer_focus = 1;

/* You can choose between:
LIBINPUT_CONFIG_SCROLL_NO_SCROLL
LIBINPUT_CONFIG_SCROLL_2FG
LIBINPUT_CONFIG_SCROLL_EDGE
LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN
*/
enum libinput_config_scroll_method scroll_method = LIBINPUT_CONFIG_SCROLL_2FG;
uint32_t scroll_button = 274;

/* You can choose between:
LIBINPUT_CONFIG_CLICK_METHOD_NONE
LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS
LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
*/
enum libinput_config_click_method click_method =
	LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;

double axis_scroll_factor = 1.0;

/* You can choose between:
LIBINPUT_CONFIG_SEND_EVENTS_ENABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE
*/
uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;

/* You can choose between:
LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT
LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
*/
enum libinput_config_accel_profile accel_profile =
	LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
double accel_speed = 0.0;
/* You can choose between:
LIBINPUT_CONFIG_TAP_MAP_LRM -- 1/2/3 finger tap maps to left/right/middle
LIBINPUT_CONFIG_TAP_MAP_LMR -- 1/2/3 finger tap maps to left/middle/right
*/
enum libinput_config_tap_button_map button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;

/* If you want to use the windows key for MODKEY, use WLR_MODIFIER_LOGO */
#define MODKEY WLR_MODIFIER_ALT

static const char *tags[] = {
	"1", "2", "3", "4", "5", "6", "7", "8", "9",
};

float focused_opacity = 1.0;
float unfocused_opacity = 1.0;

int32_t border_radius = 0;
int32_t border_radius_location_default = CORNER_LOCATION_ALL;
int32_t blur = 0;
int32_t blur_layer = 0;
int32_t blur_optimized = 1;

struct blur_data blur_params;

int32_t blur_params_num_passes = 1;
int32_t blur_params_radius = 5;
float blur_params_noise = 0.02;
float blur_params_brightness = 0.9;
float blur_params_contrast = 0.9;
float blur_params_saturation = 1.2;

int32_t shadows = 0;
int32_t shadow_only_floating = 1;
int32_t layer_shadows = 0;
uint32_t shadows_size = 10;
double shadows_blur = 15;
int32_t shadows_position_x = 0;
int32_t shadows_position_y = 0;
float shadowscolor[] = COLOR(0x000000ff);
