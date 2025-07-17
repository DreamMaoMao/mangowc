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
	if (strcmp(m->pertag->ltidxs[m->pertag->curtag]->name, "scroller") == 0)
		return true;
	if (strcmp(m->pertag->ltidxs[m->pertag->curtag]->name,
			   "vertical_scroller") == 0)
		return true;
	return false;
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
