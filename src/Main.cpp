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

	// window class struct
	// every window must be associated with one
	WNDCLASS wc = {}; 
	wc.lpszClassName  = CLASS_NAME;                  // class name, must be unique within the process
	wc.lpfnWndProc    = &ClockWinProc;               // function pointer to a custom window procedure
	wc.hInstance      = hInstance;                   // handle to application instance, get it from wWinMain
	wc.hCursor        = LoadCursor(NULL, IDC_ARROW); // load a cursor, or the cursor will show as a busy spinner

	RegisterClass(&wc);

	int x = CW_USEDEFAULT;
	int y = CW_USEDEFAULT;
	int w = 75;
	int h = 22;

	HWND hwnd = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_LAYERED, // Optional window styles.
		CLASS_NAME,                     // Window class
		NULL,                           // Window text
		WS_POPUP,                       // Window style - popup means we can have any size! Used overlap before, that required some min size

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
	SetLayeredWindowAttributes(hwnd, transparent, 0, LWA_COLORKEY);
	SetWindowLong(hwnd, GWL_STYLE, WS_EX_TRANSPARENT);
	ShowWindow(hwnd, SW_SHOW);

	SetTimer(hwnd, 1, 1000, NULL); // update in one second
	UpdateWindow(hwnd);            // update once right now


	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

struct WinDrag {
	RECT winDragStart;
	POINT mouseDragStart;
	bool isDragging;
};
WinDrag winDrag = { 0 };


LRESULT CALLBACK ClockWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {

		case WM_CLOSE: {
			DestroyWindow(hwnd);
			return 0;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		case WM_RBUTTONDOWN:
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
		case WM_LBUTTONDOWN: {
			GetWindowRect(hwnd, &winDrag.winDragStart);
			GetCursorPos(&winDrag.mouseDragStart);
			winDrag.isDragging = true;
			SetCapture(hwnd); // SetCapture allows us to recieve events even when mouse is outside of our window, which might happen if you drag too fast
			break;
		}
		case WM_LBUTTONUP: {
			winDrag.isDragging = false;
			ReleaseCapture(); // ReleaseCapture after calling SetCapture to go back to normal
			break;
		}
		case WM_MOUSEMOVE: {
			if (winDrag.isDragging) {
				POINT curr = { 0 };
				GetCursorPos(&curr);

				long dx = curr.x - winDrag.mouseDragStart.x;
				long dy = curr.y - winDrag.mouseDragStart.y;
				int w = winDrag.winDragStart.right - winDrag.winDragStart.left;
				int h = winDrag.winDragStart.bottom - winDrag.winDragStart.top;
				SetWindowPos(hwnd, NULL, winDrag.winDragStart.left + dx, winDrag.winDragStart.top + dy, w, h, 0);
			}
			break;
		}
		case WM_TIMER: {                     // WM_TIMER is invoked from SetTimer once per second
			InvalidateRect(hwnd, NULL, true); // will invoke a WM_PAINT event
			SetTimer(hwnd, 1, 1000, NULL);    // call this timer again in one second
			break;
		}
		case WM_PAINT: {                     // WM_PAINT invoked from InvalidateRect()
			PAINTSTRUCT paint = { 0 };
			HDC hdc = BeginPaint(hwnd, &paint);

			SYSTEMTIME time = { 0 };
			GetLocalTime(&time);
			WORD hour = time.wHour;
			WORD min  = time.wMinute;
			WORD sec  = time.wSecond;

			const char* preHour = hour <= 9 ? "0" : "";
			const char* preMin  = min <= 9 ?  "0" : "";
			const char* preSec  = sec <= 9 ?  "0" : "";

			char timeStr[16] = { 0 };
			sprintf_s(timeStr, 16, "%s%d:%s%d:%s%d", preHour, hour, preMin, min, preSec, sec);
			int len = 0;
			while (timeStr[len++] != NULL && len < 16) {} // LoL - get the lenght of the text

			RECT rect = { 0 };
			GetClientRect(hwnd, &rect);
			int w = rect.right - rect.left;
			int h = rect.bottom - rect.top;
			SetBkColor(hdc, RGB(0, 0, 0));
			SetTextColor(hdc, RGB(0, 255, 0));
			TextOutA(hdc, rect.left + 10, rect.top + 2, timeStr, len);
			EndPaint(hwnd, &paint);
			break;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


