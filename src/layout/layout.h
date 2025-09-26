static void tile(Monitor *m);
static void center_tile(Monitor *m);
static void overview(Monitor *m);
static void grid(Monitor *m);
static void scroller(Monitor *m);
static void deck(Monitor *mon);
static void dwindle(Monitor *mon);
static void spiral(Monitor *mon);
static void monocle(Monitor *m);
static void vertical_tile(Monitor *m);
static void vertical_overview(Monitor *m);
static void vertical_grid(Monitor *m);
static void vertical_scroller(Monitor *m);
static void vertical_deck(Monitor *mon);
static void vertical_dwindle(Monitor *mon);
static void vertical_spiral(Monitor *mon);

/* layout(s) */
Layout overviewlayout = {"󰃇", overview, "overview"};

Layout layouts[] = {
	// 最少两个,不能删除少于两个
	/* symbol     arrange function   name */
	{"S", scroller, "scroller"}, // 滚动布局
	{"T", tile, "tile"},		 // 堆栈布局
	{"G", grid, "grid"},
	{"M", monocle, "monocle"},
	{"D", dwindle, "dwindle"},
	{"P", spiral, "spiral"},
	{"K", deck, "deck"},
	{"CT", center_tile, "center_tile"},
	{"VS", vertical_scroller, "vertical_scroller"},
	{"VT", vertical_tile, "vertical_tile"},
	{"VD", vertical_dwindle, "vertical_dwindle"},
	{"VP", vertical_spiral, "vertical_spiral"},
	{"VG", vertical_grid, "vertical_grid"},
	{"VK", vertical_deck, "vertical_deck"},
};