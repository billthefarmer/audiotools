////////////////////////////////////////////////////////////////////////////////
//
//  LMS - A Level Measuring Set written in C.
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
//  Bill Farmer  william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#include "LMS.h"

// Application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
		   HINSTANCE hPrevInstance,
		   LPSTR lpszCmdLine,
		   int nCmdShow)
{
    // Check for a previous instance of this app
    if (!hPrevInstance)
	if (!RegisterMainClass(hInstance))
	    return FALSE;

    // Save the application-instance handle.
    hInst = hInstance;

    // Initialize common controls to get the new style controls, also
    // dependent on manifest file
    InitCommonControls();

    // Start Gdiplus
    GdiplusStartup(&token, &input, NULL);

    // Create the main window.
    window.hwnd =
	CreateWindow(WCLASS, "Level Measuring Set",
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
	 LoadIcon(hInst, "LMS"),
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
        {
            // Get the window and client dimensions
            GetWindowRect(hWnd, &window.rect);
            GetClientRect(hWnd, &window.clnt);

            // Calculate desired window width and height
            int border =
                (window.rect.right - window.rect.left) - window.clnt.right;
            int header =
                (window.rect.bottom - window.rect.top) - window.clnt.bottom;

            int width  = WIDTH + border;
            int height = HEIGHT + header;

            // Set new dimensions
            SetWindowPos(hWnd, NULL, 0, 0,
                         width, height,
                         SWP_NOMOVE | SWP_NOZORDER);

            // Get client dimensions
            GetWindowRect(hWnd, &window.rect);
            GetClientRect(hWnd, &window.clnt);

            width = window.clnt.right - window.clnt.left;
            height = window.clnt.bottom - window.clnt.top;

            // Create status bar
            status.hwnd =
                CreateWindow(STATUSCLASSNAME, " Level measuring set",
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

            // Create display
            display.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, MARGIN,
                             DISPLAY_WIDTH, DISPLAY_HEIGHT, hWnd,
                             (HMENU)DISPLAY_ID, hInst, NULL);

            GetWindowRect(display.hwnd, &display.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&display.rect, 2);

            // Add display to tooltip
            tooltip.info.uId = (UINT_PTR)display.hwnd;
            tooltip.info.lpszText = (LPSTR)"Frequency and level display";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create meter
            meter.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, display.rect.bottom + SPACING,
                             METER_WIDTH, METER_HEIGHT, hWnd,
                             (HMENU)METER_ID, hInst, NULL);

            GetWindowRect(meter.hwnd, &meter.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&meter.rect, 2);

            // Add meter to tooltip
            tooltip.info.uId = (UINT_PTR)meter.hwnd;
            tooltip.info.lpszText = (LPSTR)"Level meter";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create spectrum
            spectrum.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, meter.rect.bottom + SPACING,
                             SPECTRUM_WIDTH, SPECTRUM_HEIGHT, hWnd,
                             (HMENU)SPECTRUM_ID, hInst, NULL);

            GetWindowRect(spectrum.hwnd, &spectrum.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&spectrum.rect, 2);

            // Add spectrum to tooltip
            tooltip.info.uId = (UINT_PTR)spectrum.hwnd;
            tooltip.info.lpszText = (LPSTR)"Frequency spectrum";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Start audio thread
            audio.thread = CreateThread(NULL, 0, AudioThread, hWnd, 0, &audio.id);

            // Start meter timer
            CreateTimerQueueTimer(&meter.timer, NULL,
                                  (WAITORTIMERCALLBACK)MeterCallback,
                                  &meter, METER_DELAY, METER_DELAY,
                                  WT_EXECUTEDEFAULT);
        }
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

        // Close
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

        // Process other messages.
    case WM_DESTROY:
        Gdiplus::GdiplusShutdown(token);
        audio.done = true;
	PostQuitMessage(0);
	break;

	// Everything else
    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
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

    return false;
}

// Draw spectrum
BOOL DrawSpectrum(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::GraphicsPath;
    using Gdiplus::SolidBrush;
    using Gdiplus::Graphics;
    using Gdiplus::PointF;
    using Gdiplus::Color;
    using Gdiplus::Pen;

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
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Dark green graticule
    SetDCPenColor(hbdc, RGB(0, 64, 0));

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

	return true;
    }

    // Graphics
    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // Move the origin
    graphics.TranslateTransform(0.0, height - 1);

    // Green pen for spectrum trace
    Pen pen(Color(0, 255, 0), 1);
    // Transparent green brush for spectrum fill
    SolidBrush brush(Color(63, 0, 255, 0));

    static float max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling
    float yscale = (float)height / max;

    max = 0.0;

    // Draw the spectrum
    PointF lastp(0.0, 0.0);
    GraphicsPath path;

    float xscale = (float)log(spectrum.length) / width;

    // Create trace
    int last = 1;
    for (int x = 0; x < width; x++)
    {
	float value = 0.0;

	int index = round(pow(M_E, x * xscale));
	for (int i = last; i <= index; i++)
	{
	    // Don't show DC component
	    if (i > 0 && i < spectrum.length)
	    {
		if (value < spectrum.data[i])
		    value = spectrum.data[i];
	    }
	}

	// Update last index
	last = index;

	if (max < value)
	    max = value;

	float y = -value * yscale;

        PointF point(x, y);
        path.AddLine(lastp, point);

        lastp = point;
    }

    graphics.DrawPath(&pen, &path);
    path.AddLine(lastp, PointF(width, 0.0));
    path.CloseFigure();
    graphics.FillPath(&brush, &path);

    // Move the origin back
    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Yellow pen for frequency trace
    SetDCPenColor(hbdc, RGB(255, 255, 0));

    int xf = round(log(spectrum.s) / xscale);

    MoveToEx(hbdc, xf, 0, NULL);
    LineTo(hbdc, xf, height);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return true;
}

// Draw display
BOOL DrawDisplay(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HFONT font;
    static HDC hbdc;

    enum
    {FONT_HEIGHT = 42};

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

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC
    if (hbdc == NULL)
    {
	// Create DC
	hbdc = CreateCompatibleDC(hdc);

	// Create bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);
    }

    // Create font
    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
        SelectObject(hbdc, font);
    }

    // Erase background
    SetViewportOrgEx(hbdc, 0, 0, NULL);
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Display frequency
    sprintf(s, "%1.2lfHz", display.f);
    SetTextAlign(hbdc, TA_TOP|TA_LEFT);
    TextOut(hbdc, 8, 8, s, strlen(s));

    // Display level
    sprintf(s, "%1.2lfdB", display.l);
    SetTextAlign(hbdc, TA_TOP|TA_RIGHT);
    TextOut(hbdc, width - 8, 8, s, strlen(s));

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return true;
}

// Draw meter
BOOL DrawMeter(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::LinearGradientBrush;
    using Gdiplus::WrapModeTileFlipX;
    using Gdiplus::GraphicsPath;
    using Gdiplus::SolidBrush; 
    using Gdiplus::Graphics; 
    using Gdiplus::Matrix;
    using Gdiplus::Color;
    using Gdiplus::Point;
    using Gdiplus::Pen;

    static HBITMAP bitmap;
    static HFONT font;
    static HDC hbdc;

    static float ml;

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

    // Create DC
    if (hbdc == NULL)
    {
	// Create DC
	hbdc = CreateCompatibleDC(hdc);
	SetTextAlign(hbdc, TA_CENTER);

	// Create bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);
    }

    // Select font
    if (font == NULL)
    {
	lf.lfHeight = height / 3;
	font = CreateFontIndirect(&lf);
        SelectObject(hbdc, font);
    }

    // Erase background
    SetViewportOrgEx(hbdc, 0, 0, NULL);
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Draw the meter scale
    static int as[] =
	{-40, -20, -14, -10, -8, -7, -6, -5,
	 -4, -3, -2, -1, 0, 1, 2, 3};

    for (int i = 0; i < Length(as); i++)
    {
	static char s[16];

	sprintf(s, "%d", abs(as[i]));

	float x = pow(10.0, (as[i] + 20.0) / 20.0) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;
	x *= (width - 22);
	x += 10;

	TextOut(hbdc, x, 0, s, strlen(s));
    }

    static int at[] =
	{-10, -5, 0, 1, 2, 3, 4, 5,
	 6, 7, 8, 9, 10, 11, 12, 13};

    for (int i = 0; i < Length(at); i++)
    {
	float x = pow(10.0, at[i] / 10.0) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;
	x *= (width - 22);
	x += 10;

	MoveToEx(hbdc, x, height * 5 / 16, NULL);
	LineTo(hbdc, x, height / 2);
    }

    for (int i = 1; i < 26; i += 2)
    {
	float x = pow(10.0, (i / 20.0)) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;
	x *= (width - 22);
	x += 10;

	MoveToEx(hbdc, x, height * 3 / 8, NULL);
	LineTo(hbdc, x, height / 2);
    }

    for (int i = 17; i < 48; i += 2)
    {
	float x = pow(10.0, (i / 40.0)) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;
	x *= (width - 22);
	x += 10;

	MoveToEx(hbdc, x, height * 7 / 16, NULL);
	LineTo(hbdc, x, height / 2);
    }

    RECT bar = {10, (height * 3 / 4) - 2,
                width - 10, (height * 3 / 4) + 2};
    FrameRect(hbdc, &bar, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

    // Do calculation
    ml = ((ml * 7.0) + meter.l) / 8.0;

    GraphicsPath path;
    path.AddLine(0, 2, 1, 1);
    path.AddLine(1, 1, 1, -2);
    path.AddLine(1, -2, -1, -2);
    path.AddLine(-1, -2, -1, 1);
    path.CloseFigure();

    LinearGradientBrush brush(Point(0, 2), Point(0, -2),
                              Color(255, 255, 255), Color(63, 63, 63));
    brush.SetWrapMode(WrapModeTileFlipX);

    Matrix matrix;
    matrix.Translate(ml * (width - 20) + 10.0, (height * 3 / 4) - 2);
    matrix.Scale(height / 12.0, -height / 12.0);

    path.Transform(&matrix);
    brush.ScaleTransform(height / 24.0, height / 24.0);

    Pen pen(Color(127, 127, 127));

    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.FillPath(&brush, &path);
    graphics.DrawPath(&pen, &path);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return true;
}

// Meter callback
VOID CALLBACK MeterCallback(PVOID lpParam, BOOL TimerFired)
{
    // Update meter
    InvalidateRgn(meter.hwnd, NULL, true);
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
    }
}

// Tooltip pop
void TooltipPop(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    SetWindowText(status.hwnd, " Level measuring set");
}

// Audio thread
DWORD WINAPI AudioThread(LPVOID lpParameter)
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

    while ((flag = GetMessage(&msg, (HWND)-1, 0, 0)) != 0 && !audio.done)
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

    waveInStop(audio.hwi);
    waveInClose(audio.hwi);

    return 0;
}

// Wave in data
void WaveInData(WPARAM wParam, LPARAM lParam)
{
    enum
    {FREQ_TIME    = 64,
     DISPLAY_TIME = 16,
     SPECTRUM_TIME = 4};

    // Create buffers for processing the audio data
    static double buffer[SAMPLES];
    static complex x[SAMPLES];

    static double xa[RANGE];
    static double xp[RANGE];
    static double xf[RANGE];

    static double fps = (double)SAMPLE_RATE / (double)SAMPLES;
    static double expect = 2.0 * M_PI * (double)STEP / (double)SAMPLES;

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
    for (int i = 0; i < SAMPLES; i++)
    {
	// Find the magnitude
	if (dmax < fabs(buffer[i]))
	    dmax = fabs(buffer[i]);

	// Calculate the window
	double window =
	    0.5 - 0.5 * cos(2.0 * M_PI * i / SAMPLES);

	// Normalise and window the input data
	x[i].r = (double)buffer[i] / norm * window;
    }

    // do FFT
    fftr(x, SAMPLES);

    // Process FFT output
    for (int i = 1; i < RANGE; i++)
    {
	double real = x[i].r;
	double imag = x[i].i;

	xa[i] = hypot(real, imag);

	// Do frequency calculation
	double p = atan2(imag, real);
	double dp = xp[i] - p;
	xp[i] = p;

	// Calculate phase difference
	dp -= (double)i * expect;

	int qpd = dp / M_PI;

	if (qpd >= 0)
	    qpd += qpd & 1;

	else
	    qpd -= qpd & 1;

	dp -=  M_PI * (double)qpd;

	// Calculate frequency difference
	double df = OVERSAMPLE * dp / (2.0 * M_PI);

	// Calculate actual frequency from slot frequency plus
	// frequency difference
	xf[i] = (i * fps + df * fps);
    }

    // Maximum FFT output
    double max = 0.0;
    double f = 0.0;

    // Find maximum value
    for (int i = 1; i < RANGE - 1; i++)
    {
	if (xa[i] > max)
	{
	    max = xa[i];
	    f = xf[i];
	}
    }

    static long freqTimer;

    if (max > MIN)
    {
	display.f = f;
        spectrum.s = f / fps;
	freqTimer = 0;
    }

    else
    {
	if (freqTimer == 64)
        {
	    display.f = 0.0;
            spectrum.s = 0.0;
        }
    }

    freqTimer++;

    double level = 0.0;

    for (int i = 0; i < STEP; i++)
	level += ((double)data[i] / 32768.0) *
	    ((double)data[i] / 32768.0);

    level = sqrt(level / STEP) * 2.0;

    double dB = log10(level) * 20.0;

    if (dB < -80.0)
	dB = -80.0;

    display.l = dB;

    meter.l = level / pow(10.0, 0.15);

    static long displayTimer;

    // Update display
    if ((displayTimer % SPECTRUM_TIME) == 0)
	InvalidateRgn(spectrum.hwnd, NULL, true);

    if ((displayTimer % DISPLAY_TIME) == 0)
	InvalidateRgn(display.hwnd, NULL, true);

    displayTimer++;
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
