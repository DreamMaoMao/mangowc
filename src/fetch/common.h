pid_t getparentprocess(pid_t p) {
	unsigned int v = 0;

	FILE *f;
	char buf[256];
	snprintf(buf, sizeof(buf) - 1, "/proc/%u/stat", (unsigned)p);

	if (!(f = fopen(buf, "r")))
		return 0;

	// 检查fscanf返回值，确保成功读取了1个参数
	if (fscanf(f, "%*u %*s %*c %u", &v) != 1) {
		fclose(f);
		return 0;
	}

	fclose(f);

	return (pid_t)v;
}

char *get_autostart_path(char *autostart_path, unsigned int buf_size) {
	const char *maomaoconfig = getenv("MAOMAOCONFIG");

	if (maomaoconfig && maomaoconfig[0] != '\0') {
		snprintf(autostart_path, buf_size, "%s/autostart.sh", maomaoconfig);
	} else {
		const char *homedir = getenv("HOME");
		if (!homedir) {
			fprintf(stderr, "Error: HOME environment variable not set.\n");
			return NULL;
		}
		snprintf(autostart_path, buf_size, "%s/.config/maomao/autostart.sh",
				 homedir);
	}

	return autostart_path;
}

char *get_layout_abbr(const char *full_name) {
	// 1. 尝试在映射表中查找
	for (int i = 0; layout_mappings[i].full_name != NULL; i++) {
		if (strcmp(full_name, layout_mappings[i].full_name) == 0) {
			return strdup(layout_mappings[i].abbr);
		}
	}

	// 2. 尝试从名称中提取并转换为小写
	const char *open = strrchr(full_name, '(');
	const char *close = strrchr(full_name, ')');
	if (open && close && close > open) {
		unsigned int len = close - open - 1;
		if (len > 0 && len <= 4) {
			char *abbr = malloc(len + 1);
			if (abbr) {
				// 提取并转换为小写
				for (unsigned int j = 0; j < len; j++) {
					abbr[j] = tolower(open[j + 1]);
				}
				abbr[len] = '\0';
				return abbr;
			}
		}
	}

	// 3. 提取前2-3个字母并转换为小写
	char *abbr = malloc(4);
	if (abbr) {
		unsigned int j = 0;
		for (unsigned int i = 0; full_name[i] != '\0' && j < 3; i++) {
			if (isalpha(full_name[i])) {
				abbr[j++] = tolower(full_name[i]);
			}
		}
		abbr[j] = '\0';

		// 确保至少2个字符
		if (j >= 2)
			return abbr;
		free(abbr);
	}

	// 4. 回退方案：使用首字母小写
	char *fallback = malloc(3);
	if (fallback) {
		fallback[0] = tolower(full_name[0]);
		fallback[1] = full_name[1] ? tolower(full_name[1]) : '\0';
		fallback[2] = '\0';
		return fallback;
	}

	// 5. 最终回退：返回 "xx"
	return strdup("xx");
}

void xytonode(double x, double y, struct wlr_surface **psurface, Client **pc,
			  LayerSurface **pl, double *nx, double *ny) {
	struct wlr_scene_node *node, *pnode;
	struct wlr_surface *surface = NULL;
	Client *c = NULL;
	LayerSurface *l = NULL;
	int layer;

	for (layer = NUM_LAYERS - 1; !surface && layer >= 0; layer--) {

		// ignore text-input layer
		if (layer == LyrIMPopup)
			continue;

		if (layer == LyrFadeOut)
			continue;

		if (!(node = wlr_scene_node_at(&layers[layer]->node, x, y, nx, ny)))
			continue;

		if (node->type == WLR_SCENE_NODE_BUFFER)
			surface = wlr_scene_surface_try_from_buffer(
						  wlr_scene_buffer_from_node(node))
						  ->surface;
		/* Walk the tree to find a node that knows the client */
		for (pnode = node; pnode && !c; pnode = &pnode->parent->node)
			c = pnode->data;
		if (c && c->type == LayerShell) {
			c = NULL;
			l = pnode->data;
		}
	}

	if (psurface)
		*psurface = surface;
	if (pc)
		*pc = c;
	if (pl)
		*pl = l;
}