static void tile(Monitor *m);
static void center_tile(Monitor *m);
static void overview(Monitor *m);
static void grid(Monitor *m);
static void scroller(Monitor *m);
static void deck(Monitor *mon);
static void monocle(Monitor *m);
static void vertical_tile(Monitor *m);
static void vertical_overview(Monitor *m);
static void vertical_grid(Monitor *m);
static void vertical_scroller(Monitor *m);
static void vertical_deck(Monitor *mon);

/* layout(s) */
Layout overviewlayout = {"󰃇", overview, "overview"};

Layout layouts[] = {
	// 最少两个,不能删除少于两个
	/* symbol     arrange function   name */
	{"S", scroller, "scroller"}, // 滚动布局
	{"T", tile, "tile"},		 // 堆栈布局
	{"G", grid, "grid"},
	{"M", monocle, "monocle"},
	{"K", deck, "deck"},
	{"CT", center_tile, "center_tile"},
	{"VS", vertical_scroller, "vertical_scroller"},
	{"VT", vertical_tile, "vertical_tile"},
	{"VG", vertical_grid, "vertical_grid"},
	{"VK", vertical_deck, "vertical_deck"},
};