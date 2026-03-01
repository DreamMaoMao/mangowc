#include <string.h>

static void copy_monitor_name(char *dst, size_t dst_size, const char *src) {
	if (!dst || dst_size == 0)
		return;
	if (!src) {
		dst[0] = '\0';
		return;
	}
	strncpy(dst, src, dst_size - 1);
	dst[dst_size - 1] = '\0';
}

int main(void) {
	char buf[32];
	char src[64];

	memset(src, 'A', sizeof(src) - 1);
	src[sizeof(src) - 1] = '\0';

	copy_monitor_name(buf, sizeof(buf), src);

	if (buf[sizeof(buf) - 1] != '\0')
		return 1;
	if (strlen(buf) != sizeof(buf) - 1)
		return 2;

	return 0;
}
