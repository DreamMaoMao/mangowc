/* SPDX-License-Identifier: GPL-2.0-only */
/* Based on labwc (https://github.com/labwc/labwc) */

#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>

/*
 * Wayland's wl_array API is a bit sparse consisting only of
 *  - init
 *  - release
 *  - add
 *  - copy
 *  - for_each
 *
 * The purpose of this header is the gather any generic wl_array helpers we
 * create.
 *
 * We take the liberty of using the wl_ suffix here to make it look a bit
 * prettier. If Wayland extend the API in future, we will sort the clash then.
 */

/**
 * wl_array_len() - return length of wl_array
 * @array: wl_array for which to calculate length
 * Note: The pointer type might not be 'char' but this is the approach that
 * wl_array_for_each() takes, so we align with their style.
 */
static inline size_t wl_array_len(struct wl_array *array) {
	return array->size / sizeof(const char *);
}

/**
 * Iterates in reverse over an array.
 * @pos: pointer that each array element will be assigned to
 * @array: wl_array to iterate over
 */
#define wl_array_for_each_reverse(pos, array)                                  \
	for (pos = !(array)->data ? NULL                                           \
							  : (void *)((const char *)(array)->data +         \
										 (array)->size - sizeof(pos));         \
		 pos && (const char *)pos >= (const char *)(array)->data; (pos)--)

/**
 * array_add() - add item to wl_array and exit on allocation error
 * @_arr: wl_array to add the item to
 * @_val: the item to add to the array
 *
 * Let us illustrate the function of this macro by an example:
 *
 *   uint32_t value = 5;
 *   array_add(array, value);
 *
 * ...is the equivalent of the code below which is how you would
 * otherwise use the wl_array API:
 *
 *   uint32_t *elm = wl_array_add(array, sizeof(uint32_t));
 *   if (!elm) {
 *       perror("failed to allocate memory");
 *       exit(EXIT_FAILURE);
 *   }
 *   *elm = value;
 */
#define array_add(_arr, _val)                                                  \
	do {                                                                       \
		__typeof__(_val) *_entry =                                             \
			wl_array_add((_arr), sizeof(__typeof__(_val)));                    \
		if (!_entry) {                                                         \
			perror("Failed to allocate memory");                               \
			exit(EXIT_FAILURE);                                                \
		}                                                                      \
		*_entry = (_val);                                                      \
	} while (0)
