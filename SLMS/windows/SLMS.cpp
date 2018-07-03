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

#include "SLMS.h"

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
                             MARGIN, MARGIN,
                             SCALE_WIDTH, SCALE_HEIGHT, hWnd,
                             (HMENU)SCALE_ID, hInst, NULL);

            GetWindowRect(scale.hwnd, &scale.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&scale.rect, 2);

            // Add scale to tooltip
            tooltip.info.uId = (UINT_PTR)scale.hwnd;
            tooltip.info.lpszText = (LPSTR)"Frequency scale";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create knob
            knob.hwnd =
                CreateWindow(KNOBCLASS, NULL,
                             WS_VISIBLE | WS_CHILD,
                             MARGIN, scale.rect.bottom + SPACING,
                             KNOB_WIDTH, KNOB_HEIGHT, hWnd,
                             (HMENU)KNOB_ID, hInst, NULL);

            GetWindowRect(knob.hwnd, &knob.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&knob.rect, 2);

            // Add knob to tooltip
            tooltip.info.uId = (UINT_PTR)knob.hwnd;
            tooltip.info.lpszText = (LPSTR)"Frequency adjustment knob";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create display
            display.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             scale.rect.right + MARGIN, MARGIN,
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
                             display.rect.left, display.rect.bottom + SPACING,
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
                             display.rect.left, meter.rect.bottom + SPACING,
                             SPECTRUM_WIDTH, SPECTRUM_HEIGHT, hWnd,
                             (HMENU)SPECTRUM_ID, hInst, NULL);

            GetWindowRect(spectrum.hwnd, &spectrum.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&spectrum.rect, 2);

            // Add spectrum to tooltip
            tooltip.info.uId = (UINT_PTR)spectrum.hwnd;
            tooltip.info.lpszText = (LPSTR)"Frequency spectrum";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create quit button
            quit.hwnd =
                CreateWindow(WC_BUTTON, "Quit",
                             WS_VISIBLE | WS_CHILD,
                             width - BUTTON_WIDTH - MARGIN,
                             spectrum.rect.bottom + MARGIN,
                             BUTTON_WIDTH, BUTTON_HEIGHT, hWnd,
                             (HMENU)QUIT_ID, hInst, NULL);

            // Start audio thread
            audio.thread = CreateThread(NULL, 0, AudioThread, hWnd, 0, &audio.id);

            // Start meter timer
            CreateTimerQueueTimer(&meter.timer, NULL,
                                  (WAITORTIMERCALLBACK)MeterCallback,
                                  &meter, METER_DELAY, METER_DELAY,
                                  WT_EXECUTEDEFAULT);

            // Update frequency
            UpdateFrequency();
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
            Gdiplus::GdiplusShutdown(token);
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
        Gdiplus::GdiplusShutdown(token);
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
	 NULL, NULL, KNOBCLASS};

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
	lf.lfHeight = height / 2;
	font = CreateFontIndirect(&lf);
	SelectObject(hbdc, font);
	SetTextAlign(hbdc, TA_CENTER | TA_BOTTOM);
    }

    // Erase background
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Translate viewport
    SetViewportOrgEx(hbdc, width / 2, height / 2, NULL);

    // Draw scale
    for (int i = 1; i < 11; i++)
    {
	int x = round(FREQ_SCALE * log10(i)) - scale.value;

	for (int j = 0; j < 4; j++)
	{
	    MoveToEx(hbdc, x, 0, NULL);
	    LineTo(hbdc, x, height / 3);
	    x += FREQ_SCALE;
	}
    }

    for (int i = 3; i < 20; i += 2)
    {
	int x = round(FREQ_SCALE * log10(i / 2.0)) - scale.value;

	for (int j = 0; j < 4; j++)
	{
	    MoveToEx(hbdc, x, height / 6, NULL);
	    LineTo(hbdc, x, height / 3);
	    x += FREQ_SCALE;
	}
    }

    int a[] = {1, 2, 3, 4, 6, 8};
    for (int i = 0; i < Length(a); i++)
    {
    	int x = round(FREQ_SCALE * log10(a[i])) - scale.value;

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

    return true;
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

	int index = (int)round(pow(M_E, x * xscale));
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

	int y = -round(value * yscale);


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

    int xf = round(log(spectrum.f * 4.0) / xscale);

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
    sprintf(s, "%1.1lfHz", display.f);
    TextOut(hbdc, 8, 8, s, strlen(s));

    // Display level
    sprintf(s, "%1.1lfdB", display.l);
    TextOut(hbdc, width / 2 + 8, 8, s, strlen(s));

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
    ml = ((ml * 7.0) + meter.level) / 8.0;

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

// Update meter
VOID UpdateMeter(METERP meter)
{
    static float ml;

    // Do calculation
    ml = ((ml * 7.0) + meter->level) / 8.0;

    int value = round(ml * MAX_METER) + MIN_METER;

    // Update meter
    SendMessage(meter->slider.hwnd, TBM_SETPOS, true, value);
}

// Draw Knob
BOOL DrawKnob(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::LinearGradientBrush;
    using Gdiplus::WrapModeTileFlipX;
    using Gdiplus::GraphicsPath;
    using Gdiplus::SolidBrush;
    using Gdiplus::Graphics;
    using Gdiplus::Matrix;
    using Gdiplus::PointF;
    using Gdiplus::Color;
    using Gdiplus::Point;

    static HBITMAP bitmap;
    static HBITMAP image;
    static HDC hbdc;
    static HDC hidc;

    // Calculate dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (bitmap == NULL)
    {
	// Create DCs
	hbdc = CreateCompatibleDC(hdc);
	hidc = CreateCompatibleDC(hdc);

	// Create new bitmaps
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	image = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);
	SelectObject(hidc, image);

        // Erase background
        FillRect(hidc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // Draw knob
        Graphics graphics(hidc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        LinearGradientBrush gradient(Point(0, 0),
                                     Point(width, height),
                                     Color(255, 255, 255, 255),
                                     Color(255, 127, 127, 127));
        graphics.FillEllipse(&gradient, 0, 0, width, height);
        SolidBrush brush(Color(255, 191, 191, 191));
        graphics.FillEllipse(&brush, 4, 4, width - 8, height - 8);
   }

    // Copy the knob bitmap
    BitBlt(hbdc, rect.left, rect.top, width, height,
    	   hidc, 0, 0, SRCCOPY);

    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    float size = width / 24.0;

    float radius = (width / 2.0) * 0.8;
    float x = width / 2.0 + sin(knob.value * M_PI * 2.0 / FREQ_SCALE) * radius;
    float y = width / 2.0 + -cos(knob.value * M_PI * 2.0 / FREQ_SCALE) * radius;

    LinearGradientBrush dimple(PointF(x - size / 2.0, y - size / 2.0),
                               PointF(x + size / 2.0, y + size / 2.0),
                               Color(127, 127, 127), Color(255, 255, 255));

    graphics.FillEllipse(&dimple, x - size / 2.0, y - size / 2.0, size, size);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
    	   hbdc, 0, 0, SRCCOPY);

    return true;
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
        scale.value = round(scale.value + 1.0);
	if (scale.value > FREQ_MAX)
	    scale.value = FREQ_MAX;
        knob.value = scale.value;
	break;

	// Left, decrease frequency
    case VK_LEFT:
        scale.value = round(scale.value - 1.0);
	if (scale.value < FREQ_MIN)
	    scale.value = FREQ_MIN;
        knob.value = scale.value;
	break;

    default:
	return;
    }

    UpdateFrequency();
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
	    move = true;

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
	    scale.value += delta * 100.0 / M_PI;

	    // Enforce limits
	    if (scale.value < FREQ_MIN)
		scale.value = FREQ_MIN;

	    if (scale.value > FREQ_MAX)
		scale.value = FREQ_MAX;

            // Update knob
            knob.value = scale.value;

	    // Update frequency
	    UpdateFrequency();
	}

	// Remember angle
	last = theta;
    }

    // Button not down
    else
	if (move)
	    move = FALSE;
}

// Update frequency
void UpdateFrequency()
{
    static double fps = (double)SAMPLE_RATE / (double)STEP;

    // Update frequency
    double frequency = pow(10.0, (double)scale.value /
			   (double)FREQ_SCALE) * 10.0;
    display.f = frequency;
    spectrum.f = frequency / fps;

    InvalidateRgn(display.hwnd, NULL, true);
    InvalidateRgn(scale.hwnd, NULL, true);
    InvalidateRgn(knob.hwnd, NULL, TRUE);
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
    // Create buffers for processing the audio data
    static double buffer[SAMPLES];
    static complex x[SAMPLES];
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

    meter.level = level / pow(10.0, 0.15);

    double dB = log10(level) * 20.0;

    if (dB < -80.0)
	dB = -80.0;

    static long timer;

    // Update display

    if ((timer % 4) == 0)
	InvalidateRgn(spectrum.hwnd, NULL, true);

    if ((timer % 16) == 0)
    {
	    display.l = dB;
	    InvalidateRgn(display.hwnd, NULL, true);
    }

    timer++;
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
