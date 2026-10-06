/*
 * Ask the Wayland compositor how fast it repeats keys.
 *
 * There is nothing to listen to here.  A Wayland compositor does not send a
 * key event per repeat the way an X server does; it tells each client the
 * delay and the rate once, in wl_keyboard.repeat_info, and every client then
 * runs its own timer.  So the repeats that put characters on the screen exist
 * only inside whichever client has the keyboard focus, and the best a bystander
 * can do is ask for the same two numbers and time its own.
 *
 * The numbers are worth the trouble: the compositor keeps its setting to
 * itself, and the kernel's own repeat, which is what the input device reports,
 * carries on at its default 250 ms and 33 ms however the desktop is configured.
 *
 * repeat_info arrives as soon as the wl_keyboard is created, before any key
 * event and without needing the focus, and again whenever the setting changes,
 * so the connection is kept open and polled rather than closed after a look.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

#include "buckle.h"

static struct wl_display *display = NULL;
static struct wl_seat *seat = NULL;
static struct wl_keyboard *keyboard = NULL;

static int have_info = 0;
static int info_delay_ms = 0;
static int info_rate_hz = 0;


static void kb_repeat_info(void *data, struct wl_keyboard *kb, int32_t rate, int32_t delay)
{
	info_rate_hz = rate;
	info_delay_ms = delay;
	have_info = 1;
}


/* The rest of the keyboard protocol is of no interest: key events reach us
 * through libinput, and they only come this way for the focused surface. */

static void kb_keymap(void *d, struct wl_keyboard *k, uint32_t f, int32_t fd, uint32_t s) {}
static void kb_enter(void *d, struct wl_keyboard *k, uint32_t s, struct wl_surface *w, struct wl_array *a) {}
static void kb_leave(void *d, struct wl_keyboard *k, uint32_t s, struct wl_surface *w) {}
static void kb_key(void *d, struct wl_keyboard *k, uint32_t s, uint32_t t, uint32_t key, uint32_t st) {}
static void kb_modifiers(void *d, struct wl_keyboard *k, uint32_t s, uint32_t a, uint32_t b, uint32_t c, uint32_t g) {}

static const struct wl_keyboard_listener keyboard_listener = {
	.keymap      = kb_keymap,
	.enter       = kb_enter,
	.leave       = kb_leave,
	.key         = kb_key,
	.modifiers   = kb_modifiers,
	.repeat_info = kb_repeat_info,
};


static void seat_capabilities(void *data, struct wl_seat *s, uint32_t caps)
{
	if((caps & WL_SEAT_CAPABILITY_KEYBOARD) && keyboard == NULL) {
		keyboard = wl_seat_get_keyboard(s);
		wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
	}
}


static void seat_name(void *data, struct wl_seat *s, const char *name) {}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_capabilities,
	.name         = seat_name,
};


static void registry_global(void *data, struct wl_registry *reg, uint32_t name,
                            const char *iface, uint32_t version)
{
	/* repeat_info is version 4 of wl_keyboard; without it there is no
	 * point in binding the seat at all. */

	if(strcmp(iface, "wl_seat") == 0 && seat == NULL && version >= 4) {
		seat = wl_registry_bind(reg, name, &wl_seat_interface, 4);
		wl_seat_add_listener(seat, &seat_listener, NULL);
	}
}


static void registry_global_remove(void *data, struct wl_registry *reg, uint32_t name) {}

static const struct wl_registry_listener registry_listener = {
	.global        = registry_global,
	.global_remove = registry_global_remove,
};


/*
 * Connect, and return a descriptor to poll for later changes to the setting,
 * or -1 when there is no compositor to ask, which is the ordinary case on a
 * console and under X11.
 */

int wl_repeat_open(void)
{
	struct wl_registry *registry;

	/* Asked to connect with neither of these set, libwayland complains on
	 * stderr, and a console session with no compositor at all is a normal
	 * way to run.  A missing socket it passes over quietly. */

	if(getenv("WAYLAND_DISPLAY") == NULL && getenv("XDG_RUNTIME_DIR") == NULL) {
		return -1;
	}

	display = wl_display_connect(NULL);
	if(display == NULL) {
		return -1;
	}

	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);

	/* One round trip for the list of globals, a second for the seat to
	 * say whether it has a keyboard, a third for repeat_info itself. */

	if(wl_display_roundtrip(display) < 0 ||
	   wl_display_roundtrip(display) < 0 ||
	   wl_display_roundtrip(display) < 0) {
		wl_display_disconnect(display);
		display = NULL;
		return -1;
	}

	return wl_display_get_fd(display);
}


/*
 * Read what the compositor has to say.  Returns 0 once the connection is gone,
 * after which the descriptor is closed and must not be polled again.
 */

int wl_repeat_dispatch(void)
{
	if(display == NULL) {
		return 0;
	}

	if(wl_display_dispatch(display) < 0) {

		/* The compositor went away; keep whatever it last said. */

		wl_display_disconnect(display);
		display = NULL;
		return 0;
	}

	return 1;
}


/*
 * Overwrite the timing with the compositor's, if it has told us any.  A rate
 * of zero is the protocol's way of saying that keys are not to repeat.
 */

int wl_repeat_get(int *delay_ms, int *period_ms)
{
	if(!have_info) {
		return 0;
	}

	*delay_ms = info_delay_ms;
	*period_ms = info_rate_hz > 0 ? 1000 / info_rate_hz : 0;

	return 1;
}
