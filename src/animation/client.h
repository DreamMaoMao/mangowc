#ifndef MANGO_ANIMATION_CLIENT_H
#define MANGO_ANIMATION_CLIENT_H

// 函数声明
void client_actual_size(Client *c, int32_t *width, int32_t *height);
void set_rect_size(struct wlr_scene_rect *rect, int32_t width, int32_t height);
enum corner_location set_client_corner_location(Client *c);
bool is_horizontal_stack_layout(Monitor *m);
bool is_horizontal_right_stack_layout(Monitor *m);
int32_t is_special_animaiton_rule(Client *c);
void set_client_open_animaiton(Client *c, struct wlr_box geo);
void snap_scene_buffer_apply_effect(struct wlr_scene_buffer *buffer, int32_t sx, int32_t sy, void *data);
void scene_buffer_apply_effect(struct wlr_scene_buffer *buffer, int32_t sx, int32_t sy, void *data);
void buffer_set_effect(Client *c, BufferData data);
void client_draw_shadow(Client *c);
void apply_border(Client *c);
struct ivec2 clip_to_hide(Client *c, struct wlr_box *clip_box);
void client_apply_clip(Client *c, float factor);
void fadeout_client_animation_next_tick(Client *c);
void client_animation_next_tick(Client *c);
void init_fadeout_client(Client *c);
void client_commit(Client *c);
void client_set_pending_state(Client *c);
void resize(Client *c, struct wlr_box geo, int32_t interact);
bool client_draw_fadeout_frame(Client *c);
void client_set_focused_opacity_animation(Client *c);
void client_set_unfocused_opacity_animation(Client *c);
bool client_apply_focus_opacity(Client *c);
bool client_draw_frame(Client *c);

#endif // CLIENT_DRAW_H