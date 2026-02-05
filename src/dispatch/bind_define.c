#include "../type.h"
#include "../layout/layout.h"
#include "../config/parse_config.h"
#include "bind_define.h"
#include "../client/client.h"

extern Server server;
extern Config config;
extern Layout layouts[];
extern const uint32_t layouts_len;

int32_t bind_to_view(const Arg *arg) {

	uint32_t target = arg->ui;

	if (view_current_to_back && server.selmon->pertag->curtag &&
		(target & TAGMASK) == (server.selmon->tagset[server.selmon->seltags])) {
		if (server.selmon->pertag->prevtag)
			target = 1 << (server.selmon->pertag->prevtag - 1);
		else
			target = 0;
	}

	if (!view_current_to_back &&
		(target & TAGMASK) == (server.selmon->tagset[server.selmon->seltags])) {
		return 0;
	}

	if ((int32_t)target == INT_MIN && server.selmon->pertag->curtag == 0) {
		if (view_current_to_back && server.selmon->pertag->prevtag)
			target = 1 << (server.selmon->pertag->prevtag - 1);
		else
			target = 0;
	}

	if (target == 0 || (int32_t)target == INT_MIN) {
		view(&(Arg){.ui = ~0 & TAGMASK, .i = arg->i}, false);
	} else {
		view(&(Arg){.ui = target, .i = arg->i}, true);
	}
	return 0;
}

int32_t chvt(const Arg *arg) {
	struct timespec ts;

	// prevent the animation to rquest the new frame
	server.allow_frame_scheduling = false;

	// backup current tag and monitor name
	if (server.selmon) {
		server.chvt_backup_tag = server.selmon->pertag->curtag;
		strncpy(server.chvt_backup_selmon, server.selmon->wlr_output->name,
				sizeof(server.chvt_backup_selmon) - 1);
	}

	wlr_session_change_vt(server.session, arg->ui);

	// wait for DRM device to stabilize and ensure the server.session state is
	// inactive
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000; // 200ms
	nanosleep(&ts, NULL);

	// allow frame scheduling,
	// because server.session state is now inactive, rendermon will not enter
	server.allow_frame_scheduling = true;
	return 1;
}

int32_t create_virtual_output(const Arg *arg) {

	if (!wlr_backend_is_multi(server.backend)) {
		wlr_log(WLR_ERROR, "Expected a multi server.backend");
		return 0;
	}

	bool done = false;
	wlr_multi_for_each_backend(server.backend, create_output, &done);

	if (!done) {
		wlr_log(WLR_ERROR, "Failed to create virtual output");
		return 0;
	}

	wlr_log(WLR_INFO, "Virtual output created");
	return 0;
}

int32_t destroy_all_virtual_output(const Arg *arg) {

	if (!wlr_backend_is_multi(server.backend)) {
		wlr_log(WLR_ERROR, "Expected a multi server.backend");
		return 0;
	}

	Monitor *m, *tmp;
	wl_list_for_each_safe(m, tmp, &server.mons, link) {
		if (wlr_output_is_headless(m->wlr_output)) {
			// if(server.selmon  == m)
			//   server.selmon  = NULL;
			wlr_output_destroy(m->wlr_output);
			wlr_log(WLR_INFO, "Virtual output destroyed");
		}
	}
	return 0;
}

int32_t defaultgaps(const Arg *arg) {
	setgaps(gappoh, gappov, gappih, gappiv);
	return 0;
}

int32_t exchange_client(const Arg *arg) {
	Client *c = server.selmon->sel;
	if (!c || c->isfloating)
		return 0;

	if ((c->isfullscreen || c->ismaximizescreen) && !is_scroller_layout(c->mon))
		return 0;

	exchange_two_client(c, direction_select(arg));
	return 0;
}

int32_t exchange_stack_client(const Arg *arg) {
	Client *c = server.selmon->sel;
	Client *tc = NULL;
	if (!c || c->isfloating || c->isfullscreen || c->ismaximizescreen)
		return 0;
	if (arg->i == NEXT) {
		tc = get_next_stack_client(c, false);
	} else {
		tc = get_next_stack_client(c, true);
	}
	if (tc)
		exchange_two_client(c, tc);
	return 0;
}

int32_t focusdir(const Arg *arg) {
	Client *c = NULL;
	c = direction_select(arg);
	c = get_focused_stack_client(c);
	if (c) {
		focusclient(c, 1);
		if (warpcursor)
			warp_cursor(c);
	} else {
		if (config.focus_cross_tag) {
			if (arg->i == LEFT || arg->i == UP)
				viewtoleft_have_client(&(Arg){0});
			if (arg->i == RIGHT || arg->i == DOWN)
				viewtoright_have_client(&(Arg){0});
		} else if (config.focus_cross_monitor) {
			focusmon(arg);
		}
	}
	return 0;
}

int32_t focuslast(const Arg *arg) {

	Client *c = NULL;
	Client *tc = NULL;
	bool begin = false;
	uint32_t target = 0;

	wl_list_for_each(c, &server.fstack, flink) {
		if (c->iskilling || c->isminimized || c->isunglobal ||
			!client_surface(c)->mapped || client_is_unmanaged(c) ||
			client_is_x11_popup(c))
			continue;

		if (server.selmon && !server.selmon->sel) {
			tc = c;
			break;
		}

		if (server.selmon && c == server.selmon->sel && !begin) {
			begin = true;
			continue;
		}

		if (begin) {
			tc = c;
			break;
		}
	}

	if (!tc || !client_surface(tc)->mapped)
		return 0;

	if ((int32_t)tc->tags > 0) {
		focusclient(tc, 1);
		target = get_tags_first_tag(tc->tags);
		view(&(Arg){.ui = target}, true);
	}
	return 0;
}

int32_t toggle_trackpad_enable(const Arg *arg) {
	disable_trackpad = !disable_trackpad;
	return 0;
}

int32_t focusmon(const Arg *arg) {
	Client *c = NULL;
	Monitor *m = NULL;
	Monitor *tm = NULL;

	if (arg->i != UNDIR) {
		tm = dirtomon(arg->i);
	} else if (arg->v) {
		wl_list_for_each(m, &server.mons, link) {
			if (!m->wlr_output->enabled) {
				continue;
			}
			if (regex_match(arg->v, m->wlr_output->name)) {
				tm = m;
				break;
			}
		}
	} else {
		return 0;
	}

	if (!tm || !tm->wlr_output->enabled || tm == server.selmon)
		return 0;

	server.selmon = tm;
	if (warpcursor) {
		warp_cursor_to_selmon(server.selmon);
	}
	c = focustop(server.selmon);
	if (!c) {
		server.selmon->sel = NULL;
		wlr_seat_pointer_notify_clear_focus(server.seat);
		wlr_seat_keyboard_notify_clear_focus(server.seat);
		focusclient(NULL, 0);
	} else
		focusclient(c, 1);

	return 0;
}

int32_t focusstack(const Arg *arg) {
	/* Focus the next or previous client (in tiling order) on server.selmon  */
	Client *sel = focustop(server.selmon);
	Client *tc = NULL;

	if (!sel)
		return 0;
	if (arg->i == NEXT) {
		tc = get_next_stack_client(sel, false);
	} else {
		tc = get_next_stack_client(sel, true);
	}
	/* If only one client is visible on server.selmon , then c == sel */

	if (!tc)
		return 0;

	focusclient(tc, 1);
	if (warpcursor)
		warp_cursor(tc);
	return 0;
}

int32_t incnmaster(const Arg *arg) {
	if (!arg || !server.selmon)
		return 0;
	server.selmon->pertag->nmasters[server.selmon->pertag->curtag] = MAX(
		server.selmon->pertag->nmasters[server.selmon->pertag->curtag] + arg->i,
		0);
	arrange(server.selmon, false, false);
	return 0;
}

int32_t incgaps(const Arg *arg) {
	setgaps(server.selmon->gappoh + arg->i, server.selmon->gappov + arg->i,
			server.selmon->gappih + arg->i, server.selmon->gappiv + arg->i);
	return 0;
}

int32_t incigaps(const Arg *arg) {
	setgaps(server.selmon->gappoh, server.selmon->gappov,
			server.selmon->gappih + arg->i, server.selmon->gappiv + arg->i);
	return 0;
}

int32_t incogaps(const Arg *arg) {
	setgaps(server.selmon->gappoh + arg->i, server.selmon->gappov + arg->i,
			server.selmon->gappih, server.selmon->gappiv);
	return 0;
}

int32_t incihgaps(const Arg *arg) {
	setgaps(server.selmon->gappoh, server.selmon->gappov,
			server.selmon->gappih + arg->i, server.selmon->gappiv);
	return 0;
}

int32_t incivgaps(const Arg *arg) {
	setgaps(server.selmon->gappoh, server.selmon->gappov, server.selmon->gappih,
			server.selmon->gappiv + arg->i);
	return 0;
}

int32_t incohgaps(const Arg *arg) {
	setgaps(server.selmon->gappoh + arg->i, server.selmon->gappov,
			server.selmon->gappih, server.selmon->gappiv);
	return 0;
}

int32_t incovgaps(const Arg *arg) {
	setgaps(server.selmon->gappoh, server.selmon->gappov + arg->i,
			server.selmon->gappih, server.selmon->gappiv);
	return 0;
}

int32_t setmfact(const Arg *arg) {
	float f;
	Client *c = NULL;

	if (!arg || !server.selmon ||
		!server.selmon->pertag->ltidxs[server.selmon->pertag->curtag]->arrange)
		return 0;
	f = arg->f < 1.0
			? arg->f +
				  server.selmon->pertag->mfacts[server.selmon->pertag->curtag]
			: arg->f - 1.0;
	if (f < 0.1 || f > 0.9)
		return 0;

	server.selmon->pertag->mfacts[server.selmon->pertag->curtag] = f;
	wl_list_for_each(c, &server.clients, link) {
		if (VISIBLEON(c, server.selmon) && ISTILED(c)) {
			c->master_mfact_per = f;
		}
	}
	arrange(server.selmon, false, false);
	return 0;
}

int32_t killclient(const Arg *arg) {
	Client *c = NULL;
	c = server.selmon->sel;
	if (c) {
		pending_kill_client(c);
	}
	return 0;
}

int32_t moveresize(const Arg *arg) {
	const char *cursors[] = {"nw-resize", "ne-resize", "sw-resize",
							 "se-resize"};

	if (server.cursor_mode != CurNormal && server.cursor_mode != CurPressed)
		return 0;
	xytonode(server.cursor->x, server.cursor->y, NULL, &server.grabc, NULL,
			 NULL, NULL);
	if (!server.grabc || client_is_unmanaged(server.grabc) ||
		server.grabc->isfullscreen || server.grabc->ismaximizescreen) {
		server.grabc = NULL;
		return 0;
	}
	/* Float the window and tell motionnotify to grab it */
	if (server.grabc->isfloating == 0 && arg->ui == CurMove) {
		server.grabc->drag_to_tile = true;
		setfloating(server.grabc, 1);
	}

	switch (server.cursor_mode = arg->ui) {
	case CurMove:

		server.grabcx = server.cursor->x - server.grabc->geom.x;
		server.grabcy = server.cursor->y - server.grabc->geom.y;
		wlr_cursor_set_xcursor(server.cursor, server.cursor_mgr, "grab");
		break;
	case CurResize:
		/* Doesn't work for X11 output - the next absolute motion event
		 * returns the server.cursor to where it started */
		if (server.grabc->isfloating) {
			server.rzcorner = drag_corner;
			server.grabcx = (int)round(server.cursor->x);
			server.grabcy = (int)round(server.cursor->y);
			if (server.rzcorner == 4)
				/* identify the closest corner index */
				server.rzcorner =
					(server.grabcx - server.grabc->geom.x <
							 server.grabc->geom.x + server.grabc->geom.width -
								 server.grabcx
						 ? 0
						 : 1) +
					(server.grabcy - server.grabc->geom.y <
							 server.grabc->geom.y + server.grabc->geom.height -
								 server.grabcy
						 ? 0
						 : 2);

			if (drag_warp_cursor) {
				server.grabcx =
					server.rzcorner & 1
						? server.grabc->geom.x + server.grabc->geom.width
						: server.grabc->geom.x;
				server.grabcy =
					server.rzcorner & 2
						? server.grabc->geom.y + server.grabc->geom.height
						: server.grabc->geom.y;
				wlr_cursor_warp_closest(server.cursor, NULL, server.grabcx,
										server.grabcy);
			}

			wlr_cursor_set_xcursor(server.cursor, server.cursor_mgr,
								   cursors[server.rzcorner]);
		} else {
			wlr_cursor_set_xcursor(server.cursor, server.cursor_mgr, "grab");
		}
		break;
	}
	return 0;
}

int32_t movewin(const Arg *arg) {
	Client *c = NULL;
	c = server.selmon->sel;
	if (!c || c->isfullscreen)
		return 0;
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

	c->iscustomsize = 1;
	c->float_geom = c->geom;
	resize(c, c->geom, 0);
	return 0;
}

int32_t quit(const Arg *arg) {
	wl_display_terminate(server.dpy);
	return 0;
}

int32_t resizewin(const Arg *arg) {
	Client *c = NULL;
	c = server.selmon->sel;
	int32_t offsetx = 0, offsety = 0;

	if (!c || c->isfullscreen || c->ismaximizescreen)
		return 0;

	int32_t animations_state_backup = animations;
	if (!c->isfloating)
		animations = 0;

	if (ISTILED(c)) {
		switch (arg->ui) {
		case NUM_TYPE_MINUS:
			offsetx = -arg->i;
			break;
		case NUM_TYPE_PLUS:
			offsetx = arg->i;
			break;
		default:
			offsetx = arg->i;
			break;
		}

		switch (arg->ui2) {
		case NUM_TYPE_MINUS:
			offsety = -arg->i2;
			break;
		case NUM_TYPE_PLUS:
			offsety = arg->i2;
			break;
		default:
			offsety = arg->i2;
			break;
		}
		resize_tile_client(c, false, offsetx, offsety, 0);
		animations = animations_state_backup;
		return 0;
	}

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

	c->iscustomsize = 1;
	c->float_geom = c->geom;
	resize(c, c->geom, 0);
	animations = animations_state_backup;
	return 0;
}

int32_t restore_minimized(const Arg *arg) {
	Client *c = NULL;

	if (server.selmon && server.selmon->isoverview)
		return 0;

	if (server.selmon && server.selmon->sel &&
		server.selmon->sel->is_in_scratchpad &&
		server.selmon->sel->is_scratchpad_show) {
		server.selmon->sel->isminimized = 0;
		server.selmon->sel->is_scratchpad_show = 0;
		server.selmon->sel->is_in_scratchpad = 0;
		server.selmon->sel->isnamedscratchpad = 0;
		setborder_color(server.selmon->sel);
		return 0;
	}

	wl_list_for_each(c, &server.clients, link) {
		if (c->isminimized && !c->isnamedscratchpad) {
			c->is_scratchpad_show = 0;
			c->is_in_scratchpad = 0;
			c->isnamedscratchpad = 0;
			show_hide_client(c);
			setborder_color(c);
			arrange(c->mon, false, false);
			focusclient(c, 0);
			warp_cursor(c);
			return 0;
		}
	}
	return 0;
}

int32_t setlayout(const Arg *arg) {
	int32_t jk;

	for (jk = 0; jk < layouts_len; jk++) {
		if (strcmp(layouts[jk].name, arg->v) == 0) {
			server.selmon->pertag->ltidxs[server.selmon->pertag->curtag] =
				&layouts[jk];
			clear_fullscreen_and_maximized_state(server.selmon);
			arrange(server.selmon, false, false);
			printstatus();
			return 0;
		}
	}
	return 0;
}

int32_t setkeymode(const Arg *arg) {
	free(server.keymode.name);
	server.keymode.name = strdup(arg->v);
	if (strcmp(server.keymode.name, "default") == 0) {
		server.keymode.is_default = true;
	} else {
		server.keymode.is_default = false;
	}
	if (strcmp(server.keymode.name, "common") == 0) {
		server.keymode.is_common = true;
	} else {
		server.keymode.is_common = false;
	}
	printstatus();
	return 1;
}

int32_t set_proportion(const Arg *arg) {

	if (server.selmon->isoverview || !is_scroller_layout(server.selmon))
		return 0;

	if (server.selmon->visible_tiling_clients == 1 &&
		!scroller_ignore_proportion_single)
		return 0;

	Client *tc = server.selmon->sel;

	if (tc) {
		tc = get_scroll_stack_head(tc);
		uint32_t max_client_width =
			server.selmon->w.width - 2 * scroller_structs - gappih;
		tc->scroller_proportion = arg->f;
		tc->geom.width = max_client_width * arg->f;
		arrange(server.selmon, false, false);
	}
	return 0;
}

int32_t smartmovewin(const Arg *arg) {
	Client *c = NULL, *tc = NULL;
	int32_t nx, ny;
	int32_t buttom, top, left, right, tar;
	c = server.selmon->sel;
	if (!c || c->isfullscreen)
		return 0;
	if (!c->isfloating)
		setfloating(server.selmon->sel, true);
	nx = c->geom.x;
	ny = c->geom.y;

	switch (arg->i) {
	case UP:
		tar = -99999;
		top = c->geom.y;
		ny -= c->mon->w.height / 4;

		wl_list_for_each(tc, &server.clients, link) {
			if (!VISIBLEON(tc, server.selmon) || !tc->isfloating || tc == c)
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

		wl_list_for_each(tc, &server.clients, link) {
			if (!VISIBLEON(tc, server.selmon) || !tc->isfloating || tc == c)
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

		wl_list_for_each(tc, &server.clients, link) {
			if (!VISIBLEON(tc, server.selmon) || !tc->isfloating || tc == c)
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
		wl_list_for_each(tc, &server.clients, link) {
			if (!VISIBLEON(tc, server.selmon) || !tc->isfloating || tc == c)
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

	c->float_geom = (struct wlr_box){
		.x = nx, .y = ny, .width = c->geom.width, .height = c->geom.height};
	c->iscustomsize = 1;
	resize(c, c->float_geom, 1);
	return 0;
}

int32_t smartresizewin(const Arg *arg) {
	Client *c = NULL, *tc = NULL;
	int32_t nw, nh;
	int32_t buttom, top, left, right, tar;
	c = server.selmon->sel;
	if (!c || c->isfullscreen)
		return 0;
	if (!c->isfloating)
		setfloating(c, true);
	nw = c->geom.width;
	nh = c->geom.height;

	switch (arg->i) {
	case UP:
		nh -= server.selmon->w.height / 8;
		nh = MAX(nh, server.selmon->w.height / 10);
		break;
	case DOWN:
		tar = -99999;
		buttom = c->geom.y + c->geom.height;
		nh += server.selmon->w.height / 8;

		wl_list_for_each(tc, &server.clients, link) {
			if (!VISIBLEON(tc, server.selmon) || !tc->isfloating || tc == c)
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
		if (c->geom.y + nh + gappov >
			server.selmon->w.y + server.selmon->w.height)
			nh = server.selmon->w.y + server.selmon->w.height - c->geom.y -
				 gappov;
		break;
	case LEFT:
		nw -= server.selmon->w.width / 16;
		nw = MAX(nw, server.selmon->w.width / 10);
		break;
	case RIGHT:
		tar = 99999;
		right = c->geom.x + c->geom.width;
		nw += server.selmon->w.width / 16;
		wl_list_for_each(tc, &server.clients, link) {
			if (!VISIBLEON(tc, server.selmon) || !tc->isfloating || tc == c)
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
		if (c->geom.x + nw + gappoh >
			server.selmon->w.x + server.selmon->w.width)
			nw = server.selmon->w.x + server.selmon->w.width - c->geom.x -
				 gappoh;
		break;
	}

	c->float_geom = (struct wlr_box){
		.x = c->geom.x, .y = c->geom.y, .width = nw, .height = nh};
	c->iscustomsize = 1;
	resize(c, c->float_geom, 1);
	return 0;
}

int32_t centerwin(const Arg *arg) {
	Client *c = NULL;
	c = server.selmon->sel;

	if (!c || c->isfullscreen || c->ismaximizescreen)
		return 0;

	if (c->isfloating) {
		c->float_geom = setclient_coordinate_center(c, c->mon, c->geom, 0, 0);
		c->iscustomsize = 1;
		resize(c, c->float_geom, 1);
		return 0;
	}

	if (!is_scroller_layout(server.selmon))
		return 0;

	Client *stack_head = get_scroll_stack_head(c);
	if (server.selmon->pertag->ltidxs[server.selmon->pertag->curtag]->id ==
		SCROLLER) {
		stack_head->geom.x =
			server.selmon->w.x +
			(server.selmon->w.width - stack_head->geom.width) / 2;
	} else {
		stack_head->geom.y =
			server.selmon->w.y +
			(server.selmon->w.height - stack_head->geom.height) / 2;
	}

	arrange(server.selmon, false, false);
	return 0;
}

int32_t spawn_shell(const Arg *arg) {
	if (!arg->v)
		return 0;

	if (fork() == 0) {
		// 1. 忽略可能导致 coredump 的信号
		signal(SIGSEGV, SIG_IGN);
		signal(SIGABRT, SIG_IGN);
		signal(SIGILL, SIG_IGN);

		dup2(STDERR_FILENO, STDOUT_FILENO);
		setsid();

		execlp("sh", "sh", "-c", arg->v, (char *)NULL);

		// fallback to bash
		execlp("bash", "bash", "-c", arg->v, (char *)NULL);

		// if execlp fails, we should not reach here
		wlr_log(WLR_ERROR,
				"mango: failed to execute command '%s' with shell: %s\n",
				arg->v, strerror(errno));
		_exit(EXIT_FAILURE);
	}
	return 0;
}

int32_t spawn(const Arg *arg) {

	if (!arg->v)
		return 0;

	if (fork() == 0) {
		// 1. 忽略可能导致 coredump 的信号
		signal(SIGSEGV, SIG_IGN);
		signal(SIGABRT, SIG_IGN);
		signal(SIGILL, SIG_IGN);

		dup2(STDERR_FILENO, STDOUT_FILENO);
		setsid();

		// 2. 解析参数
		char *argv[64];
		int32_t argc = 0;
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
		wlr_log(WLR_ERROR, "mango: execvp '%s' failed: %s\n", argv[0],
				strerror(errno));
		_exit(EXIT_FAILURE); // 使用 _exit 避免缓冲区刷新等操作
	}
	return 0;
}

int32_t spawn_on_empty(const Arg *arg) {
	bool is_empty = true;
	Client *c = NULL;

	wl_list_for_each(c, &server.clients, link) {
		if (arg->ui & c->tags && c->mon == server.selmon) {
			is_empty = false;
			break;
		}
	}
	if (!is_empty) {
		view(arg, true);
		return 0;
	} else {
		view(arg, true);
		spawn_shell(arg);
	}
	return 0;
}

int32_t switch_keyboard_layout(const Arg *arg) {
	if (!server.kb_group || !server.kb_group->wlr_group || !server.seat) {
		wlr_log(WLR_ERROR, "Invalid keyboard group or server.seat ");
		return 0;
	}

	struct wlr_keyboard *keyboard = &server.kb_group->wlr_group->keyboard;
	if (!keyboard || !keyboard->keymap) {
		wlr_log(WLR_ERROR, "Invalid keyboard or keymap");
		return 0;
	}

	// 1. 获取当前布局和计算下一个布局
	xkb_layout_index_t current = xkb_state_serialize_layout(
		keyboard->xkb_state, XKB_STATE_LAYOUT_EFFECTIVE);
	const int32_t num_layouts = xkb_keymap_num_layouts(keyboard->keymap);
	if (num_layouts < 2) {
		wlr_log(WLR_INFO, "Only one layout available");
		return 0;
	}
	xkb_layout_index_t next = (current + 1) % num_layouts;

	// 6. 应用新 keymap
	uint32_t depressed = keyboard->modifiers.depressed;
	uint32_t latched = keyboard->modifiers.latched;
	// uint32_t locked = keyboard->modifiers.locked;

	wlr_keyboard_set_keymap(keyboard, keyboard->keymap);
	wlr_keyboard_notify_modifiers(keyboard, depressed, latched, server.locked,
								  next);
	keyboard->modifiers.group = 0;

	// 7. 更新 server.seat
	wlr_seat_set_keyboard(server.seat, keyboard);
	wlr_seat_keyboard_notify_modifiers(server.seat, &keyboard->modifiers);

	InputDevice *id;
	wl_list_for_each(id, &server.inputdevices, link) {
		if (id->wlr_device->type != WLR_INPUT_DEVICE_KEYBOARD) {
			continue;
		}

		struct wlr_keyboard *tkb = (struct wlr_keyboard *)id->device_data;

		wlr_keyboard_set_keymap(tkb, keyboard->keymap);
		wlr_keyboard_notify_modifiers(tkb, depressed, latched, server.locked,
									  next);
		tkb->modifiers.group = 0;

		// 7. 更新 server.seat
		wlr_seat_set_keyboard(server.seat, tkb);
		wlr_seat_keyboard_notify_modifiers(server.seat, &tkb->modifiers);
	}

	printstatus();
	return 0;
}

int32_t switch_layout(const Arg *arg) {

	int32_t jk, ji;
	char *target_layout_name = NULL;
	uint32_t len;

	if (config.circle_layout_count != 0) {
		for (jk = 0; jk < config.circle_layout_count; jk++) {

			len = MAX(strlen(config.circle_layout[jk]),
					  strlen(server.selmon->pertag
								 ->ltidxs[server.selmon->pertag->curtag]
								 ->name));

			if (strncmp(
					config.circle_layout[jk],
					server.selmon->pertag->ltidxs[server.selmon->pertag->curtag]
						->name,
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

		for (ji = 0; ji < layouts_len; ji++) {
			len = MAX(strlen(layouts[ji].name), strlen(target_layout_name));
			if (strncmp(layouts[ji].name, target_layout_name, len) == 0) {
				server.selmon->pertag->ltidxs[server.selmon->pertag->curtag] =
					&layouts[ji];

				break;
			}
		}
		clear_fullscreen_and_maximized_state(server.selmon);
		arrange(server.selmon, false, false);
		printstatus();
		return 0;
	}

	for (jk = 0; jk < layouts_len; jk++) {
		if (strcmp(layouts[jk].name,
				   server.selmon->pertag->ltidxs[server.selmon->pertag->curtag]
					   ->name) == 0) {
			server.selmon->pertag->ltidxs[server.selmon->pertag->curtag] =
				jk == layouts_len- 1 ? &layouts[0] : &layouts[jk + 1];
			clear_fullscreen_and_maximized_state(server.selmon);
			arrange(server.selmon, false, false);
			printstatus();
			return 0;
		}
	}
	return 0;
}

int32_t switch_proportion_preset(const Arg *arg) {
	float target_proportion = 0;

	if (config.scroller_proportion_preset_count == 0) {
		return 0;
	}

	if (server.selmon->isoverview || !is_scroller_layout(server.selmon))
		return 0;

	if (server.selmon->visible_tiling_clients == 1 &&
		!scroller_ignore_proportion_single)
		return 0;

	Client *tc = server.selmon->sel;

	if (tc) {
		tc = get_scroll_stack_head(tc);
		for (int32_t i = 0; i < config.scroller_proportion_preset_count; i++) {
			if (config.scroller_proportion_preset[i] ==
				tc->scroller_proportion) {
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

		uint32_t max_client_width =
			server.selmon->w.width - 2 * scroller_structs - gappih;
		tc->scroller_proportion = target_proportion;
		tc->geom.width = max_client_width * target_proportion;
		arrange(server.selmon, false, false);
	}
	return 0;
}

int32_t tag(const Arg *arg) {
	Client *target_client = server.selmon->sel;
	tag_client(arg, target_client);
	return 0;
}

int32_t tagmon(const Arg *arg) {
	Monitor *m = NULL, *cm = NULL;
	Client *c = focustop(server.selmon);

	if (!c)
		return 0;

	if (arg->i != UNDIR) {
		m = dirtomon(arg->i);
	} else if (arg->v) {
		wl_list_for_each(cm, &server.mons, link) {
			if (!cm->wlr_output->enabled) {
				continue;
			}
			if (regex_match(arg->v, cm->wlr_output->name)) {
				m = cm;
				break;
			}
		}
	} else {
		return 0;
	}

	if (!m || !m->wlr_output->enabled)
		return 0;

	uint32_t newtags = arg->ui ? arg->ui : arg->i2 ? c->tags : 0;
	uint32_t target;

	if (c->mon == m) {
		view(&(Arg){.ui = newtags}, true);
		return 0;
	}

	if (c == server.selmon->sel) {
		server.selmon->sel = NULL;
	}

	setmon(c, m, newtags, true);
	client_update_oldmonname_record(c, m);

	reset_foreign_tolevel(c);

	c->float_geom.width = (int32_t)(c->float_geom.width * c->mon->w.width /
									server.selmon->w.width);
	c->float_geom.height = (int32_t)(c->float_geom.height * c->mon->w.height /
									 server.selmon->w.height);
	server.selmon = c->mon;
	c->float_geom = setclient_coordinate_center(c, c->mon, c->float_geom, 0, 0);

	// 重新计算居中的坐标
	// 重新计算居中的坐标
	if (c->isfloating) {
		c->geom = c->float_geom;
		target = get_tags_first_tag(c->tags);
		view(&(Arg){.ui = target}, true);
		focusclient(c, 1);
		resize(c, c->geom, 1);
	} else {
		server.selmon = c->mon;
		target = get_tags_first_tag(c->tags);
		view(&(Arg){.ui = target}, true);
		focusclient(c, 1);
		arrange(server.selmon, false, false);
	}
	if (warpcursor) {
		warp_cursor_to_selmon(c->mon);
	}
	return 0;
}

int32_t tagsilent(const Arg *arg) {
	Client *fc = NULL;
	Client *target_client = NULL;

	if (!server.selmon || !server.selmon->sel)
		return 0;

	target_client = server.selmon->sel;
	target_client->tags = arg->ui & TAGMASK;
	wl_list_for_each(fc, &server.clients, link) {
		if (fc && fc != target_client && target_client->tags & fc->tags &&
			ISFULLSCREEN(fc) && !target_client->isfloating) {
			clear_fullscreen_flag(fc);
		}
	}
	exit_scroller_stack(target_client);
	focusclient(focustop(server.selmon), 1);
	arrange(target_client->mon, false, false);
	return 0;
}

int32_t tagtoleft(const Arg *arg) {
	if (server.selmon->sel != NULL &&
		__builtin_popcount(server.selmon->tagset[server.selmon->seltags] &
						   TAGMASK) == 1 &&
		server.selmon->tagset[server.selmon->seltags] > 1) {
		tag(&(Arg){.ui = server.selmon->tagset[server.selmon->seltags] >> 1,
				   .i = arg->i});
	}
	return 0;
}

int32_t tagtoright(const Arg *arg) {
	if (server.selmon->sel != NULL &&
		__builtin_popcount(server.selmon->tagset[server.selmon->seltags] &
						   TAGMASK) == 1 &&
		server.selmon->tagset[server.selmon->seltags] & (TAGMASK >> 1)) {
		tag(&(Arg){.ui = server.selmon->tagset[server.selmon->seltags] << 1,
				   .i = arg->i});
	}
	return 0;
}

int32_t toggle_named_scratchpad(const Arg *arg) {
	Client *target_client = NULL;
	char *arg_id = arg->v;
	char *arg_title = arg->v2;

	target_client = get_client_by_id_or_title(arg_id, arg_title);

	if (!target_client && arg->v3) {
		Arg arg_spawn = {.v = arg->v3};
		spawn_shell(&arg_spawn);
		return 0;
	}

	target_client->isnamedscratchpad = 1;

	apply_named_scratchpad(target_client);
	return 0;
}

int32_t toggle_render_border(const Arg *arg) {
	server.render_border = !server.render_border;
	arrange(server.selmon, false, false);
	return 0;
}

int32_t toggle_scratchpad(const Arg *arg) {
	Client *c = NULL;
	bool hit = false;
	Client *tmp = NULL;

	if (server.selmon && server.selmon->isoverview)
		return 0;

	wl_list_for_each_safe(c, tmp, &server.clients, link) {
		if (!scratchpad_cross_monitor && c->mon != server.selmon) {
			continue;
		}

		if (single_scratchpad && c->isnamedscratchpad && !c->isminimized) {
			set_minimized(c);
			continue;
		}

		if (c->isnamedscratchpad)
			continue;

		if (hit)
			continue;

		hit = switch_scratchpad_client_state(c);
	}
	return 0;
}

int32_t togglefakefullscreen(const Arg *arg) {
	Client *sel = focustop(server.selmon);
	if (sel)
		setfakefullscreen(sel, !sel->isfakefullscreen);
	return 0;
}

int32_t togglefloating(const Arg *arg) {
	Client *sel = focustop(server.selmon);

	if (server.selmon && server.selmon->isoverview)
		return 0;

	if (!sel)
		return 0;

	if ((sel->isfullscreen || sel->ismaximizescreen)) {
		sel->isfloating = 1;
	} else {
		sel->isfloating = !sel->isfloating;
	}

	setfloating(sel, sel->isfloating);
	return 0;
}

int32_t togglefullscreen(const Arg *arg) {
	Client *sel = focustop(server.selmon);
	if (!sel)
		return 0;

	sel->is_scratchpad_show = 0;
	sel->is_in_scratchpad = 0;
	sel->isnamedscratchpad = 0;

	if (sel->isfullscreen)
		setfullscreen(sel, 0);
	else
		setfullscreen(sel, 1);
	return 0;
}

int32_t toggleglobal(const Arg *arg) {
	if (!server.selmon->sel)
		return 0;
	if (server.selmon->sel->is_in_scratchpad) {
		server.selmon->sel->is_in_scratchpad = 0;
		server.selmon->sel->is_scratchpad_show = 0;
		server.selmon->sel->isnamedscratchpad = 0;
	}
	server.selmon->sel->isglobal ^= 1;
	if (server.selmon->sel->isglobal && (server.selmon->sel->prev_in_stack ||
										 server.selmon->sel->next_in_stack)) {
		exit_scroller_stack(server.selmon->sel);
		arrange(server.selmon, false, false);
	}
	setborder_color(server.selmon->sel);
	return 0;
}

int32_t togglegaps(const Arg *arg) {
	server.enablegaps ^= 1;
	arrange(server.selmon, false, false);
	return 0;
}

int32_t togglemaximizescreen(const Arg *arg) {
	Client *sel = focustop(server.selmon);
	if (!sel)
		return 0;

	sel->is_scratchpad_show = 0;
	sel->is_in_scratchpad = 0;
	sel->isnamedscratchpad = 0;

	if (sel->ismaximizescreen)
		setmaximizescreen(sel, 0);
	else
		setmaximizescreen(sel, 1);

	setborder_color(sel);
	return 0;
}

int32_t toggleoverlay(const Arg *arg) {
	if (!server.selmon->sel || !server.selmon->sel->mon ||
		server.selmon->sel->isfullscreen) {
		return 0;
	}

	server.selmon->sel->isoverlay ^= 1;

	if (server.selmon->sel->isoverlay) {
		wlr_scene_node_reparent(&server.selmon->sel->scene->node,
								server.layers[LyrOverlay]);
		wlr_scene_node_raise_to_top(&server.selmon->sel->scene->node);
	} else if (client_should_overtop(server.selmon->sel) &&
			   server.selmon->sel->isfloating) {
		wlr_scene_node_reparent(&server.selmon->sel->scene->node,
								server.layers[LyrTop]);
	} else {
		wlr_scene_node_reparent(
			&server.selmon->sel->scene->node,
			server.layers[server.selmon->sel->isfloating ? LyrTop : LyrTile]);
	}
	setborder_color(server.selmon->sel);
	return 0;
}

int32_t toggletag(const Arg *arg) {
	uint32_t newtags;
	Client *sel = focustop(server.selmon);
	if (!sel)
		return 0;

	if ((int32_t)arg->ui == INT_MIN && sel->tags != (~0 & TAGMASK)) {
		newtags = ~0 & TAGMASK;
	} else if ((int32_t)arg->ui == INT_MIN && sel->tags == (~0 & TAGMASK)) {
		newtags = 1 << (sel->mon->pertag->curtag - 1);
	} else {
		newtags = sel->tags ^ (arg->ui & TAGMASK);
	}

	if (newtags) {
		sel->tags = newtags;
		focusclient(focustop(server.selmon), 1);
		arrange(server.selmon, false, false);
	}
	printstatus();
	return 0;
}

int32_t toggleview(const Arg *arg) {
	uint32_t newtagset;
	uint32_t target;

	target = arg->ui == 0 ? ~0 & TAGMASK : arg->ui;

	newtagset = server.selmon ? server.selmon->tagset[server.selmon->seltags] ^
									(target & TAGMASK)
							  : 0;

	if (newtagset) {
		server.selmon->tagset[server.selmon->seltags] = newtagset;
		focusclient(focustop(server.selmon), 1);
		arrange(server.selmon, false, false);
	}
	printstatus();
	return 0;
}

int32_t viewtoleft(const Arg *arg) {
	uint32_t target = server.selmon->tagset[server.selmon->seltags];

	if (server.selmon->isoverview || server.selmon->pertag->curtag == 0) {
		return 0;
	}

	target >>= 1;

	if (target == 0) {
		return 0;
	}

	if (!server.selmon ||
		(target) == server.selmon->tagset[server.selmon->seltags])
		return 0;

	view(&(Arg){.ui = target & TAGMASK, .i = arg->i}, true);
	return 0;
}

int32_t viewtoright(const Arg *arg) {
	if (server.selmon->isoverview || server.selmon->pertag->curtag == 0) {
		return 0;
	}
	uint32_t target = server.selmon->tagset[server.selmon->seltags];
	target <<= 1;

	if (!server.selmon ||
		(target) == server.selmon->tagset[server.selmon->seltags])
		return 0;
	if (!(target & TAGMASK)) {
		return 0;
	}

	view(&(Arg){.ui = target & TAGMASK, .i = arg->i}, true);
	return 0;
}

int32_t viewtoleft_have_client(const Arg *arg) {
	uint32_t n;
	uint32_t current =
		get_tags_first_tag_num(server.selmon->tagset[server.selmon->seltags]);
	bool found = false;

	if (server.selmon->isoverview) {
		return 0;
	}

	if (current <= 1)
		return 0;

	for (n = current - 1; n >= 1; n--) {
		if (get_tag_status(n, server.selmon)) {
			found = true;
			break;
		}
	}

	if (found)
		view(&(Arg){.ui = (1 << (n - 1)) & TAGMASK, .i = arg->i}, true);
	return 0;
}

int32_t viewtoright_have_client(const Arg *arg) {
	uint32_t n;
	uint32_t current =
		get_tags_first_tag_num(server.selmon->tagset[server.selmon->seltags]);
	bool found = false;

	if (server.selmon->isoverview) {
		return 0;
	}

	if (current >= LENGTH(tags))
		return 0;

	for (n = current + 1; n <= LENGTH(tags); n++) {
		if (get_tag_status(n, server.selmon)) {
			found = true;
			break;
		}
	}

	if (found)
		view(&(Arg){.ui = (1 << (n - 1)) & TAGMASK, .i = arg->i}, true);
	return 0;
}

int32_t viewcrossmon(const Arg *arg) {
	focusmon(&(Arg){.v = arg->v, .i = UNDIR});
	view_in_mon(arg, true, server.selmon, true);
	return 0;
}

int32_t tagcrossmon(const Arg *arg) {
	if (!server.selmon || !server.selmon->sel)
		return 0;

	if (regex_match(server.selmon->wlr_output->name, arg->v)) {
		tag_client(arg, server.selmon->sel);
		return 0;
	}

	tagmon(&(Arg){.ui = arg->ui, .i = UNDIR, .v = arg->v});
	return 0;
}

int32_t comboview(const Arg *arg) {
	uint32_t newtags = arg->ui & TAGMASK;

	if (!newtags || !server.selmon)
		return 0;

	if (server.tag_combo) {
		server.selmon->tagset[server.selmon->seltags] |= newtags;
		focusclient(focustop(server.selmon), 1);
		arrange(server.selmon, false, false);
	} else {
		server.tag_combo = true;
		view(&(Arg){.ui = newtags}, false);
	}

	printstatus();
	return 0;
}

int32_t zoom(const Arg *arg) {
	Client *c = NULL, *sel = focustop(server.selmon);

	if (!sel || !server.selmon ||
		!server.selmon->pertag->ltidxs[server.selmon->pertag->curtag]
			 ->arrange ||
		sel->isfloating)
		return 0;

	/* Search for the first tiled window that is not sel, marking sel as
	 * NULL if we pass it along the way */
	wl_list_for_each(c, &server.clients,
					 link) if (VISIBLEON(c, server.selmon) && !c->isfloating) {
		if (c != sel)
			break;
		sel = NULL;
	}

	/* Return if no other tiled window was found */
	if (&c->link == &server.clients)
		return 0;

	/* If we passed sel, move c to the front; otherwise, move sel to the
	 * front */
	if (!sel)
		sel = c;
	wl_list_remove(&sel->link);
	wl_list_insert(&server.clients, &sel->link);

	focusclient(sel, 1);
	arrange(server.selmon, false, false);
	return 0;
}

int32_t setoption(const Arg *arg) {
	parse_option(&config, arg->v, arg->v2);
	override_config();
	reset_option();
	return 0;
}

int32_t minimized(const Arg *arg) {

	if (server.selmon && server.selmon->isoverview)
		return 0;

	if (server.selmon->sel && !server.selmon->sel->isminimized) {
		set_minimized(server.selmon->sel);
	}
	return 0;
}

int32_t toggleoverview(const Arg *arg) {
	Client *c = NULL;

	if (server.selmon->isoverview && ov_tab_mode && arg->i != 1 &&
		server.selmon->sel) {
		focusstack(&(Arg){.i = 1});
		return 0;
	}

	server.selmon->isoverview ^= 1;
	uint32_t target;
	uint32_t visible_client_number = 0;

	if (server.selmon->isoverview) {
		wl_list_for_each(c, &server.clients,
						 link) if (c && c->mon == server.selmon &&
								   !client_is_unmanaged(c) &&
								   !client_is_x11_popup(c) && !c->isminimized &&
								   !c->isunglobal) {
			visible_client_number++;
		}
		if (visible_client_number > 0) {
			target = ~0 & TAGMASK;
		} else {
			server.selmon->isoverview ^= 1;
			return 0;
		}
	} else if (!server.selmon->isoverview && server.selmon->sel) {
		target = get_tags_first_tag(server.selmon->sel->tags);
	} else if (!server.selmon->isoverview && !server.selmon->sel) {
		target = (1 << (server.selmon->pertag->prevtag - 1));
		view(&(Arg){.ui = target}, false);
		refresh_monitors_workspaces_status(server.selmon);
		return 0;
	}

	// 正常视图到overview,退出所有窗口的浮动和全屏状态参与平铺,
	// overview到正常视图,还原之前退出的浮动和全屏窗口状态
	if (server.selmon->isoverview) {
		wl_list_for_each(c, &server.clients, link) {
			if (c && c->mon == server.selmon && !client_is_unmanaged(c) &&
				!client_is_x11_popup(c) && !c->isunglobal)
				overview_backup(c);
		}
	} else {
		wl_list_for_each(c, &server.clients, link) {
			if (c && c->mon == server.selmon && !c->iskilling &&
				!client_is_unmanaged(c) && !c->isunglobal &&
				!client_is_x11_popup(c) && client_surface(c)->mapped)
				overview_restore(c, &(Arg){.ui = target});
		}
	}

	view(&(Arg){.ui = target}, false);

	refresh_monitors_workspaces_status(server.selmon);
	return 0;
}

int32_t disable_monitor(const Arg *arg) {
	Monitor *m = NULL;
	struct wlr_output_state state = {0};
	wl_list_for_each(m, &server.mons, link) {
		if (regex_match(arg->v, m->wlr_output->name)) {
			wlr_output_state_set_enabled(&state, false);
			wlr_output_commit_state(m->wlr_output, &state);
			m->asleep = 1;
			updatemons(NULL, NULL);
			break;
		}
	}
	return 0;
}

int32_t enable_monitor(const Arg *arg) {
	Monitor *m = NULL;
	struct wlr_output_state state = {0};
	wl_list_for_each(m, &server.mons, link) {
		if (regex_match(arg->v, m->wlr_output->name)) {
			wlr_output_state_set_enabled(&state, true);
			wlr_output_commit_state(m->wlr_output, &state);
			m->asleep = 0;
			updatemons(NULL, NULL);
			break;
		}
	}
	return 0;
}

int32_t toggle_monitor(const Arg *arg) {
	Monitor *m = NULL;
	struct wlr_output_state state = {0};
	wl_list_for_each(m, &server.mons, link) {
		if (regex_match(arg->v, m->wlr_output->name)) {
			wlr_output_state_set_enabled(&state, !m->wlr_output->enabled);
			wlr_output_commit_state(m->wlr_output, &state);
			m->asleep = !m->wlr_output->enabled;
			updatemons(NULL, NULL);
			break;
		}
	}
	return 0;
}

int32_t scroller_stack(const Arg *arg) {
	Client *c = server.selmon->sel;
	Client *stack_head = NULL;
	Client *source_stack_head = NULL;
	if (!c || !c->mon || c->isfloating || !is_scroller_layout(server.selmon))
		return 0;

	if (c && (!client_only_in_one_tag(c) || c->isglobal || c->isunglobal))
		return 0;

	bool is_horizontal_layout =
		c->mon->pertag->ltidxs[c->mon->pertag->curtag]->id == SCROLLER ? true
																	   : false;

	Client *target_client = find_client_by_direction(c, arg, false, true);

	if (target_client && (!client_only_in_one_tag(target_client) ||
						  target_client->isglobal || target_client->isunglobal))
		return 0;

	if (target_client) {
		stack_head = get_scroll_stack_head(target_client);
	}

	if (c) {
		source_stack_head = get_scroll_stack_head(c);
	}

	if (stack_head == source_stack_head) {
		return 0;
	}

	if (c->isfullscreen) {
		setfullscreen(c, 0);
	}

	if (c->ismaximizescreen) {
		setmaximizescreen(c, 0);
	}

	if (c->prev_in_stack) {
		if ((is_horizontal_layout && arg->i == LEFT) ||
			(!is_horizontal_layout && arg->i == UP)) {
			exit_scroller_stack(c);
			wl_list_remove(&c->link);
			wl_list_insert(source_stack_head->link.prev, &c->link);
			arrange(server.selmon, false, false);

		} else if ((is_horizontal_layout && arg->i == RIGHT) ||
				   (!is_horizontal_layout && arg->i == DOWN)) {
			exit_scroller_stack(c);
			wl_list_remove(&c->link);
			wl_list_insert(&source_stack_head->link, &c->link);
			arrange(server.selmon, false, false);
		}
		return 0;
	} else if (c->next_in_stack) {
		Client *next_in_stack = c->next_in_stack;
		if ((is_horizontal_layout && arg->i == LEFT) ||
			(!is_horizontal_layout && arg->i == UP)) {
			exit_scroller_stack(c);
			wl_list_remove(&c->link);
			wl_list_insert(next_in_stack->link.prev, &c->link);
			arrange(server.selmon, false, false);
		} else if ((is_horizontal_layout && arg->i == RIGHT) ||
				   (!is_horizontal_layout && arg->i == DOWN)) {
			exit_scroller_stack(c);
			wl_list_remove(&c->link);
			wl_list_insert(&next_in_stack->link, &c->link);
			arrange(server.selmon, false, false);
		}
		return 0;
	}

	if (!target_client || target_client->mon != c->mon) {
		return 0;
	}

	exit_scroller_stack(c);

	// Find the tail of target_client's stack
	Client *stack_tail = target_client;
	while (stack_tail->next_in_stack) {
		stack_tail = stack_tail->next_in_stack;
	}

	// Add c to the stack
	stack_tail->next_in_stack = c;
	c->prev_in_stack = stack_tail;
	c->next_in_stack = NULL;

	if (stack_head->ismaximizescreen) {
		setmaximizescreen(stack_head, 0);
	}

	if (stack_head->isfullscreen) {
		setfullscreen(stack_head, 0);
	}

	arrange(server.selmon, false, false);
	return 0;
}