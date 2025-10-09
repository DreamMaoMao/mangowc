Monitor *dirtomon(enum wlr_direction dir) {
	struct wlr_output *next;
	if (!wlr_output_layout_get(output_layout, selmon->wlr_output))
		return selmon;
	if ((next = wlr_output_layout_adjacent_output(output_layout, 1 << dir,
												  selmon->wlr_output,
												  selmon->m.x, selmon->m.y)))
		return next->data;
	if ((next = wlr_output_layout_farthest_output(
			 output_layout,
			 dir ^ (WLR_DIRECTION_LEFT | WLR_DIRECTION_RIGHT |
					WLR_DIRECTION_UP | WLR_DIRECTION_DOWN),
			 selmon->wlr_output, selmon->m.x, selmon->m.y)))
		return next->data;
	return selmon;
}

bool is_scroller_layout(Monitor *m) {

	if (m->pertag->ltidxs[m->pertag->curtag]->id == SCROLLER)
		return true;

	if (m->pertag->ltidxs[m->pertag->curtag]->id == VERTICAL_SCROLLER)
		return true;

	return false;
}

unsigned int get_tag_status(unsigned int tag, Monitor *m) {
	Client *c = NULL;
	unsigned int status = 0;
	wl_list_for_each(c, &clients, link) {
		if (c->mon == m && c->tags & 1 << (tag - 1) & TAGMASK) {
			if (c->isurgent) {
				status = 2;
				break;
			}
			status = 1;
		}
	}
	return status;
}

unsigned int get_tags_first_tag_num(unsigned int source_tags) {
	unsigned int i, tag;
	tag = 0;

	if (!source_tags) {
		return selmon->pertag->curtag;
	}

	for (i = 0; !(tag & 1) && source_tags != 0 && i < LENGTH(tags); i++) {
		tag = source_tags >> i;
	}

	if (i == 1) {
		return 1;
	} else if (i > 9) {
		return 9;
	} else {
		return i;
	}
}

// 获取tags中最前面的tag的tagmask
unsigned int get_tags_first_tag(unsigned int source_tags) {
	unsigned int i, tag;
	tag = 0;

	if (!source_tags) {
		return selmon->pertag->curtag;
	}

	for (i = 0; !(tag & 1) && source_tags != 0 && i < LENGTH(tags); i++) {
		tag = source_tags >> i;
	}

	if (i == 1) {
		return 1;
	} else if (i > 9) {
		return 1 << 8;
	} else {
		return 1 << (i - 1);
	}
}

Monitor *xytomon(double x, double y) {
	struct wlr_output *o = wlr_output_layout_output_at(output_layout, x, y);
	return o ? o->data : NULL;
}
