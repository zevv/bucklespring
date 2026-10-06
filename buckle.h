#ifndef buckle_h
#define buckle_h

int play(int code, int press);

/*
 * The mouse events that can be made to click, as a bit set, for --no-click.
 * "side" and "extra" are the two thumb buttons, X buttons 8 and 9; "hwheel"
 * is the sideways tilt or second wheel.
 */

#define MOUSE_LEFT	(1 << 0)
#define MOUSE_MIDDLE	(1 << 1)
#define MOUSE_RIGHT	(1 << 2)
#define MOUSE_SIDE	(1 << 3)
#define MOUSE_EXTRA	(1 << 4)
#define MOUSE_WHEEL	(1 << 5)
#define MOUSE_HWHEEL	(1 << 6)

#define MOUSE_ALL	(MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT | \
			 MOUSE_SIDE | MOUSE_EXTRA | \
			 MOUSE_WHEEL | MOUSE_HWHEEL)

int mouse_enabled(int which);

/* What the tray drives the program through; all of it is thread safe. */

int buckle_muted(void);
void buckle_set_muted(int on);
int buckle_gain(void);
void buckle_set_gain(int gain);
int buckle_mute_keycode(void);
const char *buckle_audio_device(void);
int buckle_set_audio_device(const char *name);
char **buckle_audio_devices(void);
void buckle_audio_devices_free(char **list);
void buckle_quit(void);

/* tray.c, built only when configured with a system tray */

int tray_init(void);
void tray_run(void);
int repeat_enabled(void);
void repeat_override(int *delay_ms, int *period_ms);
int scan(int verbose);
void printd(const char *fmt, ...);
void open_console(void);

/* wl-repeat.c, built only for the libinput backend */

int wl_repeat_open(void);
int wl_repeat_dispatch(void);
int wl_repeat_get(int *delay_ms, int *period_ms);

#endif
