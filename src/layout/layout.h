static void tile(Monitor *m);
static void center_tile(Monitor *m);
static void right_tile(Monitor *m);
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
static void tgmix(Monitor *m);

/* layout(s) */
Layout overviewlayout = {"󰃇", overview, "overview"};

enum {
	TILE,
	SCROLLER,
	GRID,
	MONOCLE,
	DECK,
	CENTER_TILE,
	VERTICAL_SCROLLER,
	VERTICAL_TILE,
	VERTICAL_GRID,
	VERTICAL_DECK,
	RIGHT_TILE,
	TGMIX,
};

Layout layouts[] = {
	// At least two required, cannot delete to less than two
	/* symbol     arrange function   name */
	{"T", tile, "tile", TILE},						 // Tile layout
	{"S", scroller, "scroller", SCROLLER},			 // Scroll layout
	{"G", grid, "grid", GRID},						 // Grid layout
	{"M", monocle, "monocle", MONOCLE},				 // Monocle layout
	{"K", deck, "deck", DECK},						 // Deck layout
	{"CT", center_tile, "center_tile", CENTER_TILE}, // Center tile layout
	{"RT", right_tile, "right_tile", RIGHT_TILE},	 // Right tile layout
	{"VS", vertical_scroller, "vertical_scroller",
	 VERTICAL_SCROLLER},								   // Vertical scroll layout
	{"VT", vertical_tile, "vertical_tile", VERTICAL_TILE}, // Vertical tile layout
	{"VG", vertical_grid, "vertical_grid", VERTICAL_GRID}, // Vertical grid layout
	{"VK", vertical_deck, "vertical_deck", VERTICAL_DECK}, // Vertical deck layout
	{"TG", tgmix, "tgmix", TGMIX},						   // Mix layout
};