// 网格布局窗口大小和位置计算
void grid(Monitor *m) {
	unsigned int i, n;
	unsigned int cx, cy, cw, ch;
	unsigned int dx;
	unsigned int cols, rows, overcols;
	Client *c = NULL;
	n = 0;
	int target_gappo = enablegaps ? m->isoverview ? overviewgappo : gappoh : 0;
	int target_gappi = enablegaps ? m->isoverview ? overviewgappi : gappih : 0;
	float single_width_ratio = m->isoverview ? 0.7 : 0.9;
	float single_height_ratio = m->isoverview ? 0.8 : 0.9;

	n = m->isoverview ? m->visible_clients : m->visible_tiling_clients;

	if (n == 0) {
		return; // 没有需要处理的客户端，直接返回
	}

	if (n == 1) {
		wl_list_for_each(c, &clients, link) {

			if (c->mon != m)
				continue;

			c->bw = m->visible_tiling_clients == 1 && no_border_when_single &&
							smartgaps
						? 0
						: borderpx;
			if (VISIBLEON(c, m) && !c->isunglobal &&
				((m->isoverview && !client_is_x11_popup(c)) || ISTILED(c))) {
				cw = (m->w.width - 2 * target_gappo) * single_width_ratio;
				ch = (m->w.height - 2 * target_gappo) * single_height_ratio;
				c->geom.x = m->w.x + (m->w.width - cw) / 2;
				c->geom.y = m->w.y + (m->w.height - ch) / 2;
				c->geom.width = cw;
				c->geom.height = ch;
				resize(c, c->geom, 0);
				return;
			}
		}
	}

	if (n == 2) {
		cw = (m->w.width - 2 * target_gappo - target_gappi) / 2;
		ch = (m->w.height - 2 * target_gappo) * 0.65;
		i = 0;
		wl_list_for_each(c, &clients, link) {
			if (c->mon != m)
				continue;

			c->bw = m->visible_tiling_clients == 1 && no_border_when_single &&
							smartgaps
						? 0
						: borderpx;
			if (VISIBLEON(c, m) && !c->isunglobal &&
				((m->isoverview && !client_is_x11_popup(c)) || ISTILED(c))) {
				if (i == 0) {
					c->geom.x = m->w.x + target_gappo;
					c->geom.y = m->w.y + (m->w.height - ch) / 2 + target_gappo;
					c->geom.width = cw;
					c->geom.height = ch;
					resize(c, c->geom, 0);
				} else if (i == 1) {
					c->geom.x = m->w.x + cw + target_gappo + target_gappi;
					c->geom.y = m->w.y + (m->w.height - ch) / 2 + target_gappo;
					c->geom.width = cw;
					c->geom.height = ch;
					resize(c, c->geom, 0);
				}
				i++;
			}
		}
		return;
	}

	// 计算列数和行数
	for (cols = 0; cols <= n / 2; cols++) {
		if (cols * cols >= n) {
			break;
		}
	}
	rows = (cols && (cols - 1) * cols >= n) ? cols - 1 : cols;

	// 计算每个客户端的高度和宽度
	ch = (m->w.height - 2 * target_gappo - (rows - 1) * target_gappi) / rows;
	cw = (m->w.width - 2 * target_gappo - (cols - 1) * target_gappi) / cols;

	// 处理多余的列
	overcols = n % cols;
	if (overcols) {
		dx = (m->w.width - overcols * cw - (overcols - 1) * target_gappi) / 2 -
			 target_gappo;
	}

	// 调整每个客户端的位置和大小
	i = 0;
	wl_list_for_each(c, &clients, link) {

		if (c->mon != m)
			continue;
		c->bw =
			m->visible_tiling_clients == 1 && no_border_when_single && smartgaps
				? 0
				: borderpx;
		if (VISIBLEON(c, m) && !c->isunglobal &&
			((m->isoverview && !client_is_x11_popup(c)) || ISTILED(c))) {
			cx = m->w.x + (i % cols) * (cw + target_gappi);
			cy = m->w.y + (i / cols) * (ch + target_gappi);
			if (overcols && i >= n - overcols) {
				cx += dx;
			}
			c->geom.x = cx + target_gappo;
			c->geom.y = cy + target_gappo;
			c->geom.width = cw;
			c->geom.height = ch;
			resize(c, c->geom, 0);
			i++;
		}
	}
}

void deck(Monitor *m) {
	unsigned int mw, my;
	int i, n = 0;
	Client *c = NULL;
	Client *fc = NULL;
	float mfact;

	unsigned int cur_gappih = enablegaps ? m->gappih : 0;
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0;
	unsigned int cur_gappov = enablegaps ? m->gappov : 0;

	cur_gappih = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappih;
	cur_gappoh = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappoh;
	cur_gappov = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappov;

	n = m->visible_tiling_clients;

	if (n == 0)
		return;

	wl_list_for_each(fc, &clients, link) {

		if (VISIBLEON(fc, m) && ISTILED(fc))
			break;
	}

	// Calculate master width using mfact from pertag
	mfact = fc->master_mfact_per > 0.0f ? fc->master_mfact_per
										: m->pertag->mfacts[m->pertag->curtag];

	// Calculate master width including outer gaps
	if (n > m->nmaster)
		mw = m->nmaster ? round((m->w.width - 2 * cur_gappoh) * mfact) : 0;
	else
		mw = m->w.width - 2 * cur_gappoh;

	i = my = 0;
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;
		if (i < m->nmaster) {
			c->master_mfact_per = mfact;
			// Master area clients
			resize(
				c,
				(struct wlr_box){.x = m->w.x + cur_gappoh,
								 .y = m->w.y + cur_gappov + my,
								 .width = mw,
								 .height = (m->w.height - 2 * cur_gappov - my) /
										   (MIN(n, m->nmaster) - i)},
				0);
			my += c->geom.height;
		} else {
			// Stack area clients
			c->master_mfact_per = mfact;
			resize(c,
				   (struct wlr_box){.x = m->w.x + mw + cur_gappoh + cur_gappih,
									.y = m->w.y + cur_gappov,
									.width = m->w.width - mw - 2 * cur_gappoh -
											 cur_gappih,
									.height = m->w.height - 2 * cur_gappov},
				   0);
			if (c == focustop(m))
				wlr_scene_node_raise_to_top(&c->scene->node);
		}
		i++;
	}
}

void horizontal_scroll_adjust_fullandmax(Client *c,
										 struct wlr_box *target_geom) {
	Monitor *m = c->mon;
	unsigned int cur_gappih = enablegaps ? m->gappih : 0;
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0;
	unsigned int cur_gappov = enablegaps ? m->gappov : 0;

	cur_gappih =
		smartgaps && m->visible_scroll_tiling_clients == 1 ? 0 : cur_gappih;
	cur_gappoh =
		smartgaps && m->visible_scroll_tiling_clients == 1 ? 0 : cur_gappoh;
	cur_gappov =
		smartgaps && m->visible_scroll_tiling_clients == 1 ? 0 : cur_gappov;

	if (c->isfullscreen) {
		target_geom->height = m->m.height;
		target_geom->width = m->m.width;
		target_geom->y = m->m.y;
		return;
	}

	if (c->ismaximizescreen) {
		target_geom->height = m->w.height - 2 * cur_gappov;
		target_geom->width = m->w.width - 2 * cur_gappoh;
		target_geom->y = m->w.y + cur_gappov;
		return;
	}

	target_geom->height = m->w.height - 2 * cur_gappov;
	target_geom->y = m->w.y + (m->w.height - target_geom->height) / 2;
}

// 滚动布局
void scroller(Monitor *m) {
	unsigned int i, n, j;
	float single_proportion = 1.0;

	Client *c = NULL, *root_client = NULL;
	Client **tempClients = NULL; // 初始化为 NULL
	struct wlr_box target_geom;
	int focus_client_index = 0;
	bool need_scroller = false;
	unsigned int cur_gappih = enablegaps ? m->gappih : 0;
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0;
	unsigned int cur_gappov = enablegaps ? m->gappov : 0;

	cur_gappih =
		smartgaps && m->visible_scroll_tiling_clients == 1 ? 0 : cur_gappih;
	cur_gappoh =
		smartgaps && m->visible_scroll_tiling_clients == 1 ? 0 : cur_gappoh;
	cur_gappov =
		smartgaps && m->visible_scroll_tiling_clients == 1 ? 0 : cur_gappov;

	unsigned int max_client_width =
		m->w.width - 2 * scroller_structs - cur_gappih;

	n = m->visible_scroll_tiling_clients;

	if (n == 0) {
		return; // 没有需要处理的客户端，直接返回
	}

	// 动态分配内存
	tempClients = malloc(n * sizeof(Client *));
	if (!tempClients) {
		// 处理内存分配失败的情况
		return;
	}

	// 第二次遍历，填充 tempClients
	j = 0;
	wl_list_for_each(c, &clients, link) {
		if (VISIBLEON(c, m) && ISSCROLLTILED(c)) {
			tempClients[j] = c;
			j++;
		}
	}

	if (n == 1 && !scroller_ignore_proportion_single &&
		!tempClients[0]->isfullscreen && !tempClients[0]->ismaximizescreen) {
		c = tempClients[0];

		single_proportion = c->scroller_proportion_single > 0.0f
								? c->scroller_proportion_single
								: scroller_default_proportion_single;

		target_geom.height = m->w.height - 2 * cur_gappov;
		target_geom.width = (m->w.width - 2 * cur_gappoh) * single_proportion;
		target_geom.x = m->w.x + (m->w.width - target_geom.width) / 2;
		target_geom.y = m->w.y + (m->w.height - target_geom.height) / 2;
		resize(c, target_geom, 0);
		free(tempClients); // 释放内存
		return;
	}

	if (m->sel && !client_is_unmanaged(m->sel) && !m->sel->isfloating) {
		root_client = m->sel;
	} else if (m->prevsel && ISSCROLLTILED(m->prevsel) &&
			   VISIBLEON(m->prevsel, m) && !client_is_unmanaged(m->prevsel)) {
		root_client = m->prevsel;
	} else {
		root_client = center_tiled_select(m);
	}

	if (!root_client) {
		free(tempClients); // 释放内存
		return;
	}

	for (i = 0; i < n; i++) {
		c = tempClients[i];
		if (root_client == c) {
			if (!c->is_pending_open_animation &&
				c->geom.x >= m->w.x + scroller_structs &&
				c->geom.x + c->geom.width <=
					m->w.x + m->w.width - scroller_structs) {
				need_scroller = false;
			} else {
				need_scroller = true;
			}
			focus_client_index = i;
			break;
		}
	}

	if (n == 1 && scroller_ignore_proportion_single) {
		need_scroller = true;
	}

	if (start_drag_window)
		need_scroller = false;

	target_geom.height = m->w.height - 2 * cur_gappov;
	target_geom.width = max_client_width * c->scroller_proportion;
	target_geom.y = m->w.y + (m->w.height - target_geom.height) / 2;
	horizontal_scroll_adjust_fullandmax(tempClients[focus_client_index],
										&target_geom);
	if (tempClients[focus_client_index]->isfullscreen) {
		target_geom.x = m->m.x;
		resize(tempClients[focus_client_index], target_geom, 0);
	} else if (tempClients[focus_client_index]->ismaximizescreen) {
		target_geom.x = m->w.x + cur_gappoh;
		resize(tempClients[focus_client_index], target_geom, 0);
	} else if (need_scroller) {
		if (scroller_focus_center ||
			((!m->prevsel ||
			  (ISSCROLLTILED(m->prevsel) &&
			   (m->prevsel->scroller_proportion * max_client_width) +
					   (root_client->scroller_proportion * max_client_width) >
				   m->w.width - 2 * scroller_structs - cur_gappih)) &&
			 scroller_prefer_center)) {
			target_geom.x = m->w.x + (m->w.width - target_geom.width) / 2;
		} else {
			target_geom.x = root_client->geom.x > m->w.x + (m->w.width) / 2
								? m->w.x + (m->w.width -
											root_client->scroller_proportion *
												max_client_width -
											scroller_structs)
								: m->w.x + scroller_structs;
		}
		resize(tempClients[focus_client_index], target_geom, 0);
	} else {
		target_geom.x = c->geom.x;
		resize(tempClients[focus_client_index], target_geom, 0);
	}

	for (i = 1; i <= focus_client_index; i++) {
		c = tempClients[focus_client_index - i];
		target_geom.width = max_client_width * c->scroller_proportion;
		horizontal_scroll_adjust_fullandmax(c, &target_geom);
		target_geom.x = tempClients[focus_client_index - i + 1]->geom.x -
						cur_gappih - target_geom.width;

		resize(c, target_geom, 0);
	}

	for (i = 1; i < n - focus_client_index; i++) {
		c = tempClients[focus_client_index + i];
		target_geom.width = max_client_width * c->scroller_proportion;
		horizontal_scroll_adjust_fullandmax(c, &target_geom);
		target_geom.x = tempClients[focus_client_index + i - 1]->geom.x +
						cur_gappih +
						tempClients[focus_client_index + i - 1]->geom.width;
		resize(c, target_geom, 0);
	}

	free(tempClients); // 最后释放内存
}

void center_tile(Monitor *m) {
	unsigned int i, n = 0, h, r, ie = enablegaps, mw, mx, my, oty, ety, tw;
	Client *c = NULL;
	Client *fc = NULL;
	double mfact = 0;
	int master_num = 0;
	int stack_num = 0;

	n = m->visible_tiling_clients;
	master_num = m->pertag->nmasters[m->pertag->curtag];
	stack_num = n - master_num;

	if (n == 0)
		return;

	// 获取第一个可见的平铺客户端用于主区域宽度百分比
	wl_list_for_each(fc, &clients, link) {
		if (VISIBLEON(fc, m) && ISTILED(fc))
			break;
	}

	// 间隙参数处理
	unsigned int cur_gappiv = enablegaps ? m->gappiv : 0; // 内部垂直间隙
	unsigned int cur_gappih = enablegaps ? m->gappih : 0; // 内部水平间隙
	unsigned int cur_gappov = enablegaps ? m->gappov : 0; // 外部垂直间隙
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0; // 外部水平间隙

	// 智能间隙处理
	cur_gappiv = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappiv;
	cur_gappih = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappih;
	cur_gappov = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappov;
	cur_gappoh = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappoh;

	unsigned int nmasters = m->pertag->nmasters[m->pertag->curtag];
	mfact = fc->master_mfact_per > 0.0f ? fc->master_mfact_per
										: m->pertag->mfacts[m->pertag->curtag];

	// 初始化区域
	mw = m->w.width;
	mx = cur_gappoh;
	my = cur_gappov;
	tw = mw;

	// 判断是否需要主区域铺满
	int should_overspread = center_master_overspread && (n <= nmasters);

	if (n > nmasters || !should_overspread) {
		// 计算主区域宽度（居中模式）
		mw = nmasters ? (m->w.width - 2 * cur_gappoh - cur_gappih * ie) * mfact
					  : 0;

		if (n - nmasters > 1) {
			// 多个堆叠窗口：主区域居中，左右两侧各有一个堆叠区域
			tw = (m->w.width - mw) / 2 - cur_gappoh - cur_gappih * ie;
			mx = cur_gappoh + tw + cur_gappih * ie;
		} else if (n - nmasters == 1) {
			// 单个堆叠窗口的处理
			if (center_when_single_stack) {
				// stack在右边，master居中，左边空着
				tw = (m->w.width - mw) / 2 - cur_gappoh - cur_gappih * ie;
				mx = cur_gappoh + tw + cur_gappih * ie; // master居中
			} else {
				// stack在右边，master在左边
				tw = m->w.width - mw - 2 * cur_gappoh - cur_gappih * ie;
				mx = cur_gappoh; // master在左边
			}
		} else {
			// 只有主区域窗口：居中显示
			tw = (m->w.width - mw) / 2 - cur_gappoh - cur_gappih * ie;
			mx = cur_gappoh + tw + cur_gappih * ie;
		}
	} else {
		// 主区域铺满模式（只有主区域窗口时）
		mw = m->w.width - 2 * cur_gappoh;
		mx = cur_gappoh;
		tw = 0; // 堆叠区域宽度为0
	}

	oty = cur_gappov;
	ety = cur_gappov;

	i = 0;
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;

		if (i < nmasters) {
			// 主区域窗口
			r = MIN(n, nmasters) - i;
			if (c->master_inner_per > 0.0f) {
				h = (m->w.height - 2 * cur_gappov -
					 cur_gappiv * ie * (master_num - 1)) *
					c->master_inner_per;
				c->master_mfact_per = mfact;
			} else {
				h = (m->w.height - my - cur_gappov -
					 cur_gappiv * ie * (r - 1)) /
					r;
				c->master_inner_per = h / (m->w.height - my - cur_gappov -
										   cur_gappiv * ie * (r - 1));
				c->master_mfact_per = mfact;
			}

			resize(c,
				   (struct wlr_box){.x = m->w.x + mx,
									.y = m->w.y + my,
									.width = mw,
									.height = h},
				   0);
			my += c->geom.height + cur_gappiv * ie;
		} else {
			// 堆叠区域窗口
			unsigned int stack_index = i - nmasters;

			if (n - nmasters == 1) {
				// 单个堆叠窗口
				r = n - i;
				if (c->stack_innder_per > 0.0f) {
					h = (m->w.height - 2 * cur_gappov -
						 cur_gappiv * ie * (stack_num - 1)) *
						c->stack_innder_per;
					c->master_mfact_per = mfact;
				} else {
					h = (m->w.height - ety - cur_gappov -
						 cur_gappiv * ie * (r - 1)) /
						r;
					c->stack_innder_per = h / (m->w.height - ety - cur_gappov -
											   cur_gappiv * ie * (r - 1));
					c->master_mfact_per = mfact;
				}

				int stack_x;
				if (center_when_single_stack) {
					// 放在右侧（master居中时，stack在右边）
					stack_x = m->w.x + mx + mw + cur_gappih * ie;
				} else {
					// 放在右侧（master在左边时，stack在右边）
					stack_x = m->w.x + mx + mw + cur_gappih * ie;
				}

				resize(c,
					   (struct wlr_box){.x = stack_x,
										.y = m->w.y + ety,
										.width = tw,
										.height = h},
					   0);
				ety += c->geom.height + cur_gappiv * ie;
			} else {
				// 多个堆叠窗口：交替放在左右两侧
				r = (n - i + 1) / 2;

				if ((stack_index % 2) ^ (n % 2 == 0)) {
					// 右侧堆叠窗口
					if (c->stack_innder_per > 0.0f) {
						h = (m->w.height - 2 * cur_gappov -
							 cur_gappiv * ie * ((stack_num + 1) / 2 - 1)) *
							c->stack_innder_per;
						c->master_mfact_per = mfact;
					} else {
						h = (m->w.height - ety - cur_gappov -
							 cur_gappiv * ie * (r - 1)) /
							r;
						c->stack_innder_per =
							h / (m->w.height - ety - cur_gappov -
								 cur_gappiv * ie * (r - 1));
						c->master_mfact_per = mfact;
					}

					int stack_x = m->w.x + mx + mw + cur_gappih * ie;

					resize(c,
						   (struct wlr_box){.x = stack_x,
											.y = m->w.y + ety,
											.width = tw,
											.height = h},
						   0);
					ety += c->geom.height + cur_gappiv * ie;
				} else {
					// 左侧堆叠窗口
					if (c->stack_innder_per > 0.0f) {
						h = (m->w.height - 2 * cur_gappov -
							 cur_gappiv * ie * (stack_num / 2 - 1)) *
							c->stack_innder_per;
						c->master_mfact_per = mfact;
					} else {
						h = (m->w.height - oty - cur_gappov -
							 cur_gappiv * ie * (r - 1)) /
							r;
						c->stack_innder_per =
							h / (m->w.height - oty - cur_gappov -
								 cur_gappiv * ie * (r - 1));
						c->master_mfact_per = mfact;
					}

					int stack_x = m->w.x + cur_gappoh;
					resize(c,
						   (struct wlr_box){.x = stack_x,
											.y = m->w.y + oty,
											.width = tw,
											.height = h},
						   0);
					oty += c->geom.height + cur_gappiv * ie;
				}
			}
		}
		i++;
	}
}

void tile(Monitor *m) {
	unsigned int i, n = 0, h, r, ie = enablegaps, mw, my, ty;
	Client *c = NULL;
	Client *fc = NULL;
	double mfact = 0;
	int master_num = 0;
	int stack_num = 0;

	n = m->visible_tiling_clients;
	master_num = m->pertag->nmasters[m->pertag->curtag];
	stack_num = n - master_num;

	if (n == 0)
		return;

	unsigned int cur_gappiv = enablegaps ? m->gappiv : 0;
	unsigned int cur_gappih = enablegaps ? m->gappih : 0;
	unsigned int cur_gappov = enablegaps ? m->gappov : 0;
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0;

	cur_gappiv = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappiv;
	cur_gappih = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappih;
	cur_gappov = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappov;
	cur_gappoh = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappoh;

	wl_list_for_each(fc, &clients, link) {

		if (VISIBLEON(fc, m) && ISTILED(fc))
			break;
	}

	mfact = fc->master_mfact_per > 0.0f ? fc->master_mfact_per
										: m->pertag->mfacts[m->pertag->curtag];

	if (n > m->pertag->nmasters[m->pertag->curtag])
		mw = m->pertag->nmasters[m->pertag->curtag]
				 ? (m->w.width + cur_gappih * ie) * mfact
				 : 0;
	else
		mw = m->w.width - 2 * cur_gappoh + cur_gappih * ie;
	i = 0;
	my = ty = cur_gappov;
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;
		if (i < m->pertag->nmasters[m->pertag->curtag]) {
			r = MIN(n, m->pertag->nmasters[m->pertag->curtag]) - i;
			if (c->master_inner_per > 0.0f) {
				h = (m->w.height - 2 * cur_gappov -
					 cur_gappiv * ie * (master_num - 1)) *
					c->master_inner_per;
				c->master_mfact_per = mfact;
			} else {
				h = (m->w.height - my - cur_gappov -
					 cur_gappiv * ie * (r - 1)) /
					r;
				c->master_inner_per = h / (m->w.height - my - cur_gappov -
										   cur_gappiv * ie * (r - 1));
				c->master_mfact_per = mfact;
			}
			resize(c,
				   (struct wlr_box){.x = m->w.x + cur_gappoh,
									.y = m->w.y + my,
									.width = mw - cur_gappih * ie,
									.height = h},
				   0);
			my += c->geom.height + cur_gappiv * ie;
		} else {
			r = n - i;
			if (c->stack_innder_per > 0.0f) {
				h = (m->w.height - 2 * cur_gappov -
					 cur_gappiv * ie * (stack_num - 1)) *
					c->stack_innder_per;
				c->master_mfact_per = mfact;
			} else {
				h = (m->w.height - ty - cur_gappov -
					 cur_gappiv * ie * (r - 1)) /
					r;
				c->stack_innder_per = h / (m->w.height - ty - cur_gappov -
										   cur_gappiv * ie * (r - 1));
				c->master_mfact_per = mfact;
			}

			// wlr_log(WLR_ERROR, "stack_innder_per: %f", c->stack_innder_per);

			resize(c,
				   (struct wlr_box){.x = m->w.x + mw + cur_gappoh,
									.y = m->w.y + ty,
									.width = m->w.width - mw - 2 * cur_gappoh,
									.height = h},
				   0);
			ty += c->geom.height + cur_gappiv * ie;
		}
		i++;
	}
}

void right_tile(Monitor *m) {
	unsigned int i, n = 0, h, r, ie = enablegaps, mw, my, ty;
	Client *c = NULL;
	Client *fc = NULL;
	double mfact = 0;
	int master_num = 0;
	int stack_num = 0;

	n = m->visible_tiling_clients;
	master_num = m->pertag->nmasters[m->pertag->curtag];
	stack_num = n - master_num;

	if (n == 0)
		return;

	unsigned int cur_gappiv = enablegaps ? m->gappiv : 0;
	unsigned int cur_gappih = enablegaps ? m->gappih : 0;
	unsigned int cur_gappov = enablegaps ? m->gappov : 0;
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0;

	cur_gappiv = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappiv;
	cur_gappih = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappih;
	cur_gappov = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappov;
	cur_gappoh = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappoh;

	wl_list_for_each(fc, &clients, link) {

		if (VISIBLEON(fc, m) && ISTILED(fc))
			break;
	}

	mfact = fc->master_mfact_per > 0.0f ? fc->master_mfact_per
										: m->pertag->mfacts[m->pertag->curtag];

	if (n > m->pertag->nmasters[m->pertag->curtag])
		mw = m->pertag->nmasters[m->pertag->curtag]
				 ? (m->w.width + cur_gappih * ie) * mfact
				 : 0;
	else
		mw = m->w.width - 2 * cur_gappoh + cur_gappih * ie;
	i = 0;
	my = ty = cur_gappov;
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;
		if (i < m->pertag->nmasters[m->pertag->curtag]) {
			r = MIN(n, m->pertag->nmasters[m->pertag->curtag]) - i;
			if (c->master_inner_per > 0.0f) {
				h = (m->w.height - 2 * cur_gappov -
					 cur_gappiv * ie * (master_num - 1)) *
					c->master_inner_per;
				c->master_mfact_per = mfact;
			} else {
				h = (m->w.height - my - cur_gappov -
					 cur_gappiv * ie * (r - 1)) /
					r;
				c->master_inner_per = h / (m->w.height - my - cur_gappov -
										   cur_gappiv * ie * (r - 1));
				c->master_mfact_per = mfact;
			}
			resize(c,
				   (struct wlr_box){.x = m->w.x + m->w.width - mw - cur_gappoh +
										 cur_gappih * ie,
									.y = m->w.y + my,
									.width = mw - cur_gappih * ie,
									.height = h},
				   0);
			my += c->geom.height + cur_gappiv * ie;
		} else {
			r = n - i;
			if (c->stack_innder_per > 0.0f) {
				h = (m->w.height - 2 * cur_gappov -
					 cur_gappiv * ie * (stack_num - 1)) *
					c->stack_innder_per;
				c->master_mfact_per = mfact;
			} else {
				h = (m->w.height - ty - cur_gappov -
					 cur_gappiv * ie * (r - 1)) /
					r;
				c->stack_innder_per = h / (m->w.height - ty - cur_gappov -
										   cur_gappiv * ie * (r - 1));
				c->master_mfact_per = mfact;
			}

			// wlr_log(WLR_ERROR, "stack_innder_per: %f", c->stack_innder_per);

			resize(c,
				   (struct wlr_box){.x = m->w.x + cur_gappoh,
									.y = m->w.y + ty,
									.width = m->w.width - mw - 2 * cur_gappoh,
									.height = h},
				   0);
			ty += c->geom.height + cur_gappiv * ie;
		}
		i++;
	}
}

void // 17
monocle(Monitor *m) {
	Client *c = NULL;
	struct wlr_box geom;

	unsigned int cur_gappov = enablegaps ? m->gappov : 0;
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0;

	cur_gappoh = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappoh;
	cur_gappov = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappov;

	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;
		geom.x = m->w.x + cur_gappoh;
		geom.y = m->w.y + cur_gappov;
		geom.width = m->w.width - 2 * cur_gappoh;
		geom.height = m->w.height - 2 * cur_gappov;
		resize(c, geom, 0);
	}
	if ((c = focustop(m)))
		wlr_scene_node_raise_to_top(&c->scene->node);
}