#include <stdio.h>

#include <X11/XKBlib.h>
#include <X11/extensions/record.h>

#include "buckle.h"

void key_pressed_cb(XPointer arg, XRecordInterceptData *d);


/*
 * X numbers the pointer buttons from one and puts the wheel among them:
 * 4 and 5 are up and down, 6 and 7 are the sideways pair, and 8 and 9 are the
 * thumb buttons.  Anything above that is a button we have no name for.
 */

static int button_bit(int button)
{
	switch(button) {
		case 1: return MOUSE_LEFT;
		case 2: return MOUSE_MIDDLE;
		case 3: return MOUSE_RIGHT;
		case 4:
		case 5: return MOUSE_WHEEL;
		case 6:
		case 7: return MOUSE_HWHEEL;
		case 8: return MOUSE_SIDE;
		case 9: return MOUSE_EXTRA;
		default: return 0;
	}
}


/* A wheel detent arrives as a press and a release in immediate succession.
 * Sounding both would make two clicks out of one notch, so only the press is
 * played, which is what the libinput backend does with a detent as well. */

static int is_wheel(int button)
{
	return button >= 4 && button <= 7;
}

int scan(int verbose)
{
	/* Initialize and start Xrecord context */
	
	XRecordRange* rr;
	XRecordClientSpec rcs;
	XRecordContext rc;

	printd("Opening Xrecord context");

	Display *dpy = XOpenDisplay(NULL);
	if(dpy == NULL) {
		fprintf(stderr, "Unable to open display\n");
		return -1;
	}
    
	rr = XRecordAllocRange ();
	if(rr == NULL) {
		fprintf(stderr, "XRecordAllocRange error\n");
		return -1;
	}

	rr->device_events.first = KeyPress;
	rr->device_events.last = ButtonReleaseMask;
	rcs = XRecordAllClients;

	rc = XRecordCreateContext (dpy, 0, &rcs, 1, &rr, 1);
	if(rc == 0) {
		fprintf(stderr, "XRecordCreateContext error\n");
		return -1;
	}

	XFree (rr);

	if(XRecordEnableContext(dpy, rc, key_pressed_cb, NULL) == 0) {
		fprintf(stderr, "XRecordEnableContext error\n");
		return -1;
	}

	/* We never get here */

	return 0;
}


/*
 * Xrecord event callback
 */

void key_pressed_cb(XPointer arg, XRecordInterceptData *d) 
{
	if (d->category != XRecordFromServer)
		return;
    
	int detail = ((unsigned char*) d->data)[1];
	int key = detail;
	int type = ((unsigned char*) d->data)[0] & 0x7F;

	/* The X server marks the KeyPress events it generates while a key is
	 * held down, and sends no KeyRelease in between, so a repeat is one
	 * extra click.  Modifiers do not repeat, XKB does not mark them as
	 * repeating keys, so holding shift stays quiet by itself. */

	int repeat = d->data[2] & 1;

	key -= 8; /* X code to scan code? */

	if(!repeat || repeat_enabled()) {

		switch (type) {
			case KeyPress:
				play(key, 1);
				break;
			case KeyRelease:
				play(key, 0);
				break;
			case ButtonPress:
				if(mouse_enabled(button_bit(detail)))
					play(0xff, 1);
				break;
			case ButtonRelease:
				if(!is_wheel(detail) && mouse_enabled(button_bit(detail)))
					play(0xff, 0);
				break;
			default:
				break;
		}
	}

	XRecordFreeData (d);
}


void open_console(void)
{
}
