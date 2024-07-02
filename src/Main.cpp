/*
	Note: the only libraries you need to link against for this single class c++ project to work are
	kernel32.lib;user32.lib;gdi32.lib;
*/

#include <windows.h>
#include <stdio.h>
#include <sysinfoapi.h> // GetLocalTime()

const wchar_t CLASS_NAME[] = L"Tommys.Desk.Clock";

LRESULT CALLBACK ClockWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
COLORREF transparent = RGB(255, 0, 0);


_Use_decl_annotations_ int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

	// We are doing pixel stuff, so don't let Windows scale window units with the system set DPI
	//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

	// window class struct
	// every window must be associated with one
	WNDCLASS wc = {};
	wc.lpszClassName  = CLASS_NAME;                  // class name, must be unique within the process
	wc.lpfnWndProc    = &ClockWinProc;               // function pointer to a custom window procedure
	wc.hInstance      = hInstance;                   // handle to application instance, get it from wWinMain
	wc.hCursor        = LoadCursor(NULL, IDC_ARROW); // load a cursor, or the cursor will show as a busy spinner
	wc.hbrBackground  = NULL;                        // we'll draw it ourselves!
	RegisterClass(&wc);

	int x = CW_USEDEFAULT;
	int y = CW_USEDEFAULT;
	int w = 75;
	int h = 22;

	HWND hwnd = CreateWindowEx(
		WS_EX_TOPMOST,                  // Optional window styles.
		CLASS_NAME,                     // Window class
		NULL,                           // Window text
		WS_POPUP/* | WS_VISIBLE*/,      // Window style - popup means we can have any size! Used overlap before, that required some min size

		// Size and position
		x, y, w, h,

		NULL,       // Parent window
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		OutputDebugString(L"error, no window handle\n");
		return -1;
	}

	SetTimer(hwnd, 1, 0, NULL);    // do first update immediately to set the time

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK ClockWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	static SYSTEMTIME time = { 0 };
	static WORD wOldSecond = 0;

	switch (uMsg) {

		case WM_CLOSE: {
			DestroyWindow(hwnd);
			return 0;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		case WM_NCRBUTTONDOWN:
		{
			// Right click inside window to get a popup asking if you want to quit
			int v = MessageBoxW(hwnd, TEXT("Are you sure you want to quit?"), TEXT("TClock"), MB_OKCANCEL);
			if (v == IDOK) {
				PostQuitMessage(0);
			}
		}

		case WM_KEYUP: {
			// Click ESC with window focus to get a popup asking if you want to quit
			if (wParam == VK_ESCAPE) {
				int v = MessageBoxW(hwnd, TEXT("Are you sure you want to quit?"), TEXT("TClock"), MB_OKCANCEL);
				if (v == IDOK) {
					PostQuitMessage(0);
				}
			}
			break;
		}

		case WM_NCHITTEST:
			// wherever you touch, Windows will think you touched the caption of this window even when it doesn't have a caption to begin with
			return HTCAPTION;

		case WM_TIMER: {                     // WM_TIMER is invoked from SetTimer() once per second
			
			// Wait until a new second (because I am a clock freak)
			while (time.wSecond == wOldSecond)
			{
				// CPU resources matters, this while loop will eat too much if not sleeping a little
				Sleep(10);

				// Get the time here
				GetLocalTime(&time);
			}

			wOldSecond = time.wSecond;

			// show window (purest form, still need to invalidate to draw)
			SetWindowLongPtrA(hwnd, GWL_STYLE, GetWindowLongPtrA(hwnd, GWL_STYLE) | WS_VISIBLE);

			InvalidateRect(hwnd, NULL, true); // will invoke a WM_PAINT event
			SetTimer(hwnd, 1, 800, NULL);     // call this timer again in one second (less to get better percision, the while loop takes care of the remaining milliseconds)

			break;
		}
		case WM_PAINT: {                     // WM_PAINT invoked from InvalidateRect()
			PAINTSTRUCT paint = { 0 };
			HDC hdc = BeginPaint(hwnd, &paint);

			char timeStr[16] = { 0 };
			sprintf_s(timeStr, 9, "%02d:%02d:%02d", time.wHour, time.wMinute, time.wSecond);

			RECT rect = { 0 };
			GetClientRect(hwnd, &rect);

			//HBRUSH hbrBlack = CreateSolidBrush(RGB(0, 0, 0));    // always create gdi object per paint
			//HGDIOBJ hbrOld = SelectObject(hdc, hbrBlack);        // always remember the old brush to select back
			//Rectangle(hdc, 0, 0, rect.right, rect.bottom);       // we are using GDI to draw, if we use FillRect() from USER, we don't need to select the brush, USER does that internally
			PatBlt(hdc, 0, 0, rect.right, rect.bottom, BLACKNESS); // hey, I remember PatBlt()! Still GDI drawing, but only the pattern (which is a solid brush), no outline drawing!
			//FillRect(hdc, &rect, hbrBlack);                      // when we let USER do all the heavy work of calling GDI and managing the brush
			//SelectObject(hdc, hbrOld);                           // Select old back (DCs are cached, meaning they will be reused, we can't leave a deleted brush in it)
			//DeleteObject(hbrBlack);                              // Delete the brush (they are also cached for memory performace, so this is actually fast to create and delete each paint)

			SIZE sz;
			GetTextExtentPointA(hdc, timeStr, 1, &sz);

			SetBkColor(hdc, RGB(0, 0, 0));
			SetTextColor(hdc, RGB(0, 255, 0));
			SetTextAlign(hdc, TA_CENTER);
			ExtTextOutA(hdc, (rect.right / 2) + (sz.cx / 2), (rect.bottom - sz.cy) / 2, 0, &rect, timeStr, 9, NULL);

			EndPaint(hwnd, &paint);
			break;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


