#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>

#include <libinput.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>

#include "buckle.h"


static void kernel_repeat_from(int fd);


static int open_restricted(const char *path, int flags, void *user_data)
{
	int fd = open(path, flags);

	if(fd < 0) {
		fprintf(stderr, "Failed to open %s (%s)\n", path, strerror(errno));
		return -errno;
	}

	kernel_repeat_from(fd);

	return fd;
}


static void close_restricted(int fd, void *user_data)
{
	close(fd);
}


static const struct libinput_interface interface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};


/*
 * libinput throws the kernel's key repeat away, "ignore kernel key repeat" in
 * evdev-fallback.c, and leaves repeating to its caller, so to make a held key
 * keep clicking we have to run the clock ourselves.  Three places can say how
 * fast, each overruling the one before it:
 *
 *  - these, the kernel's own defaults, in case the first two say nothing;
 *  - EVIOCGREP on the input device, which is the truth on a console;
 *  - wl_keyboard.repeat_info from the compositor, which is the truth under
 *    Wayland, where the kernel's setting is left at its default and ignored;
 *  - and --repeat-delay and --repeat-rate, for when we have guessed wrong.
 *
 * None of them tell us when a repeat actually happens.  Under Wayland nobody
 * could: the compositor hands every client the same two numbers and each runs
 * its own timer, so the repeats that put characters on the screen exist only
 * inside the focused application.  We keep time alongside it rather than
 * following it.
 */

#define REPEAT_DELAY_MS 250
#define REPEAT_PERIOD_MS 33

static int repeat_delay_ms = REPEAT_DELAY_MS;
static int repeat_period_ms = REPEAT_PERIOD_MS;

static int kernel_delay_ms = REPEAT_DELAY_MS;
static int kernel_period_ms = REPEAT_PERIOD_MS;
static int kernel_known = 0;

static int repeat_key = -1;
static struct timespec repeat_at;


/*
 * Take the repeat setting from the first device that has one.  Keyboards are
 * opened before any key can arrive on them, and in practice they all sit at
 * the same kernel default, so there is little to be gained by tracking the
 * devices apart.
 */

static void kernel_repeat_from(int fd)
{
	unsigned int rep[2];

	if(kernel_known || ioctl(fd, EVIOCGREP, rep) < 0) {
		return;
	}

	kernel_delay_ms = (int)rep[0];
	kernel_period_ms = (int)rep[1];
	kernel_known = 1;
}


/*
 * Settle on a delay and a period, most authoritative source last.  Called
 * again whenever the compositor changes its mind.
 */

static void repeat_refresh(void)
{
	int delay = kernel_delay_ms;
	int period = kernel_period_ms;

	wl_repeat_get(&delay, &period);
	repeat_override(&delay, &period);

	repeat_delay_ms = delay;
	repeat_period_ms = period;

	printd("Key repeat: %d ms delay, %d ms period", delay, period);
}


/*
 * The kernel repeats whichever key went down last, modifiers included, but no
 * display server passes those repeats on and hearing shift chatter while it is
 * held would be odd.  Leave them out, which is also what keeps the mute
 * sequence, two taps on scroll lock, from being drummed out by a held key.
 */

static int is_modifier(uint32_t key)
{
	switch(key) {
		case KEY_LEFTSHIFT:
		case KEY_RIGHTSHIFT:
		case KEY_LEFTCTRL:
		case KEY_RIGHTCTRL:
		case KEY_LEFTALT:
		case KEY_RIGHTALT:
		case KEY_LEFTMETA:
		case KEY_RIGHTMETA:
		case KEY_CAPSLOCK:
		case KEY_NUMLOCK:
		case KEY_SCROLLLOCK:
			return 1;
		default:
			return 0;
	}
}


static void repeat_after(int ms)
{
	clock_gettime(CLOCK_MONOTONIC, &repeat_at);
	repeat_at.tv_nsec += (long)ms * 1000000;
	if(repeat_at.tv_nsec >= 1000000000) {
		repeat_at.tv_nsec -= 1000000000;
		repeat_at.tv_sec++;
	}
}


/*
 * Milliseconds left until the next repeat is due, 0 if it is due now and -1
 * if no key is repeating, which is also what poll() wants for "wait forever".
 */

static int repeat_timeout(void)
{
	struct timespec now;
	long long ms;

	if(repeat_key < 0) {
		return -1;
	}

	clock_gettime(CLOCK_MONOTONIC, &now);
	ms = (long long)(repeat_at.tv_sec - now.tv_sec) * 1000
	   + (repeat_at.tv_nsec - now.tv_nsec) / 1000000;

	return ms > 0 ? (int)ms : 0;
}


static void handle_key(struct libinput_event *ev)
{
	struct libinput_event_keyboard *k = libinput_event_get_keyboard_event(ev);
	enum libinput_key_state state = libinput_event_keyboard_get_key_state(k);
	uint32_t key = libinput_event_keyboard_get_key(k);
	int pressed = state == LIBINPUT_KEY_STATE_PRESSED;

	play(key, pressed);

	if(!repeat_enabled()) {
		return;
	}

	if(pressed) {
		/* A new key down cancels the repeat of the previous one, as
		 * the kernel does.  A period of zero is how the compositor
		 * says that keys are not to repeat at all. */
		repeat_key = (is_modifier(key) || repeat_period_ms <= 0)
		           ? -1 : (int)key;
		if(repeat_key >= 0) {
			repeat_after(repeat_delay_ms);
		}
	} else if((int)key == repeat_key) {
		repeat_key = -1;
	}
}

static int button_bit(uint32_t button)
{
	switch(button) {
		case BTN_LEFT:   return MOUSE_LEFT;
		case BTN_MIDDLE: return MOUSE_MIDDLE;
		case BTN_RIGHT:  return MOUSE_RIGHT;
		case BTN_SIDE:   return MOUSE_SIDE;
		case BTN_EXTRA:  return MOUSE_EXTRA;
		default:         return 0;
	}
}


static void handle_button(struct libinput_event *ev)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	enum libinput_button_state state = libinput_event_pointer_get_button_state(p);
	uint32_t button = libinput_event_pointer_get_button(p);

	if(mouse_enabled(button_bit(button)))
		play(0xff, state == LIBINPUT_BUTTON_STATE_PRESSED);
}

/*
 * A wheel detent is 120 units, the v120 convention libinput took from Windows.
 * A high resolution wheel reports fractions of that, so what is left over is
 * carried to the next event rather than rounded away.  Unlike a button there
 * is no press and release to follow, so a detent makes a single click.
 *
 * Only a real wheel is handled, not LIBINPUT_EVENT_POINTER_SCROLL_FINGER or
 * _CONTINUOUS: two fingers on a touchpad scroll smoothly rather than in
 * detents, and there is nothing there to click once per.
 */

#define WHEEL_DETENT 120

static void handle_scroll_axis(struct libinput_event_pointer *p,
		enum libinput_pointer_axis axis, int which, int *acc)
{
	double v120;

	if(!mouse_enabled(which) || !libinput_event_pointer_has_axis(p, axis)) {
		return;
	}

	v120 = libinput_event_pointer_get_scroll_value_v120(p, axis);
	*acc += (int)(v120 < 0 ? -v120 : v120);

	while(*acc >= WHEEL_DETENT) {
		play(0xff, 1);
		*acc -= WHEEL_DETENT;
	}
}


static void handle_scroll(struct libinput_event *ev)
{
	static int acc_vert = 0;
	static int acc_horz = 0;

	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);

	handle_scroll_axis(p, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
			MOUSE_WHEEL, &acc_vert);
	handle_scroll_axis(p, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL,
			MOUSE_HWHEEL, &acc_horz);
}


static void handle_events(struct libinput *li)
{
	struct libinput_event *ev;
		
	libinput_dispatch(li);

	while((ev = libinput_get_event(li))) {

		switch(libinput_event_get_type(ev)) {
			case LIBINPUT_EVENT_KEYBOARD_KEY:
				handle_key(ev);
				break;
			case LIBINPUT_EVENT_POINTER_BUTTON:
				handle_button(ev);
				break;
			case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
				handle_scroll(ev);
				break;
			default:
				break;
		}

		libinput_event_destroy(ev);
		libinput_dispatch(li);
	}
}


static void log_handler(struct libinput *li, enum libinput_log_priority priority,
		const char *format, va_list args)
{
	vprintf(format, args);
}


int scan(int verbose)
{
	struct udev *udev;
	struct libinput *li;

	udev = udev_new();
        if (!udev) {
                fprintf(stderr, "Failed to initialize udev\n");
                return -1;
        }

	li = libinput_udev_create_context(&interface, NULL, udev);
	if(!li) {
		fprintf(stderr, "Failed to initialize context\n");
		return -1;
	}

	if(verbose) {
		libinput_log_set_handler(li, log_handler);
		libinput_log_set_priority(li, LIBINPUT_LOG_PRIORITY_DEBUG);
	}

	if (libinput_udev_assign_seat(li, "seat0")) {
		fprintf(stderr, "Failed to set seat\n");
		return -1;
	}

	libinput_dispatch(li);

	/* The compositor, if there is one, is polled alongside the input
	 * devices so that a change to the repeat setting is picked up while
	 * we are running rather than only at startup. */

	struct pollfd fds[2];
	int nfds = 1;

	fds[0].fd = libinput_get_fd(li);
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	fds[1].fd = wl_repeat_open();
	fds[1].events = POLLIN;
	fds[1].revents = 0;
	if(fds[1].fd >= 0) {
		nfds = 2;
	}

	repeat_refresh();

	while(poll(fds, nfds, repeat_timeout()) > -1) {

		/* POLLHUP and friends matter as much as POLLIN here: once the
		 * connection is gone its descriptor is closed, and polling a
		 * closed descriptor would spin on POLLNVAL. */

		if(nfds == 2 && fds[1].revents != 0) {
			if(!wl_repeat_dispatch()) {
				nfds = 1;
			}
			repeat_refresh();
		}

		if(repeat_key >= 0 && repeat_timeout() == 0) {
			play(repeat_key, 1);
			repeat_after(repeat_period_ms);
		}

		handle_events(li);
	}

	return 0;
}


void open_console(void)
{
}

