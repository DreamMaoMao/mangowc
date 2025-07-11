struct dvec2 calculate_animation_curve_at(double t, int type) {
	struct dvec2 point;
	double *animation_curve;
	if (type == MOVE) {
		animation_curve = animation_curve_move;
	} else if (type == OPEN) {
		animation_curve = animation_curve_open;
	} else if (type == TAG) {
		animation_curve = animation_curve_tag;
	} else if (type == CLOSE) {
		animation_curve = animation_curve_close;
	} else {
		animation_curve = animation_curve_move;
	}

	point.x = 3 * t * (1 - t) * (1 - t) * animation_curve[0] +
			  3 * t * t * (1 - t) * animation_curve[2] + t * t * t;

	point.y = 3 * t * (1 - t) * (1 - t) * animation_curve[1] +
			  3 * t * t * (1 - t) * animation_curve[3] + t * t * t;

	return point;
}

void init_baked_points(void) {
	baked_points_move = calloc(BAKED_POINTS_COUNT, sizeof(*baked_points_move));
	baked_points_open = calloc(BAKED_POINTS_COUNT, sizeof(*baked_points_open));
	baked_points_tag = calloc(BAKED_POINTS_COUNT, sizeof(*baked_points_tag));
	baked_points_close =
		calloc(BAKED_POINTS_COUNT, sizeof(*baked_points_close));

	for (unsigned int i = 0; i < BAKED_POINTS_COUNT; i++) {
		baked_points_move[i] = calculate_animation_curve_at(
			(double)i / (BAKED_POINTS_COUNT - 1), MOVE);
	}
	for (unsigned int i = 0; i < BAKED_POINTS_COUNT; i++) {
		baked_points_open[i] = calculate_animation_curve_at(
			(double)i / (BAKED_POINTS_COUNT - 1), OPEN);
	}
	for (unsigned int i = 0; i < BAKED_POINTS_COUNT; i++) {
		baked_points_tag[i] = calculate_animation_curve_at(
			(double)i / (BAKED_POINTS_COUNT - 1), TAG);
	}
	for (unsigned int i = 0; i < BAKED_POINTS_COUNT; i++) {
		baked_points_close[i] = calculate_animation_curve_at(
			(double)i / (BAKED_POINTS_COUNT - 1), CLOSE);
	}
}

double find_animation_curve_at(double t, int type) {
	unsigned int down = 0;
	unsigned int up = BAKED_POINTS_COUNT - 1;

	unsigned int middle = (up + down) / 2;
	struct dvec2 *baked_points;
	if (type == MOVE) {
		baked_points = baked_points_move;
	} else if (type == OPEN) {
		baked_points = baked_points_open;
	} else if (type == TAG) {
		baked_points = baked_points_tag;
	} else if (type == CLOSE) {
		baked_points = baked_points_close;
	} else {
		baked_points = baked_points_move;
	}

	while (up - down != 1) {
		if (baked_points[middle].x <= t) {
			down = middle;
		} else {
			up = middle;
		}
		middle = (up + down) / 2;
	}
	return baked_points[up].y;
}

double output_frame_duration_ms() {
	int32_t refresh_total = 0;
	Monitor *m;
	wl_list_for_each(m, &mons, link) {
		if (!m->wlr_output->enabled) {
			continue;
		}
		refresh_total += m->wlr_output->refresh;
	}
	return 1000000.0 / refresh_total;
}

static bool scene_node_snapshot(struct wlr_scene_node *node, int lx, int ly,
								struct wlr_scene_tree *snapshot_tree) {
	if (!node->enabled && node->type != WLR_SCENE_NODE_TREE) {
		return true;
	}

	lx += node->x;
	ly += node->y;

	struct wlr_scene_node *snapshot_node = NULL;
	switch (node->type) {
	case WLR_SCENE_NODE_TREE:;
		struct wlr_scene_tree *scene_tree = wlr_scene_tree_from_node(node);
		struct wlr_scene_node *child;
		wl_list_for_each(child, &scene_tree->children, link) {
			scene_node_snapshot(child, lx, ly, snapshot_tree);
		}
		break;
	case WLR_SCENE_NODE_RECT:;

		// struct wlr_scene_rect *scene_rect = wlr_scene_rect_from_node(node);

		// struct wlr_scene_rect *snapshot_rect =
		// 	wlr_scene_rect_create(snapshot_tree, scene_rect->width,
		// 						  scene_rect->height, scene_rect->color);
		// snapshot_rect->node.data = scene_rect->node.data;
		// if (snapshot_rect == NULL) {
		// 	return false;
		// }
		// snapshot_node = &snapshot_rect->node;
		break;
	case WLR_SCENE_NODE_BUFFER:;

		struct wlr_scene_buffer *scene_buffer =
			wlr_scene_buffer_from_node(node);

		struct wlr_scene_buffer *snapshot_buffer =
			wlr_scene_buffer_create(snapshot_tree, NULL);
		if (snapshot_buffer == NULL) {
			return false;
		}
		snapshot_node = &snapshot_buffer->node;
		snapshot_buffer->node.data = scene_buffer->node.data;

		wlr_scene_buffer_set_dest_size(snapshot_buffer, scene_buffer->dst_width,
									   scene_buffer->dst_height);
		wlr_scene_buffer_set_opaque_region(snapshot_buffer,
										   &scene_buffer->opaque_region);
		wlr_scene_buffer_set_source_box(snapshot_buffer,
										&scene_buffer->src_box);
		wlr_scene_buffer_set_transform(snapshot_buffer,
									   scene_buffer->transform);
		wlr_scene_buffer_set_filter_mode(snapshot_buffer,
										 scene_buffer->filter_mode);

		// Effects
		wlr_scene_buffer_set_opacity(snapshot_buffer, scene_buffer->opacity);

		snapshot_buffer->node.data = scene_buffer->node.data;

		struct wlr_scene_surface *scene_surface =
			wlr_scene_surface_try_from_buffer(scene_buffer);
		if (scene_surface != NULL && scene_surface->surface->buffer != NULL) {
			wlr_scene_buffer_set_buffer(snapshot_buffer,
										&scene_surface->surface->buffer->base);
		} else {
			wlr_scene_buffer_set_buffer(snapshot_buffer, scene_buffer->buffer);
		}
		break;
	}

	if (snapshot_node != NULL) {
		wlr_scene_node_set_position(snapshot_node, lx, ly);
	}

	return true;
}

struct wlr_scene_tree *wlr_scene_tree_snapshot(struct wlr_scene_node *node,
											   struct wlr_scene_tree *parent) {
	struct wlr_scene_tree *snapshot = wlr_scene_tree_create(parent);
	if (snapshot == NULL) {
		return NULL;
	}

	// Disable and enable the snapshot tree like so to atomically update
	// the scene-graph. This will prevent over-damaging or other weirdness.
	wlr_scene_node_set_enabled(&snapshot->node, false);

	if (!scene_node_snapshot(node, 0, 0, snapshot)) {
		wlr_scene_node_destroy(&snapshot->node);
		return NULL;
	}

	wlr_scene_node_set_enabled(&snapshot->node, true);

	return snapshot;
}