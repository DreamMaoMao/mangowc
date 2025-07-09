void vertical_fibonacci(Monitor *mon, int s) {
	unsigned int i = 0, n = 0, nx, ny, nw, nh;
	Client *c;
	unsigned int cur_gappih = enablegaps ? mon->gappih : 0;
	unsigned int cur_gappiv = enablegaps ? mon->gappiv : 0;
	unsigned int cur_gappoh = enablegaps ? mon->gappoh : 0;
	unsigned int cur_gappov = enablegaps ? mon->gappov : 0;

	cur_gappih = smartgaps && mon->visible_tiling_clients == 1 ? 0 : cur_gappih;
	cur_gappiv = smartgaps && mon->visible_tiling_clients == 1 ? 0 : cur_gappiv;
	cur_gappoh = smartgaps && mon->visible_tiling_clients == 1 ? 0 : cur_gappoh;
	cur_gappov = smartgaps && mon->visible_tiling_clients == 1 ? 0 : cur_gappov;
	// Count visible clients
	wl_list_for_each(c, &clients, link) if (VISIBLEON(c, mon) && ISTILED(c))
		n++;

	if (n == 0)
		return;

	// Initial dimensions including outer gaps
	nx = mon->w.x + cur_gappoh;
	ny = mon->w.y + cur_gappov;
	nw = mon->w.width - 2 * cur_gappoh;
	nh = mon->w.height - 2 * cur_gappov;

	// First pass: calculate client geometries
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, mon) || !ISTILED(c))
			continue;

		c->bw = mon->visible_tiling_clients == 1 && no_border_when_single &&
						smartgaps
					? 0
					: borderpx;
		if ((i % 2 && nw / 2 > 2 * c->bw) || (!(i % 2) && nh / 2 > 2 * c->bw)) {
			if (i < n - 1) {
				if (i % 2) {
					if (i == 1) {
						nw = nw * mon->pertag->smfacts[mon->pertag->curtag];
					} else {
						nw = (nw - cur_gappih) / 2;
					}
				} else {
					nh = (nh - cur_gappiv) / 2;
				}

				if ((i % 4) == 2 && !s)
					ny += nh + cur_gappiv;
				else if ((i % 4) == 3 && !s)
					nx += nw + cur_gappih;
			}

			if ((i % 4) == 0) {
				if (s)
					nx += nw + cur_gappih;
				else
					nx -= nw + cur_gappih;
			} else if ((i % 4) == 1)
				ny += nh + cur_gappiv;
			else if ((i % 4) == 2)
				nx += nw + cur_gappih;
			else if ((i % 4) == 3) {
				if (s)
					ny += nh + cur_gappiv;
				else
					ny -= nh + cur_gappiv;
			}

			if (i == 0) {
				if (n != 1)
					nh = (mon->w.height - 2 * cur_gappov) *
						 mon->pertag->mfacts[mon->pertag->curtag];
				nx = mon->w.x + cur_gappoh;
			} else if (i == 1) {
				nh = mon->w.height - 2 * cur_gappov - nh - cur_gappiv;
			} else if (i == 2) {
				nw = mon->w.width - 2 * cur_gappoh - nw - cur_gappih;
			}
			i++;
		}

		c->geom = (struct wlr_box){.x = nx, .y = ny, .width = nw, .height = nh};
	}

	// Second pass: apply gaps between clients
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, mon) || !ISTILED(c))
			continue;

		unsigned int right_gap = 0;
		unsigned int bottom_gap = 0;
		Client *nc;

		wl_list_for_each(nc, &clients, link) {
			if (!VISIBLEON(nc, mon) || !ISTILED(nc))
				continue;

			if (c == nc)
				continue;

			// Check for right neighbor
			if (c->geom.y == nc->geom.y &&
				c->geom.x + c->geom.width == nc->geom.x) {
				right_gap = cur_gappih;
			}

			// Check for bottom neighbor
			if (c->geom.x == nc->geom.x &&
				c->geom.y + c->geom.height == nc->geom.y) {
				bottom_gap = cur_gappiv;
			}
		}

		resize(c,
			   (struct wlr_box){.x = c->geom.x,
								.y = c->geom.y,
								.width = c->geom.width - right_gap,
								.height = c->geom.height - bottom_gap},
			   0);
	}
}

void vertical_dwindle(Monitor *mon) { vertical_fibonacci(mon, 1); }

void vertical_spiral(Monitor *mon) { vertical_fibonacci(mon, 0); }

void vertical_grid(Monitor *m) {
	unsigned int i, n;
	unsigned int cx, cy, cw, ch;
	unsigned int dy;
	unsigned int rows, cols, overrows;
	Client *c;
	n = 0;

	wl_list_for_each(c, &clients, link) {
		if (VISIBLEON(c, m) && !c->isunglobal &&
			((m->isoverview && !client_should_ignore_focus(c)) || ISTILED(c))) {
			n++;
		}
	}

	if (n == 0) {
		return;
	}

	if (n == 1) {
		wl_list_for_each(c, &clients, link) {
			c->bw = m->visible_tiling_clients == 1 && no_border_when_single &&
							smartgaps
						? 0
						: borderpx;
			if (VISIBLEON(c, m) && !c->isunglobal &&
				((m->isoverview && !client_should_ignore_focus(c)) ||
				 ISTILED(c))) {
				ch = (m->w.height - 2 * overviewgappo) * 0.7;
				cw = (m->w.width - 2 * overviewgappo) * 0.8;
				c->geom.x = m->w.x + (m->w.width - cw) / 2;
				c->geom.y = m->w.y + (m->w.height - ch) / 2;
				c->geom.width = cw - 2 * c->bw;
				c->geom.height = ch - 2 * c->bw;
				resize(c, c->geom, 0);
				return;
			}
		}
	}

	if (n == 2) {
		ch = (m->w.height - 2 * overviewgappo - overviewgappi) / 2;
		cw = (m->w.width - 2 * overviewgappo) * 0.65;
		i = 0;
		wl_list_for_each(c, &clients, link) {
			c->bw = m->visible_tiling_clients == 1 && no_border_when_single &&
							smartgaps
						? 0
						: borderpx;
			if (VISIBLEON(c, m) && !c->isunglobal &&
				((m->isoverview && !client_should_ignore_focus(c)) ||
				 ISTILED(c))) {
				if (i == 0) {
					c->geom.x = m->w.x + (m->w.width - cw) / 2 + overviewgappo;
					c->geom.y = m->w.y + overviewgappo;
					c->geom.width = cw - 2 * c->bw;
					c->geom.height = ch - 2 * c->bw;
					resize(c, c->geom, 0);
				} else if (i == 1) {
					c->geom.x = m->w.x + (m->w.width - cw) / 2 + overviewgappo;
					c->geom.y = m->w.y + ch + overviewgappo + overviewgappi;
					c->geom.width = cw - 2 * c->bw;
					c->geom.height = ch - 2 * c->bw;
					resize(c, c->geom, 0);
				}
				i++;
			}
		}
		return;
	}

	for (rows = 0; rows <= n / 2; rows++) {
		if (rows * rows >= n) {
			break;
		}
	}
	cols = (rows && (rows - 1) * rows >= n) ? rows - 1 : rows;

	cw = (m->w.width - 2 * overviewgappo - (cols - 1) * overviewgappi) / cols;
	ch = (m->w.height - 2 * overviewgappo - (rows - 1) * overviewgappi) / rows;

	overrows = n % rows;
	if (overrows) {
		dy =
			(m->w.height - overrows * ch - (overrows - 1) * overviewgappi) / 2 -
			overviewgappo;
	}

	i = 0;
	wl_list_for_each(c, &clients, link) {
		c->bw =
			m->visible_tiling_clients == 1 && no_border_when_single && smartgaps
				? 0
				: borderpx;
		if (VISIBLEON(c, m) && !c->isunglobal &&
			((m->isoverview && !client_should_ignore_focus(c)) || ISTILED(c))) {
			cx = m->w.x + (i / rows) * (cw + overviewgappi);
			cy = m->w.y + (i % rows) * (ch + overviewgappi);
			if (overrows && i >= n - overrows) {
				cy += dy;
			}
			c->geom.x = cx + overviewgappo;
			c->geom.y = cy + overviewgappo;
			c->geom.width = cw - 2 * c->bw;
			c->geom.height = ch - 2 * c->bw;
			resize(c, c->geom, 0);
			i++;
		}
	}
}

void vertical_deck(Monitor *m) {
	unsigned int mh, mx;
	int i, n = 0;
	Client *c;
	unsigned int cur_gappiv = enablegaps ? m->gappiv : 0;
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0;
	unsigned int cur_gappov = enablegaps ? m->gappov : 0;

	cur_gappiv = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappiv;
	cur_gappoh = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappoh;
	cur_gappov = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappov;

	wl_list_for_each(c, &clients, link) if (VISIBLEON(c, m) && ISTILED(c)) n++;
	if (n == 0)
		return;

	float mfact = m->pertag ? m->pertag->mfacts[m->pertag->curtag] : m->mfact;

	if (n > m->nmaster)
		mh = m->nmaster ? round((m->w.height - 2 * cur_gappov) * mfact) : 0;
	else
		mh = m->w.height - 2 * cur_gappov;

	i = mx = 0;
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;
		if (i < m->nmaster) {
			resize(
				c,
				(struct wlr_box){.x = m->w.x + cur_gappoh + mx,
								 .y = m->w.y + cur_gappov,
								 .width = (m->w.width - 2 * cur_gappoh - mx) /
										  (MIN(n, m->nmaster) - i),
								 .height = mh},
				0);
			mx += c->geom.width;
		} else {
			resize(c,
				   (struct wlr_box){.x = m->w.x + cur_gappoh,
									.y = m->w.y + mh + cur_gappov + cur_gappiv,
									.width = m->w.width - 2 * cur_gappoh,
									.height = m->w.height - mh -
											  2 * cur_gappov - cur_gappiv},
				   0);
			if (c == focustop(m))
				wlr_scene_node_raise_to_top(&c->scene->node);
		}
		i++;
	}
}

void vertical_scroller(Monitor *m) {
	unsigned int i, n;
	Client *c, *root_client = NULL;
	Client **tempClients = NULL;
	n = 0;
	struct wlr_box target_geom;
	int focus_client_index = 0;
	bool need_scroller = false;
	unsigned int cur_gappiv = enablegaps ? m->gappiv : 0;
	unsigned int cur_gappov = enablegaps ? m->gappov : 0;
	unsigned int cur_gappoh = enablegaps ? m->gappoh : 0;

	cur_gappiv = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappiv;
	cur_gappov = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappov;
	cur_gappoh = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gappoh;

	unsigned int max_client_height =
		m->w.height - 2 * scroller_structs - cur_gappiv;

	wl_list_for_each(c, &clients, link) {
		if (VISIBLEON(c, m) && ISTILED(c)) {
			n++;
		}
	}

	if (n == 0) {
		return;
	}

	tempClients = malloc(n * sizeof(Client *));
	if (!tempClients) {
		return;
	}

	n = 0;
	wl_list_for_each(c, &clients, link) {
		if (VISIBLEON(c, m) && ISTILED(c)) {
			tempClients[n] = c;
			n++;
		}
	}

	if (n == 1) {
		c = tempClients[0];
		target_geom.width = m->w.width - 2 * cur_gappoh;
		target_geom.height =
			(m->w.height - 2 * cur_gappov) * scroller_default_proportion_single;
		target_geom.x = m->w.x + (m->w.width - target_geom.width) / 2;
		target_geom.y = m->w.y + (m->w.height - target_geom.height) / 2;
		resize(c, target_geom, 0);
		free(tempClients);
		return;
	}

	if (m->sel && !client_is_unmanaged(m->sel) && !m->sel->isfloating &&
		!m->sel->ismaxmizescreen && !m->sel->isfullscreen) {
		root_client = m->sel;
	} else if (m->prevsel && !client_is_unmanaged(m->prevsel) &&
			   !m->prevsel->isfloating && !m->prevsel->ismaxmizescreen &&
			   !m->prevsel->isfullscreen) {
		root_client = m->prevsel;
	} else {
		root_client = center_select(m);
	}

	if (!root_client) {
		free(tempClients);
		return;
	}

	for (i = 0; i < n; i++) {
		c = tempClients[i];
		if (root_client == c) {
			if (!c->is_open_animation &&
				c->geom.y >= m->w.y + scroller_structs &&
				c->geom.y + c->geom.height <=
					m->w.y + m->w.height - scroller_structs) {
				need_scroller = false;
			} else {
				need_scroller = true;
			}
			focus_client_index = i;
			break;
		}
	}

	target_geom.width = m->w.width - 2 * cur_gappoh;
	target_geom.height = max_client_height * c->scroller_proportion;
	target_geom.x = m->w.x + (m->w.width - target_geom.width) / 2;

	if (need_scroller) {
		if (scroller_focus_center ||
			((!m->prevsel ||
			  (m->prevsel->scroller_proportion * max_client_height) +
					  (root_client->scroller_proportion * max_client_height) >
				  m->w.height - 2 * scroller_structs - cur_gappiv) &&
			 scroller_prefer_center)) {
			target_geom.y = m->w.y + (m->w.height - target_geom.height) / 2;
		} else {
			target_geom.y = root_client->geom.y > m->w.y + (m->w.height) / 2
								? m->w.y + (m->w.height -
											root_client->scroller_proportion *
												max_client_height -
											scroller_structs)
								: m->w.y + scroller_structs;
		}
		resize(tempClients[focus_client_index], target_geom, 0);
	} else {
		target_geom.y = c->geom.y;
		resize(tempClients[focus_client_index], target_geom, 0);
	}

	for (i = 1; i <= focus_client_index; i++) {
		c = tempClients[focus_client_index - i];
		target_geom.height = max_client_height * c->scroller_proportion;
		target_geom.y = tempClients[focus_client_index - i + 1]->geom.y -
						cur_gappiv - target_geom.height;
		resize(c, target_geom, 0);
	}

	for (i = 1; i < n - focus_client_index; i++) {
		c = tempClients[focus_client_index + i];
		target_geom.height = max_client_height * c->scroller_proportion;
		target_geom.y = tempClients[focus_client_index + i - 1]->geom.y +
						cur_gappiv +
						tempClients[focus_client_index + i - 1]->geom.height;
		resize(c, target_geom, 0);
	}

	free(tempClients);
}

void vertical_tile(Monitor *m) {
	unsigned int i, n = 0, w, r, ie = enablegaps, mh, mx, tx;
	Client *c;

	wl_list_for_each(c, &clients, link) if (VISIBLEON(c, m) && ISTILED(c)) n++;
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

	if (n > selmon->pertag->nmasters[selmon->pertag->curtag])
		mh = selmon->pertag->nmasters[selmon->pertag->curtag]
				 ? (m->w.height + cur_gappiv * ie) *
					   selmon->pertag->mfacts[selmon->pertag->curtag]
				 : 0;
	else
		mh = m->w.height - 2 * cur_gappoh + cur_gappiv * ie;
	i = 0;
	mx = tx = cur_gappov;
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;
		if (i < selmon->pertag->nmasters[selmon->pertag->curtag]) {
			r = MIN(n, selmon->pertag->nmasters[selmon->pertag->curtag]) - i;
			w = (m->w.width - mx - cur_gappov - cur_gappiv * ie * (r - 1)) / r;
			resize(c,
				   (struct wlr_box){.x = m->w.x + mx,
									.y = m->w.y + cur_gappoh,
									.width = w,
									.height = mh - cur_gappih * ie},
				   0);
			mx += c->geom.width + cur_gappiv * ie;
		} else {
			r = n - i;
			w = (m->w.width - tx - cur_gappov - cur_gappiv * ie * (r - 1)) / r;
			resize(
				c,
				(struct wlr_box){.x = m->w.x + tx,
								 .y = m->w.y + mh + cur_gappoh,
								 .width = w,
								 .height = m->w.height - mh - 2 * cur_gappoh},
				0);
			tx += c->geom.width + cur_gappiv * ie;
		}
		i++;
	}
}

void vertical_monocle(Monitor *m) {
	Client *c;

	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;
		resize(c, m->w, 0);
	}
	if ((c = focustop(m)))
		wlr_scene_node_raise_to_top(&c->scene->node);
}