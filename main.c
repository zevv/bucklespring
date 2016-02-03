/*
 * OpenAL example
 *
 * Copyright(C) Florian Fainelli <f.fainelli@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/record.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>


#define TEST_ERROR(_msg)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		fprintf(stderr, _msg "\n");	\
		return -1;		\
	}


/* 
 * Horizontal position on keyboard for each key as they are located on my model-D
 */

static int keyloc[][32] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x66, 0x68, 0x1c, -1 },
	{ 0x01, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x6f, 0x6b, 0x6d, -1 },
	{ 0x29, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, -1 },
	{ 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x2b,-1 },
	{ 0x3a, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x1c, -1 },
	{ 0x2a, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, -1 },
	{ 0x1d, 0x7d, 0x38, 0x39, 0x64, 0x61, 0x67, -1 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x6c, 0x6a, -1 },
};


/*
 * Find horizontal position of the given key on the keyboard. returns -1.0 for
 * left to 1.0 for right 
 */

static double find_key_loc(int code)
{
	int row;
	int col, keycol = 0;

	for(row=0; row<8; row++) {
		for(col=0; col<32; col++) {
			if(keyloc[row][col] == code) keycol = col+1;
			if(keyloc[row][col] == -1) break;
		}
		if(keycol) {
			return -1 + 2.0 * (double) (keycol-1) / (col-1);
		}
	}
	return 0;
}


static int play(int code, int press)
{
	ALCenum error;
	
	code -= 8; /* X code to scan code? */

	static ALuint buf[512] = { 0 };
	static ALuint src[512] = { 0 };

	int idx = code + press * 256;

	if(src[idx] == 0) {

		char fname[256];
		snprintf(fname, sizeof(fname), "wav/%02x-%d.wav", code, press);

		buf[idx] = alutCreateBufferFromFile(fname);
		if(buf[idx] == 0) {
			fprintf(stderr, "%s\n", alutGetErrorString (alutGetError ()));
			return -1;
		}
	
		double x = find_key_loc(code);

		alGenSources((ALuint)1, &src[idx]);
		TEST_ERROR("source generation");

		alSource3f(src[idx], AL_POSITION, -x, 0, 1.0);

		alSourcei(src[idx], AL_BUFFER, buf[idx]);
		TEST_ERROR("buffer binding");

	}


	alSourcePlay(src[idx]);
	TEST_ERROR("source playing");

	printf("d\n");

	return 0;
}


void key_pressed_cb(XPointer arg, XRecordInterceptData *d) 
{
	if (d->category != XRecordFromServer)
		return;
    
	int key = ((unsigned char*) d->data)[1];
	int type = ((unsigned char*) d->data)[0] & 0x7F;
	int repeat = d->data[2] & 1;

	if(!repeat) {

		switch (type) {
			case KeyPress:
				play(key, 1);
				break;
			case KeyRelease:
				play(key, 0);
				break;
			default:
				break;
		}
	}

	XRecordFreeData (d);
}


int main(int argc, char **argv)
{
	ALboolean enumeration;
	const ALCchar *defaultDeviceName = argv[1];
	ALCdevice *device;
	ALCcontext *context;
	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	ALCenum error;


	/* Create openal context */

	alutInit(0, NULL);

	if (!defaultDeviceName)
		defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

	device = alcOpenDevice(defaultDeviceName);
	if (!device) {
		fprintf(stderr, "unable to open default device\n");
		return -1;
	}

	alGetError();

	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context)) {
		fprintf(stderr, "failed to make default context\n");
		return -1;
	}
	TEST_ERROR("make default context");
		
	alListener3f(AL_POSITION, 0, 0, 0);
	alListener3f(AL_VELOCITY, 0, 0, 0);
	alListenerfv(AL_ORIENTATION, listenerOri);


	/* Initialize and start Xrecord context */

	Display *dpy = XOpenDisplay(NULL);
	Window win = XDefaultRootWindow(dpy);
    
	XRecordRange* rr;
	XRecordClientSpec rcs;
	XRecordContext rc;

	rr = XRecordAllocRange ();
	if(rr == NULL) {
		fprintf(stderr, "XRecordAllocRange error\n");
		exit(1);
	}

	rr->device_events.first = KeyPress;
	rr->device_events.last = KeyRelease;
	
	rcs = XRecordAllClients;

	rc = XRecordCreateContext (dpy, 0, &rcs, 1, &rr, 1);
	if(rc == 0) {
		fprintf(stderr, "XRecordCreateContext error\n");
		exit(1);
	}

	XFree (rr);

	if(XRecordEnableContext(dpy, rc, key_pressed_cb, NULL) == NULL) {
		fprintf(stderr, "XRecordEnableContext error\n");
		exit(1);
	}


	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);

	return 0;
}
