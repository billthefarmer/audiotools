////////////////////////////////////////////////////////////////////////////////
//
//  SLMS - A Selective Level Measuring Set written in C.
//
//  Copyright (C) 2011  Bill Farmer
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
//  Bill Farmer  william j farmer [at] yahoo [dot] co [dot] uk.
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

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS   "MainWClass"
#define KNOBCLASS "KnobClass"

#define MIN 0.5

// Tool ids

enum
    {DISPLAY_ID = 100,
     SPECTRUM_ID,
     METER_ID,
     SCALE_ID,
     KNOB_ID,
     QUIT_ID,
     SLIDER_ID,
     STATUS_ID};

// Timer values

enum
    {METER_DELAY = 100};

// Dimensions

enum
    {WIDTH      = 480,
     HEIGHT     = 276};

// Wave in values

enum
    {SAMPLE_RATE = 44100L,
     BITS_PER_SAMPLE = 16,
     BLOCK_ALIGN = 2,
     CHANNELS = 1};

// Frequency scale

enum
    {FREQ_SCALE = 250,
     FREQ_MAX   = 850,
     FREQ_MIN   = 0};

// Slider values

enum
    {MAX_METER = 200,
     REF_METER = 0,
     MIN_METER = 0};

// Audio processing values

enum
    {OVERSAMPLE = 4,
     SAMPLES = 4096,
     RANGE = SAMPLES * 7 / 64,
     STEP = SAMPLES / OVERSAMPLE};

// Global data

HINSTANCE hInst;

typedef struct
{
    double r;
    double i;
} complex;

typedef struct
{
    HWND hwnd;
    RECT rect;
} TOOL, *TOOLP;

TOOL window;
TOOL status;
TOOL knob;
TOOL quit;

typedef struct
{
    HWND hwnd;
    double f;
    UINT length;
    double *data;
} SPECTRUM, *SPECTRUMP;

SPECTRUM spectrum;

typedef struct
{
    HWND hwnd;
    double f;
    double l;
} DISPLAY, *DISPLAYP;

DISPLAY display;

typedef struct
{
    HWND hwnd;
    HANDLE timer;
    double l;
    TOOL slider;
} METER, *METERP;

METER meter;

typedef struct
{
    HWND hwnd;
    INT v;
} SCALE, *SCALEP;

SCALE scale =
    {NULL, FREQ_SCALE * 2};

typedef struct
{
    HWND hwnd;
    TOOLINFO info;
} TOOLTIP, *TOOLTIPP;

TOOLTIP tooltip;

typedef struct
{
    DWORD id;
    HWAVEIN hwi;
    HANDLE thread;
} AUDIO, *AUDIOP;

AUDIO audio;

// Function prototypes.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KnobProc(HWND, UINT, WPARAM, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
BOOL RegisterKnobClass(HINSTANCE);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawSpectrum(HDC, RECT);
BOOL DrawDisplay(HDC, RECT);
BOOL DrawMeter(HDC, RECT);
BOOL DrawScale(HDC, RECT);
BOOL DrawKnob(HDC, RECT);
VOID CharPressed(WPARAM, LPARAM);
VOID KeyDown(WPARAM, LPARAM);
VOID KnobClicked(HWND, WPARAM, LPARAM);
VOID MouseMove(HWND, WPARAM, LPARAM);
VOID TooltipShow(WPARAM, LPARAM);
VOID TooltipPop(WPARAM, LPARAM);
DWORD WINAPI AudioThread(LPVOID);
VOID WaveInData(WPARAM, LPARAM);
VOID UpdateMeter(METERP);
VOID CALLBACK MeterCallback(PVOID, BOOL);
VOID UpdateValues(VOID);
VOID fftr(complex *, int);

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

    // Register knob class

    RegisterKnobClass(hInstance);

    // Create the main window.

    window.hwnd =
	CreateWindow(WCLASS, "Selective Level Measuring Set",
		     WS_OVERLAPPED | WS_MINIMIZEBOX |
		     WS_SYSMENU,
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
	 LoadIcon(hInst, "SLMS"),
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

	// Create status bar

	status.hwnd =
	    CreateWindow(STATUSCLASSNAME,
			 " Turn knob to adjust measurement frequency",
			 WS_VISIBLE | WS_CHILD,
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

	// Create scale

	scale.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 8, 8, 160, 40, hWnd,
			 (HMENU)SCALE_ID, hInst, NULL);

	// Add scale to tooltip

	tooltip.info.uId = (UINT_PTR)scale.hwnd;
	tooltip.info.lpszText = "Frequency scale";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create knob

	knob.hwnd =
	    CreateWindow(KNOBCLASS, NULL,
			 WS_VISIBLE | WS_CHILD,
			 4, 52, 168, 168, hWnd,
			 (HMENU)KNOB_ID, hInst, NULL);

	// Add knob to tooltip

	tooltip.info.uId = (UINT_PTR)knob.hwnd;
	tooltip.info.lpszText = "Frequency adjustment knob";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create display

	display.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 176, 8, width - 184, 50, hWnd,
			 (HMENU)DISPLAY_ID, hInst, NULL);

	// Add display to tooltip

	tooltip.info.uId = (UINT_PTR)display.hwnd;
	tooltip.info.lpszText = "Frequency and level display";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create meter

	meter.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 176, 66, width - 184, 50, hWnd,
			 (HMENU)METER_ID, hInst, NULL);

	// Add meter to tooltip

	tooltip.info.uId = (UINT_PTR)meter.hwnd;
	tooltip.info.lpszText = "Level meter";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create meter slider

	meter.slider.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_HORZ | TBS_NOTICKS | TBS_TOP,
			 178, 92, width - 188, 20, hWnd,
			 (HMENU)SLIDER_ID, hInst, NULL);

	SendMessage(meter.slider.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(MIN_METER, MAX_METER));
	SendMessage(meter.slider.hwnd, TBM_SETPOS, TRUE, MIN_METER);

	// Add slider to tooltip

	tooltip.info.uId = (UINT_PTR)meter.slider.hwnd;
	tooltip.info.lpszText = "Level meter";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create spectrum

	spectrum.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 176, 122, width - 184, 50, hWnd,
			 (HMENU)SPECTRUM_ID, hInst, NULL);

	// Add spectrum to tooltip

	tooltip.info.uId = (UINT_PTR)spectrum.hwnd;
	tooltip.info.lpszText = "Frequency spectrum";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create quit button

	quit.hwnd =
	    CreateWindow(WC_BUTTON, "Quit",
			 WS_VISIBLE | WS_CHILD,
			 width - 80, 194, 72, 24, hWnd,
			 (HMENU)QUIT_ID, hInst, NULL);

	// Start audio thread

	audio.thread = CreateThread(NULL, 0, AudioThread, hWnd, 0, &audio.id);

	// Start meter timer

	CreateTimerQueueTimer(&meter.timer, NULL,
			      (WAITORTIMERCALLBACK)MeterCallback,
			      &meter, METER_DELAY, METER_DELAY,
			      WT_EXECUTEDEFAULT);

	// Update frequency

	UpdateValues();
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

	// Char pressed

    case WM_CHAR:
	CharPressed(wParam, lParam);
	break;

	// Key pressed

    case WM_KEYDOWN:
	KeyDown(wParam, lParam);
	break;

	// Buttons

    case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	    // Quit

	case QUIT_ID:
	    waveInStop(audio.hwi);
	    waveInClose(audio.hwi);
	    PostQuitMessage(0);
	    break;
	}
	break;

	// Notify

    case WM_NOTIFY:
	switch (((LPNMHDR)lParam)->code)
	{
	    // Tooltip

	case TTN_SHOW:
	    TooltipShow(wParam, lParam);
	    break;

	case TTN_POP:
	    TooltipPop(wParam, lParam);
	    break;
	}
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

// Register class

BOOL RegisterKnobClass(HINSTANCE hinst)
{
    // Fill in the window class structure with parameters
    // that describe the main window.

    WNDCLASS wc = 
	{0, KnobProc,
	 0, 0, hinst,
	 NULL, LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, KNOBCLASS};

    // Register the window class.

    return RegisterClass(&wc);
}

// Draw item

BOOL DrawItem(WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpdi = (LPDRAWITEMSTRUCT)lParam;
    UINT state = lpdi->itemState;
    RECT rect = lpdi->rcItem;
    HDC hdc = lpdi->hDC;

    SetGraphicsMode(hdc, GM_ADVANCED);

    switch (wParam)
    {
	// Scale

    case SCALE_ID:
	return DrawScale(hdc, rect);
	break;

	// Spectrum

    case SPECTRUM_ID:
	return DrawSpectrum(hdc, rect);
	break;

	// Display

    case DISPLAY_ID:
	return DrawDisplay(hdc, rect);
	break;

	// Meter

    case METER_ID:
	return DrawMeter(hdc, rect);
	break;
    }
}

// Knob procedure

LRESULT CALLBACK KnobProc(HWND hWnd, UINT uMsg,
			  WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    RECT rect;
    HDC hdc;

    switch (uMsg)
    {
	// Paint

    case WM_PAINT:
	hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &rect);
	DrawKnob(hdc, rect);
	EndPaint(hWnd, &ps);
	break;

	// Char pressed

    case WM_CHAR:
    	CharPressed(wParam, lParam);
    	break;

	// Key pressed

    case WM_KEYDOWN:
    	KeyDown(wParam, lParam);
    	break;

	// Mouse move

    case WM_MOUSEMOVE:
    	MouseMove(hWnd, wParam, lParam);
    	break;

	// Everything else

    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Draw Scale

BOOL DrawScale(HDC hdc, RECT rect)
{
    // Font height

    enum
    {FONT_HEIGHT = 16};

    static HBITMAP bitmap;
    static HFONT font;
    static HDC hbdc;

    // Plain vanilla font

    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_NORMAL,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create bitmap

    if (bitmap == NULL)
    {
	bitmap = CreateCompatibleBitmap(hdc, width, height);

	// Create DC

	hbdc = CreateCompatibleDC(hdc);
	SelectObject(hbdc, bitmap);
	SelectObject(hbdc, GetStockObject(DC_PEN));

	// Create font

	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
	SelectObject(hbdc, font);
	SetTextAlign(hbdc, TA_CENTER | TA_BOTTOM);
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, GetStockObject(WHITE_BRUSH));

    // Translate viewport

    SetViewportOrgEx(hbdc, width / 2, height / 2, NULL);

    // Draw scale

    for (int i = 1; i < 11; i++)
    {
	int x = round(FREQ_SCALE * log10(i)) - scale.v;

	for (int j = 0; j < 4; j++)
	{
	    MoveToEx(hbdc, x, 0, NULL);
	    LineTo(hbdc, x, height / 2);
	    x += FREQ_SCALE;
	}
    }

    for (int i = 3; i < 20; i += 2)
    {
	int x = round(FREQ_SCALE * log10(i / 2.0)) - scale.v;

	for (int j = 0; j < 4; j++)
	{
	    MoveToEx(hbdc, x, 6, NULL);
	    LineTo(hbdc, x, height / 2);
	    x += FREQ_SCALE;
	}
    }

    int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < LENGTH(a); i++)
    {
    	int x = round(FREQ_SCALE * log10(a[i])) - scale.v;

    	for (int j = 0; j < 2; j++)
    	{
	    static char s[8];

	    sprintf(s, "%d", a[i]);
    	    TextOut(hbdc, x, 0, s, strlen(s)); 

	    sprintf(s, "%d", a[i] * 10);
    	    TextOut(hbdc, x + FREQ_SCALE, 0, s, strlen(s));

    	    x += 2 * FREQ_SCALE;
    	}
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Draw centre line

    MoveToEx(hbdc, width / 2, 0, NULL);
    LineTo(hbdc, width / 2, height);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
    	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw spectrum

BOOL DrawSpectrum(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HDC hbdc;

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC

    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);
	SelectObject(hbdc, GetStockObject(DC_PEN));
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, GetStockObject(BLACK_BRUSH));

    // Dark green graticule

    SetDCPenColor(hbdc, RGB(0, 128, 0));

    // Draw graticule

    for (int i = 4; i < width; i += 5)
    {
	MoveToEx(hbdc, i, 0, NULL);
	LineTo(hbdc, i, height);
    }

    for (int i = 4; i < height; i += 5)
    {
	MoveToEx(hbdc, 0, i, NULL);
	LineTo(hbdc, width, i);
    }

    // Don't attempt the trace until there's a buffer

    if (spectrum.data == NULL)
    {
	// Copy the bitmap

	BitBlt(hdc, rect.left, rect.top, width, height,
	       hbdc, 0, 0, SRCCOPY);

	return TRUE;
    }

    // Move the origin

    SetViewportOrgEx(hbdc, 0, height - 1, NULL);

    // Green pen for spectrum trace

    SetDCPenColor(hbdc, RGB(0, 255, 0));

    static float max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling

    float yscale = (float)height / max;

    max = 0.0;

    // Draw the spectrum

    MoveToEx(hbdc, 0, 0, NULL);

    float xscale = (float)spectrum.length / (float)width;

    for (int x = 0; x < width; x++)
    {
	float value = 0.0;

	// Don't show DC component

	if (x > 0)
	{
	    for (int j = 0; j < xscale; j++)
	    {
		int n = x * xscale + j;

		if (value < spectrum.data[n])
		    value = spectrum.data[n];
	    }
	}

	if (max < value)
	    max = value;

	int y = -round(value * yscale);

	LineTo(hbdc, x, y);
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw display

BOOL DrawDisplay(HDC hdc, RECT rect)
{
    static HFONT font;

    enum
    {FONT_HEIGHT = 32};

    // Bold font

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

    // Create fonts

    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
    }

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions

    int width = rect.right - rect.left;

    SelectObject(hdc, font);

    // Display frequency

    sprintf(s, "%7.1lfHz    ", display.f);
    TextOut(hdc, 4, 8, s, strlen(s));

    // Display level

    sprintf(s, "%5.1lfdB  ", display.l);
    TextOut(hdc, width / 2 + 4, 8, s, strlen(s));

    return TRUE;
}

// Draw meter

BOOL DrawMeter(HDC hdc, RECT rect)
{
    static HFONT font;

    // Plain vanilla font

    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_NORMAL,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Select font

    if (font == NULL)
    {
	lf.lfHeight = 16;
	font = CreateFontIndirect(&lf);
    }

    // Calculate dimensions

    int width = rect.right - rect.left;

    // Move the origin

    SetViewportOrgEx(hdc, 2, 2, NULL);

    SelectObject(hdc, font);

    SetTextAlign(hdc, TA_CENTER);

    // Draw the meter scale

    static int as[] =
	{-40, -20, -14, -10, -8, -7, -6, -5,
	 -4, -3, -2, -1, 0, 1, 2, 3};

    for (int i = 0; i < LENGTH(as); i++)
    {
	static char s[16];

	sprintf(s, "%d", abs(as[i]));

	float x = pow(10.0, (as[i] + 20.0) / 20.0) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;

	x *= (width - 22);

	x += 10;

	TextOut(hdc, x, 0, s, strlen(s));
    }

    MoveToEx(hdc, 3, 20, NULL);
    LineTo(hdc, 8, 20);

    MoveToEx(hdc, width - 8, 20, NULL);
    LineTo(hdc, width - 3, 20);
    MoveToEx(hdc, width - 6, 18, NULL);
    LineTo(hdc, width - 6, 23);

    static int at[] =
	{-10, -5, 0, 1, 2, 3, 4, 5,
	 6, 7, 8, 9, 10, 11, 12, 13};

    for (int i = 0; i < LENGTH(at); i++)
    {
	float x = pow(10.0, at[i] / 10.0) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;

	x *= (width - 22);

	x += 10;

	MoveToEx(hdc, x, 16, NULL);
	LineTo(hdc, x, 24);
    }

    for (int i = 1; i < 26; i += 2)
    {
	float x = pow(10.0, (i / 20.0)) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;

	x *= (width - 22);

	x += 10;

	MoveToEx(hdc, x, 18, NULL);
	LineTo(hdc, x, 24);
    }

    for (int i = 17; i < 48; i += 2)
    {
	float x = pow(10.0, (i / 40.0)) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;

	x *= (width - 22);

	x += 10;

	MoveToEx(hdc, x, 20, NULL);
	LineTo(hdc, x, 24);
    }

    return TRUE;
}

// Meter callback

VOID CALLBACK MeterCallback(PVOID lpParam, BOOL TimerFired)
{
    // Update meter

    UpdateMeter(&meter);
}

// Update meter

VOID UpdateMeter(METERP meter)
{
    static float ml;

    // Do calculation

    ml = ((ml * 7.0) + meter->l) / 8.0;

    int value = round(ml * MAX_METER) + MIN_METER;

    // Update meter

    SendMessage(meter->slider.hwnd, TBM_SETPOS, TRUE, value);
}

// Draw Knob

BOOL DrawKnob(HDC hdc, RECT rect)
{
    // Calculate dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Load bitmap

    HBITMAP bitmap =
	LoadImage(hInst, "Knob", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

    // Create DC

    HDC hbdc = CreateCompatibleDC(hdc);

    // Select the bitmap

    SelectObject(hbdc, bitmap);

    // Select a brush

    SelectObject(hbdc, GetSysColorBrush(COLOR_BTNFACE));

    // Get the colour of the centre pixel

    COLORREF colour = GetPixel(hbdc, width / 2, height / 2);

    // Flood fill the bitmap

    ExtFloodFill(hbdc, width / 2, height / 2, colour, FLOODFILLSURFACE);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
    	   hbdc, 0, 0, SRCCOPY);

    DeleteObject(bitmap);
    DeleteObject(hbdc);

    return TRUE;
}

// Char pressed

void CharPressed(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    default:
	return;
    }
}

// Key pressed

void KeyDown(WPARAM wParam, LPARAM lParam)
{
    int value;

    switch(wParam)
    {
	// Right, increase frequency

    case VK_RIGHT:
	if (++scale.v > FREQ_MAX)
	    scale.v = FREQ_MAX;
	break;

	// Left, decrease frequency

    case VK_LEFT:
	if (--scale.v < FREQ_MIN)
	    scale.v = FREQ_MIN;
	break;

    default:
	return;
    }

    UpdateValues();
}

// Knob clicked

void KnobClicked(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    switch(HIWORD(wParam))
    {
    case STN_CLICKED:
	break;
    }
}

// Mouse move

void MouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    static float last;
    static BOOL move;

    // Get points

    POINTS points = MAKEPOINTS(lParam);

    // Button down

    if (wParam & MK_LBUTTON)
    {
	RECT rect;

	// Get bounds

	GetClientRect(hwnd, &rect);

	int w2 = rect.right / 2;
	int h2 = rect.bottom / 2;

	// Calculate position relative to centre

	int x = points.x - w2;
	int y = points.y - h2;

	// Calculate angle

	float theta = atan2(x, -y);

	// First point

	if (!move)
	    move = TRUE;

	// More points

	else
	{
	    // Difference

	    float delta = theta - last;

	    // Allow for crossing origin

	    if (delta > M_PI)
		delta -= 2.0 * M_PI;

	    if (delta < -M_PI)
		delta += 2.0 * M_PI;

	    // Calculate scale value

	    scale.v += round(delta * 100.0 / M_PI);

	    // Enforce limits

	    if (scale.v < FREQ_MIN)
		scale.v = FREQ_MIN;

	    if (scale.v > FREQ_MAX)
		scale.v = FREQ_MAX;

	    // Update frequency

	    UpdateValues();
	}

	// Remember angle

	last = theta;
    }

    // Button not down

    else
	if (move)
	    move = FALSE;
}

// Update values

void UpdateValues()
{
    static double fps = (double)SAMPLE_RATE / (double)SAMPLES;

    // Update frequency

    double frequency = pow(10.0, (double)scale.v / FREQ_SCALE) * 10.0;
    display.f = frequency;
    spectrum.f = frequency /fps;

    InvalidateRgn(display.hwnd, NULL, TRUE);
    InvalidateRgn(scale.hwnd, NULL, TRUE);
}

// Tooltip show

void TooltipShow(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    switch (GetDlgCtrlID((HWND)pnmh->idFrom))
    {
    case DISPLAY_ID:
	SetWindowText(status.hwnd, " Frequency and level display");
	break;

    case METER_ID:
	SetWindowText(status.hwnd, " Level meter");
	break;

    case SPECTRUM_ID:
	SetWindowText(status.hwnd, " Frequency spectrum");
	break;

    case SCALE_ID:
	SetWindowText(status.hwnd, " Frequency scale");
	break;

    case KNOB_ID:
	SetWindowText(status.hwnd, " Frequency adjustment knob");
	break;
    }
}

// Tooltip pop

void TooltipPop(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    SetWindowText(status.hwnd, " Turn knob to adjust measurement frequency");
}

// Audio thread

DWORD WINAPI AudioThread(LPVOID lpParam)
{
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

    // Create the waveform audio input buffers and structures

    static short data[4][STEP];
    static WAVEHDR hdrs[4] =
	{{(LPSTR)data[0], sizeof(data[0]), 0, 0, 0, 0},
	 {(LPSTR)data[1], sizeof(data[1]), 0, 0, 0, 0},
	 {(LPSTR)data[2], sizeof(data[2]), 0, 0, 0, 0},
	 {(LPSTR)data[3], sizeof(data[3]), 0, 0, 0, 0}};

    for (int i = 0; i < LENGTH(hdrs); i++)
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
    // Create buffers for processing the audio data

    static double buffer[SAMPLES];
    static complex x[STEP];
    static double xa[RANGE];

    static double K = 2.0 * M_PI / (double)SAMPLE_RATE;

    // Initialise data structs

    if (spectrum.data == NULL)
    {
	spectrum.data = xa;
	spectrum.length = RANGE;
    }

    // Copy the input data

    memmove(buffer, buffer + STEP, (SAMPLES - STEP) * sizeof(double));

    short *data = (short *)((WAVEHDR *)lParam)->lpData;

    for (int i = 0; i < STEP; i++)
	buffer[SAMPLES - STEP + i] = (double)data[i];

    // Give the buffer back

    waveInAddBuffer(audio.hwi, (WAVEHDR *)lParam, sizeof(WAVEHDR));

    // Maximum data value

    static double dmax;

    if (dmax < 4096.0)
	dmax = 4096.0;

    // Calculate normalising value

    double norm = dmax;
    dmax = 0.0;

    // Copy data to FFT input arrays

    for (int i = 0; i < STEP; i++)
    {
	// Find the magnitude

	if (dmax < fabs(buffer[i]))
	    dmax = fabs(buffer[i]);

	// Calculate the window

	double window =
	    0.5 - 0.5 * cos(2.0 * M_PI *
			    i / STEP);

	// Normalise and window the input data

	x[i].r = (double)buffer[i] / norm * window;
    }

    // do FFT

    fftr(x, STEP);

    // Process FFT output

    for (int i = 1; i < RANGE; i++)
    {
	double real = x[i].r;
	double imag = x[i].i;

	xa[i] = hypot(real, imag);
    }

    // Do cross correlation

    double imag = 0.0;
    double real = 0.0;

    for (int i = 0; i < SAMPLES; i++)
    {
	double window =
	    (0.5 - 0.5 * cos(2.0 * M_PI * i / SAMPLES));

	imag += (buffer[i] / 32768.0) * window * sin(i * display.f * K);

	real += (buffer[i] / 32768.0) * window * cos(i * display.f * K);
    }

    double level = hypot(real, imag);

    level = level / (SAMPLES / (4.0 * sqrt(2.0)));

    meter.l = level / pow(10.0, 0.15);

    double dB = log10(level) * 20.0;

    if (dB < -80.0)
	dB = -80.0;

    static long n;

    // Update display

    if ((n % 4) == 0)
	InvalidateRgn(spectrum.hwnd, NULL, TRUE);

    if ((n % 16) == 0)
    {
	    display.l = dB;
	    InvalidateRgn(display.hwnd, NULL, TRUE);
    }

    n++;
}

// Real to complex FFT, ignores imaginary values in input array

void fftr(complex a[], int n)
{
    double norm = sqrt(1.0 / n);

    for (int i = 0, j = 0; i < n; i++)
    {
	if (j >= i)
	{
	    double tr = a[j].r * norm;

	    a[j].r = a[i].r * norm;
	    a[j].i = 0.0;

	    a[i].r = tr;
	    a[i].i = 0.0;
	}

	int m = n / 2;
	while (m >= 1 && j >= m)
	{
	    j -= m;
	    m /= 2;
	}
	j += m;
    }
    
    for (int mmax = 1, istep = 2 * mmax; mmax < n;
	 mmax = istep, istep = 2 * mmax)
    {
	double delta = (M_PI / mmax);
	for (int m = 0; m < mmax; m++)
	{
	    double w = m * delta;
	    double wr = cos(w);
	    double wi = sin(w);

	    for (int i = m; i < n; i += istep)
	    {
		int j = i + mmax;
		double tr = wr * a[j].r - wi * a[j].i;
		double ti = wr * a[j].i + wi * a[j].r;
		a[j].r = a[i].r - tr;
		a[j].i = a[i].i - ti;
		a[i].r += tr;
		a[i].i += ti;
	    }
	}
    }
}
