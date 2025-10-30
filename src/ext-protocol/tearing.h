#include <wlr/types/wlr_tearing_control_v1.h>

struct tearing_controller {
	struct wlr_tearing_control_v1 *tearing_control;
	struct wl_listener set_hint;
	struct wl_listener destroy;
};

typedef struct SendFrameDoneData {
	struct timespec when;
	struct Monitor *mon;
} SendFrameDoneData;

struct wlr_tearing_control_manager_v1 *tearing_control;
struct wl_listener tearing_new_object;

static void handle_controller_set_hint(struct wl_listener *listener,
									   void *data) {
	struct tearing_controller *controller =
		wl_container_of(listener, controller, set_hint);
	Client *c = get_client_from_surface(controller->tearing_control->surface);
	if (c) {
		/*
		 * tearing_control->current is actually an enum:
		 * WP_TEARING_CONTROL_V1_PRESENTATION_HINT_VSYNC = 0
		 * WP_TEARING_CONTROL_V1_PRESENTATION_HINT_ASYNC = 1
		 *
		 * Using it as a bool here allows us to not ship the XML.
		 */
		c->tearing_hint = controller->tearing_control->current;
	}
}

static void handle_controller_destroy(struct wl_listener *listener,
									  void *data) {
	struct tearing_controller *controller =
		wl_container_of(listener, controller, destroy);
	wl_list_remove(&controller->set_hint.link);
	wl_list_remove(&controller->destroy.link);
	free(controller);
}

void handle_tearing_new_object(struct wl_listener *listener, void *data) {
	struct wlr_tearing_control_v1 *new_tearing_control = data;

	enum wp_tearing_control_v1_presentation_hint hint =
		wlr_tearing_control_manager_v1_surface_hint_from_surface(
			tearing_control, new_tearing_control->surface);
	wlr_log(WLR_DEBUG, "New presentation hint %d received for surface %p", hint,
			new_tearing_control->surface);

	struct tearing_controller *controller =
		ecalloc(1, sizeof(struct tearing_controller));
	controller->tearing_control = new_tearing_control;

	controller->set_hint.notify = handle_controller_set_hint;
	wl_signal_add(&new_tearing_control->events.set_hint, &controller->set_hint);

	controller->destroy.notify = handle_controller_destroy;
	wl_signal_add(&new_tearing_control->events.destroy, &controller->destroy);
}

bool output_get_tearing_allowance(Monitor *m) {
	/* never allow tearing when disabled */
	if (!allow_tearing) {
		return false;
	}

	Client *c = selmon->sel;

	/* tearing is only allowed for the output with the active client */
	if (!c || c->mon != m) {
		return false;
	}

	/* allow tearing for any window when requested or forced */
	if (allow_tearing == TEARING_ENABLED) {
		if (c->force_tearing == STATE_UNSPECIFIED) {
			return c->tearing_hint;
		} else {
			return c->force_tearing == STATE_ENABLED;
		}
	}

	/* remaining tearing options apply only to full-screen windows */
	if (!c->isfullscreen) {
		return false;
	}

	if (c->force_tearing == STATE_UNSPECIFIED) {
		/* honor the tearing hint or the fullscreen-force preference */
		return c->tearing_hint || allow_tearing == TEARING_FULLSCREEN_FORCED;
	}

	/* honor tearing as requested by action */
	return c->force_tearing == STATE_ENABLED;
}

void sendframedoneiterator(struct wlr_scene_buffer *buffer, int x, int y,
						   void *user_data) {
	SendFrameDoneData *data = user_data;
	if (buffer->primary_output != data->mon->scene_output)
		return;

	wlr_scene_buffer_send_frame_done(buffer, &data->when);
}
