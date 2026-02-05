#ifndef MANGO_CLIENT_CLIENT_H
#define MANGO_CLIENT_CLIENT_H

/* 基础客户端类型检测和表面处理 */
int32_t client_is_x11(Client *c);
struct wlr_surface *client_surface(Client *c);
int32_t toplevel_from_wlr_surface(struct wlr_surface *s, Client **pc, LayerSurface **pl);

/* 客户端激活和焦点控制 */
void client_activate_surface(struct wlr_surface *s, int32_t activated);
void client_notify_enter(struct wlr_surface *s, struct wlr_keyboard *kb);
int32_t client_should_ignore_focus(Client *c);
int32_t client_is_x11_popup(Client *c);
int32_t client_wants_focus(Client *c);

/* 客户端信息获取 */
const char *client_get_appid(Client *c);
int32_t client_get_pid(Client *c);
const char *client_get_title(Client *c);
Client *client_get_parent(Client *c);
int32_t client_has_children(Client *c);
int32_t client_is_float_type(Client *c);
int32_t client_is_rendered_on_mon(Client *c, Monitor *m);
int32_t client_is_stopped(Client *c);
int32_t client_is_unmanaged(Client *c);
int32_t client_wants_fullscreen(Client *c);
bool client_request_minimize(Client *c, void *data);
bool client_request_maximize(Client *c, void *data);

/* 客户端几何和布局 */
void client_get_clip(Client *c, struct wlr_box *clip);
void client_get_geometry(Client *c, struct wlr_box *geom);
void client_set_size_bound(Client *c);

/* 客户端状态设置 */
void client_send_close(Client *c);
void client_set_border_color(Client *c, const float color[4]);
void client_set_fullscreen(Client *c, int32_t fullscreen);
void client_set_scale(struct wlr_surface *s, float scale);
uint32_t client_set_size(Client *c, uint32_t width, uint32_t height);
void client_set_minimized(Client *c, bool minimized);
void client_set_maximized(Client *c, bool maximized);
void client_set_tiled(Client *c, uint32_t edges);
void client_set_suspended(Client *c, int32_t suspended);

/* 特殊客户端行为 */
int32_t client_should_global(Client *c);
int32_t client_should_overtop(Client *c);

#endif // MANGO_CLIENT_CLIENT_H