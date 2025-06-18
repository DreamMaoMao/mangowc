// SPDX-License-Identifier: GPL-2.0-only
/* Based on labwc (https://github.com/labwc/labwc) */

#include "transaction-addon.h"
#include "../common/list.h"
#include "../common/mem.h"
#include <assert.h>
#include <wayland-server-core.h>

void dwl_transaction_op_destroy(struct dwl_transaction_op *trans_op) {
	wl_signal_emit_mutable(&trans_op->events.destroy, trans_op);
	wl_list_remove(&trans_op->link);
	free(trans_op);
}

static void transaction_destroy(struct wl_list *list) {
	struct dwl_transaction_op *trans_op, *trans_op_tmp;
	wl_list_for_each_safe(trans_op, trans_op_tmp, list, link) {
		dwl_transaction_op_destroy(trans_op);
	}
}

void dwl_resource_addon_destroy(struct dwl_wl_resource_addon *addon) {
	assert(addon);
	assert(addon->ctx);

	addon->ctx->ref_count--;
	assert(addon->ctx->ref_count >= 0);

	if (!addon->ctx->ref_count) {
		transaction_destroy(&addon->ctx->transaction_ops);
		free(addon->ctx);
	}

	free(addon);
}

struct dwl_wl_resource_addon *
dwl_resource_addon_create(struct dwl_transaction_session_context *ctx) {
	struct dwl_wl_resource_addon *addon = znew(*addon);
	if (!ctx) {
		ctx = znew(*ctx);
		wl_list_init(&ctx->transaction_ops);
	}
	addon->ctx = ctx;
	addon->ctx->ref_count++;
	return addon;
}

struct dwl_transaction_op *
dwl_transaction_op_add(struct dwl_transaction_session_context *ctx,
					   uint32_t pending_change, void *src, void *data) {
	assert(ctx);

	struct dwl_transaction_op *trans_op = znew(*trans_op);
	trans_op->change = pending_change;
	trans_op->src = src;
	trans_op->data = data;

	wl_signal_init(&trans_op->events.destroy);
	wl_list_append(&ctx->transaction_ops, &trans_op->link);

	return trans_op;
}
