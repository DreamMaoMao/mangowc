void bind_to_view(const Arg *arg) { view(arg, true); }
void chvt(const Arg *arg) { wlr_session_change_vt(session, arg->ui); }
/**
 * This command is intended for developer use only.
 */
void create_virtual_output(const Arg *arg) {

	if (!wlr_backend_is_multi(backend)) {
		wlr_log(WLR_ERROR, "Expected a multi backend");
		return;
	}

	bool done = false;
	wlr_multi_for_each_backend(backend, create_output, &done);

	if (!done) {
		wlr_log(WLR_ERROR, "Failed to create virtual output");
		return;
	}

	wlr_log(WLR_INFO, "Virtual output created");
	return;
}

void destroy_all_virtual_output(const Arg *arg) {

	if (!wlr_backend_is_multi(backend)) {
		wlr_log(WLR_ERROR, "Expected a multi backend");
		return;
	}

	Monitor *m, *tmp;
	wl_list_for_each_safe(m, tmp, &mons, link) {
		if (wlr_output_is_headless(m->wlr_output)) {
			// if(selmon == m)
			//   selmon = NULL;
			wlr_output_destroy(m->wlr_output);
			wlr_log(WLR_INFO, "Virtual output destroyed");
		}
	}
}

void defaultgaps(const Arg *arg) { setgaps(gappoh, gappov, gappih, gappiv); }
void exchange_client(const Arg *arg) {
	Client *c = selmon->sel;
	if (!c || c->isfloating || c->isfullscreen || c->ismaxmizescreen)
		return;
	exchange_two_client(c, direction_select(arg));
}

void focusdir(const Arg *arg) {
	Client *c;
	c = direction_select(arg);
	if (c) {
		focusclient(c, 1);
		if (warpcursor)
			warp_cursor(c);
	} else {
		if (config.focus_cross_tag) {
			if (arg->i == LEFT || arg->i == UP)
				viewtoleft_have_client(NULL);
			if (arg->i == RIGHT || arg->i == DOWN)
				viewtoright_have_client(NULL);
		}
	}
}
void focuslast(const Arg *arg) {

	Client *c, *prev = NULL;
	wl_list_for_each(c, &fstack, flink) {
		if (c->iskilling || c->isminied || c->isunglobal)
			continue;
		if (c)
			break;
	}

	struct wl_list *prev_node = c->flink.next;
	prev = wl_container_of(prev_node, prev, flink);

	unsigned int target;
	if (prev) {
		if (prev->mon != selmon) {
			selmon = prev->mon;
			warp_cursor_to_selmon(selmon);
		}
		target = get_tags_first_tag(prev->tags);
		view(&(Arg){.ui = target}, true);
		focusclient(prev, 1);
	}
}

void focusmon(const Arg *arg) {
	Client *c;
	Monitor *m = NULL;
	int i = 0, nmons = wl_list_length(&mons);
	if (nmons && arg->i != UNDIR) {
		do /* don't switch to disabled mons */
			selmon = dirtomon(arg->i);
		while (!selmon->wlr_output->enabled && i++ < nmons);
	} else if (arg->v) {
		wl_list_for_each(m, &mons, link) {
			if (!m->wlr_output->enabled) {
				continue;
			}
			if (regex_match(arg->v, m->wlr_output->name)) {
				selmon = m;
				break;
			}
		}
	} else {
		return;
	}
	warp_cursor_to_selmon(selmon);
	c = focustop(selmon);
	if (!c) {
		selmon->sel = NULL;
		wlr_seat_pointer_notify_clear_focus(seat);
		wlr_seat_keyboard_notify_clear_focus(seat);
	} else
		focusclient(c, 1);
}

void // 17
focusstack(const Arg *arg) {
	/* Focus the next or previous client (in tiling order) on selmon */
	Client *c, *sel = focustop(selmon);
	if (!sel || sel->isfullscreen)
		return;
	if (arg->i > 0) {
		wl_list_for_each(c, &sel->link, link) {
			if (&c->link == &clients || c->isunglobal)
				continue; /* wrap past the sentinel node */
			if (VISIBLEON(c, selmon))
				break; /* found it */
		}
	} else {
		wl_list_for_each_reverse(c, &sel->link, link) {
			if (&c->link == &clients)
				continue; /* wrap past the sentinel node */
			if (VISIBLEON(c, selmon) || c->isunglobal)
				break; /* found it */
		}
	}
	/* If only one client is visible on selmon, then c == sel */
	focusclient(c, 1);
	if (warpcursor)
		warp_cursor(c);
}

void incnmaster(const Arg *arg) {
	if (!arg || !selmon)
		return;
	selmon->pertag->nmasters[selmon->pertag->curtag] =
		MAX(selmon->pertag->nmasters[selmon->pertag->curtag] + arg->i, 0);
	arrange(selmon, false);
}

void incgaps(const Arg *arg) {
	setgaps(selmon->gappoh + arg->i, selmon->gappov + arg->i,
			selmon->gappih + arg->i, selmon->gappiv + arg->i);
}

void incigaps(const Arg *arg) {
	setgaps(selmon->gappoh, selmon->gappov, selmon->gappih + arg->i,
			selmon->gappiv + arg->i);
}

void incogaps(const Arg *arg) {
	setgaps(selmon->gappoh + arg->i, selmon->gappov + arg->i, selmon->gappih,
			selmon->gappiv);
}

void incihgaps(const Arg *arg) {
	setgaps(selmon->gappoh, selmon->gappov, selmon->gappih + arg->i,
			selmon->gappiv);
}

void incivgaps(const Arg *arg) {
	setgaps(selmon->gappoh, selmon->gappov, selmon->gappih,
			selmon->gappiv + arg->i);
}

void incohgaps(const Arg *arg) {
	setgaps(selmon->gappoh + arg->i, selmon->gappov, selmon->gappih,
			selmon->gappiv);
}

void incovgaps(const Arg *arg) {
	setgaps(selmon->gappoh, selmon->gappov + arg->i, selmon->gappih,
			selmon->gappiv);
}

void increase_proportion(const Arg *arg) {
	if (selmon->sel) {
		unsigned int max_client_width =
			selmon->w.width - 2 * scroller_structs - gappih;
		selmon->sel->scroller_proportion =
			MIN(MAX(arg->f + selmon->sel->scroller_proportion, 0.1), 1.0);
		selmon->sel->geom.width = max_client_width * arg->f;
		arrange(selmon, false);
	}
}

void setsmfact(const Arg *arg) {
	float f;

	if (!arg || !selmon ||
		!selmon->pertag->ltidxs[selmon->pertag->curtag]->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->pertag->smfacts[selmon->pertag->curtag]
					 : arg->f - 1.0;
	if (f < 0.1 || f > 0.9)
		return;
	// selmon->mfact = f;
	selmon->pertag->smfacts[selmon->pertag->curtag] = f;
	arrange(selmon, false);
}
/* arg > 1.0 will set mfact absolutely */
void // 17
setmfact(const Arg *arg) {
	float f;

	if (!arg || !selmon ||
		!selmon->pertag->ltidxs[selmon->pertag->curtag]->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->pertag->mfacts[selmon->pertag->curtag]
					 : arg->f - 1.0;
	if (f < 0.1 || f > 0.9)
		return;
	// selmon->mfact = f;
	selmon->pertag->mfacts[selmon->pertag->curtag] = f;
	arrange(selmon, false);
}

void killclient(const Arg *arg) {
	Client *c;
	c = selmon->sel;
	if (c) {
		pending_kill_client(c);
	}
}

void // 17
moveresize(const Arg *arg) {
	if (cursor_mode != CurNormal && cursor_mode != CurPressed)
		return;
	xytonode(cursor->x, cursor->y, NULL, &grabc, NULL, NULL, NULL);
	if (!grabc || client_is_unmanaged(grabc) || grabc->isfullscreen)
		return;

	/* Float the window and tell motionnotify to grab it */
	if (grabc->isfloating == 0) {
		grabc->drag_to_tile = true;
		setfloating(grabc, 1);
	}

	switch (cursor_mode = arg->ui) {
	case CurMove:
		grabcx = cursor->x - grabc->geom.x;
		grabcy = cursor->y - grabc->geom.y;
		wlr_cursor_set_xcursor(cursor, cursor_mgr, "all-scroll");
		break;
	case CurResize:
		/* Doesn't work for X11 output - the next absolute motion event
		 * returns the cursor to where it started */
		wlr_cursor_warp_closest(cursor, NULL, grabc->geom.x + grabc->geom.width,
								grabc->geom.y + grabc->geom.height);
		wlr_cursor_set_xcursor(cursor, cursor_mgr, "bottom_right_corner");
		break;
	}
}

void movewin(const Arg *arg) {
	Client *c;
	c = selmon->sel;
	if (!c || c->isfullscreen)
		return;
	if (!c->isfloating)
		togglefloating(NULL);

	switch (arg->ui) {
	case NUM_TYPE_MINUS:
		c->geom.x -= arg->i;
		break;
	case NUM_TYPE_PLUS:
		c->geom.x += arg->i;
		break;
	default:
		c->geom.x = arg->i;
		break;
	}

	switch (arg->ui2) {
	case NUM_TYPE_MINUS:
		c->geom.y -= arg->i2;
		break;
	case NUM_TYPE_PLUS:
		c->geom.y += arg->i2;
		break;
	default:
		c->geom.y = arg->i2;
		break;
	}

	c->oldgeom = c->geom;
	resize(c, c->geom, 0);
}

void // 0.5 custom
quit(const Arg *arg) {
	wl_display_terminate(dpy);
}

void resizewin(const Arg *arg) {
	Client *c;
	c = selmon->sel;
	if (!c || c->isfullscreen)
		return;
	if (!c->isfloating)
		togglefloating(NULL);

	switch (arg->ui) {
	case NUM_TYPE_MINUS:
		c->geom.width -= arg->i;
		break;
	case NUM_TYPE_PLUS:
		c->geom.width += arg->i;
		break;
	default:
		c->geom.width = arg->i;
		break;
	}

	switch (arg->ui2) {
	case NUM_TYPE_MINUS:
		c->geom.height -= arg->i2;
		break;
	case NUM_TYPE_PLUS:
		c->geom.height += arg->i2;
		break;
	default:
		c->geom.height = arg->i2;
		break;
	}

	c->oldgeom = c->geom;
	resize(c, c->geom, 0);
}
void restore_minized(const Arg *arg) {
	Client *c;
	if (selmon && selmon->sel && selmon->sel->is_in_scratchpad &&
		selmon->sel->is_scratchpad_show) {
		selmon->sel->isminied = 0;
		selmon->sel->is_scratchpad_show = 0;
		selmon->sel->is_in_scratchpad = 0;
		selmon->sel->isnamedscratchpad = 0;
		setborder_color(selmon->sel);
		return;
	}

	wl_list_for_each(c, &clients, link) {
		if (c->isminied) {
			show_hide_client(c);
			c->is_scratchpad_show = 0;
			c->is_in_scratchpad = 0;
			c->isnamedscratchpad = 0;
			setborder_color(c);
			break;
		}
	}
}

void // 17
setlayout(const Arg *arg) {
	int jk;
	for (jk = 0; jk < LENGTH(layouts); jk++) {
		if (strcmp(layouts[jk].name, arg->v) == 0) {
			selmon->pertag->ltidxs[selmon->pertag->curtag] = &layouts[jk];

			arrange(selmon, false);
			printstatus();
			return;
		}
	}
}

void set_proportion(const Arg *arg) {
	if (selmon->sel) {
		unsigned int max_client_width =
			selmon->w.width - 2 * scroller_structs - gappih;
		selmon->sel->scroller_proportion = arg->f;
		selmon->sel->geom.width = max_client_width * arg->f;
		// resize(selmon->sel, selmon->sel->geom, 0);
		arrange(selmon, false);
	}
}

void smartmovewin(const Arg *arg) {
	Client *c, *tc;
	int nx, ny;
	int buttom, top, left, right, tar;
	c = selmon->sel;
	if (!c || c->isfullscreen)
		return;
	if (!c->isfloating)
		setfloating(selmon->sel, true);
	nx = c->geom.x;
	ny = c->geom.y;

	switch (arg->i) {
	case UP:
		tar = -99999;
		top = c->geom.y;
		ny -= c->mon->w.height / 4;

		wl_list_for_each(tc, &clients, link) {
			if (!VISIBLEON(tc, selmon) || !tc->isfloating || tc == c)
				continue;
			if (c->geom.x + c->geom.width < tc->geom.x ||
				c->geom.x > tc->geom.x + tc->geom.width)
				continue;
			buttom = tc->geom.y + tc->geom.height + gappiv;
			if (top > buttom && ny < buttom) {
				tar = MAX(tar, buttom);
			};
		}

		ny = tar == -99999 ? ny : tar;
		ny = MAX(ny, c->mon->w.y + c->mon->gappov);
		break;
	case DOWN:
		tar = 99999;
		buttom = c->geom.y + c->geom.height;
		ny += c->mon->w.height / 4;

		wl_list_for_each(tc, &clients, link) {
			if (!VISIBLEON(tc, selmon) || !tc->isfloating || tc == c)
				continue;
			if (c->geom.x + c->geom.width < tc->geom.x ||
				c->geom.x > tc->geom.x + tc->geom.width)
				continue;
			top = tc->geom.y - gappiv;
			if (buttom < top && (ny + c->geom.height) > top) {
				tar = MIN(tar, top - c->geom.height);
			};
		}
		ny = tar == 99999 ? ny : tar;
		ny = MIN(ny, c->mon->w.y + c->mon->w.height - c->geom.height -
						 c->mon->gappov);
		break;
	case LEFT:
		tar = -99999;
		left = c->geom.x;
		nx -= c->mon->w.width / 6;

		wl_list_for_each(tc, &clients, link) {
			if (!VISIBLEON(tc, selmon) || !tc->isfloating || tc == c)
				continue;
			if (c->geom.y + c->geom.height < tc->geom.y ||
				c->geom.y > tc->geom.y + tc->geom.height)
				continue;
			right = tc->geom.x + tc->geom.width + gappih;
			if (left > right && nx < right) {
				tar = MAX(tar, right);
			};
		}

		nx = tar == -99999 ? nx : tar;
		nx = MAX(nx, c->mon->w.x + c->mon->gappoh);
		break;
	case RIGHT:
		tar = 99999;
		right = c->geom.x + c->geom.width;
		nx += c->mon->w.width / 6;
		wl_list_for_each(tc, &clients, link) {
			if (!VISIBLEON(tc, selmon) || !tc->isfloating || tc == c)
				continue;
			if (c->geom.y + c->geom.height < tc->geom.y ||
				c->geom.y > tc->geom.y + tc->geom.height)
				continue;
			left = tc->geom.x - gappih;
			if (right < left && (nx + c->geom.width) > left) {
				tar = MIN(tar, left - c->geom.width);
			};
		}
		nx = tar == 99999 ? nx : tar;
		nx = MIN(nx, c->mon->w.x + c->mon->w.width - c->geom.width -
						 c->mon->gappoh);
		break;
	}

	c->oldgeom = (struct wlr_box){
		.x = nx, .y = ny, .width = c->geom.width, .height = c->geom.height};

	resize(c, c->oldgeom, 1);
}

void smartresizewin(const Arg *arg) {
	Client *c, *tc;
	int nw, nh;
	int buttom, top, left, right, tar;
	c = selmon->sel;
	if (!c || c->isfullscreen)
		return;
	if (!c->isfloating)
		setfloating(c, true);
	nw = c->geom.width;
	nh = c->geom.height;

	switch (arg->i) {
	case UP:
		nh -= selmon->w.height / 8;
		nh = MAX(nh, selmon->w.height / 10);
		break;
	case DOWN:
		tar = -99999;
		buttom = c->geom.y + c->geom.height;
		nh += selmon->w.height / 8;

		wl_list_for_each(tc, &clients, link) {
			if (!VISIBLEON(tc, selmon) || !tc->isfloating || tc == c)
				continue;
			if (c->geom.x + c->geom.width < tc->geom.x ||
				c->geom.x > tc->geom.x + tc->geom.width)
				continue;
			top = tc->geom.y - gappiv;
			if (buttom < top && (nh + c->geom.y) > top) {
				tar = MAX(tar, top - c->geom.y);
			};
		}
		nh = tar == -99999 ? nh : tar;
		if (c->geom.y + nh + gappov > selmon->w.y + selmon->w.height)
			nh = selmon->w.y + selmon->w.height - c->geom.y - gappov;
		break;
	case LEFT:
		nw -= selmon->w.width / 16;
		nw = MAX(nw, selmon->w.width / 10);
		break;
	case RIGHT:
		tar = 99999;
		right = c->geom.x + c->geom.width;
		nw += selmon->w.width / 16;
		wl_list_for_each(tc, &clients, link) {
			if (!VISIBLEON(tc, selmon) || !tc->isfloating || tc == c)
				continue;
			if (c->geom.y + c->geom.height < tc->geom.y ||
				c->geom.y > tc->geom.y + tc->geom.height)
				continue;
			left = tc->geom.x - gappih;
			if (right < left && (nw + c->geom.x) > left) {
				tar = MIN(tar, left - c->geom.x);
			};
		}

		nw = tar == 99999 ? nw : tar;
		if (c->geom.x + nw + gappoh > selmon->w.x + selmon->w.width)
			nw = selmon->w.x + selmon->w.width - c->geom.x - gappoh;
		break;
	}

	c->oldgeom = (struct wlr_box){
		.x = c->geom.x, .y = c->geom.y, .width = nw, .height = nh};

	resize(c, c->oldgeom, 1);
}

void spawn(const Arg *arg) {

	if (!arg->v)
		return;

	if (fork() == 0) {
		// 1. 忽略可能导致 coredump 的信号
		signal(SIGSEGV, SIG_IGN);
		signal(SIGABRT, SIG_IGN);
		signal(SIGILL, SIG_IGN);

		dup2(STDERR_FILENO, STDOUT_FILENO);
		setsid();

		// 2. 解析参数
		char *argv[64];
		int argc = 0;
		char *token = strtok((char *)arg->v, " ");
		while (token != NULL && argc < 63) {
			wordexp_t p;
			if (wordexp(token, &p, 0) == 0) {
				argv[argc++] = p.we_wordv[0];
			} else {
				argv[argc++] = token;
			}
			token = strtok(NULL, " ");
		}
		argv[argc] = NULL;

		// 3. 执行命令
		execvp(argv[0], argv);

		// 4. execvp 失败时：打印错误并直接退出（避免 coredump）
		wlr_log(WLR_ERROR, "dwl: execvp '%s' failed: %s\n", argv[0],
				strerror(errno));
		_exit(EXIT_FAILURE); // 使用 _exit 避免缓冲区刷新等操作
	}
}

void spawn_on_empty(const Arg *arg) {
	bool is_empty = true;
	Client *c;

	wl_list_for_each(c, &clients, link) {
		if (arg->ui & c->tags && c->mon == selmon) {
			is_empty = false;
			break;
		}
	}
	if (!is_empty) {
		view(arg, true);
		return;
	} else {
		view(arg, true);
		spawn(arg);
	}
}

void switch_keyboard_layout(const Arg *arg) {
	if (!kb_group || !kb_group->wlr_group || !seat) {
		wlr_log(WLR_ERROR, "Invalid keyboard group or seat");
		return;
	}

	struct wlr_keyboard *keyboard = &kb_group->wlr_group->keyboard;
	if (!keyboard || !keyboard->keymap) {
		wlr_log(WLR_ERROR, "Invalid keyboard or keymap");
		return;
	}

	// 1. 获取当前布局和计算下一个布局
	xkb_layout_index_t current = xkb_state_serialize_layout(
		keyboard->xkb_state, XKB_STATE_LAYOUT_EFFECTIVE);
	const int num_layouts = xkb_keymap_num_layouts(keyboard->keymap);
	if (num_layouts < 2) {
		wlr_log(WLR_INFO, "Only one layout available");
		return;
	}
	xkb_layout_index_t next = (current + 1) % num_layouts;

	// 2. 创建上下文
	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (!context) {
		wlr_log(WLR_ERROR, "Failed to create XKB context");
		return;
	}

	// 3. 分配并获取布局缩写
	char **layout_ids = calloc(num_layouts, sizeof(char *));
	if (!layout_ids) {
		wlr_log(WLR_ERROR, "Failed to allocate layout IDs");
		goto cleanup_context;
	}

	for (int i = 0; i < num_layouts; i++) {
		layout_ids[i] =
			get_layout_abbr(xkb_keymap_layout_get_name(keyboard->keymap, i));
		if (!layout_ids[i]) {
			wlr_log(WLR_ERROR, "Failed to get layout abbreviation");
			goto cleanup_layouts;
		}
	}

	// 4. 直接修改 rules.layout（保持原有逻辑）
	struct xkb_rule_names rules = xkb_rules;
	char *layout_buf = (char *)rules.layout; // 假设这是可修改的

	// 清空原有内容（安全方式）
	unsigned int layout_buf_size = strlen(layout_buf) + 1;
	memset(layout_buf, 0, layout_buf_size);

	// 构建新的布局字符串
	for (int i = 0; i < num_layouts; i++) {
		const char *layout = layout_ids[(next + i) % num_layouts];

		if (i > 0) {
			strncat(layout_buf, ",", layout_buf_size - strlen(layout_buf) - 1);
		}

		if (strchr(layout, ',')) {
			// 处理包含逗号的布局名
			char *quoted = malloc(strlen(layout) + 3);
			if (quoted) {
				snprintf(quoted, strlen(layout) + 3, "\"%s\"", layout);
				strncat(layout_buf, quoted,
						layout_buf_size - strlen(layout_buf) - 1);
				free(quoted);
			}
		} else {
			strncat(layout_buf, layout,
					layout_buf_size - strlen(layout_buf) - 1);
		}
	}

	// 5. 创建新 keymap
	struct xkb_keymap *new_keymap =
		xkb_keymap_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (!new_keymap) {
		wlr_log(WLR_ERROR, "Failed to create keymap for layouts: %s",
				rules.layout);
		goto cleanup_layouts;
	}

	// 6. 应用新 keymap
	unsigned int depressed = keyboard->modifiers.depressed;
	unsigned int latched = keyboard->modifiers.latched;
	unsigned int locked = keyboard->modifiers.locked;

	wlr_keyboard_set_keymap(keyboard, new_keymap);
	wlr_keyboard_notify_modifiers(keyboard, depressed, latched, locked, 0);
	keyboard->modifiers.group = 0;

	// 7. 更新 seat
	wlr_seat_set_keyboard(seat, keyboard);
	wlr_seat_keyboard_notify_modifiers(seat, &keyboard->modifiers);

	// 8. 清理资源
	xkb_keymap_unref(new_keymap);

cleanup_layouts:
	for (int i = 0; i < num_layouts; i++) {
		free(layout_ids[i]);
	}
	free(layout_ids);

cleanup_context:
	xkb_context_unref(context);
}

void switch_layout(const Arg *arg) {

	int jk, ji;
	char *target_layout_name = NULL;
	unsigned int len;

	if (config.circle_layout_count != 0) {
		for (jk = 0; jk < config.circle_layout_count; jk++) {

			len = MAX(
				strlen(config.circle_layout[jk]),
				strlen(selmon->pertag->ltidxs[selmon->pertag->curtag]->name));

			if (strncmp(config.circle_layout[jk],
						selmon->pertag->ltidxs[selmon->pertag->curtag]->name,
						len) == 0) {
				target_layout_name = jk == config.circle_layout_count - 1
										 ? config.circle_layout[0]
										 : config.circle_layout[jk + 1];
				break;
			}
		}

		if (!target_layout_name) {
			target_layout_name = config.circle_layout[0];
		}

		for (ji = 0; ji < LENGTH(layouts); ji++) {
			len = MAX(strlen(layouts[ji].name), strlen(target_layout_name));
			if (strncmp(layouts[ji].name, target_layout_name, len) == 0) {
				selmon->pertag->ltidxs[selmon->pertag->curtag] = &layouts[ji];
				break;
			}
		}

		arrange(selmon, false);
		printstatus();
		return;
	}

	for (jk = 0; jk < LENGTH(layouts); jk++) {
		if (strcmp(layouts[jk].name,
				   selmon->pertag->ltidxs[selmon->pertag->curtag]->name) == 0) {
			selmon->pertag->ltidxs[selmon->pertag->curtag] =
				jk == LENGTH(layouts) - 1 ? &layouts[0] : &layouts[jk + 1];
			arrange(selmon, false);
			printstatus();
			return;
		}
	}
}

void switch_proportion_preset(const Arg *arg) {
	float target_proportion = 0;

	if (config.scroller_proportion_preset_count == 0) {
		return;
	}

	if (selmon->sel) {

		for (int i = 0; i < config.scroller_proportion_preset_count; i++) {
			if (config.scroller_proportion_preset[i] ==
				selmon->sel->scroller_proportion) {
				if (i == config.scroller_proportion_preset_count - 1) {
					target_proportion = config.scroller_proportion_preset[0];
					break;
				} else {
					target_proportion =
						config.scroller_proportion_preset[i + 1];
					break;
				}
			}
		}

		if (target_proportion == 0) {
			target_proportion = config.scroller_proportion_preset[0];
		}

		unsigned int max_client_width =
			selmon->w.width - 2 * scroller_structs - gappih;
		selmon->sel->scroller_proportion = target_proportion;
		selmon->sel->geom.width = max_client_width * target_proportion;
		// resize(selmon->sel, selmon->sel->geom, 0);
		arrange(selmon, false);
	}
}
void tag(const Arg *arg) {
	Client *target_client = selmon->sel;
	tag_client(arg, target_client);
}

void tagmon(const Arg *arg) {
	Monitor *m;
	Client *c = focustop(selmon);

	if (!c)
		return;

	m = dirtomon(arg->i);

	if (!m || !m->wlr_output->enabled || m == c->mon)
		return;

	unsigned int newtags = arg->ui ? c->tags : 0;
	unsigned int target;
	if (c == selmon->sel) {
		selmon->sel = NULL;
	}

	setmon(c, m, newtags, true);
	client_update_oldmonname_record(c, m);

	reset_foreign_tolevel(c);
	// 重新计算居中的坐标
	if (c->isfloating) {
		c->geom.width =
			(int)(c->geom.width * c->mon->w.width / selmon->w.width);
		c->geom.height =
			(int)(c->geom.height * c->mon->w.height / selmon->w.height);
		selmon = c->mon;
		c->geom = setclient_coordinate_center(c, c->geom, 0, 0);
		target = get_tags_first_tag(c->tags);
		view(&(Arg){.ui = target}, true);
		focusclient(c, 1);
		c->oldgeom = c->geom;
		resize(c, c->geom, 1);
	} else {
		selmon = c->mon;
		target = get_tags_first_tag(c->tags);
		view(&(Arg){.ui = target}, true);
		focusclient(c, 1);
		arrange(selmon, false);
	}
	warp_cursor_to_selmon(c->mon);
}

void tagsilent(const Arg *arg) {
	Client *fc;
	Client *target_client = selmon->sel;
	target_client->tags = arg->ui & TAGMASK;
	wl_list_for_each(fc, &clients, link) {
		if (fc && fc != target_client && target_client->tags & fc->tags &&
			ISFULLSCREEN(fc) && !target_client->isfloating) {
			clear_fullscreen_flag(fc);
		}
	}
	focusclient(focustop(selmon), 1);
	arrange(target_client->mon, false);
}

void tagtoleft(const Arg *arg) {
	if (selmon->sel != NULL &&
		__builtin_popcount(selmon->tagset[selmon->seltags] & TAGMASK) == 1 &&
		selmon->tagset[selmon->seltags] > 1) {
		tag(&(Arg){.ui = selmon->tagset[selmon->seltags] >> 1});
	}
}

void tagtoright(const Arg *arg) {
	if (selmon->sel != NULL &&
		__builtin_popcount(selmon->tagset[selmon->seltags] & TAGMASK) == 1 &&
		selmon->tagset[selmon->seltags] & (TAGMASK >> 1)) {
		tag(&(Arg){.ui = selmon->tagset[selmon->seltags] << 1});
	}
}

void toggle_named_scratchpad(const Arg *arg) {
	Client *target_client = NULL;
	char *arg_id = arg->v;
	char *arg_title = arg->v2;

	target_client = get_client_by_id_or_title(arg_id, arg_title);

	if (!target_client && arg->v3) {
		Arg arg_spawn = {.v = arg->v3};
		spawn(&arg_spawn);
		return;
	}

	target_client->isnamedscratchpad = 1;
	target_client->scratchpad_width = arg->ui;
	target_client->scratchpad_height = arg->ui2;

	apply_named_scratchpad(target_client);
}
void toggle_render_border(const Arg *arg) {
	render_border = !render_border;
	arrange(selmon, false);
}
void toggle_scratchpad(const Arg *arg) {
	Client *c;
	bool hit = false;
	Client *tmp = NULL;
	wl_list_for_each_safe(c, tmp, &clients, link) {
		if (c->mon != selmon) {
			continue;
		}

		if (single_scratchpad && c->isnamedscratchpad && !c->isminied) {
			set_minized(c);
			continue;
		}

		if (c->isnamedscratchpad)
			continue;

		if (hit)
			continue;

		hit = switch_scratchpad_client_state(c);
	}
}

void togglefakefullscreen(const Arg *arg) {
	Client *sel = focustop(selmon);
	if (sel)
		setfakefullscreen(sel, !sel->isfakefullscreen);
}
void togglefloating(const Arg *arg) {
	Client *sel = focustop(selmon);

	if (!sel)
		return;

	if ((sel->isfullscreen || sel->ismaxmizescreen)) {
		sel->isfloating = 1;
	} else {
		sel->isfloating = !sel->isfloating;
	}

	setfloating(sel, sel->isfloating);
}

void togglefullscreen(const Arg *arg) {
	Client *sel = focustop(selmon);
	if (!sel)
		return;

	sel->is_scratchpad_show = 0;
	sel->is_in_scratchpad = 0;
	sel->isnamedscratchpad = 0;

	if (sel->isfullscreen || sel->ismaxmizescreen)
		setfullscreen(sel, 0);
	else
		setfullscreen(sel, 1);
}

void toggleglobal(const Arg *arg) {
	if (!selmon->sel)
		return;
	if (selmon->sel->is_in_scratchpad) {
		selmon->sel->is_in_scratchpad = 0;
		selmon->sel->is_scratchpad_show = 0;
		selmon->sel->isnamedscratchpad = 0;
	}
	selmon->sel->isglobal ^= 1;
	//   selmon->sel->tags =
	//       selmon->sel->isglobal ? TAGMASK : selmon->tagset[selmon->seltags];
	//   focustop(selmon);
	setborder_color(selmon->sel);
}

void togglegaps(const Arg *arg) {
	enablegaps ^= 1;
	arrange(selmon, false);
}

void togglemaxmizescreen(const Arg *arg) {
	Client *sel = focustop(selmon);
	if (!sel)
		return;

	sel->is_scratchpad_show = 0;
	sel->is_in_scratchpad = 0;
	sel->isnamedscratchpad = 0;

	if (sel->isfullscreen || sel->ismaxmizescreen)
		setmaxmizescreen(sel, 0);
	else
		setmaxmizescreen(sel, 1);
}
void toggleoverlay(const Arg *arg) {
	if (!selmon->sel || !selmon->sel->mon || selmon->sel->isfullscreen) {
		return;
	}

	selmon->sel->isoverlay ^= 1;

	if (selmon->sel->isoverlay) {
		wlr_scene_node_reparent(&selmon->sel->scene->node, layers[LyrOverlay]);
		wlr_scene_node_raise_to_top(&selmon->sel->scene->node);
	} else if (client_should_overtop(selmon->sel) && selmon->sel->isfloating) {
		wlr_scene_node_reparent(&selmon->sel->scene->node,
								layers[LyrFSorOverTop]);
	} else {
		wlr_scene_node_reparent(
			&selmon->sel->scene->node,
			layers[selmon->sel->isfloating ? LyrFloat : LyrTile]);
	}
	setborder_color(selmon->sel);
}
void toggletag(const Arg *arg) {
	unsigned int newtags;
	Client *sel = focustop(selmon);
	if (!sel)
		return;
	newtags = sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		sel->tags = newtags;
		focusclient(focustop(selmon), 1);
		arrange(selmon, false);
	}
	printstatus();
}

void toggleview(const Arg *arg) {
	unsigned int newtagset =
		selmon ? selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK) : 0;

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		focusclient(focustop(selmon), 1);
		arrange(selmon, false);
	}
	printstatus();
}
void viewtoleft(const Arg *arg) {
	unsigned int tmptag;
	unsigned int target = selmon->tagset[selmon->seltags];

	if (selmon->isoverview || selmon->pertag->curtag == 0) {
		return;
	}

	target >>= 1;

	if (target == 0) {
		return;
	}

	if (!selmon || (target) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (target) {
		selmon->tagset[selmon->seltags] = target;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = selmon->pertag->curtag - 1;
	} else {
		tmptag = selmon->pertag->prevtag;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = tmptag;
	}

	focusclient(focustop(selmon), 1);
	arrange(selmon, true);
	printstatus();
}
void viewtoright(const Arg *arg) {
	if (selmon->isoverview || selmon->pertag->curtag == 0) {
		return;
	}
	unsigned int tmptag;
	unsigned int target = selmon->tagset[selmon->seltags];
	target <<= 1;

	if (!selmon || (target) == selmon->tagset[selmon->seltags])
		return;
	if (!(target & TAGMASK)) {
		return;
	}
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (target) {
		selmon->tagset[selmon->seltags] = target;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = selmon->pertag->curtag + 1;
	} else {
		tmptag = selmon->pertag->prevtag;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = tmptag;
	}

	focusclient(focustop(selmon), 1);
	arrange(selmon, true);
	printstatus();
}
void viewtoleft_have_client(const Arg *arg) {
	unsigned int tmptag;
	Client *c;
	unsigned int found = 0;
	unsigned int n = 1;
	unsigned int target = selmon->tagset[selmon->seltags];

	if (selmon->isoverview || selmon->pertag->curtag == 0) {
		return;
	}

	for (target >>= 1; target > 0 && n <= LENGTH(tags); target >>= 1, n++) {
		wl_list_for_each(c, &clients, link) {
			if (c->mon == selmon && target & c->tags) {
				found = 1;
				break;
			}
		}
		if (found) {
			break;
		}
	}

	if (target == 0) {
		return;
	}

	if (!selmon || (target) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */

	int new_tag = selmon->pertag->curtag - n;
	if (new_tag < 1)
		new_tag = 1;

	if (target) {
		selmon->tagset[selmon->seltags] = target;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = new_tag;
	} else {
		tmptag = selmon->pertag->prevtag;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = tmptag;
	}

	focusclient(focustop(selmon), 1);
	arrange(selmon, true);
	printstatus();
}
void viewtoright_have_client(const Arg *arg) {
	unsigned int tmptag;
	Client *c;
	unsigned int found = 0;
	unsigned int n = 1;
	unsigned int target = selmon->tagset[selmon->seltags];

	if (selmon->isoverview || selmon->pertag->curtag == 0) {
		return;
	}

	for (target <<= 1; target & TAGMASK && n <= LENGTH(tags);
		 target <<= 1, n++) {
		wl_list_for_each(c, &clients, link) {
			if (c->mon == selmon && target & c->tags) {
				found = 1;
				break;
			}
		}
		if (found) {
			break;
		}
	}

	if (!(target & TAGMASK)) {
		return;
	}

	if (!selmon || (target) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */

	int new_tag = selmon->pertag->curtag + n;
	if (new_tag > LENGTH(tags))
		new_tag = LENGTH(tags);

	if (target) {
		selmon->tagset[selmon->seltags] = target;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = new_tag;
	} else {
		tmptag = selmon->pertag->prevtag;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = tmptag;
	}

	focusclient(focustop(selmon), 1);
	arrange(selmon, true);
	printstatus();
}
void zoom(const Arg *arg) {
	Client *c, *sel = focustop(selmon);

	if (!sel || !selmon ||
		!selmon->pertag->ltidxs[selmon->pertag->curtag]->arrange ||
		sel->isfloating)
		return;

	/* Search for the first tiled window that is not sel, marking sel as
	 * NULL if we pass it along the way */
	wl_list_for_each(c, &clients,
					 link) if (VISIBLEON(c, selmon) && !c->isfloating) {
		if (c != sel)
			break;
		sel = NULL;
	}

	/* Return if no other tiled window was found */
	if (&c->link == &clients)
		return;

	/* If we passed sel, move c to the front; otherwise, move sel to the
	 * front */
	if (!sel)
		sel = c;
	wl_list_remove(&sel->link);
	wl_list_insert(&clients, &sel->link);

	focusclient(sel, 1);
	arrange(selmon, false);
}
