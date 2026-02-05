#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

/* ====================== 宏定义和常量 ====================== */
#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif

// 整数和浮点数截断宏
#define CLAMP_INT(x, min, max) ((int32_t)(x) < (int32_t)(min) ? (int32_t)(min) : ((int32_t)(x) > (int32_t)(max) ? (int32_t)(max) : (int32_t)(x)))
#define CLAMP_FLOAT(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

// 修改后的宏定义
#define CHVT(n)                                                                \
	{                                                                          \
		WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT,                                  \
			{.keysym = XKB_KEY_XF86Switch_VT_##n, .type = KEY_TYPE_SYM}, chvt, \
		{                                                                      \
			.ui = (n)                                                          \
		}                                                                      \
	}

// 数字类型枚举
enum { NUM_TYPE_MINUS, NUM_TYPE_PLUS, NUM_TYPE_DEFAULT };
// 键类型枚举
enum { KEY_TYPE_CODE, KEY_TYPE_SYM };


/* ====================== 结构体定义 ====================== */
typedef struct {
    uint32_t keycode1;
    uint32_t keycode2;
    uint32_t keycode3;
} MultiKeycode;

typedef struct {
    xkb_keysym_t keysym;
    MultiKeycode keycode;
    int32_t type;
} KeySymCode;

typedef struct {
    uint32_t mod;
    KeySymCode keysymcode;
    int32_t (*func)(const Arg *);
    Arg arg;
    int32_t keymode_idx;
    bool islockapply;
    bool isreleaseapply;
    bool ispassapply;
} KeyBinding;

typedef struct {
    char *type;
    char *value;
} ConfigEnv;

typedef struct {
    const char *id;
    const char *title;
    uint32_t tags;
    int32_t isfloating;
    int32_t isfullscreen;
    float scroller_proportion;
    const char *animation_type_open;
    const char *animation_type_close;
    const char *layer_animation_type_open;
    const char *layer_animation_type_close;
    int32_t isnoborder;
    int32_t isnoshadow;
    int32_t isnoradius;
    int32_t isnoanimation;
    int32_t isopensilent;
    int32_t istagsilent;
    int32_t isnamedscratchpad;
    int32_t isunglobal;
    int32_t isglobal;
    int32_t isoverlay;
    int32_t allow_shortcuts_inhibit;
    int32_t ignore_maximize;
    int32_t ignore_minimize;
    int32_t isnosizehint;
    const char *monitor;
    int32_t offsetx;
    int32_t offsety;
    int32_t width;
    int32_t height;
    int32_t nofocus;
    int32_t nofadein;
    int32_t nofadeout;
    int32_t no_force_center;
    int32_t isterm;
    int32_t allow_csd;
    int32_t force_maximize;
    int32_t force_tearing;
    int32_t noswallow;
    int32_t noblur;
    float focused_opacity;
    float unfocused_opacity;
    float scroller_proportion_single;
    uint32_t passmod;
    xkb_keysym_t keysym;
    KeyBinding globalkeybinding;
} ConfigWinRule;

typedef struct {
    const char *name;
    int32_t rr;
    float scale;
    int32_t x, y;
    int32_t width, height;
    float refresh;
    int32_t vrr;
} ConfigMonitorRule;

typedef struct {
    uint32_t mod;
    uint32_t button;
    int32_t (*func)(const Arg *);
    Arg arg;
} MouseBinding;

typedef struct {
    uint32_t mod;
    uint32_t dir;
    int32_t (*func)(const Arg *);
    Arg arg;
} AxisBinding;

typedef struct {
    uint32_t fold;
    int32_t (*func)(const Arg *);
    Arg arg;
} SwitchBinding;

typedef struct {
    uint32_t mod;
    uint32_t motion;
    uint32_t fingers_count;
    int32_t (*func)(const Arg *);
    Arg arg;
} GestureBinding;

typedef struct {
    int32_t id;
    char *layout_name;
    char *monitor_name;
    float mfact;
    int32_t nmaster;
    int32_t no_render_border;
    int32_t no_hide;
} ConfigTagRule;

typedef struct {
    char *layer_name;
    char *animation_type_open;
    char *animation_type_close;
    int32_t noblur;
    int32_t noanim;
    int32_t noshadow;
} ConfigLayerRule;

typedef struct {
    // 动画设置
    int32_t animations;
    int32_t layer_animations;
    char animation_type_open[10];
    char animation_type_close[10];
    char layer_animation_type_open[10];
    char layer_animation_type_close[10];
    int32_t animation_fade_in;
    int32_t animation_fade_out;
    int32_t tag_animation_direction;
    float zoom_initial_ratio;
    float zoom_end_ratio;
    float fadein_begin_opacity;
    float fadeout_begin_opacity;
    uint32_t animation_duration_move;
    uint32_t animation_duration_open;
    uint32_t animation_duration_tag;
    uint32_t animation_duration_close;
    uint32_t animation_duration_focus;
    double animation_curve_move[4];
    double animation_curve_open[4];
    double animation_curve_tag[4];
    double animation_curve_close[4];
    double animation_curve_focus[4];
    double animation_curve_opafadein[4];
    double animation_curve_opafadeout[4];
    
    // 滚动布局设置
    int32_t scroller_structs;
    float scroller_default_proportion;
    float scroller_default_proportion_single;
    int32_t scroller_ignore_proportion_single;
    int32_t scroller_focus_center;
    int32_t scroller_prefer_center;
    int32_t edge_scroller_pointer_focus;
    int32_t focus_cross_monitor;
    int32_t exchange_cross_monitor;
    int32_t scratchpad_cross_monitor;
    int32_t focus_cross_tag;
    int32_t view_current_to_back;
    int32_t no_border_when_single;
    int32_t no_radius_when_single;
    int32_t snap_distance;
    int32_t enable_floating_snap;
    int32_t drag_tile_to_tile;
    uint32_t swipe_min_threshold;
    float focused_opacity;
    float unfocused_opacity;
    float *scroller_proportion_preset;
    int32_t scroller_proportion_preset_count;
    
    // 布局设置
    char **circle_layout;
    int32_t circle_layout_count;
    uint32_t new_is_master;
    float default_mfact;
    uint32_t default_nmaster;
    int32_t center_master_overspread;
    int32_t center_when_single_stack;
    
    // 概述模式设置
    uint32_t hotarea_size;
    uint32_t hotarea_corner;
    uint32_t enable_hotarea;
    uint32_t ov_tab_mode;
    int32_t overviewgappi;
    int32_t overviewgappo;
    uint32_t cursor_hide_timeout;
    
    // 其他设置
    uint32_t axis_bind_apply_timeout;
    uint32_t focus_on_activate;
    int32_t idleinhibit_ignore_visible;
    int32_t sloppyfocus;
    int32_t warpcursor;
    int32_t drag_corner;
    int32_t drag_warp_cursor;
    
    // 键盘设置
    int32_t repeat_rate;
    int32_t repeat_delay;
    uint32_t numlockon;
    
    // 触控板和鼠标设置
    int32_t disable_trackpad;
    int32_t tap_to_click;
    int32_t tap_and_drag;
    int32_t drag_lock;
    int32_t mouse_natural_scrolling;
    int32_t trackpad_natural_scrolling;
    int32_t disable_while_typing;
    int32_t left_handed;
    int32_t middle_button_emulation;
    uint32_t accel_profile;
    double accel_speed;
    uint32_t scroll_method;
    uint32_t scroll_button;
    uint32_t click_method;
    uint32_t send_events_mode;
    uint32_t button_map;
    double axis_scroll_factor;
    
    // 外观效果
    int32_t blur;
    int32_t blur_layer;
    int32_t blur_optimized;
    int32_t border_radius;
    struct blur_data blur_params;
    int32_t shadows;
    int32_t shadow_only_floating;
    int32_t layer_shadows;
    uint32_t shadows_size;
    float shadows_blur;
    int32_t shadows_position_x;
    int32_t shadows_position_y;
    float shadowscolor[4];
    
    // 间距和边框
    int32_t smartgaps;
    uint32_t gappih;
    uint32_t gappiv;
    uint32_t gappoh;
    uint32_t gappov;
    uint32_t borderpx;
    float scratchpad_width_ratio;
    float scratchpad_height_ratio;
    
    // 颜色设置
    float rootcolor[4];
    float bordercolor[4];
    float focuscolor[4];
    float maximizescreencolor[4];
    float urgentcolor[4];
    float scratchpadcolor[4];
    float globalcolor[4];
    float overlaycolor[4];
    
    // 自动启动
    char autostart[3][256];
    
    // 规则数组
    ConfigTagRule *tag_rules;
    int32_t tag_rules_count;
    ConfigLayerRule *layer_rules;
    int32_t layer_rules_count;
    ConfigWinRule *window_rules;
    int32_t window_rules_count;
    ConfigMonitorRule *monitor_rules;
    int32_t monitor_rules_count;
    
    // 绑定数组
    KeyBinding *key_bindings;
    int32_t key_bindings_count;
    MouseBinding *mouse_bindings;
    int32_t mouse_bindings_count;
    AxisBinding *axis_bindings;
    int32_t axis_bindings_count;
    SwitchBinding *switch_bindings;
    int32_t switch_bindings_count;
    GestureBinding *gesture_bindings;
    int32_t gesture_bindings_count;
    
    // 环境变量和命令
    ConfigEnv **env;
    int32_t env_count;
    char **exec;
    int32_t exec_count;
    char **exec_once;
    int32_t exec_once_count;
    
    // 光标设置
    char *cursor_theme;
    uint32_t cursor_size;
    
    // 杂项
    int32_t single_scratchpad;
    int32_t xwayland_persistence;
    int32_t syncobj_enable;
    int32_t allow_tearing;
    int32_t allow_shortcuts_inhibit;
    int32_t allow_lock_transparent;
    
    // 键盘布局
    struct xkb_rule_names xkb_rules;
    
    // 按键模式
    KeyMode *keymodes;
    int32_t keymodes_count;
    int32_t current_keymode_idx;
    
    // XKB上下文
    struct xkb_context *ctx;
    struct xkb_keymap *keymap;
} Config;

/* ====================== 全局变量 ====================== */
extern KeyBinding default_key_bindings[];
typedef int32_t (*FuncType)(const Arg *);

/* ====================== 字符串处理函数 ====================== */
void trim_whitespace(char *str);
char *sanitize_string(char *str);
bool starts_with_ignore_case(const char *str, const char *prefix);
char *combine_args_until_empty(char *values[], int count);

/* ====================== 解析函数 ====================== */
// 通用解析
int32_t parse_double_array(const char *input, double *output, int32_t max_count);
uint32_t parse_num_type(char *str);
int64_t parse_color(const char *hex_str);

// 方向解析
int32_t parse_circle_direction(const char *str);
int32_t parse_direction(const char *str);
int32_t parse_fold_state(const char *str);
uint32_t parse_button(const char *str);
int32_t parse_mouse_action(const char *str);

// 按键绑定解析
void parse_bind_flags(const char *str, KeyBinding *kb);
uint32_t parse_mod(const char *mod_str);
KeySymCode parse_key(const char *key_str, bool isbindsym);
int32_t find_keycodes_for_keysym(struct xkb_keymap *keymap, xkb_keysym_t sym, MultiKeycode *multi_kc);

// 颜色转换
void convert_hex_to_rgba(float *color, uint32_t hex);

// 函数名解析
FuncType parse_func_name(char *func_name, Arg *arg, char *arg_value, char *arg_value2, 
                         char *arg_value3, char *arg_value4, char *arg_value5);

/* ====================== 配置解析核心函数 ====================== */
void parse_config_file(Config *config, const char *file_path);
bool parse_config_line(Config *config, const char *line);
bool parse_option(Config *config, char *key, char *value);
void parse_config(void);

/* ====================== KeyMode 管理函数 ====================== */
int32_t find_or_add_keymode(Config *config, const char *name);
void set_current_keymode(Config *config, const char *name);

/* ====================== 配置应用函数 ====================== */
void set_env(void);
void run_exec(void);
void run_exec_once(void);
void set_default_key_bindings(Config *config);
void override_config(void);
void set_value_default(void);

/* ====================== 配置重新应用函数 ====================== */
void reset_option(void);
int32_t reload_config(const Arg *arg);
void reset_blur_params(void);
void reapply_monitor_rules(void);
void reapply_cursor_style(void);
void reapply_border(void);
void reapply_keyboard(void);
void reapply_pointer(void);
void reapply_master(void);
void reapply_tagrule(void);
void parse_tagrule(Monitor *m);

/* ====================== 配置清理函数 ====================== */
void free_config(void);
void free_circle_layout(Config *config);
void free_baked_points(void);
void cleanup_config_keymap(void);
void create_config_keymap(void);

#endif // CONFIG_PARSER_H