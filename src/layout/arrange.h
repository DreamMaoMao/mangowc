#ifndef TILE_RESIZE_H
#define TILE_RESIZE_H

/* 平铺窗口大小比例设置 */
void set_size_per(Monitor *m, Client *c);

/* 水平布局平铺窗口调整 */
void resize_tile_master_horizontal(Client *rec, bool isdrag, int32_t offsetx, 
                                   int32_t offsety, uint32_t time, int32_t type);

/* 垂直布局平铺窗口调整 */
void resize_tile_master_vertical(Client *rec, bool isdrag, int32_t offsetx, 
                                 int32_t offsety, uint32_t time, int32_t type);

/* 滚动布局平铺窗口调整 */
void resize_tile_scroller(Client *rec, bool isdrag, int32_t offsetx, 
                          int32_t offsety, uint32_t time, bool isvertical);

/* 通用平铺窗口调整入口函数 */
void resize_tile_client(Client *rec, bool isdrag, int32_t offsetx, 
                        int32_t offsety, uint32_t time);

/* 重置显示器上所有平铺窗口的比例 */
void reset_size_per_mon(Monitor *m, int32_t tile_cilent_num,
                        double total_left_stack_hight_percent,
                        double total_right_stack_hight_percent,
                        double total_stack_hight_percent,
                        double total_master_inner_percent, int32_t master_num,
                        int32_t stack_num);

/* 浮动窗口调整大小 */
void resize_floating_window(Client *rec);

/* 排列显示器上的所有窗口 */
void arrange(Monitor *m, bool want_animation, bool from_view);

#endif // TILE_RESIZE_H