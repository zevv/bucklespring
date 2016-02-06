
#ifdef WIN32

#include <windows.h>
#include <winuser.h>

#include "buckle.h"

LRESULT CALLBACK handle_kbh(int nCode, WPARAM wParam, LPARAM lParam);


static HHOOK kbh = NULL; 


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

	int updown = 0;
	int code = ev->scanCode;

	if(ev->vkCode == 27) {
		exit(0);
	}

	switch (wParam) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			updown = 1;
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			updown = 0;
			break;

		default:
			break;
	}

	play(code, updown);

	LRESULT hook_result = -1;
	if (nCode < 0) {
		hook_result = CallNextHookEx(kbh, nCode, wParam, lParam);
	}

	return hook_result;
}


#endif

