#include "../type.h"
#include "layout.h"
#include "../animation/tag.h"

extern Server server;
extern Layout layouts[];
extern Layout overviewlayout;

void set_size_per(Monitor *m, Client *c) {
	Client *fc = NULL;
	bool found = false;

	if (!m || !c)
		return;

	wl_list_for_each(fc, &server.clients, link) {
		if (VISIBLEON(fc, m) && ISTILED(fc) && fc != c) {
			c->master_mfact_per = fc->master_mfact_per;
			c->master_inner_per = fc->master_inner_per;
			c->stack_inner_per = fc->stack_inner_per;
			found = true;
			break;
		}
	}

	if (!found) {
		c->master_mfact_per = m->pertag->mfacts[m->pertag->curtag];
		c->master_inner_per = 1.0f;
		c->stack_inner_per = 1.0f;
	}
}

void resize_tile_master_horizontal(Client *rec, bool isdrag, int32_t offsetx,
								   int32_t offsety, uint32_t time,
								   int32_t type) {
	Client *tc = NULL;
	float delta_x, delta_y;
	Client *next = NULL;
	Client *prev = NULL;
	Client *nextnext = NULL;
	Client *prevprev = NULL;
	struct wl_list *node;
	bool begin_find_nextnext = false;
	bool begin_find_prevprev = false;

	// 从当前节点的下一个开始遍历
	for (node = rec->link.next; node != &server.clients;
		 node = node->next) {
		tc = wl_container_of(node, tc, link);
		if (begin_find_nextnext && VISIBLEON(tc, rec->mon) &&
			ISTILED(tc)) {
			nextnext = tc;
			break;
		}

		if (!begin_find_nextnext && VISIBLEON(tc, rec->mon) &&
			ISTILED(tc)) { // 根据你的实际字段名调整
			next = tc;
			begin_find_nextnext = true;
			continue;
		}
	}

	// 从当前节点的上一个开始遍历
	for (node = rec->link.prev; node != &server.clients;
		 node = node->prev) {
		tc = wl_container_of(node, tc, link);

		if (begin_find_prevprev && VISIBLEON(tc, rec->mon) &&
			ISTILED(tc)) {
			prevprev = tc;
			break;
		}

		if (!begin_find_prevprev && VISIBLEON(tc, rec->mon) &&
			ISTILED(tc)) { // 根据你的实际字段名调整
			prev = tc;
			begin_find_prevprev = true;
			continue;
		}
	}

	if (!server.start_drag_window && isdrag) {
		server.drag_begin_cursorx = server.cursor->x;
		server.drag_begin_cursory = server.cursor->y;
		server.start_drag_window = true;
		// 记录初始状态
		rec->old_master_mfact_per = rec->master_mfact_per;
		rec->old_master_inner_per = rec->master_inner_per;
		rec->old_stack_inner_per = rec->stack_inner_per;
		rec->cursor_in_upper_half =
			server.cursor->y <
			rec->geom.y + rec->geom.height / 2;
		rec->cursor_in_left_half =
			server.cursor->x <
			rec->geom.x + rec->geom.width / 2;
		// 记录初始几何信息
		rec->drag_begin_geom = rec->geom;
	} else {
		// 计算相对于屏幕尺寸的比例变化
		if (isdrag) {

			offsetx = server.cursor->x - server.drag_begin_cursorx;
			offsety = server.cursor->y - server.drag_begin_cursory;
		} else {
			rec->old_master_mfact_per = rec->master_mfact_per;
			rec->old_master_inner_per = rec->master_inner_per;
			rec->old_stack_inner_per = rec->stack_inner_per;
			rec->drag_begin_geom = rec->geom;
			rec->cursor_in_upper_half = true;
			rec->cursor_in_left_half = false;
		}

		if (rec->ismaster) {
			delta_x = (float)(offsetx) * (rec->old_master_mfact_per) /
					  rec->drag_begin_geom.width;
			delta_y = (float)(offsety) * (rec->old_master_inner_per) /
					  rec->drag_begin_geom.height;
		} else {
			delta_x = (float)(offsetx) *
					  (1 - rec->old_master_mfact_per) /
					  rec->drag_begin_geom.width;
			delta_y = (float)(offsety) * (rec->old_stack_inner_per) /
					  rec->drag_begin_geom.height;
		}
		bool moving_up;
		bool moving_down;

		if (!isdrag) {
			moving_up = offsety < 0 ? true : false;
			moving_down = offsety > 0 ? true : false;
		} else {
			moving_up = server.cursor->y < server.drag_begin_cursory;
			moving_down = server.cursor->y > server.drag_begin_cursory;
		}

		if (rec->ismaster && !prev) {
			if (moving_up) {
				delta_y = -fabsf(delta_y);
			} else {
				delta_y = fabsf(delta_y);
			}
		} else if (rec->ismaster && next && !next->ismaster) {
			if (moving_up) {
				delta_y = fabsf(delta_y);
			} else {
				delta_y = -fabsf(delta_y);
			}
		} else if (!rec->ismaster && prev && prev->ismaster) {
			if (moving_up) {
				delta_y = -fabsf(delta_y);
			} else {
				delta_y = fabsf(delta_y);
			}
		} else if (!rec->ismaster && !next) {
			if (moving_up) {
				delta_y = fabsf(delta_y);
			} else {
				delta_y = -fabsf(delta_y);
			}
		} else if (type == CENTER_TILE && !rec->ismaster &&
				   !nextnext) {
			if (moving_up) {
				delta_y = fabsf(delta_y);
			} else {
				delta_y = -fabsf(delta_y);
			}
		} else if (type == CENTER_TILE && !rec->ismaster && prevprev &&
				   prevprev->ismaster) {
			if (moving_up) {
				delta_y = -fabsf(delta_y);
			} else {
				delta_y = fabsf(delta_y);
			}
		} else if ((rec->cursor_in_upper_half && moving_up) ||
				   (!rec->cursor_in_upper_half && moving_down)) {
			// 光标在窗口上方且向上移动，或在窗口下方且向下移动 → 增加高度
			delta_y = fabsf(delta_y);
			delta_y = delta_y * 2;
		} else {
			// 其他情况 → 减小高度
			delta_y = -fabsf(delta_y);
			delta_y = delta_y * 2;
		}

		if (!rec->ismaster && rec->isleftstack &&
			type == CENTER_TILE) {
			delta_x = delta_x * -1.0f;
		}

		if (rec->ismaster && type == CENTER_TILE &&
			rec->cursor_in_left_half) {
			delta_x = delta_x * -1.0f;
		}

		if (rec->ismaster && type == CENTER_TILE) {
			delta_x = delta_x * 2;
		}

		if (type == RIGHT_TILE) {
			delta_x = delta_x * -1.0f;
		}

		// 直接设置新的比例，基于初始值 + 变化量
		float new_master_mfact_per =
			rec->old_master_mfact_per + delta_x;
		float new_master_inner_per =
			rec->old_master_inner_per + delta_y;
		float new_stack_inner_per = rec->old_stack_inner_per + delta_y;

		// 应用限制，确保比例在合理范围内
		new_master_mfact_per = fmaxf(0.1f, fminf(0.9f, new_master_mfact_per));
		new_master_inner_per = fmaxf(0.1f, fminf(0.9f, new_master_inner_per));
		new_stack_inner_per = fmaxf(0.1f, fminf(0.9f, new_stack_inner_per));

		// 应用到所有平铺窗口
		wl_list_for_each(tc, &server.clients, link) {
			if (VISIBLEON(tc, rec->mon) && ISTILED(tc)) {
				tc->master_mfact_per = new_master_mfact_per;
			}
		}

		rec->master_inner_per = new_master_inner_per;
		rec->stack_inner_per = new_stack_inner_per;

		if (!isdrag) {
			arrange(rec->mon, false, false);
			return;
		}

		if (server.last_apply_drag_time == 0 ||
			time - server.last_apply_drag_time > drag_tile_refresh_interval) {
			arrange(rec->mon, false, false);
			server.last_apply_drag_time = time;
		}
	}
}

void resize_tile_master_vertical(Client *rec, bool isdrag, int32_t offsetx,
								 int32_t offsety, uint32_t time, int32_t type) {
	Client *tc = NULL;
	float delta_x, delta_y;
	Client *next = NULL;
	Client *prev = NULL;
	struct wl_list *node;

	// 从当前节点的下一个开始遍历
	for (node = rec->link.next; node != &server.clients;
		 node = node->next) {
		tc = wl_container_of(node, tc, link);

		if (VISIBLEON(tc, rec->mon) &&
			ISTILED(tc)) { // 根据你的实际字段名调整
			next = tc;
			break;
		}
	}

	// 从当前节点的上一个开始遍历
	for (node = rec->link.prev; node != &server.clients;
		 node = node->prev) {
		tc = wl_container_of(node, tc, link);

		if (VISIBLEON(tc, rec->mon) &&
			ISTILED(tc)) { // 根据你的实际字段名调整
			prev = tc;
			break;
		}
	}

	if (!server.start_drag_window && isdrag) {
		server.drag_begin_cursorx = server.cursor->x;
		server.drag_begin_cursory = server.cursor->y;
		server.start_drag_window = true;

		// 记录初始状态
		rec->old_master_mfact_per = rec->master_mfact_per;
		rec->old_master_inner_per = rec->master_inner_per;
		rec->old_stack_inner_per = rec->stack_inner_per;
		rec->cursor_in_upper_half =
			server.cursor->y <
			rec->geom.y + rec->geom.height / 2;
		rec->cursor_in_left_half =
			server.cursor->x <
			rec->geom.x + rec->geom.width / 2;
		// 记录初始几何信息
		rec->drag_begin_geom = rec->geom;
	} else {
		// 计算相对于屏幕尺寸的比例变化
		// 计算相对于屏幕尺寸的比例变化
		if (isdrag) {

			offsetx = server.cursor->x - server.drag_begin_cursorx;
			offsety = server.cursor->y - server.drag_begin_cursory;
		} else {
			rec->old_master_mfact_per = rec->master_mfact_per;
			rec->old_master_inner_per = rec->master_inner_per;
			rec->old_stack_inner_per = rec->stack_inner_per;
			rec->drag_begin_geom = rec->geom;
			rec->cursor_in_upper_half = true;
			rec->cursor_in_left_half = false;
		}

		if (rec->ismaster) {
			// 垂直版本：左右移动调整高度比例，上下移动调整宽度比例
			delta_x = (float)(offsetx) * (rec->old_master_inner_per) /
					  rec->drag_begin_geom.width;
			delta_y = (float)(offsety) * (rec->old_master_mfact_per) /
					  rec->drag_begin_geom.height;
		} else {
			delta_x = (float)(offsetx) * (rec->old_stack_inner_per) /
					  rec->drag_begin_geom.width;
			delta_y = (float)(offsety) *
					  (1 - rec->old_master_mfact_per) /
					  rec->drag_begin_geom.height;
		}

		bool moving_left;
		bool moving_right;

		if (!isdrag) {
			moving_left = offsetx < 0 ? true : false;
			moving_right = offsetx > 0 ? true : false;
		} else {
			moving_left = server.cursor->x < server.drag_begin_cursorx;
			moving_right = server.cursor->x > server.drag_begin_cursorx;
		}

		// 调整主区域和栈区域的高度比例（垂直分割）
		if (rec->ismaster && !prev) {
			if (moving_left) {
				delta_x = -fabsf(delta_x); // 向上移动减少主区域高度
			} else {
				delta_x = fabsf(delta_x); // 向下移动增加主区域高度
			}
		} else if (rec->ismaster && next && !next->ismaster) {
			if (moving_left) {
				delta_x = fabsf(delta_x); // 向上移动增加主区域高度
			} else {
				delta_x = -fabsf(delta_x); // 向下移动减少主区域高度
			}
		} else if (!rec->ismaster && prev && prev->ismaster) {
			if (moving_left) {
				delta_x = -fabsf(delta_x); // 向上移动减少栈区域高度
			} else {
				delta_x = fabsf(delta_x); // 向下移动增加栈区域高度
			}
		} else if (!rec->ismaster && !next) {
			if (moving_left) {
				delta_x = fabsf(delta_x); // 向上移动增加栈区域高度
			} else {
				delta_x = -fabsf(delta_x); // 向下移动减少栈区域高度
			}
		} else if ((rec->cursor_in_left_half && moving_left) ||
				   (!rec->cursor_in_left_half && moving_right)) {
			// 光标在窗口左侧且向左移动，或在窗口右侧且向右移动 → 增加宽度
			delta_x = fabsf(delta_x);
			delta_x = delta_x * 2;
		} else {
			// 其他情况 → 减小宽度
			delta_x = -fabsf(delta_x);
			delta_x = delta_x * 2;
		}

		// 直接设置新的比例，基于初始值 + 变化量
		float new_master_mfact_per = rec->old_master_mfact_per +
									 delta_y; // 垂直：delta_y调整主区域高度
		float new_master_inner_per = rec->old_master_inner_per +
									 delta_x; // 垂直：delta_x调整主区域内部宽度
		float new_stack_inner_per = rec->old_stack_inner_per +
									delta_x; // 垂直：delta_x调整栈区域内部宽度

		// 应用限制，确保比例在合理范围内
		new_master_mfact_per = fmaxf(0.1f, fminf(0.9f, new_master_mfact_per));
		new_master_inner_per = fmaxf(0.1f, fminf(0.9f, new_master_inner_per));
		new_stack_inner_per = fmaxf(0.1f, fminf(0.9f, new_stack_inner_per));

		// 应用到所有平铺窗口
		wl_list_for_each(tc, &server.clients, link) {
			if (VISIBLEON(tc, rec->mon) && ISTILED(tc)) {
				tc->master_mfact_per = new_master_mfact_per;
			}
		}

		rec->master_inner_per = new_master_inner_per;
		rec->stack_inner_per = new_stack_inner_per;

		if (!isdrag) {
			arrange(rec->mon, false, false);
			return;
		}

		if (server.last_apply_drag_time == 0 ||
			time - server.last_apply_drag_time > drag_tile_refresh_interval) {
			arrange(rec->mon, false, false);
			server.last_apply_drag_time = time;
		}
	}
}

void resize_tile_scroller(Client *rec, bool isdrag, int32_t offsetx,
						  int32_t offsety, uint32_t time, bool isvertical) {
	float delta_x, delta_y;
	float new_scroller_proportion;
	float new_stack_proportion;
	Client *stack_head = get_scroll_stack_head(rec);

	if (rec && rec->mon->visible_tiling_clients == 1 &&
		!scroller_ignore_proportion_single)
		return;

	if (!server.start_drag_window && isdrag) {
		server.drag_begin_cursorx = server.cursor->x;
		server.drag_begin_cursory = server.cursor->y;
		server.start_drag_window = true;

		// 记录初始状态
		stack_head->old_scroller_pproportion = stack_head->scroller_proportion;
		rec->old_stack_proportion = rec->stack_proportion;

		rec->cursor_in_left_half =
			server.cursor->x <
			rec->geom.x + rec->geom.width / 2;
		rec->cursor_in_upper_half =
			server.cursor->y <
			rec->geom.y + rec->geom.height / 2;
		// 记录初始几何信息
		rec->drag_begin_geom = rec->geom;
	} else {
		// 计算相对于屏幕尺寸的比例变化
		// 计算相对于屏幕尺寸的比例变化
		if (isdrag) {

			offsetx = server.cursor->x - server.drag_begin_cursorx;
			offsety = server.cursor->y - server.drag_begin_cursory;
		} else {
			rec->old_master_mfact_per = rec->master_mfact_per;
			rec->old_master_inner_per = rec->master_inner_per;
			rec->old_stack_inner_per = rec->stack_inner_per;
			rec->drag_begin_geom = rec->geom;
			stack_head->old_scroller_pproportion =
				stack_head->scroller_proportion;
			rec->old_stack_proportion = rec->stack_proportion;
			rec->cursor_in_upper_half = false;
			rec->cursor_in_left_half = false;
		}

		if (isvertical) {
			delta_y = (float)(offsety) *
					  (stack_head->old_scroller_pproportion) /
					  rec->drag_begin_geom.height;
			delta_x = (float)(offsetx) * (rec->old_stack_proportion) /
					  rec->drag_begin_geom.width;
		} else {
			delta_x = (float)(offsetx) *
					  (stack_head->old_scroller_pproportion) /
					  rec->drag_begin_geom.width;
			delta_y = (float)(offsety) * (rec->old_stack_proportion) /
					  rec->drag_begin_geom.height;
		}

		bool moving_up;
		bool moving_down;
		bool moving_left;
		bool moving_right;

		if (!isdrag) {
			moving_up = offsety < 0 ? true : false;
			moving_down = offsety > 0 ? true : false;
			moving_left = offsetx < 0 ? true : false;
			moving_right = offsetx > 0 ? true : false;
		} else {
			moving_up = server.cursor->y < server.drag_begin_cursory;
			moving_down = server.cursor->y > server.drag_begin_cursory;
			moving_left = server.cursor->x < server.drag_begin_cursorx;
			moving_right = server.cursor->x > server.drag_begin_cursorx;
		}

		if ((rec->cursor_in_upper_half && moving_up) ||
			(!rec->cursor_in_upper_half && moving_down)) {
			// 光标在窗口上方且向上移动，或在窗口下方且向下移动 → 增加高度
			delta_y = fabsf(delta_y);
		} else {
			// 其他情况 → 减小高度
			delta_y = -fabsf(delta_y);
		}

		if ((rec->cursor_in_left_half && moving_left) ||
			(!rec->cursor_in_left_half && moving_right)) {
			delta_x = fabsf(delta_x);
		} else {
			delta_x = -fabsf(delta_x);
		}

		if (isvertical) {
			if (!rec->next_in_stack && rec->prev_in_stack &&
				!isdrag) {
				delta_x = delta_x * -1.0f;
			}
			if (!rec->next_in_stack && rec->prev_in_stack &&
				isdrag) {
				if (moving_right) {
					delta_x = -fabsf(delta_x);
				} else {
					delta_x = fabsf(delta_x);
				}
			}
			if (!rec->prev_in_stack && rec->next_in_stack &&
				isdrag) {
				if (moving_left) {
					delta_x = -fabsf(delta_x);
				} else {
					delta_x = fabsf(delta_x);
				}
			}

			if (isdrag) {
				if (moving_up) {
					delta_y = -fabsf(delta_y);
				} else {
					delta_y = fabsf(delta_y);
				}
			}

		} else {
			if (!rec->next_in_stack && rec->prev_in_stack &&
				!isdrag) {
				delta_y = delta_y * -1.0f;
			}
			if (!rec->next_in_stack && rec->prev_in_stack &&
				isdrag) {
				if (moving_down) {
					delta_y = -fabsf(delta_y);
				} else {
					delta_y = fabsf(delta_y);
				}
			}
			if (!rec->prev_in_stack && rec->next_in_stack &&
				isdrag) {
				if (moving_up) {
					delta_y = -fabsf(delta_y);
				} else {
					delta_y = fabsf(delta_y);
				}
			}

			if (isdrag) {
				if (moving_left) {
					delta_x = -fabsf(delta_x);
				} else {
					delta_x = fabsf(delta_x);
				}
			}
		}

		// 直接设置新的比例，基于初始值 + 变化量
		if (isvertical) {
			new_scroller_proportion =
				stack_head->old_scroller_pproportion + delta_y;
			new_stack_proportion = rec->old_stack_proportion + delta_x;

		} else {
			new_scroller_proportion =
				stack_head->old_scroller_pproportion + delta_x;
			new_stack_proportion = rec->old_stack_proportion + delta_y;
		}

		// 应用限制，确保比例在合理范围内
		new_scroller_proportion =
			fmaxf(0.1f, fminf(1.0f, new_scroller_proportion));
		new_stack_proportion = fmaxf(0.1f, fminf(1.0f, new_stack_proportion));

		rec->stack_proportion = new_stack_proportion;

		stack_head->scroller_proportion = new_scroller_proportion;

		if (!isdrag) {
			arrange(rec->mon, false, false);
			return;
		}

		if (server.last_apply_drag_time == 0 ||
			time - server.last_apply_drag_time > drag_tile_refresh_interval) {
			arrange(rec->mon, false, false);
			server.last_apply_drag_time = time;
		}
	}
}

void resize_tile_client(Client *rec, bool isdrag, int32_t offsetx,
						int32_t offsety, uint32_t time) {

	if (!rec || rec->isfullscreen ||
		rec->ismaximizescreen)
		return;

	if (rec->mon->isoverview)
		return;

	const Layout *current_layout =
		rec->mon->pertag->ltidxs[rec->mon->pertag->curtag];
	if (current_layout->id == TILE || current_layout->id == DECK ||
		current_layout->id == CENTER_TILE || current_layout->id == RIGHT_TILE ||
		(current_layout->id == TGMIX &&
		 rec->mon->visible_tiling_clients <= 3)

	) {
		resize_tile_master_horizontal(rec, isdrag, offsetx, offsety,
									  time, current_layout->id);
	} else if (current_layout->id == VERTICAL_TILE ||
			   current_layout->id == VERTICAL_DECK) {
		resize_tile_master_vertical(rec, isdrag, offsetx, offsety,
									time, current_layout->id);
	} else if (current_layout->id == SCROLLER) {
		resize_tile_scroller(rec, isdrag, offsetx, offsety, time,
							 false);
	} else if (current_layout->id == VERTICAL_SCROLLER) {
		resize_tile_scroller(rec, isdrag, offsetx, offsety, time,
							 true);
	}
}

void reset_size_per_mon(Monitor *m, int32_t tile_cilent_num,
						double total_left_stack_hight_percent,
						double total_right_stack_hight_percent,
						double total_stack_hight_percent,
						double total_master_inner_percent, int32_t master_num,
						int32_t stack_num) {
	Client *c = NULL;
	int32_t i = 0;
	uint32_t stack_index = 0;
	uint32_t nmasters = m->pertag->nmasters[m->pertag->curtag];

	if (m->pertag->ltidxs[m->pertag->curtag]->id != CENTER_TILE) {

		wl_list_for_each(c, &server.clients, link) {
			if (VISIBLEON(c, m) && ISTILED(c)) {
				if (total_master_inner_percent > 0.0 && i < nmasters) {
					c->ismaster = true;
					c->stack_inner_per = stack_num ? 1.0f / stack_num : 1.0f;
					c->master_inner_per =
						c->master_inner_per / total_master_inner_percent;
				} else {
					c->ismaster = false;
					c->master_inner_per =
						master_num > 0 ? 1.0f / master_num : 1.0f;
					c->stack_inner_per =
						total_stack_hight_percent
							? c->stack_inner_per / total_stack_hight_percent
							: 1.0f;
				}
				i++;
			}
		}
	} else {
		wl_list_for_each(c, &server.clients, link) {
			if (VISIBLEON(c, m) && ISTILED(c)) {
				if (total_master_inner_percent > 0.0 && i < nmasters) {
					c->ismaster = true;
					if ((stack_index % 2) ^ (tile_cilent_num % 2 == 0)) {
						c->stack_inner_per =
							stack_num > 1 ? 1.0f / ((stack_num - 1) / 2) : 1.0f;

					} else {
						c->stack_inner_per =
							stack_num > 1 ? 2.0f / stack_num : 1.0f;
					}

					c->master_inner_per =
						c->master_inner_per / total_master_inner_percent;
				} else {
					stack_index = i - nmasters;

					c->ismaster = false;
					c->master_inner_per =
						master_num > 0 ? 1.0f / master_num : 1.0f;
					if ((stack_index % 2) ^ (tile_cilent_num % 2 == 0)) {
						c->stack_inner_per =
							total_right_stack_hight_percent
								? c->stack_inner_per /
									  total_right_stack_hight_percent
								: 1.0f;
					} else {
						c->stack_inner_per =
							total_left_stack_hight_percent
								? c->stack_inner_per /
									  total_left_stack_hight_percent
								: 1.0f;
					}
				}
				i++;
			}
		}
	}
}

void resize_floating_window(Client *rec) {
	int cdx = (int)round(server.cursor->x) - server.grabcx;
	int cdy = (int)round(server.cursor->y) - server.grabcy;

	cdx = !(server.rzcorner & 1) &&
				  rec->geom.width - 2 * (int)rec->bw - cdx < 1
			  ? 0
			  : cdx;
	cdy =
		!(server.rzcorner & 2) &&
				rec->geom.height - 2 * (int)rec->bw - cdy < 1
			? 0
			: cdy;

	const struct wlr_box box = {
		.x = rec->geom.x + (server.rzcorner & 1 ? 0 : cdx),
		.y = rec->geom.y + (server.rzcorner & 2 ? 0 : cdy),
		.width = rec->geom.width + (server.rzcorner & 1 ? cdx : -cdx),
		.height =
			rec->geom.height + (server.rzcorner & 2 ? cdy : -cdy)};

	rec->float_geom = box;

	resize(rec, box, 1);
	server.grabcx += cdx;
	server.grabcy += cdy;
}

void // 17
arrange(Monitor *m, bool want_animation, bool from_view) {
	Client *c = NULL;
	double total_stack_inner_percent = 0;
	double total_master_inner_percent = 0;
	double total_right_stack_hight_percent = 0;
	double total_left_stack_hight_percent = 0;
	int32_t i = 0;
	int32_t nmasters = 0;
	int32_t stack_index = 0;
	int32_t master_num = 0;
	int32_t stack_num = 0;

	if (!m)
		return;

	if (!m->wlr_output->enabled)
		return;
	m->visible_clients = 0;
	m->visible_tiling_clients = 0;
	m->visible_scroll_tiling_clients = 0;

	wl_list_for_each(c, &server.clients, link) {

		if (!client_only_in_one_tag(c) || c->isglobal || c->isunglobal) {
			exit_scroller_stack(c);
		}

		if (from_view && (c->isglobal || c->isunglobal)) {
			set_size_per(m, c);
		}

		if (c->mon == m && (c->isglobal || c->isunglobal)) {
			c->tags = m->tagset[m->seltags];
		}

		if (from_view && m->sel == NULL && c->isglobal && VISIBLEON(c, m)) {
			focusclient(c, 1);
		}

		if (VISIBLEON(c, m)) {
			if (from_view && !client_only_in_one_tag(c)) {
				set_size_per(m, c);
			}

			if (!c->isunglobal)
				m->visible_clients++;

			if (ISTILED(c)) {
				m->visible_tiling_clients++;
			}

			if (ISSCROLLTILED(c) && !c->prev_in_stack) {
				m->visible_scroll_tiling_clients++;
			}
		}
	}

	nmasters = m->pertag->nmasters[m->pertag->curtag];

	wl_list_for_each(c, &server.clients, link) {
		if (c->iskilling)
			continue;

		if (c->mon == m) {
			if (VISIBLEON(c, m)) {
				if (ISTILED(c)) {

					if (i < nmasters) {
						master_num++;
						total_master_inner_percent += c->master_inner_per;
					} else {
						stack_num++;
						total_stack_inner_percent += c->stack_inner_per;
						stack_index = i - nmasters;
						if ((stack_index % 2) ^
							(m->visible_tiling_clients % 2 == 0)) {
							c->isleftstack = false;
							total_right_stack_hight_percent +=
								c->stack_inner_per;
						} else {
							c->isleftstack = true;
							total_left_stack_hight_percent +=
								c->stack_inner_per;
						}
					}

					i++;
				}

				set_arrange_visible(m, c, want_animation);
			} else {
				set_arrange_hidden(m, c, want_animation);
			}
		}

		if (c->mon == m && c->ismaximizescreen && !c->animation.tagouted &&
			!c->animation.tagouting && VISIBLEON(c, m)) {
			reset_maximizescreen_size(c);
		}
	}

	reset_size_per_mon(
		m, m->visible_tiling_clients, total_left_stack_hight_percent,
		total_right_stack_hight_percent, total_stack_inner_percent,
		total_master_inner_percent, master_num, stack_num);

	if (m->isoverview) {
		overviewlayout.arrange(m);
	} else {
		m->pertag->ltidxs[m->pertag->curtag]->arrange(m);
	}

	if (!server.start_drag_window) {
		motionnotify(0, NULL, 0, 0, 0, 0);
		checkidleinhibitor(NULL);
	}

	printstatus();
}
