void tile(Monitor *m);
void center_tile(Monitor *m);
void right_tile(Monitor *m);
void overview(Monitor *m);
void grid(Monitor *m);
void scroller(Monitor *m);
void deck(Monitor *mon);
void monocle(Monitor *m);
void vertical_tile(Monitor *m);
void vertical_overview(Monitor *m);
void vertical_grid(Monitor *m);
void vertical_scroller(Monitor *m);
void vertical_deck(Monitor *mon);
void tgmix(Monitor *m);

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