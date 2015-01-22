////////////////////////////////////////////////////////////////////////////////
//
//  Scope - An Audio Oscilloscope written in C.
//
//  Copyright (C) 2010  Bill Farmer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//  Bill Farmer  william j farmer [at] tiscali [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#define WINVER       0x0500
#define _WIN32_IE    0x0501
#define _WIN32_WINNT 0x0500

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

// Macros

#define Length(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"
#define PCLASS "PopupClass"

// Tool ids

enum
    {SCOPE_ID = 100,
     XSCALE_ID,
     YSCALE_ID,
     TOOLBAR_ID,
     STATUS_ID,

     LEVEL_ID,
     BRIGHT_ID,
     SINGLE_ID,
     TRIGGER_ID,
     SYNC_ID,
     TIMEBASE_ID,
     STORAGE_ID,
     CLEAR_ID,
     LEFT_ID,
     RIGHT_ID,
     START_ID,
     END_ID,
     RESET_ID};

// Bitmap ids

enum
    {LEVEL_BM,
     BRIGHT_BM,
     SINGLE_BM,
     TRIGGER_BM,
     POSITIVE_BM,
     NEGATIVE_BM,
     TIMEBASE_BM,
     STORAGE_BM,
     CLEAR_BM,
     LEFT_BM,
     RIGHT_BM,
     START_BM,
     END_BM,
     RESET_BM};

enum
    {HEIGHT     = 480,
     WIDTH      = 640,
     MAX_HEIGHT = 768,
     MAX_WIDTH = 1024};

enum
    {SCALE_HEIGHT = 24,
     SCALE_WIDTH  = 16};

// Wave in values

enum
    {SAMPLE_RATE = 44100L,
     BITS_PER_SAMPLE = 16,
     BLOCK_ALIGN = 2,
     CHANNELS = 1};

// Audio processing values

enum
    {SAMPLES = 262144,
     STEP = 4096};

// Slider values

enum
    {MAX_VOL  = 100,
     MIN_VOL  = 0,
     STEP_VOL = 10};

// State machine values

enum
    {INIT,
     FIRST,
     NEXT,
     LAST};

// Global data

HINSTANCE hInst;

typedef struct
{
    HWND hwnd;
    RECT rect;
} TOOL, *TOOLP;

TOOL window;
TOOL popup;

TOOL toolbar;
TOOL status;

typedef struct
{
    HWND hwnd;
    TOOLINFO info;
} TOOLTIP, *TOOLTIPP;

TOOLTIP tooltip;

typedef struct
{
    HWND hwnd;
    int index;
    int length;
    short *data;
    float scale;
    float start;
    BOOL bright;
    BOOL single;
    BOOL trigger;
    BOOL polarity;
    BOOL storage;
    BOOL clear;
} SCOPE, *SCOPEP;

SCOPE scope;

typedef struct
{
    HWND hwnd;
    RECT rect;
    float step;
    float start;
    float scale;
} XSCALE, *XSCALEP;

XSCALE xscale;

TOOL yscale;
TOOL level;

typedef struct
{
    DWORD id;
    HWAVEIN hwi;
    HANDLE thread;
} AUDIO, *AUDIOP;

AUDIO audio;

typedef struct
{
    HMIXER hmx;
    MIXERLINE *pmxl;
    MIXERCONTROL *pmxc;
    MIXERLINECONTROLS *pmxlc;
    MIXERCONTROLDETAILS *pmxcd;
    MIXERCONTROLDETAILS_UNSIGNED *pmxcdu;
} MIXER, *MIXERP;

MIXER mixer;

typedef struct
{
    int index;
    float values[12];
    char *strings[12];
    int counts[12];
} TIMEBASE, *TIMEBASEP;

TIMEBASE timebase =
    {3,
     {0.1, 0.2, 0.5, 1.0,
      2.0, 5.0, 10.0, 20.0,
      50.0, 100.0, 200.0, 500.0},
     {"0.1 ms", "0.2 ms", "0.5 ms",
      "1.0 ms", "2.0 ms", "5.0 ms",
      "10 ms", "20 ms", "50 ms",
      "0.1 sec", "0.2 sec", "0.5 sec"},
     {128, 256, 512, 1024,
      2048, 4096, 8192, 16384,
      32768, 65536, 131072, 262144}};

// Function prototypes.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PopupProc(HWND, UINT, WPARAM, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
BOOL RegisterPopupClass(HINSTANCE);
BOOL DisplayLevelControl(HWND, WPARAM, LPARAM);
BOOL DisplayTimebaseMenu(HWND, WPARAM, LPARAM);
BOOL CALLBACK EnumChildProc(HWND, LPARAM);
BOOL FocusLost(HWND, WPARAM, LPARAM);
BOOL ChangeLevel(WPARAM, LPARAM);
BOOL LevelChange(WPARAM, LPARAM);
BOOL ScopeClicked(WPARAM, LPARAM);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawXScale(HDC, RECT);
BOOL DrawYScale(HDC, RECT);
BOOL DrawYScale(HDC, RECT);
BOOL DrawScope(HDC, RECT);
BOOL AddToolbarBitmap(HWND, LPCTSTR);
BOOL AddToolbarButtons(HWND);
BOOL WindowResizing(HWND, WPARAM, LPARAM);
DWORD WINAPI AudioThread(LPVOID);
VOID WaveInData(WPARAM, LPARAM);
VOID KeyDown(WPARAM, LPARAM);
BOOL UpdateStatus(VOID);

// Application entry point.

int WINAPI WinMain(HINSTANCE hInstance,
		   HINSTANCE hPrevInstance,
		   LPSTR lpszCmdLine,
		   int nCmdShow)
{
    // Initialize common controls to get the new style controls, also
    // dependent on manifest file

    InitCommonControls();

    // Check for a previous instance of this app

    if (!hPrevInstance)
	if (!RegisterMainClass(hInstance))
	    return FALSE;

    // Save the application-instance handle.

    hInst = hInstance;

    // Get saved status

    // GetStatus();

    // Create the main window.

    window.hwnd =
	CreateWindow(WCLASS, "Audio Oscilloscope",
		     WS_OVERLAPPED | WS_MINIMIZEBOX |
		     WS_SIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,
		     CW_USEDEFAULT, CW_USEDEFAULT,
		     WIDTH, HEIGHT,
		     NULL, 0, hInst, NULL);

    // If the main window cannot be created, terminate
    // the application.

    if (!window.hwnd)
	return FALSE;

    // Show the window and send a WM_PAINT message to the window
    // procedure.

    ShowWindow(window.hwnd, nCmdShow);
    UpdateWindow(window.hwnd);

    // Process messages

    MSG msg;
    BOOL flag;

    while ((flag = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0)
    {
	if (flag == -1)
	    break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Register class

BOOL RegisterMainClass(HINSTANCE hInst)
{
    // Fill in the window class structure with parameters
    // that describe the main window.

    WNDCLASS wc = 
	{0, MainWndProc, 0, 0, hInst,
	 LoadIcon(hInst, "Scope"),
	 LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, WCLASS};

    // Register the window class.

    return RegisterClass(&wc);
}

// Main window procedure

LRESULT CALLBACK MainWndProc(HWND hWnd,
			     UINT uMsg,
			     WPARAM wParam,
			     LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:

	// Get the window and client dimensions

	GetClientRect(hWnd, &window.rect);

	int width = window.rect.right - window.rect.left;
	int height = window.rect.bottom - window.rect.top;

	// Create toolbar

	toolbar.hwnd =
	    CreateWindow(TOOLBARCLASSNAME, NULL,
			 WS_VISIBLE | WS_CHILD | TBSTYLE_TOOLTIPS,
			 0, 0, 0, 0,
			 hWnd, (HMENU)TOOLBAR_ID, hInst, NULL);

	SendMessage(toolbar.hwnd, TB_BUTTONSTRUCTSIZE, 
		    (WPARAM)sizeof(TBBUTTON), 0);

	SendMessage(toolbar.hwnd, TB_SETBITMAPSIZE, 0, MAKELONG(24, 24));
	SendMessage(toolbar.hwnd, TB_SETMAXTEXTROWS, 0, 0);

	// Add bitmap

	AddToolbarBitmap(toolbar.hwnd, "Toolbar");

	// Add buttons

	AddToolbarButtons(toolbar.hwnd);

	// Resize toolbar

	SendMessage(toolbar.hwnd, TB_AUTOSIZE, 0, 0); 

	GetWindowRect(toolbar.hwnd, &toolbar.rect);
	MapWindowPoints(NULL, hWnd, (POINT *)&toolbar.rect, 2);

	// Create status bar

	status.hwnd =
	    CreateWindow(STATUSCLASSNAME, " Timebase: 1.0ms",
			 WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP,
			 0, 0, 0, 0,
			 hWnd, (HMENU)STATUS_ID, hInst, NULL);

	GetWindowRect(status.hwnd, &status.rect);
	MapWindowPoints(NULL, hWnd, (POINT *)&status.rect, 2);

	// Create tooltip

	tooltip.hwnd =
	    CreateWindow(TOOLTIPS_CLASS, NULL,
			 WS_POPUP | TTS_ALWAYSTIP,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 hWnd, NULL, hInst, NULL);

	SetWindowPos(tooltip.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
		     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	tooltip.info.cbSize = sizeof(tooltip.info);
	tooltip.info.hwnd = hWnd;
	tooltip.info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;

	// Create X scale

	xscale.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 0, status.rect.top - SCALE_HEIGHT,
			 width, SCALE_HEIGHT,
			 hWnd, (HMENU)XSCALE_ID, hInst, NULL);

	GetWindowRect(xscale.hwnd, &xscale.rect);
	MapWindowPoints(NULL, hWnd, (POINT *)&xscale.rect, 2);

	// Add X scale to tooltip

	tooltip.info.uId = (UINT_PTR)xscale.hwnd;
	tooltip.info.lpszText = "X scale";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create Y scale

	yscale.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 0, toolbar.rect.bottom,
			 SCALE_WIDTH, xscale.rect.top - toolbar.rect.bottom,
			 hWnd, (HMENU)YSCALE_ID, hInst, NULL);

	GetWindowRect(yscale.hwnd, &yscale.rect);
	MapWindowPoints(NULL, hWnd, (POINT *)&yscale.rect, 2);

	// Add Y scale to tooltip

	tooltip.info.uId = (UINT_PTR)yscale.hwnd;
	tooltip.info.lpszText = "Y scale";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create scope

	scope.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 yscale.rect.right, toolbar.rect.bottom,
			 width - yscale.rect.right,
			 xscale.rect.top - toolbar.rect.bottom,
			 hWnd, (HMENU)SCOPE_ID, hInst, NULL);

	// Add scope to tooltip

	tooltip.info.uId = (UINT_PTR)scope.hwnd;
	tooltip.info.lpszText = "Scope display";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Start audio thread

	audio.thread = CreateThread(NULL, 0, AudioThread, hWnd, 0, &audio.id);

	break;

	// Colour static text

    case WM_CTLCOLORSTATIC:
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
    	break;

	// Draw item

    case WM_DRAWITEM:
	return DrawItem(wParam, lParam);
	break;

	// Disable menus by capturing this message

    case WM_INITMENU:
	break;

	// Capture system character key to stop pop up menus and other
	// nonsense

    case WM_SYSCHAR:
	break;

	// Buttons

    case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	    // Level

	case LEVEL_ID:
	    DisplayLevelControl(hWnd, wParam, lParam);
	    break;

	    // Bright line

	case BRIGHT_ID:
	    scope.bright = !scope.bright;
	    break;

	    // Single shot

	case SINGLE_ID:
	    scope.single = !scope.single;
	    break;

	    // Trigger

	case TRIGGER_ID:
	    scope.trigger = TRUE;
	    break;

	    // Sync

	case SYNC_ID:
	    scope.polarity = !scope.polarity;
	    SendMessage(toolbar.hwnd, TB_CHANGEBITMAP, SYNC_ID,
			scope.polarity? NEGATIVE_BM: POSITIVE_BM);
	    break;

	    // Timebase

	case TIMEBASE_ID:
	    DisplayTimebaseMenu(hWnd, wParam, lParam);
	    break;

	    // Storage

	case STORAGE_ID:
	    scope.storage = !scope.storage;
	    break;

	    // Clear

	case CLEAR_ID:
	    scope.clear = TRUE;
	    break;

	    // Left

	case LEFT_ID:
	    xscale.start -= xscale.step;
	    if (xscale.start < 0)
		xscale.start = 0;

	    float scale = 1.0 / (((float)SAMPLE_RATE / 100000.0) * scope.scale);

	    scope.start = xscale.start / scale;
	    InvalidateRgn(xscale.hwnd, NULL, TRUE);
	    break;

	// Right

	case RIGHT_ID:
	    scale = 1.0 / (((float)SAMPLE_RATE / 100000.0) * scope.scale);

	    xscale.start += xscale.step;
	    scope.start = xscale.start / scale;

	    if (scope.start >= scope.length)
	    {
		xscale.start -= xscale.step;
		scope.start = xscale.start / scale;
	    }

	    InvalidateRgn(xscale.hwnd, NULL, TRUE);
	    break;

	    // Start

	case START_ID:
	    scope.start = 0;
	    scope.index = 0;
	    xscale.start = 0;
	    InvalidateRgn(xscale.hwnd, NULL, TRUE);
	    break;

	    // End

	case END_ID:
	    scale = 1.0 / (((float)SAMPLE_RATE / 100000.0) * scope.scale);

	    while (scope.start < scope.length)
	    {
		xscale.start += xscale.step;
		scope.start = xscale.start / scale;
	    }

	    xscale.start -= xscale.step;
	    scope.start = xscale.start / scale;
	    InvalidateRgn(xscale.hwnd, NULL, TRUE);
	    break;

	    // Reset

	case RESET_ID:
	    scope.index = 0;
	    scope.start = 0;
	    scope.bright = FALSE;
	    scope.single = FALSE;
	    scope.polarity = FALSE;
	    scope.storage = FALSE;

	    SendMessage(toolbar.hwnd, TB_CHECKBUTTON, BRIGHT_ID, FALSE);
	    SendMessage(toolbar.hwnd, TB_CHECKBUTTON, SINGLE_ID, FALSE);
	    SendMessage(toolbar.hwnd, TB_CHANGEBITMAP, SYNC_ID, POSITIVE_BM);
	    SendMessage(toolbar.hwnd, TB_CHECKBUTTON, STORAGE_ID, FALSE);
	    break;

	    // Scope

	case SCOPE_ID:
	    ScopeClicked(wParam, lParam);
	    break;
	}

	// Update status

	UpdateStatus();

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

	// Menu command

    case WM_MENUCOMMAND:

	// Set up timebase

	timebase.index = wParam;
	break;

	// Notify

    case WM_NOTIFY:
	switch (((LPNMHDR)lParam)->code)
	{
	case TBN_DROPDOWN:
	    switch (((LPNMTOOLBAR)lParam)->iItem)
	    {
	    case LEVEL_ID:
		DisplayLevelControl(hWnd, wParam, lParam);
		break;

	    case TIMEBASE_ID:
		DisplayTimebaseMenu(hWnd, wParam, lParam);
		break;
	    }
	    return TBDDRET_DEFAULT;
	    break;
	}
	break;

	// Size

    case WM_SIZE:
	EnumChildWindows(hWnd, EnumChildProc, (LPARAM)hWnd);
	break;

	// Sizing

    case WM_SIZING:
	return WindowResizing(hWnd, wParam, lParam);
	break;

	// Key pressed

    case WM_KEYDOWN:
    	KeyDown(wParam, lParam);
    	break;

	// Mixer control change

    case MM_MIXM_CONTROL_CHANGE:
	LevelChange(wParam, lParam);
	break;

        // Process other messages.

    case WM_DESTROY:
	waveInStop(audio.hwi);
	waveInClose(audio.hwi);
	PostQuitMessage(0);
	break;

	// Everything else

    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Display level control

BOOL DisplayLevelControl(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    RECT rect;

    // Get the coordinates of the button

    SendMessage(((LPNMHDR)lParam)->hwndFrom, TB_GETRECT,
		(WPARAM)((LPNMTOOLBAR)lParam)->iItem, (LPARAM)&rect);

    MapWindowPoints(hWnd, NULL, (LPPOINT)&rect, 2);

    // Create window

    if (popup.hwnd == NULL)
    {
	RegisterPopupClass(hInst);

	popup.hwnd =
	    CreateWindow(PCLASS, NULL,
			 WS_VISIBLE | WS_POPUP | WS_BORDER,
			 rect.left, rect.bottom + 2,
			 rect.right - rect.left, 128,
			 hWnd, 0, hInst, NULL);
    }

    // Show level control

    else
    {
	if (!IsWindowVisible(popup.hwnd))
	    SetWindowPos(popup.hwnd, NULL, rect.left, rect.bottom + 2, 0, 0,
			 SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	else
	    ShowWindow(popup.hwnd, FALSE);
    }

    // Process messages, so the toolbar button stays down while the
    // popup window is visible

    MSG msg;
    BOOL flag;

    while ((IsWindowVisible(popup.hwnd)) &&
	   ((flag = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0))
    {
	if (flag == -1)
	    break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Register class

BOOL RegisterPopupClass(HINSTANCE hInst)
{
    // Fill in the window class structure with parameters
    // that describe the main window.

    WNDCLASS wc = 
	{0, PopupProc, 0, 0, hInst,
	 LoadIcon(NULL, IDC_ICON),
	 LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, PCLASS};

    // Register the window class.

    return RegisterClass(&wc);
}

// Enum child proc for window resizing

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
    // Switch by id to resize tool windows. This function to get the
    // tool id doesn't work according to the documentation, but is the
    // only way to do this that I have found

    switch ((DWORD)GetMenu(hWnd))
    {
	// Toolbar, let it resize itself

    case TOOLBAR_ID:
	SendMessage(hWnd, WM_SIZE, 0, lParam);
	GetWindowRect(hWnd, &toolbar.rect);
	MapWindowPoints(NULL, (HWND)lParam, (POINT *)&toolbar.rect, 2);
	break;

	// Status bar, let it resize itself

    case STATUS_ID:
	SendMessage(hWnd, WM_SIZE, 0, lParam);
	GetWindowRect(hWnd, &status.rect);
	MapWindowPoints(NULL, (HWND)lParam, (POINT *)&status.rect, 2);
	break;

	// X scale, resize it

    case XSCALE_ID:
	MoveWindow(hWnd, status.rect.left, status.rect.top - SCALE_HEIGHT,
		   status.rect.right, SCALE_HEIGHT, FALSE);
	InvalidateRgn(hWnd, NULL, TRUE);
	GetWindowRect(hWnd, &xscale.rect);
	MapWindowPoints(NULL, (HWND)lParam, (POINT *)&xscale.rect, 2);
	break;

	// Y scale, resize it

    case YSCALE_ID:
	MoveWindow(hWnd, toolbar.rect.left, toolbar.rect.bottom,
		   SCALE_WIDTH, xscale.rect.top - toolbar.rect.bottom, FALSE);
	InvalidateRgn(hWnd, NULL, TRUE);
	GetWindowRect(hWnd, &yscale.rect);
	MapWindowPoints(NULL, (HWND)lParam, (POINT *)&yscale.rect, 2);
	break;

	// Scope, resize it

    case SCOPE_ID:
	MoveWindow(hWnd, yscale.rect.right, toolbar.rect.bottom,
		   toolbar.rect.right - yscale.rect.right,
		   xscale.rect.top - toolbar.rect.bottom, FALSE);
	InvalidateRgn(hWnd, NULL, TRUE);
	break;
    }

    return TRUE;
}

// Create toolbar bitmap

BOOL AddToolbarBitmap(HWND control, LPCTSTR name)
{
    // Load bitmap

    HBITMAP hbm =
	LoadImage(hInst, name, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

    // Create DC

    HDC hdc = CreateCompatibleDC(NULL);

    // Select the bitmap

    SelectObject(hdc, hbm);

    // Select a brush

    SelectObject(hdc, GetSysColorBrush(COLOR_BTNFACE));

    // Get the colour of the first pixel

    COLORREF colour = GetPixel(hdc, 0, 0);

    // Flood fill the bitmap

    ExtFloodFill(hdc, 0, 0, colour, FLOODFILLSURFACE);

    // And the centres of the icons

    for (int x = 11; x < 336; x += 24)
	if (GetPixel(hdc, x, 11) == colour)
	    ExtFloodFill(hdc, x, 11, colour, FLOODFILLSURFACE);

    // Delete the DC

    DeleteObject(hdc);

    // Add bitmap

    TBADDBITMAP bitmap =
	{NULL, (UINT_PTR)hbm};

    SendMessage(control, TB_ADDBITMAP, 14, (LPARAM)&bitmap);
}

BOOL AddToolbarButtons(HWND control)
{
    // Define the buttons

    TBBUTTON buttons[] =
	{{0, 0, 0, BTNS_SEP},
	 {LEVEL_BM, LEVEL_ID, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN,
	  {0}, 0, (INT_PTR)"Input level, click to pop up level control"},
	 {0, 0, 0, BTNS_SEP},
	 {BRIGHT_BM, BRIGHT_ID, TBSTATE_ENABLED, BTNS_CHECK,
	  {0}, 0, (INT_PTR)"Bright line, click to turn off sync"},
	 {0, 0, 0, BTNS_SEP},
	 {SINGLE_BM, SINGLE_ID, TBSTATE_ENABLED, BTNS_CHECK,
	  {0}, 0, (INT_PTR)"Single shot, click to enable"},
	 {TRIGGER_BM, TRIGGER_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"Trigger, click to trigger trace"},
	 {0, 0, 0, BTNS_SEP},
	 {POSITIVE_BM, SYNC_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"Sync polarity, click to change"},
	 {0, 0, 0, BTNS_SEP},
	 {TIMEBASE_BM, TIMEBASE_ID, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN,
	  {0}, 0, (INT_PTR)"Timebase, click to pop up menu"},
	 {0, 0, 0, BTNS_SEP},
	 {STORAGE_BM, STORAGE_ID, TBSTATE_ENABLED, BTNS_CHECK,
	  {0}, 0, (INT_PTR)"Storage, click to enable"},
	 {CLEAR_BM, CLEAR_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"Clear, click to clear scope window"},
	 {0, 0, 0, BTNS_SEP},
	 {LEFT_BM, LEFT_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"Left, click to shift trace left"},
	 {RIGHT_BM, RIGHT_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"Right, click to shift trace right"},
	 {START_BM, START_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"Start, click to shift trace to start"},
	 {END_BM, END_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"End, click to shift trace to end"},
	 {0, 0, 0, BTNS_SEP},
	 {RESET_BM, RESET_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"Reset, click to reset"}};

    // Add to toolbar

    SendMessage(control, TB_ADDBUTTONS,
		Length(buttons), (LPARAM)&buttons);
}

// Window resizing

BOOL WindowResizing(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PRECT rectp = (PRECT)lParam;

    // Minimum size

    if (rectp->right - rectp->left < WIDTH)
	rectp->right = rectp->left + WIDTH;

    if (rectp->bottom - rectp->top < HEIGHT)
	rectp->bottom = rectp->top + HEIGHT;

    // Maximum size, can be defeated by dragging bottom edge to
    // desktop toolbar in Windows 7, probably Vista

    if (rectp->right - rectp->left > MAX_WIDTH)
    	rectp->right = rectp->left + MAX_WIDTH;

    if (rectp->bottom - rectp->top > MAX_HEIGHT)
    	rectp->bottom = rectp->top + MAX_HEIGHT;

    return TRUE;
}

// Popup window procedure

LRESULT CALLBACK PopupProc(HWND hWnd, UINT uMsg,
			   WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:

	// Get the window and client dimensions

	GetClientRect(hWnd, &popup.rect);

	int width = popup.rect.right - popup.rect.left;
	int height = popup.rect.bottom - popup.rect.top;

	// Create level control

	level.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_VERT | TBS_NOTICKS,
			 0, 0,
			 width, height, hWnd,
			 (HMENU)LEVEL_ID, hInst, NULL);

	// Set the slider range

	SendMessage(level.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(MIN_VOL, MAX_VOL));
	SendMessage(level.hwnd, TBM_SETPAGESIZE, 0, STEP_VOL);

	// Add level to tooltip

	tooltip.info.uId = (UINT_PTR)level.hwnd;
	tooltip.info.lpszText = "Input level";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	if (mixer.pmxcd != NULL)
	{
	    // Get the value

	    mixerGetControlDetails((HMIXEROBJ)mixer.hmx, mixer.pmxcd,
				   MIXER_GETCONTROLDETAILSF_VALUE);

	    // Set the slider

	    int value = MAX_VOL - (mixer.pmxcdu->dwValue * (MAX_VOL - MIN_VOL) /
				   (mixer.pmxc->Bounds.dwMaximum -
				    mixer.pmxc->Bounds.dwMinimum));

	    SendMessage(level.hwnd, TBM_SETPOS, TRUE, value);
	}

	else
	    EnableWindow(level.hwnd, FALSE);

	break;

	// Level control

    case WM_VSCROLL:
	ChangeLevel(wParam, lParam);

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

	// Focus lost

    case WM_KILLFOCUS:
	FocusLost(hWnd, wParam, lParam);
	break;

	// Everything else

    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Focus lost

BOOL FocusLost(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // Check window

    if (GetActiveWindow() == hWnd)
	return FALSE;

    POINT point;

    // Get cursor

    GetCursorPos(&point);

    // Get window

    HWND window = WindowFromPoint(point);

    // Check for toolbar

    switch ((DWORD)GetMenu(window))
    {
    case TOOLBAR_ID:
	break;

    default:
	ShowWindow(hWnd, FALSE);
	return TRUE;
    }

    // Check button

    MapWindowPoints(NULL, window, &point, 1);

    int item = SendMessage(window, TB_HITTEST, 0, (LPARAM)&point);

    TBBUTTON button;

    if (!SendMessage(window, TB_GETBUTTON, item, (LPARAM)&button))
    {
	ShowWindow(hWnd, FALSE);
	return FALSE;
    }

    // Check for level button

    switch (button.idCommand)
    {
    case LEVEL_ID:
	return FALSE;
	break;

    default:
	ShowWindow(hWnd, FALSE);
	return TRUE;
    }

    return FALSE;
}

// Change Level

BOOL ChangeLevel(WPARAM wParam, LPARAM lParam)
{
    if (mixer.pmxcdu == NULL)
	return FALSE;

    switch (LOWORD(wParam))
    {
	// Adjustments

    case TB_PAGEDOWN:
    case TB_PAGEUP:
	break;

    case TB_THUMBPOSITION:
    case TB_THUMBTRACK:
	break;

    default:
	return FALSE;
    }

    int value = SendMessage(level.hwnd, TBM_GETPOS, 0, 0);

    mixer.pmxcdu->dwValue = ((mixer.pmxc->Bounds.dwMaximum -
			      mixer.pmxc->Bounds.dwMinimum) *
			     (MAX_VOL - value)) / (MAX_VOL - MIN_VOL);

    mixerSetControlDetails((HMIXEROBJ)mixer.hmx, mixer.pmxcd,
			   MIXER_SETCONTROLDETAILSF_VALUE);

    return TRUE;
}

// Level change

BOOL LevelChange(WPARAM wParam, LPARAM lParam)
{
    if (mixer.pmxcd == NULL)
	return FALSE;

    if (lParam == mixer.pmxcd->dwControlID)
    {
	// Get the value

	mixerGetControlDetails((HMIXEROBJ)mixer.hmx, mixer.pmxcd,
			       MIXER_GETCONTROLDETAILSF_VALUE);

	// Set the slider

	int value = MAX_VOL - (mixer.pmxcdu->dwValue * (MAX_VOL - MIN_VOL) /
			       (mixer.pmxc->Bounds.dwMaximum -
				mixer.pmxc->Bounds.dwMinimum));

	SendMessage(level.hwnd, TBM_SETPOS, TRUE, value);
    }

    return TRUE;
}

// Display timebase menu

BOOL DisplayTimebaseMenu(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    RECT rect;

    // Get the coordinates of the button

    SendMessage(((LPNMHDR)lParam)->hwndFrom, TB_GETRECT,
    		(WPARAM)((LPNMTOOLBAR)lParam)->iItem, (LPARAM)&rect);

    // Convert to screen coordinates

    MapWindowPoints(((LPNMHDR)lParam)->hwndFrom,
    		    HWND_DESKTOP, (LPPOINT)&rect, 2);

    HMENU menu = CreatePopupMenu();

    MENUINFO info =
	{sizeof(MENUINFO),
	 MIM_STYLE,
	 MNS_NOTIFYBYPOS};

    SetMenuInfo(menu, &info);

    for (int i = 0; i < Length(timebase.strings); i++)
    	AppendMenu(menu, (i == timebase.index)? MF_STRING | MF_CHECKED:
		   MF_STRING, i, timebase.strings[i]);

    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		   rect.left, rect.bottom,
		   0, hWnd, NULL);
}

// Scope clicked

BOOL ScopeClicked(WPARAM wParam, LPARAM lParam)
{
    POINT point;

    GetCursorPos(&point);
    MapWindowPoints(HWND_DESKTOP, (HWND)lParam, &point, 1);

    scope.index = point.x;
}

// Key pressed

void KeyDown(WPARAM wParam, LPARAM lParam)
{
    RECT rect;

    GetClientRect(scope.hwnd, &rect);

    switch(wParam)
    {
	// Left

    case VK_LEFT:
	if (--scope.index < 0)
	    scope.index = 0;
	break;

	// Right

    case VK_RIGHT:
	if (++scope.index >= rect.right)
	    scope.index = 0;
	break;

    default:
	return;
    }
}

// Draw item

BOOL DrawItem(WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpdi = (LPDRAWITEMSTRUCT)lParam;
    RECT rect = lpdi->rcItem;
    HDC hdc = lpdi->hDC;

    SetGraphicsMode(hdc, GM_ADVANCED);

    switch (wParam)
    {
	// X scale

    case XSCALE_ID:
	return DrawXScale(hdc, rect);
	break;

	// Y scale

    case YSCALE_ID:
	return DrawYScale(hdc, rect);
	break;

	// Scope

    case SCOPE_ID:
	return DrawScope(hdc, rect);
	break;
    }
}

// Draw X scale

BOOL DrawXScale(HDC hdc, RECT rect)
{
    // Font height

    enum
    {FONT_HEIGHT = 16};

    static HFONT font;

    // Plain vanilla font

    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    // Create font

    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
    }

    SelectObject(hdc, font);
    SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);

    // Calculate dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    FillRect(hdc, &rect, GetStockObject(WHITE_BRUSH));

    // Move the origin

    SetViewportOrgEx(hdc, SCALE_WIDTH, 0, NULL);

    // Draw scale

    for (int x = 0; x < width - SCALE_WIDTH; x += 50)
    {
	MoveToEx(hdc, x, 0, NULL);
	LineTo(hdc, x, height / 3);
    }

    for (int x = 0; x < width - SCALE_WIDTH; x += 10)
    {
	MoveToEx(hdc, x, 0, NULL);
	LineTo(hdc, x, height / 4);
    }

    if (xscale.scale < 100.0)
    {
	static char s[16];

	TextOut(hdc, 0, height, "ms", 2);

	for (int x = 100; x < width - SCALE_WIDTH; x += 100)
	{
	    sprintf(s, "%0.1f", (xscale.start + (x * xscale.scale)) / 100.0);
	    TextOut(hdc, x, height, s, strlen(s));
	}
    }

    else
    {
	static char s[16];

	TextOut(hdc, 0, height, "sec", 3);

	for (int x = 100; x < width - SCALE_WIDTH; x += 100)
	{
	    sprintf(s, "%0.1f", (xscale.start + (x * xscale.scale)) / 100000.0);
	    TextOut(hdc, x, height, s, strlen(s));
	}
    }
}

// Draw Y scale

BOOL DrawYScale(HDC hdc, RECT rect)
{
    // Calculate dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    FillRect(hdc, &rect, GetStockObject(WHITE_BRUSH));

    // Move the origin

    SetViewportOrgEx(hdc, width / 2, height / 2, NULL);

    // Draw scale

    for (int y = 0; y < height / 2; y += 50)
    {
	MoveToEx(hdc, 0, y, NULL);
	LineTo(hdc, width / 2, y);

	MoveToEx(hdc, 0, -y, NULL);
	LineTo(hdc, width / 2, -y);
    }

    for (int y = 0; y < height / 2; y += 10)
    {
	MoveToEx(hdc, width / 8, y, NULL);
	LineTo(hdc, width / 2, y);

	MoveToEx(hdc, width / 8, -y, NULL);
	LineTo(hdc, width / 2, -y);
    }
}

// Draw scope

BOOL DrawScope(HDC hdc, RECT rect)
{
    // Font height

    enum
    {FONT_HEIGHT = 12};

    static SIZE size;

    static HDC hbdc;
    static HDC hgdc;

    static HBITMAP bitmap;
    static HBITMAP graticule;

    static HFONT font;

    // Plain vanilla font

    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    static char s[16];

    // Create font

    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
    }

    // Calculate bitmap dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC

    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	hgdc = CreateCompatibleDC(hdc);

	SelectObject(hbdc, GetStockObject(DC_PEN));
	SelectObject(hgdc, GetStockObject(DC_PEN));

	SelectObject(hbdc, font);
    }

    // Create new bitmaps if resized

    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmaps

	if (bitmap != NULL)
	{
	    DeleteObject(bitmap);
	    DeleteObject(graticule);
	}

	// Create new bitmaps

	bitmap = CreateCompatibleBitmap(hdc, width, height);
	graticule = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);
	SelectObject(hgdc, graticule);

	size.cx = width;
	size.cy = height;

	FillRect(hgdc, &rect, GetStockObject(BLACK_BRUSH));

	// Dark green graticule

	SetDCPenColor(hgdc, RGB(0, 128, 0));

	// Draw graticule

	for (int i = 0; i < width; i += 10)
	{
	    MoveToEx(hgdc, i, 0, NULL);
	    LineTo(hgdc, i, height);
	}

	for (int i = 0; i < height / 2; i += 10)
	{
	    MoveToEx(hgdc, 0, height / 2 + i, NULL);
	    LineTo(hgdc, width, height / 2 + i);

	    MoveToEx(hgdc, 0, height / 2 - i, NULL);
	    LineTo(hgdc, width, height / 2 - i);
	}

	scope.clear = TRUE;
    }

    // Don't attempt the trace until there's a buffer

    if (scope.data == NULL)
    {
	// Copy the bitmap

	BitBlt(hdc, rect.left, rect.top, width, height,
	       hgdc, 0, 0, SRCCOPY);

	return TRUE;
    }

    // Erase background

    if (!scope.storage || scope.clear)
    {
	// Copy the graticule

	BitBlt(hbdc, rect.left, rect.top, width, height,
	       hgdc, 0, 0, SRCCOPY);

	scope.clear = FALSE;
    }

    // Calculate scale etc

    float xscale = 1.0 / (((float)SAMPLE_RATE / 100000.0) * scope.scale);
    int xstart = round(scope.start);
    int xstep = round(1.0 / xscale);
    int xstop = round(xstart + ((float)width / xscale));

    if (xstop > scope.length)
	xstop = scope.length;

    // Calculate scale

    static int max;

    if (max < 4096)
	max = 4096;

    int yscale = max / (height / 2);

    max = 0;

    // Move the origin

    SetViewportOrgEx(hbdc, 0, height / 2, NULL);

    // Green pen for scope trace

    SelectObject(hbdc, GetStockObject(DC_PEN));
    SelectObject(hbdc, GetStockObject(NULL_BRUSH));
    SetDCPenColor(hbdc, RGB(0, 255, 0));

    // Draw the trace

    MoveToEx(hbdc, 0, 0, NULL);

    if (xscale < 1.0)
    {
	for (int i = 0; i <= xstop - xstart; i += xstep)
	{
	    if (max < abs(scope.data[i + xstart]))
		max = abs(scope.data[i + xstart]);

	    int x = round((float)i * xscale);
	    int y = -round((float)scope.data[i + xstart] / yscale);
	    LineTo(hbdc, x, y);
	}
    }

    else
    {
	for (int i = 0; i <= xstop - xstart; i++)
	{
	    if (max < abs(scope.data[i + xstart]))
		max = abs(scope.data[i + xstart]);

	    int x = round((float)i * xscale);
	    int y = -round((float)scope.data[i + xstart] / yscale);
	    LineTo(hbdc, x, y);

	    // Draw points at max resolution

	    if (timebase.index == 0)
		Rectangle(hbdc, x - 2, y - 2, x + 2, y + 2);
	}
    }

    SetTextColor(hbdc, RGB(0, 255, 0));
    SetBkMode(hbdc, TRANSPARENT);

    // Draw index

    if (scope.index != 0 && !scope.storage)
    {
	MoveToEx(hbdc, scope.index, height / 2, NULL);
	LineTo(hbdc, scope.index, -height / 2);

	int i = round((float)scope.index / xscale);
	int y = -round((float)scope.data[i + xstart] / yscale);

	SetTextAlign(hbdc, TA_LEFT | (y > 0)? TA_TOP: TA_BOTTOM);

	sprintf(s, "%0.3f", (float)scope.data[i + xstart] / 32768.0);
	TextOut(hbdc, scope.index, y, s, strlen(s));

	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);

	if (scope.scale < 100.0)
	{
	    sprintf(s, (scope.scale < 1.0)? "%0.3f %f": 
		    (scope.scale < 10.0)? "%0.2f %f": "%0.1f %f",
		    ((scope.start * xscale) +
		     (scope.index * scope.scale)) / 100.0,
		    (float)(scope.start + scope.index) / xscale);
	    TextOut(hbdc, scope.index, height / 2, s, strlen(s));
	}

	else
	{
	    sprintf(s,  "%0.3f %f", ((scope.start * xscale) +
				     (scope.index * scope.scale)) / 100000.0,
		    (float)(scope.start + scope.index) / xscale);
	    TextOut(hbdc, scope.index, height / 2, s, strlen(s));
	}
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Audio thread

DWORD WINAPI AudioThread(LPVOID lpParameter)
{
    // Set up timebase

    scope.scale = timebase.values[timebase.index];
    xscale.scale = scope.scale;
    xscale.step = 500 * xscale.scale;

    // Update display

    InvalidateRgn(xscale.hwnd, NULL, TRUE);

    UpdateStatus();

    // Create wave format structure

    static WAVEFORMATEX wf =
	{WAVE_FORMAT_PCM, CHANNELS,
	 SAMPLE_RATE, SAMPLE_RATE * BLOCK_ALIGN,
	 BLOCK_ALIGN, BITS_PER_SAMPLE, 0};

    MMRESULT mmr;

    // Open a waveform audio input device

    mmr = waveInOpen(&audio.hwi, WAVE_MAPPER | WAVE_FORMAT_DIRECT, &wf,
		     (DWORD_PTR)audio.id,  (DWORD_PTR)NULL, CALLBACK_THREAD);

    if (mmr != MMSYSERR_NOERROR)
    {
	static char s[64];

	waveInGetErrorText(mmr, s, sizeof(s));
	MessageBox(window.hwnd, s, "WaveInOpen", MB_OK | MB_ICONERROR);
	return mmr;
    }

    do
    {
	// Mixer structures

	static MIXERLINE mxl =
	    {sizeof(MIXERLINE)};

	static MIXERCONTROL mxc =
	    {sizeof(MIXERCONTROL)};

	static MIXERLINECONTROLS mxlc =
	    {sizeof(MIXERLINECONTROLS)};

	static MIXERCONTROLDETAILS mxcd =
	    {sizeof(MIXERCONTROLDETAILS)};

	static MIXERCONTROLDETAILS_UNSIGNED mxcdu;

	// Open a mixer device

	mmr = mixerOpen(&mixer.hmx, (UINT)audio.hwi, (DWORD_PTR)window.hwnd, 0,
			CALLBACK_WINDOW | MIXER_OBJECTF_HWAVEIN);

	if (mmr != MMSYSERR_NOERROR)
	    break;

	// Mixer line types

	DWORD types[] =
	    {MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE,
	     MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY,
	     MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER,
	     MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE,
	     MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER,
	     MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC,
	     MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED,
	     MIXERLINE_COMPONENTTYPE_SRC_DIGITAL,
	     MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT,
	     MIXERLINE_COMPONENTTYPE_SRC_ANALOG,
	     MIXERLINE_COMPONENTTYPE_SRC_LINE};

	// Get mixer line info

	for (int i = 0; i < Length(types); i++)
	{
	    // Try a component type

	    mxl.dwComponentType = types[i];

	    // Get the info

	    mmr = mixerGetLineInfo((HMIXEROBJ)mixer.hmx, &mxl,
				   MIXER_GETLINEINFOF_COMPONENTTYPE);

	    // Try again if error

	    if (mmr != MMSYSERR_NOERROR)
		continue;


	    // Check if line is active

	    if (mxl.fdwLine & MIXERLINE_LINEF_ACTIVE)
	    {
		mixer.pmxl = &mxl;
		break;
	    }
	}

	// No mixer line

	if (mixer.pmxl == NULL)
	    break;

	// Get a volume control

	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(mxc);
	mxlc.pamxctrl = &mxc;

	mmr = mixerGetLineControls((HMIXEROBJ)mixer.hmx, &mxlc,
				   MIXER_GETLINECONTROLSF_ONEBYTYPE);

	if (mmr != MMSYSERR_NOERROR)
	    break;

	// Got a volume control

	mixer.pmxc = &mxc;

	// Get the control details

	mxcd.dwControlID = mxc.dwControlID;
	mxcd.cChannels = MIXERCONTROL_CONTROLF_UNIFORM;
	mxcd.cbDetails = sizeof(mxcdu);
	mxcd.paDetails = &mxcdu;

	mmr = mixerGetControlDetails((HMIXEROBJ)mixer.hmx, &mxcd,
				     MIXER_GETCONTROLDETAILSF_VALUE);

	if (mmr != MMSYSERR_NOERROR)
	    break;

	mixer.pmxcd = &mxcd;
	mixer.pmxcdu = &mxcdu;

    } while (FALSE);

    // Create the waveform audio input buffers and structures

    static short data[4][STEP];
    static WAVEHDR hdrs[4] =
	{{(LPSTR)data[0], sizeof(data[0]), 0, 0, 0, 0},
	 {(LPSTR)data[1], sizeof(data[1]), 0, 0, 0, 0},
	 {(LPSTR)data[2], sizeof(data[2]), 0, 0, 0, 0},
	 {(LPSTR)data[3], sizeof(data[3]), 0, 0, 0, 0}};

    for (int i = 0; i < Length(hdrs); i++)
    {
	// Prepare a waveform audio input header

	mmr = waveInPrepareHeader(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static char s[64];

	    waveInGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, "WaveInPrepareHeader",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}

	// Add a waveform audio input buffer

	mmr = waveInAddBuffer(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static char s[64];

	    waveInGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, "WaveInAddBuffer",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}
    }

    // Start the waveform audio input

    mmr = waveInStart(audio.hwi);
    if (mmr != MMSYSERR_NOERROR)
    {
	static char s[64];

	waveInGetErrorText(mmr, s, sizeof(s));
	MessageBox(window.hwnd, s, "WaveInStart", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Create a message loop for processing thread messages

    MSG msg;
    BOOL flag;

    while ((flag = GetMessage(&msg, (HWND)-1, 0, 0)) != 0)
    {
	if (flag == -1)
	    break;

	// Process messages

	switch (msg.message)
	{
	    // Audio input opened

	case MM_WIM_OPEN:
	    // Not used
	    break;

	    // Audio input data

	case MM_WIM_DATA:
	    WaveInData(msg.wParam, msg.lParam);
	    break;

	    // Audio input closed

	case MM_WIM_CLOSE:
	    // Not used
	    break;
	}
    }

    return msg.wParam;
}

// Wave in data

void WaveInData(WPARAM wParam, LPARAM lParam)
{
    static int index;
    static int count;
    static short state;
    static short last;

    // Create buffer for processing the audio data

    static short buffer[SAMPLES];

    // Initialise data structs

    if (scope.data == NULL)
    {
	scope.data = buffer;
	scope.length = timebase.counts[timebase.index];
    }

    // Copy the input data

    short *data = (short *)((WAVEHDR *)lParam)->lpData;

    // State machine for sync and copying data to display buffer

    switch (state)
    {
	// INIT: waiting for sync

    case INIT:

	index = 0;

	if (scope.bright)
	    state++;

	else
	{
	    if (scope.single && !scope.trigger)
		break;

	    // Initialise sync

	    int dx = 0;

	    // Sync polarity

	    if (scope.polarity)
	    {
		for (int i = 0; i < STEP; i++)
		{
		    dx = data[i] - last;

		    if (dx < 0 && last > 0 && data[i] < 0)
		    {
			index = i;
			state++;
			break;
		    }

		    last = data[i];
		}
	    }

	    else
	    {
		for (int i = 0; i < STEP; i++)
		{
		    dx = data[i] - last;

		    if (dx > 0 && last < 0 && data[i] > 0)
		    {
			index = i;
			state++;
			break;
		    }

		    last = data[i];
		}
	    }
	}

	// No sync, try next time

	if (state == 0)
	    break;

	// Reset trigger

	if (scope.single && scope.trigger)
	    scope.trigger = FALSE;

	// FIRST: First chunk of data

    case FIRST:

	// Update count

	count = timebase.counts[timebase.index];
	scope.length = count;

	// Copy data

	memmove(buffer, data + index, (STEP - index) * sizeof(short));
	index = STEP - index;

	// If done, wait for sync again

	if (index >= count)
	    state = INIT;

	// Else get some more data next time

	else
	    state++;
	break;

	// NEXT: Subsequent chunks of data

    case NEXT:

	// Copy data

	memmove(buffer + index, data, STEP * sizeof(short));
	index += STEP;

	// Done, wait for sync again

	if (index >= count)
	    state = INIT;

	// Else if last but one chunk, get last chunk next time

	else if (index + STEP >= count)
	    state++;
	break;

	// LAST: Last chunk of data

    case LAST:

	// Copy data

	memmove(buffer + index, data, (count - index) * sizeof(short));

	// Wait for sync next time

	state = INIT;
	break;
    }

    // Give the buffer back

    waveInAddBuffer(audio.hwi, (WAVEHDR *)lParam, sizeof(WAVEHDR));


    if (scope.scale != timebase.values[timebase.index])
    {
	// Set up scale

	scope.scale = timebase.values[timebase.index];
	xscale.scale = scope.scale;
	xscale.step = 500 * xscale.scale;

	// Reset start

	scope.start = 0;
	xscale.start = 0;

	// Update display

	InvalidateRgn(xscale.hwnd, NULL, TRUE);

	UpdateStatus();
    }

    // Update display

    InvalidateRgn(scope.hwnd, NULL, TRUE);
}

// Update status

BOOL UpdateStatus()
{
    static char s[128];

    // Timebase

    sprintf(s, (timebase.values[timebase.index] < 100)?
	    " Timebase %0.1f ms": " Timebase %0.1f sec",
	    (timebase.values[timebase.index] < 100)?
	    timebase.values[timebase.index]:
	    timebase.values[timebase.index] / 1000.0);

    // Buttons

    if (scope.bright)
	strcat(s, "  bright line");

    if (scope.single)
	strcat(s, "  single shot");

    if (scope.polarity)
	strcat(s, "  reverse sync");

    if (scope.storage)
	strcat(s, "  storage mode");

    // Update statsu

    SendMessage(status.hwnd, SB_SETTEXT, 0, (LPARAM)s);

    return TRUE;
}
