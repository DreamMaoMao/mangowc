#ifndef MANGO_ANIMATION_COMMON_H
#define MANGO_ANIMATION_COMMON_H

// 动画曲线相关函数
struct dvec2 calculate_animation_curve_at(double t, int32_t type);
void init_baked_points(void);
double find_animation_curve_at(double t, int32_t type);

// 场景快照函数
bool scene_node_snapshot(struct wlr_scene_node *node, int32_t lx, int32_t ly, struct wlr_scene_tree *snapshot_tree);
struct wlr_scene_tree *wlr_scene_tree_snapshot(struct wlr_scene_node *node, struct wlr_scene_tree *parent);

// 屏幕刷新函数
void request_fresh_all_monitors(void);

#endif // MANGO_ANIMATION_COMMON_H