pid_t getparentprocess(pid_t p) {
	uint32_t v = 0;

	FILE *f;
	char buf[256];
	snprintf(buf, sizeof(buf) - 1, "/proc/%u/stat", (unsigned)p);

	if (!(f = fopen(buf, "r")))
		return 0;

	// Check fscanf return value to ensure 1 parameter was successfully read
	if (fscanf(f, "%*u %*s %*c %u", &v) != 1) {
		fclose(f);
		return 0;
	}

	fclose(f);

	return (pid_t)v;
}

int32_t isdescprocess(pid_t p, pid_t c) {
	while (p != c && c != 0)
		c = getparentprocess(c);

	return (int32_t)c;
}

// Buffer size for layout abbreviations (must match kb_layout buffer in dwl-ipc.h)
#define LAYOUT_ABBR_SIZE 32

void get_layout_abbr(char *abbr, const char *full_name) {
	// Clear output buffer
	abbr[0] = '\0';

	// 1. Try to find in mapping table
	for (int32_t i = 0; layout_mappings[i].full_name != NULL; i++) {
		if (strcmp(full_name, layout_mappings[i].full_name) == 0) {
			strncpy(abbr, layout_mappings[i].abbr, LAYOUT_ABBR_SIZE - 1);
			abbr[LAYOUT_ABBR_SIZE - 1] = '\0';
			return;
		}
	}

	// 2. Try to extract and convert to lowercase from name
	const char *open = strrchr(full_name, '(');
	const char *close = strrchr(full_name, ')');
	if (open && close && close > open) {
		uint32_t len = close - open - 1;
		if (len > 0 && len <= 4) {
			// Extract and convert to lowercase
			for (uint32_t j = 0; j < len; j++) {
				abbr[j] = tolower(open[j + 1]);
			}
			abbr[len] = '\0';
			return;
		}
	}

	// 3. Extract first 2-3 letters and convert to lowercase
	uint32_t j = 0;
	for (uint32_t i = 0; full_name[i] != '\0' && j < 3; i++) {
		if (isalpha(full_name[i])) {
			abbr[j++] = tolower(full_name[i]);
		}
	}
	abbr[j] = '\0';

	// Ensure at least 2 characters
	if (j >= 2) {
		return;
	}

	// 4. Fallback: use first letter in lowercase
	if (j == 1) {
		abbr[1] = full_name[1] ? tolower(full_name[1]) : '\0';
		abbr[2] = '\0';
	} else {
		// 5. Final fallback: return "xx"
		abbr[0] = 'x';
		abbr[1] = 'x';
		abbr[2] = '\0';
	}
}

void xytonode(double x, double y, struct wlr_surface **psurface, Client **pc,
			  LayerSurface **pl, double *nx, double *ny) {
	struct wlr_scene_node *node, *pnode;
	struct wlr_surface *surface = NULL;
	Client *c = NULL;
	LayerSurface *l = NULL;
	int32_t layer;

	for (layer = NUM_LAYERS - 1; !surface && layer >= 0; layer--) {

		if (layer == LyrFadeOut)
			continue;

		if (!(node = wlr_scene_node_at(&layers[layer]->node, x, y, nx, ny)))
			continue;

		if (!node->enabled)
			continue;

		if (node->type == WLR_SCENE_NODE_BUFFER)
			surface = wlr_scene_surface_try_from_buffer(
						  wlr_scene_buffer_from_node(node))
						  ->surface;
		else if (node->type == WLR_SCENE_NODE_RECT) {
			surface = NULL;
			break;
		}

		/*  start from the topmost layer,
			find a surface that can be focused by pointer,
			impopup is neither a client nor a layer surface.*/
		if (layer == LyrIMPopup) {
			c = NULL;
			l = NULL;
		} else {
			for (pnode = node; pnode && !c; pnode = &pnode->parent->node)
				c = pnode->data;
			if (c && c->type == LayerShell) {
				c = NULL;
				l = pnode->data;
			}
		}
	}

	if (psurface)
		*psurface = surface;
	if (pc)
		*pc = c;
	if (pl)
		*pl = l;
}