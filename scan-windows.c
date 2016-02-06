
#ifdef WIN32

#include <windows.h>
#include <winuser.h>
#include <stdio.h>

#include "buckle.h"

LRESULT CALLBACK handle_kbh(int nCode, WPARAM wParam, LPARAM lParam);


static HHOOK kbh = NULL; 
static int state[256] = { 0 };


int scan(void)
{
	HINSTANCE hInst;

	kbh = SetWindowsHookEx(WH_KEYBOARD_LL, handle_kbh, hInst, 0);

	MSG msg;

	while(GetMessage(&msg, (HWND) NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


LRESULT CALLBACK handle_kbh(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *ev = (KBDLLHOOKSTRUCT *)lParam;

	printd("vkCode=%d scanCode=%d flags=%d time=%d",
			(int)ev->vkCode, (int)ev->scanCode, (int)ev->flags, (int)ev->time);

	int updown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
	int code = ev->scanCode;

	if(code < 256) {
		if(state[code] != updown) {
			play(code, updown);
			state[code] = updown;
		}
	}

	return CallNextHookEx(kbh, nCode, wParam, lParam);
}


#endif

