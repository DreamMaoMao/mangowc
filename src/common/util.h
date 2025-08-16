/* See LICENSE.dwm file for copyright and license details. */
#include <wayland-util.h>

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
int fd_set_nonblock(int fd);
int regex_match(const char *pattern_mb, const char *str_mb);
void wl_list_append(struct wl_list *list, struct wl_list *object);
