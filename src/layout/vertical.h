void vertical_tile(Monitor *m) {
	unsigned int i, n = 0, w, r, ie = enablegaps, mh, mx, tx;
	Client *c = NULL;
	Client *fc = NULL;
	double mfact = 0;

	n = m->visible_tiling_clients;

	if (n == 0)
		return;

	unsigned int cur_gapih = enablegaps ? m->gappih : 0;
	unsigned int cur_gapiv = enablegaps ? m->gappiv : 0;
	unsigned int cur_gapoh = enablegaps ? m->gappoh : 0;
	unsigned int cur_gapov = enablegaps ? m->gappov : 0;

	cur_gapih = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gapih;
	cur_gapiv = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gapiv;
	cur_gapoh = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gapoh;
	cur_gapov = smartgaps && m->visible_tiling_clients == 1 ? 0 : cur_gapov;

	wl_list_for_each(fc, &clients, link) {
		if (VISIBLEON(fc, m) && ISTILED(fc))
			break;
	}

	mfact = fc->master_mfact_per > 0.0f ? fc->master_mfact_per
										: m->pertag->mfacts[m->pertag->curtag];

	if (n > m->pertag->nmasters[m->pertag->curtag])
		mh = m->pertag->nmasters[m->pertag->curtag]
				 ? (m->w.height + cur_gapiv * ie) * mfact
				 : 0;
	else
		mh = m->w.height - 2 * cur_gapov + cur_gapiv * ie;

	i = 0;
	mx = tx = cur_gapih;
	wl_list_for_each(c, &clients, link) {
		if (!VISIBLEON(c, m) || !ISTILED(c))
			continue;
		if (i < m->pertag->nmasters[m->pertag->curtag]) {
			r = MIN(n, m->pertag->nmasters[m->pertag->curtag]) - i;
			if (c->master_inner_per > 0.0f) {
				w = (m->w.width - 2 * cur_gapih - cur_gapih * ie * (r - 1)) *
					c->master_inner_per;
				c->master_mfact_per = mfact;
			} else {
				w = (m->w.width - mx - cur_gapih - cur_gapih * ie * (r - 1)) /
					r;
				c->master_inner_per = w / (m->w.width - mx - cur_gapih -
										   cur_gapih * ie * (r - 1));
				c->master_mfact_per = mfact;
			}
			resize(c,
				   (struct wlr_box){.x = m->w.x + mx,
									.y = m->w.y + cur_gapov,
									.width = w,
									.height = mh - cur_gapiv * ie},
				   0);
			mx += c->geom.width + cur_gapih * ie;
		} else {
			r = n - i;
			if (c->slave_innder_per > 0.0f) {
				w = (m->w.width - 2 * cur_gapih - cur_gapih * ie * (r - 1)) *
					c->slave_innder_per;
				c->master_mfact_per = mfact;
			} else {
				w = (m->w.width - tx - cur_gapih - cur_gapih * ie * (r - 1)) /
					r;
				c->slave_innder_per = w / (m->w.width - tx - cur_gapih -
										   cur_gapih * ie * (r - 1));
				c->master_mfact_per = mfact;
			}

			resize(c,
				   (struct wlr_box){.x = m->w.x + tx,
									.y = m->w.y + mh + cur_gapov,
									.width = w,
									.height = m->w.height - mh - 2 * cur_gapov},
				   0);
			tx += c->geom.width + cur_gapih * ie;
		}
		i++;
	}
}

void vertical_scroller(Monitor *m) {
	unsigned int i, n, j;
	Client *c = NULL, *root_client = NULL;
	Client **tempClients = NULL;
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

	n = m->visible_tiling_clients;

	if (n == 0) {
		return;
	}

	tempClients = malloc(n * sizeof(Client *));
	if (!tempClients) {
		return;
	}

	j = 0;
	wl_list_for_each(c, &clients, link) {
		if (VISIBLEON(c, m) && ISTILED(c)) {
			tempClients[j] = c;
			j++;
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
	} else if (m->prevsel && ISTILED(m->prevsel) && VISIBLEON(m->prevsel, m) &&
			   !client_is_unmanaged(m->prevsel)) {
		root_client = m->prevsel;
	} else {
		root_client = center_tiled_select(m);
	}

	if (!root_client) {
		free(tempClients);
		return;
	}

	for (i = 0; i < n; i++) {
		c = tempClients[i];
		if (root_client == c) {
			if (!c->is_pending_open_animation &&
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
			  (ISTILED(m->prevsel) &&
			   (m->prevsel->scroller_proportion * max_client_height) +
					   (root_client->scroller_proportion * max_client_height) >
				   m->w.height - 2 * scroller_structs - cur_gappiv)) &&
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
