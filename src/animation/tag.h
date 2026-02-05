#ifndef MANGO_ANIMATION_TAG_H
#define MANGO_ANIMATION_TAG_H

// 标签切换动画函数
void set_tagin_animation(Monitor *m, Client *c);
void set_arrange_visible(Monitor *m, Client *c, bool want_animation);
void set_tagout_animation(Monitor *m, Client *c);
void set_arrange_hidden(Monitor *m, Client *c, bool want_animation);

#endif // MANGO_ANIMATION_TAG_H