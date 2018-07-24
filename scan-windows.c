#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "buckle.h"

void open_console();
LRESULT CALLBACK handle_kbh(int nCode, WPARAM wParam, LPARAM lParam);


static HHOOK kbh = NULL; 
static int state[256] = { 0 };


int scan(int verbose)
{
	HINSTANCE hInst = GetModuleHandle(NULL);

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
			play(code, updown, 0);
			state[code] = updown;
		}
	}

	return CallNextHookEx(kbh, nCode, wParam, lParam);
}


void open_console()
{
	int hConHandle;
	INT_PTR lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	FILE *fp;

	AllocConsole();

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = 500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	lStdHandle = (INT_PTR)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	*stderr = *fp;
	setvbuf(fp, NULL, _IONBF, 0 );
}
