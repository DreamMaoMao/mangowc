void set_size_per(Monitor *m, Client *c) {
	Client *fc = NULL;
	bool found = false;
	wl_list_for_each(fc, &clients, link) {
		if (VISIBLEON(fc, m) && ISTILED(fc) && fc != c) {
			c->master_mfact_per = fc->master_mfact_per;
			c->master_inner_per = fc->master_inner_per;
			c->stack_innder_per = fc->stack_innder_per;
			found = true;
			break;
		}
	}

	if (!found) {
		c->master_mfact_per = 0.5f;
		c->master_inner_per = 1.0f;
		c->stack_innder_per = 1.0f;
	}
}

void resize_tile_master_horizontal(Client *grabc, bool isdrag, int offsetx,
								   int offsety, unsigned int time, int type) {
	Client *tc = NULL;
	float delta_x, delta_y;
	Client *next = NULL;
	Client *prev = NULL;
	Client *nextnext = NULL;
	Client *prevprev = NULL;
	double refresh_interval = 1000000.0 / grabc->mon->wlr_output->refresh;
	struct wl_list *node;
	bool begin_find_nextnext = false;
	bool begin_find_prevprev = false;

	// 从当前节点的下一个开始遍历
	for (node = grabc->link.next; node != &clients; node = node->next) {
		tc = wl_container_of(node, tc, link);
		if (begin_find_nextnext && VISIBLEON(tc, grabc->mon) && ISTILED(tc)) {
			nextnext = tc;
			break;
		}

		if (!begin_find_nextnext && VISIBLEON(tc, grabc->mon) &&
			ISTILED(tc)) { // 根据你的实际字段名调整
			next = tc;
			begin_find_nextnext = true;
			continue;
		}
	}

	// 从当前节点的上一个开始遍历
	for (node = grabc->link.prev; node != &clients; node = node->prev) {
		tc = wl_container_of(node, tc, link);

		if (begin_find_prevprev && VISIBLEON(tc, grabc->mon) && ISTILED(tc)) {
			prevprev = tc;
			break;
		}

		if (!begin_find_prevprev && VISIBLEON(tc, grabc->mon) &&
			ISTILED(tc)) { // 根据你的实际字段名调整
			prev = tc;
			begin_find_prevprev = true;
			continue;
		}
	}

	if (!start_drag_window && isdrag) {
		drag_begin_cursorx = cursor->x;
		drag_begin_cursory = cursor->y;
		start_drag_window = true;
		// 记录初始状态
		grabc->old_master_mfact_per = grabc->master_mfact_per;
		grabc->old_master_inner_per = grabc->master_inner_per;
		grabc->old_stack_innder_per = grabc->stack_innder_per;
		grabc->cursor_in_upper_half =
			cursor->y < grabc->geom.y + grabc->geom.height / 2;
		grabc->cursor_in_left_half =
			cursor->x < grabc->geom.x + grabc->geom.width / 2;
		// 记录初始几何信息
		grabc->drag_begin_geom = grabc->geom;
	} else {
		// 计算相对于屏幕尺寸的比例变化
		if (isdrag) {

			offsetx = cursor->x - drag_begin_cursorx;
			offsety = cursor->y - drag_begin_cursory;
		} else {
			grabc->old_master_mfact_per = grabc->master_mfact_per;
			grabc->old_master_inner_per = grabc->master_inner_per;
			grabc->old_stack_innder_per = grabc->stack_innder_per;
			grabc->drag_begin_geom = grabc->geom;
			grabc->cursor_in_upper_half = false;
			grabc->cursor_in_left_half = false;
		}

		if (grabc->ismaster) {
			delta_x = (float)(offsetx) * (grabc->old_master_mfact_per) /
					  grabc->drag_begin_geom.width;
			delta_y = (float)(offsety) * (grabc->old_master_inner_per) /
					  grabc->drag_begin_geom.height;
		} else {
			delta_x = (float)(offsetx) * (1 - grabc->old_master_mfact_per) /
					  grabc->drag_begin_geom.width;
			delta_y = (float)(offsety) * (grabc->old_stack_innder_per) /
					  grabc->drag_begin_geom.height;
		}
		bool moving_up;
		bool moving_down;

		if (!isdrag) {
			moving_up = offsety < 0 ? true : false;
			moving_down = offsety > 0 ? true : false;
		} else {
			moving_up = cursor->y < drag_begin_cursory;
			moving_down = cursor->y > drag_begin_cursory;
		}

		if (grabc->ismaster && !prev) {
			if (moving_up) {
				delta_y = -fabsf(delta_y);
			} else {
				delta_y = fabsf(delta_y);
			}
		} else if (grabc->ismaster && next && !next->ismaster) {
			if (moving_up) {
				delta_y = fabsf(delta_y);
			} else {
				delta_y = -fabsf(delta_y);
			}
		} else if (!grabc->ismaster && prev && prev->ismaster) {
			if (moving_up) {
				delta_y = -fabsf(delta_y);
			} else {
				delta_y = fabsf(delta_y);
			}
		} else if (!grabc->ismaster && !next) {
			if (moving_up) {
				delta_y = fabsf(delta_y);
			} else {
				delta_y = -fabsf(delta_y);
			}
		} else if (type == CENTER_TILE && !grabc->ismaster && !nextnext) {
			if (moving_up) {
				delta_y = fabsf(delta_y);
			} else {
				delta_y = -fabsf(delta_y);
			}
		} else if (type == CENTER_TILE && !grabc->ismaster && prevprev &&
				   prevprev->ismaster) {
			if (moving_up) {
				delta_y = -fabsf(delta_y);
			} else {
				delta_y = fabsf(delta_y);
			}
		} else if ((grabc->cursor_in_upper_half && moving_up) ||
				   (!grabc->cursor_in_upper_half && moving_down)) {
			// 光标在窗口上方且向上移动，或在窗口下方且向下移动 → 增加高度
			delta_y = fabsf(delta_y);
			delta_y = delta_y * 2;
		} else {
			// 其他情况 → 减小高度
			delta_y = -fabsf(delta_y);
			delta_y = delta_y * 2;
		}

		if (!grabc->ismaster && grabc->isleftstack && type == CENTER_TILE) {
			delta_x = delta_x * -1.0f;
		}

		if (grabc->ismaster && type == CENTER_TILE &&
			grabc->cursor_in_left_half) {
			delta_x = delta_x * -1.0f;
		}

		if (grabc->ismaster && type == CENTER_TILE) {
			delta_x = delta_x * 2;
		}

		// 直接设置新的比例，基于初始值 + 变化量
		float new_master_mfact_per = grabc->old_master_mfact_per + delta_x;
		float new_master_inner_per = grabc->old_master_inner_per + delta_y;
		float new_stack_innder_per = grabc->old_stack_innder_per + delta_y;

		// 应用限制，确保比例在合理范围内
		new_master_mfact_per = fmaxf(0.1f, fminf(0.9f, new_master_mfact_per));
		new_master_inner_per = fmaxf(0.1f, fminf(0.9f, new_master_inner_per));
		new_stack_innder_per = fmaxf(0.1f, fminf(0.9f, new_stack_innder_per));

		// 应用到所有平铺窗口
		wl_list_for_each(tc, &clients, link) {
			if (VISIBLEON(tc, grabc->mon) && ISTILED(tc)) {
				tc->master_mfact_per = new_master_mfact_per;
			}
		}

		grabc->master_inner_per = new_master_inner_per;
		grabc->stack_innder_per = new_stack_innder_per;

		if (!isdrag) {
			arrange(grabc->mon, false);
			return;
		}

		if (last_apply_drap_time == 0 ||
			time - last_apply_drap_time > refresh_interval) {
			arrange(grabc->mon, false);
			last_apply_drap_time = time;
		}
	}
}

void resize_tile_master_vertical(Client *grabc, bool isdrag, int offsetx,
								 int offsety, unsigned int time, int type) {
	Client *tc = NULL;
	float delta_x, delta_y;
	Client *next = NULL;
	Client *prev = NULL;
	double refresh_interval = 1000000.0 / grabc->mon->wlr_output->refresh;
	struct wl_list *node;

	// 从当前节点的下一个开始遍历
	for (node = grabc->link.next; node != &clients; node = node->next) {
		tc = wl_container_of(node, tc, link);

		if (VISIBLEON(tc, grabc->mon) &&
			ISTILED(tc)) { // 根据你的实际字段名调整
			next = tc;
			break;
		}
	}

	// 从当前节点的上一个开始遍历
	for (node = grabc->link.prev; node != &clients; node = node->prev) {
		tc = wl_container_of(node, tc, link);

		if (VISIBLEON(tc, grabc->mon) &&
			ISTILED(tc)) { // 根据你的实际字段名调整
			prev = tc;
			break;
		}
	}

	if (!start_drag_window && isdrag) {
		drag_begin_cursorx = cursor->x;
		drag_begin_cursory = cursor->y;
		start_drag_window = true;

		// 记录初始状态
		grabc->old_master_mfact_per = grabc->master_mfact_per;
		grabc->old_master_inner_per = grabc->master_inner_per;
		grabc->old_stack_innder_per = grabc->stack_innder_per;
		grabc->cursor_in_upper_half =
			cursor->y < grabc->geom.y + grabc->geom.height / 2;
		grabc->cursor_in_left_half =
			cursor->x < grabc->geom.x + grabc->geom.width / 2;
		// 记录初始几何信息
		grabc->drag_begin_geom = grabc->geom;
	} else {
		// 计算相对于屏幕尺寸的比例变化
		// 计算相对于屏幕尺寸的比例变化
		if (isdrag) {

			offsetx = cursor->x - drag_begin_cursorx;
			offsety = cursor->y - drag_begin_cursory;
		} else {
			grabc->old_master_mfact_per = grabc->master_mfact_per;
			grabc->old_master_inner_per = grabc->master_inner_per;
			grabc->old_stack_innder_per = grabc->stack_innder_per;
			grabc->drag_begin_geom = grabc->geom;
			grabc->cursor_in_upper_half = false;
			grabc->cursor_in_left_half = false;
		}

		if (grabc->ismaster) {
			// 垂直版本：左右移动调整高度比例，上下移动调整宽度比例
			delta_x = (float)(offsetx) * (grabc->old_master_inner_per) /
					  grabc->drag_begin_geom.width;
			delta_y = (float)(offsety) * (grabc->old_master_mfact_per) /
					  grabc->drag_begin_geom.height;
		} else {
			delta_x = (float)(offsetx) * (grabc->old_stack_innder_per) /
					  grabc->drag_begin_geom.width;
			delta_y = (float)(offsety) * (1 - grabc->old_master_mfact_per) /
					  grabc->drag_begin_geom.height;
		}

		bool moving_left;
		bool moving_right;

		if (!isdrag) {
			moving_left = offsetx < 0 ? true : false;
			moving_right = offsetx > 0 ? true : false;
		} else {
			moving_left = cursor->x < drag_begin_cursorx;
			moving_right = cursor->x > drag_begin_cursorx;
		}

		// 调整主区域和栈区域的高度比例（垂直分割）
		if (grabc->ismaster && !prev) {
			if (moving_left) {
				delta_x = -fabsf(delta_x); // 向上移动减少主区域高度
			} else {
				delta_x = fabsf(delta_x); // 向下移动增加主区域高度
			}
		} else if (grabc->ismaster && next && !next->ismaster) {
			if (moving_left) {
				delta_x = fabsf(delta_x); // 向上移动增加主区域高度
			} else {
				delta_x = -fabsf(delta_x); // 向下移动减少主区域高度
			}
		} else if (!grabc->ismaster && prev && prev->ismaster) {
			if (moving_left) {
				delta_x = -fabsf(delta_x); // 向上移动减少栈区域高度
			} else {
				delta_x = fabsf(delta_x); // 向下移动增加栈区域高度
			}
		} else if (!grabc->ismaster && !next) {
			if (moving_left) {
				delta_x = fabsf(delta_x); // 向上移动增加栈区域高度
			} else {
				delta_x = -fabsf(delta_x); // 向下移动减少栈区域高度
			}
		} else if ((grabc->cursor_in_left_half && moving_left) ||
				   (!grabc->cursor_in_left_half && moving_right)) {
			// 光标在窗口左侧且向左移动，或在窗口右侧且向右移动 → 增加宽度
			delta_x = fabsf(delta_x);
			delta_x = delta_x * 2;
		} else {
			// 其他情况 → 减小宽度
			delta_x = -fabsf(delta_x);
			delta_x = delta_x * 2;
		}

		// 直接设置新的比例，基于初始值 + 变化量
		float new_master_mfact_per = grabc->old_master_mfact_per +
									 delta_y; // 垂直：delta_y调整主区域高度
		float new_master_inner_per = grabc->old_master_inner_per +
									 delta_x; // 垂直：delta_x调整主区域内部宽度
		float new_stack_innder_per = grabc->old_stack_innder_per +
									 delta_x; // 垂直：delta_x调整栈区域内部宽度

		// 应用限制，确保比例在合理范围内
		new_master_mfact_per = fmaxf(0.1f, fminf(0.9f, new_master_mfact_per));
		new_master_inner_per = fmaxf(0.1f, fminf(0.9f, new_master_inner_per));
		new_stack_innder_per = fmaxf(0.1f, fminf(0.9f, new_stack_innder_per));

		// 应用到所有平铺窗口
		wl_list_for_each(tc, &clients, link) {
			if (VISIBLEON(tc, grabc->mon) && ISTILED(tc)) {
				tc->master_mfact_per = new_master_mfact_per;
			}
		}

		grabc->master_inner_per = new_master_inner_per;
		grabc->stack_innder_per = new_stack_innder_per;

		if (!isdrag) {
			arrange(grabc->mon, false);
			return;
		}

		if (last_apply_drap_time == 0 ||
			time - last_apply_drap_time > refresh_interval) {
			arrange(grabc->mon, false);
			last_apply_drap_time = time;
		}
	}
}

void resize_tile_scroller(Client *grabc, bool isdrag, int offsetx, int offsety,
						  unsigned int time, bool isvertical) {
	float delta_x, delta_y;
	float new_scroller_proportion;
	double refresh_interval = 1000000.0 / grabc->mon->wlr_output->refresh;

	if (!start_drag_window && isdrag) {
		drag_begin_cursorx = cursor->x;
		drag_begin_cursory = cursor->y;
		start_drag_window = true;

		// 记录初始状态
		grabc->old_scroller_pproportion = grabc->scroller_proportion;

		grabc->cursor_in_left_half =
			cursor->x < grabc->geom.x + grabc->geom.width / 2;
		grabc->cursor_in_upper_half =
			cursor->y < grabc->geom.y + grabc->geom.height / 2;
		// 记录初始几何信息
		grabc->drag_begin_geom = grabc->geom;
	} else {
		// 计算相对于屏幕尺寸的比例变化
		// 计算相对于屏幕尺寸的比例变化
		if (isdrag) {

			offsetx = cursor->x - drag_begin_cursorx;
			offsety = cursor->y - drag_begin_cursory;
		} else {
			grabc->old_master_mfact_per = grabc->master_mfact_per;
			grabc->old_master_inner_per = grabc->master_inner_per;
			grabc->old_stack_innder_per = grabc->stack_innder_per;
			grabc->drag_begin_geom = grabc->geom;
			grabc->old_scroller_pproportion = grabc->scroller_proportion;
			grabc->cursor_in_upper_half = false;
			grabc->cursor_in_left_half = false;
		}

		delta_x = (float)(offsetx) * (grabc->old_scroller_pproportion) /
				  grabc->drag_begin_geom.width;
		delta_y = (float)(offsety) * (grabc->old_scroller_pproportion) /
				  grabc->drag_begin_geom.height;

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
			moving_up = cursor->y < drag_begin_cursory;
			moving_down = cursor->y > drag_begin_cursory;
			moving_left = cursor->x < drag_begin_cursorx;
			moving_right = cursor->x > drag_begin_cursorx;
		}

		if ((grabc->cursor_in_upper_half && moving_up) ||
			(!grabc->cursor_in_upper_half && moving_down)) {
			// 光标在窗口上方且向上移动，或在窗口下方且向下移动 → 增加高度
			delta_y = fabsf(delta_y);
		} else {
			// 其他情况 → 减小高度
			delta_y = -fabsf(delta_y);
		}

		if ((grabc->cursor_in_left_half && moving_left) ||
			(!grabc->cursor_in_left_half && moving_right)) {
			delta_x = fabsf(delta_x);
		} else {
			delta_x = -fabsf(delta_x);
		}

		// 直接设置新的比例，基于初始值 + 变化量
		if (isvertical) {
			new_scroller_proportion = grabc->old_scroller_pproportion + delta_y;
		} else {
			new_scroller_proportion = grabc->old_scroller_pproportion + delta_x;
		}

		// 应用限制，确保比例在合理范围内
		new_scroller_proportion =
			fmaxf(0.1f, fminf(0.9f, new_scroller_proportion));

		grabc->scroller_proportion = new_scroller_proportion;

		if (!isdrag) {
			arrange(grabc->mon, false);
			return;
		}

		if (last_apply_drap_time == 0 ||
			time - last_apply_drap_time > refresh_interval) {
			arrange(grabc->mon, false);
			last_apply_drap_time = time;
		}
	}
}

void resize_tile_client(Client *grabc, bool isdrag, int offsetx, int offsety,
						unsigned int time) {
	const Layout *current_layout =
		grabc->mon->pertag->ltidxs[grabc->mon->pertag->curtag];
	if (current_layout->id == TILE || current_layout->id == DECK ||
		current_layout->id == CENTER_TILE

	) {
		resize_tile_master_horizontal(grabc, isdrag, offsetx, offsety, time,
									  current_layout->id);
	} else if (current_layout->id == VERTICAL_TILE) {
		resize_tile_master_vertical(grabc, isdrag, offsetx, offsety, time,
									current_layout->id);
	} else if (current_layout->id == SCROLLER) {
		resize_tile_scroller(grabc, isdrag, offsetx, offsety, time, false);
	} else if (current_layout->id == VERTICAL_SCROLLER) {
		resize_tile_scroller(grabc, isdrag, offsetx, offsety, time, true);
	}
}

void reset_size_per_mon(Monitor *m, int tile_cilent_num,
						double total_left_stack_hight_percent,
						double total_right_stack_hight_percent,
						double total_stack_hight_percent,
						double total_master_inner_percent, int master_num,
						int stack_num) {
	Client *c = NULL;
	int i = 0;
	unsigned int stack_index;
	unsigned int nmasters = m->pertag->nmasters[m->pertag->curtag];

	if (m->pertag->ltidxs[m->pertag->curtag]->id != CENTER_TILE) {

		wl_list_for_each(c, &clients, link) {
			if (VISIBLEON(c, m) && ISTILED(c)) {

				if (total_master_inner_percent <= 0.0)
					return;
				if (i < m->pertag->nmasters[m->pertag->curtag]) {
					c->ismaster = true;
					c->stack_innder_per = stack_num ? 1.0f / stack_num : 1.0f;
					c->master_inner_per =
						c->master_inner_per / total_master_inner_percent;
				} else {
					c->ismaster = false;
					c->master_inner_per = 1.0f / master_num;
					c->stack_innder_per =
						total_stack_hight_percent
							? c->stack_innder_per / total_stack_hight_percent
							: 1.0f;
				}
				i++;
			}
		}
	} else {
		wl_list_for_each(c, &clients, link) {
			if (VISIBLEON(c, m) && ISTILED(c)) {

				if (total_master_inner_percent <= 0.0)
					return;
				if (i < m->pertag->nmasters[m->pertag->curtag]) {
					c->ismaster = true;
					c->stack_innder_per =
						stack_num > 1 ? 2.0f / stack_num : 1.0f;
					c->master_inner_per =
						c->master_inner_per / total_master_inner_percent;
				} else {
					stack_index = i - nmasters;

					c->ismaster = false;
					c->master_inner_per = 1.0f / master_num;
					if ((stack_index % 2) ^ (tile_cilent_num % 2 == 0)) {
						c->stack_innder_per =
							total_right_stack_hight_percent
								? c->stack_innder_per /
									  total_right_stack_hight_percent
								: 1.0f;
					} else {
						c->stack_innder_per =
							total_left_stack_hight_percent
								? c->stack_innder_per /
									  total_left_stack_hight_percent
								: 1.0f;
					}
				}
				i++;
			}
		}
	}
}

void // 17
arrange(Monitor *m, bool want_animation) {
	Client *c = NULL;
	double total_stack_innder_percent = 0;
	double total_master_inner_percent = 0;
	double total_right_stack_hight_percent = 0;
	double total_left_stack_hight_percent = 0;
	int i = 0;
	int nmasters = 0;
	int stack_index = 0;
	int master_num = 0;
	int stack_num = 0;

	if (!m)
		return;

	if (!m->wlr_output->enabled)
		return;
	m->visible_clients = 0;
	m->visible_tiling_clients = 0;

	wl_list_for_each(c, &clients, link) {
		if (VISIBLEON(c, m)) {
			m->visible_clients++;
			if (ISTILED(c)) {
				m->visible_tiling_clients++;
			}
		}
	}

	nmasters = m->pertag->nmasters[m->pertag->curtag];

	wl_list_for_each(c, &clients, link) {
		if (c->iskilling)
			continue;

		if (c->mon == m && (c->isglobal || c->isunglobal)) {
			c->tags = m->tagset[m->seltags];
			if (c->mon->sel == NULL)
				focusclient(c, 0);
		}

		if (c->mon == m) {
			if (VISIBLEON(c, m)) {
				if (ISTILED(c)) {

					if (i < m->pertag->nmasters[m->pertag->curtag]) {
						master_num++;
						total_master_inner_percent += c->master_inner_per;
					} else {
						stack_num++;
						total_stack_innder_percent += c->stack_innder_per;
						stack_index = i - nmasters;
						if ((stack_index % 2) ^
							(m->visible_tiling_clients % 2 == 0)) {
							c->isleftstack = false;
							total_right_stack_hight_percent +=
								c->stack_innder_per;
						} else {
							c->isleftstack = true;
							total_left_stack_hight_percent +=
								c->stack_innder_per;
						}
					}

					i++;
				}

				set_arrange_visible(m, c, want_animation);
			} else {
				set_arrange_hidden(m, c, want_animation);
			}
		}

		if (c->mon == m && c->ismaxmizescreen && !c->animation.tagouted &&
			!c->animation.tagouting && VISIBLEON(c, m)) {
			reset_maxmizescreen_size(c);
		}
	}

	reset_size_per_mon(
		m, m->visible_tiling_clients, total_left_stack_hight_percent,
		total_right_stack_hight_percent, total_stack_innder_percent,
		total_master_inner_percent, master_num, stack_num);

	if (m->isoverview) {
		overviewlayout.arrange(m);
	} else {
		m->pertag->ltidxs[m->pertag->curtag]->arrange(m);
	}

	if (!start_drag_window) {
		motionnotify(0, NULL, 0, 0, 0, 0);
		checkidleinhibitor(NULL);
	}
}
