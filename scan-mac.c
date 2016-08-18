
#ifdef __APPLE__

#include <ApplicationServices/ApplicationServices.h>
#include "buckle.h"

/* 
 * From https://www.virtualbox.org/svn/vbox/trunk/src/VBox/Frontends/VirtualBox/src/platform/darwin/DarwinKeyboard.cpp 
 */

static const int mactoset1[] =
{
	/*  set-1                           SDL_QuartzKeys.h    */
	0x1e,            /* QZ_a            0x00 */
	0x1f,            /* QZ_s            0x01 */
	0x20,            /* QZ_d            0x02 */
	0x21,            /* QZ_f            0x03 */
	0x23,            /* QZ_h            0x04 */
	0x22,            /* QZ_g            0x05 */
	0x2c,            /* QZ_z            0x06 */
	0x2d,            /* QZ_x            0x07 */
	0x2e,            /* QZ_c            0x08 */
	0x2f,            /* QZ_v            0x09 */
	0x56,            /* between lshift and z. 'INT 1'? */
	0x30,            /* QZ_b            0x0B */
	0x10,            /* QZ_q            0x0C */
	0x11,            /* QZ_w            0x0D */
	0x12,            /* QZ_e            0x0E */
	0x13,            /* QZ_r            0x0F */
	0x15,            /* QZ_y            0x10 */
	0x14,            /* QZ_t            0x11 */
	0x02,            /* QZ_1            0x12 */
	0x03,            /* QZ_2            0x13 */
	0x04,            /* QZ_3            0x14 */
	0x05,            /* QZ_4            0x15 */
	0x07,            /* QZ_6            0x16 */
	0x06,            /* QZ_5            0x17 */
	0x0d,            /* QZ_EQUALS       0x18 */
	0x0a,            /* QZ_9            0x19 */
	0x08,            /* QZ_7            0x1A */
	0x0c,            /* QZ_MINUS        0x1B */
	0x09,            /* QZ_8            0x1C */
	0x0b,            /* QZ_0            0x1D */
	0x1b,            /* QZ_RIGHTBRACKET 0x1E */
	0x18,            /* QZ_o            0x1F */
	0x16,            /* QZ_u            0x20 */
	0x1a,            /* QZ_LEFTBRACKET  0x21 */
	0x17,            /* QZ_i            0x22 */
	0x19,            /* QZ_p            0x23 */
	0x1c,            /* QZ_RETURN       0x24 */
	0x26,            /* QZ_l            0x25 */
	0x24,            /* QZ_j            0x26 */
	0x28,            /* QZ_QUOTE        0x27 */
	0x25,            /* QZ_k            0x28 */
	0x27,            /* QZ_SEMICOLON    0x29 */
	0x2b,            /* QZ_BACKSLASH    0x2A */
	0x33,            /* QZ_COMMA        0x2B */
	0x35,            /* QZ_SLASH        0x2C */
	0x31,            /* QZ_n            0x2D */
	0x32,            /* QZ_m            0x2E */
	0x34,            /* QZ_PERIOD       0x2F */
	0x0f,            /* QZ_TAB          0x30 */
	0x39,            /* QZ_SPACE        0x31 */
	0x29,            /* QZ_BACKQUOTE    0x32 */
	0x0e,            /* QZ_BACKSPACE    0x33 */
	0x9c,            /* QZ_IBOOK_ENTER  0x34 */
	0x01,            /* QZ_ESCAPE       0x35 */
	0x5c,            /* QZ_RMETA        0x36 */
	0x5b,            /* QZ_LMETA        0x37 */
	0x2a,            /* QZ_LSHIFT       0x38 */
	0x3a,            /* QZ_CAPSLOCK     0x39 */
	0x38,            /* QZ_LALT         0x3A */
	0x1d,            /* QZ_LCTRL        0x3B */
	0x36,            /* QZ_RSHIFT       0x3C */
	0x38,            /* QZ_RALT         0x3D */
	0x1d,            /* QZ_RCTRL        0x3E */
	0,               /*                      */
	0,               /*                      */
	0x53,            /* QZ_KP_PERIOD    0x41 */
	0,               /*                      */
	0x37,            /* QZ_KP_MULTIPLY  0x43 */
	0,               /*                      */
	0x4e,            /* QZ_KP_PLUS      0x45 */
	0,               /*                      */
	0x45,            /* QZ_NUMLOCK      0x47 */
	0,               /*                      */
	0,               /*                      */
	0,               /*                      */
	0x35,            /* QZ_KP_DIVIDE    0x4B */
	0x1c,            /* QZ_KP_ENTER     0x4C */
	0,               /*                      */
	0x4a,            /* QZ_KP_MINUS     0x4E */
	0,               /*                      */
	0,               /*                      */
	0x0d/*?*/,       /* QZ_KP_EQUALS    0x51 */
	0x52,            /* QZ_KP0          0x52 */
	0x4f,            /* QZ_KP1          0x53 */
	0x50,            /* QZ_KP2          0x54 */
	0x51,            /* QZ_KP3          0x55 */
	0x4b,            /* QZ_KP4          0x56 */
	0x4c,            /* QZ_KP5          0x57 */
	0x4d,            /* QZ_KP6          0x58 */
	0x47,            /* QZ_KP7          0x59 */
	0,               /*                      */
	0x48,            /* QZ_KP8          0x5B */
	0x49,            /* QZ_KP9          0x5C */
	0x7d,            /* yen, | (JIS)    0x5D */
	0x73,            /* _, ro (JIS)     0x5E */
	0,               /*                      */
	0x3f,            /* QZ_F5           0x60 */
	0x40,            /* QZ_F6           0x61 */
	0x41,            /* QZ_F7           0x62 */
	0x3d,            /* QZ_F3           0x63 */
	0x42,            /* QZ_F8           0x64 */
	0x43,            /* QZ_F9           0x65 */
	0x29,            /* Zen/Han (JIS)   0x66 */
	0x57,            /* QZ_F11          0x67 */
	0x29,            /* Zen/Han (JIS)   0x68 */
	0x37,            /* QZ_PRINT / F13  0x69 */
	0x63,            /* QZ_F16          0x6A */
	0x46,            /* QZ_SCROLLOCK    0x6B */
	0,               /*                      */
	0x44,            /* QZ_F10          0x6D */
	0x5d,            /*                      */
	0x58,            /* QZ_F12          0x6F */
	0,               /*                      */
	0/* 0xe1,0x1d,0x45*/, /* QZ_PAUSE        0x71 */
	0x52,            /* QZ_INSERT / HELP 0x72 */
	0x47,            /* QZ_HOME         0x73 */
	0x49,            /* QZ_PAGEUP       0x74 */
	0x53,            /* QZ_DELETE       0x75 */
	0x3e,            /* QZ_F4           0x76 */
	0x4f,            /* QZ_END          0x77 */
	0x3c,            /* QZ_F2           0x78 */
	0x51,            /* QZ_PAGEDOWN     0x79 */
	0x3b,            /* QZ_F1           0x7A */
	0x4b,            /* QZ_LEFT         0x7B */
	0x4d,            /* QZ_RIGHT        0x7C */
	0x50,            /* QZ_DOWN         0x7D */
	0x48,            /* QZ_UP           0x7E */
	0,/*0x5e|K_EX*/  /* QZ_POWER        0x7F */ /* have different break key! */
	/* do NEVER deliver the Power
	 * scancode as e.g. Windows will
	 * handle it, @bugref{7692}. */
};


/* 
 * Adapted from https://danielbeard.wordpress.com/2010/10/29/listening-for-global-keypresses-in-osx/ 
 */

CGEventRef myCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
	if ((type != kCGEventKeyDown) && (type != kCGEventKeyUp))
		return event;

	int mackeycode = (int)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
	printd("Mac keycode: %d", mackeycode);

	if (mackeycode >= sizeof(mactoset1)/sizeof(mactoset1[0]))
		return event;

	int key = mactoset1[mackeycode];

	if (CGEventGetIntegerValueField(event, kCGKeyboardEventAutorepeat))
		return event;

	switch (type) {
		case kCGEventKeyDown:
			play(key, 1);
			break;
		case kCGEventKeyUp:
			play(key, 0);
			break;
		default:
			break;
	}

	return event;
}


int scan(void)
{
	CFMachPortRef      eventTap;
	CGEventMask        eventMask;
	CFRunLoopSourceRef runLoopSource;

	/* Create an event tap. We are interested in key presses. */

	eventMask = ((1 << kCGEventKeyDown) | (1 << kCGEventKeyUp));
	eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, myCGEventCallback, NULL);
	if (!eventTap) {
		fprintf(stderr, "failed to create event tap\n");
		exit(1);
	}

	/* Create a run loop source. */
	runLoopSource = CFMachPortCreateRunLoopSource( kCFAllocatorDefault, eventTap, 0);

	/* Add to the current run loop. */
	CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);

	/* Enable the event tap. */
	CGEventTapEnable(eventTap, true);

	/* Set it all running. */
	CFRunLoopRun();

	return 0;
}


void open_console(void)
{
}

#endif

