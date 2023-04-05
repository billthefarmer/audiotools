////////////////////////////////////////////////////////////////////////////////
//
//  SigGen - A signal generator written in C++.
//
//  Copyright (C) 2018  Bill Farmer
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

#include "SigGen.h"

// Application entry point.

int WINAPI WinMain(HINSTANCE hInstance,
		   HINSTANCE hPrevInstance,
		   LPSTR lpszCmdLine,
		   int nCmdShow)
{
    // Check for a previous instance of this app
    if (!hPrevInstance)
	if (!RegisterMainClass(hInstance))
	    return false;

    // Save the application-instance handle.
    hInst = hInstance;

    // Initialize common controls to get the new style controls, also
    // dependent on manifest file
    InitCommonControls();

    // Start Gdiplus
    GdiplusStartup(&token, &input, NULL);

    // Register knob class
    RegisterKnobClass(hInstance);

    // Get saved status
    GetSavedStatus();

    // Create the main window.
    window.hwnd = CreateWindow(WCLASS, "Audio Signal Generator",
                               WS_OVERLAPPED | WS_MINIMIZEBOX |
                               WS_SYSMENU | WS_CLIPCHILDREN,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               NULL, 0, hInst, NULL);

    // If the main window cannot be created, terminate
    // the application.
    if (!window.hwnd)
	return false;

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
	{0, MainWndProc,
	 0, 0, hInst,
	 LoadIcon(hInst, "SigGen"),
	 LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, WCLASS};

    // Register the window class.
    return RegisterClass(&wc);
}

// Main window procedure
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg,
			     WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        {
            // Get the window and client dimensions
            GetWindowRect(hWnd, &window.wind);
            GetClientRect(hWnd, &window.rect);

            // Calculate desired window width and height
            int border =
                (window.wind.right - window.wind.left) - window.rect.right;
            int header =
                (window.wind.bottom - window.wind.top) - window.rect.bottom;

            int width  = WIDTH + border;
            int height = HEIGHT + header;

            // Set new dimensions
            SetWindowPos(hWnd, NULL, 0, 0,
                         width, height,
                         SWP_NOMOVE | SWP_NOZORDER);

            // Get client dimensions
            GetWindowRect(hWnd, &window.wind);
            GetClientRect(hWnd, &window.rect);

            width = window.rect.right - window.rect.left;
            height = window.rect.bottom - window.rect.top;

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

            // Create knob
            knob.hwnd =
                CreateWindow(KCLASS, NULL,
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

            // Create frequency slider
            fine.hwnd =
                CreateWindow(TRACKBAR_CLASS, NULL,
                             WS_VISIBLE | WS_CHILD |
                             TBS_VERT | TBS_AUTOTICKS,
                             knob.rect.right + MARGIN + SPACING,
                             knob.rect.top,
                             SLIDER_WIDTH, SLIDER_HEIGHT, hWnd,
                             (HMENU)FINE_ID, hInst, NULL);

            GetWindowRect(fine.hwnd, &fine.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&fine.rect, 2);

            SendMessage(fine.hwnd, TBM_SETRANGE, true,
                        MAKELONG(FINE_MIN, FINE_MAX));
            SendMessage(fine.hwnd, TBM_SETTICFREQ, FINE_STEP, 0);
            SendMessage(fine.hwnd, TBM_SETPAGESIZE, 0, FINE_STEP);
            SendMessage(fine.hwnd, TBM_SETPOS, true, fine.value);

            // Add slider to tooltip
            tooltip.info.uId = (UINT_PTR)fine.hwnd;
            tooltip.info.lpszText = (LPSTR)"Fine frequency adjustment";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create level slider
            level.hwnd =
                CreateWindow(TRACKBAR_CLASS, NULL,
                             WS_VISIBLE | WS_CHILD |
                             TBS_VERT | TBS_AUTOTICKS | TBS_LEFT,
                             fine.rect.right + SPACING, fine.rect.top,
                             SLIDER_WIDTH, SLIDER_HEIGHT, hWnd,
                             (HMENU)LEVEL_ID, hInst, NULL);

            GetWindowRect(level.hwnd, &level.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&level.rect, 2);

            SendMessage(level.hwnd, TBM_SETRANGE, true,
                        MAKELONG(LEVEL_MIN, LEVEL_MAX));
            SendMessage(level.hwnd, TBM_SETTICFREQ, FINE_STEP, 0);
            SendMessage(level.hwnd, TBM_SETPAGESIZE, 0, LEVEL_STEP);
            SendMessage(level.hwnd, TBM_SETPOS, true, level.value);

            // Add slider to tooltip
            tooltip.info.uId = (UINT_PTR)level.hwnd;
            tooltip.info.lpszText = (LPSTR)"Level adjustment";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create sine button
            buttons.sine.hwnd =
                CreateWindow(WC_BUTTON, "Sine",
                             WS_VISIBLE | WS_CHILD | WS_GROUP |
                             BS_AUTORADIOBUTTON | BS_PUSHLIKE,
                             display.rect.right - BUTTON_WIDTH,
                             level.rect.top,
                             BUTTON_WIDTH, BUTTON_HEIGHT, hWnd,
                             (HMENU)SINE_ID, hInst, NULL);

            GetWindowRect(buttons.sine.hwnd, &buttons.sine.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&buttons.sine.rect, 2);

            if (audio.wave == SINE)
                SendMessage(buttons.sine.hwnd, BM_SETCHECK, BST_CHECKED, 0);

            // Create square button
            buttons.square.hwnd =
                CreateWindow(WC_BUTTON, "Square",
                             WS_VISIBLE | WS_CHILD |
                             BS_AUTORADIOBUTTON | BS_PUSHLIKE,
                             buttons.sine.rect.left,
                             buttons.sine.rect.bottom + SPACING,
                             BUTTON_WIDTH, BUTTON_HEIGHT, hWnd,
                             (HMENU)SQUARE_ID, hInst, NULL);

            GetWindowRect(buttons.square.hwnd, &buttons.square.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&buttons.square.rect, 2);

            if (audio.wave == SQUARE)
                SendMessage(buttons.square.hwnd, BM_SETCHECK, BST_CHECKED, 0);

            // Create sawtooth button
            buttons.sawtooth.hwnd =
                CreateWindow(WC_BUTTON, "Sawtooth",
                             WS_VISIBLE | WS_CHILD |
                             BS_AUTORADIOBUTTON | BS_PUSHLIKE,
                             buttons.square.rect.left,
                             buttons.square.rect.bottom + SPACING,
                             BUTTON_WIDTH, BUTTON_HEIGHT, hWnd,
                             (HMENU)SAWTOOTH_ID, hInst, NULL);

            GetWindowRect(buttons.sawtooth.hwnd, &buttons.sawtooth.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&buttons.sawtooth.rect, 2);

            if (audio.wave == SAWTOOTH)
                SendMessage(buttons.sawtooth.hwnd, BM_SETCHECK, BST_CHECKED, 0);

            // Create mute button
            buttons.mute.hwnd =
                CreateWindow(WC_BUTTON, "Mute",
                             WS_VISIBLE | WS_CHILD |
                             BS_AUTOCHECKBOX | BS_PUSHLIKE,
                             buttons.sawtooth.rect.left,
                             buttons.sawtooth.rect.bottom + SPACING,
                             BUTTON_WIDTH, BUTTON_HEIGHT, hWnd,
                             (HMENU)MUTE_ID, hInst, NULL);

            GetWindowRect(buttons.mute.hwnd, &buttons.mute.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&buttons.mute.rect, 2);

            if (audio.mute)
                SendMessage(buttons.mute.hwnd, BM_SETCHECK, BST_CHECKED, 0);

            // Create exact button
            buttons.exact.hwnd =
                CreateWindow(WC_BUTTON, "Exact...",
                             WS_VISIBLE | WS_CHILD,
                             buttons.mute.rect.left,
                             buttons.mute.rect.bottom + SPACING,
                             BUTTON_WIDTH, BUTTON_HEIGHT, hWnd,
                             (HMENU)EXACT_ID, hInst, NULL);

            GetWindowRect(buttons.exact.hwnd, &buttons.exact.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&buttons.exact.rect, 2);

            // Create quit button
            buttons.quit.hwnd =
                CreateWindow(WC_BUTTON, "Quit",
                             WS_VISIBLE | WS_CHILD,
                             buttons.exact.rect.left,
                             buttons.exact.rect.bottom + SPACING,
                             BUTTON_WIDTH, BUTTON_HEIGHT, hWnd,
                             (HMENU)QUIT_ID, hInst, NULL);

            GetWindowRect(buttons.quit.hwnd, &buttons.quit.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&buttons.quit.rect, 2);

            // Create status bar
            status.hwnd =
                CreateWindow(STATUSCLASSNAME,
                             " Turn knob to adjust frequency,"
                             " fine left, level right slider",
                             WS_VISIBLE | WS_CHILD,
                             0, 0, 0, 0, hWnd,
                             (HMENU)STATUS_ID, hInst, NULL);

            GetWindowRect(status.hwnd, &status.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&status.rect, 2);

            // Start audio thread
            audio.thread = CreateThread(NULL, 0, AudioThread,
                                        hWnd, 0, &audio.id);
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
	    // Sine
	case SINE_ID:
	    audio.wave = SINE;
	    break;

	    // Square
	case SQUARE_ID:
	    audio.wave = SQUARE;
	    break;

	    // Sawtooth
	case SAWTOOTH_ID:
	    audio.wave = SAWTOOTH;
	    break;

	    // Mute
	case MUTE_ID:
	    audio.mute = !audio.mute;
	    break;

	    // Exact
	case EXACT_ID:
            DisplayExact(wParam, lParam);
	    break;

	    // Quit
	case QUIT_ID:
            DestroyWindow(hWnd);
	    break;
	}

        HKEY hkey;
        LONG error;

        // Save values
        error = RegCreateKeyEx(HKEY_CURRENT_USER,
                               "SOFTWARE\\Audiotools\\SigGen", 0,
                               NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

        if (error == ERROR_SUCCESS)
        {
            RegSetValueEx(hkey, WAVE, 0, REG_DWORD,
                          (LPBYTE)&audio.wave, sizeof(audio.wave));
            RegSetValueEx(hkey, MUTE, 0, REG_DWORD,
                          (LPBYTE)&audio.mute, sizeof(audio.mute));
            RegCloseKey(hkey);
        }

        else
        {
            static TCHAR s[64];

            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
                          0, s, sizeof(s), NULL);
            MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
        }

	// Set the focus back to the window
	SetFocus(hWnd);
	break;

	// Slider change
    case WM_VSCROLL:
	SliderChange(wParam, lParam);

	// Set the focus back to the window
	SetFocus(hWnd);
	break;

	// Set the focus back to the window by clicking
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
	SetFocus(hWnd);
	break;

    case WM_RBUTTONDOWN:
	// Not used
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

// Register class
BOOL RegisterKnobClass(HINSTANCE hinst)
{
    // Fill in the window class structure with parameters
    // that describe the main window.
    WNDCLASS wc = 
	{0, KnobWndProc,
	 0, 0, hinst,
	 NULL, LoadCursor(NULL, IDC_ARROW),
	 NULL, NULL, KCLASS};

    // Register the window class.
    return RegisterClass(&wc);
}

// Knob procedure
LRESULT CALLBACK KnobWndProc(HWND hWnd, UINT uMsg,
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
	DrawKnob(hdc, rect, 0);
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

// Draw item
BOOL DrawItem(WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpdi = (LPDRAWITEMSTRUCT)lParam;
    UINT state = lpdi->itemState;
    RECT rect = lpdi->rcItem;
    HDC hdc = lpdi->hDC;

    switch (wParam)
    {
	// Scale
    case SCALE_ID:
	return DrawScale(hdc, rect, state);
	break;

	// Display
    case DISPLAY_ID:
	return DrawDisplay(hdc, rect, state);
	break;

	// Knob
    case KNOB_ID:
	return DrawKnob(hdc, rect, state);
	break;
    }

    return false;
}

// Draw Scale
BOOL DrawScale(HDC hdc, RECT rect, UINT state)
{
    static HBITMAP bitmap;
    static HFONT font;
    static HDC hbdc;

    // Plain vanilla font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_NORMAL,
	 false, false, false,
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

// Draw Display
BOOL DrawDisplay(HDC hdc, RECT rect, UINT state)
{
    static HFONT font;
    static char text[16];

    // Bold font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 false, false, false,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions
    int x = rect.left + 8;
    int y = rect.top;
    int height = rect.bottom - rect.top;

    // Create font
    if (font == NULL)
    {
	lf.lfHeight = height / 2;
	font = CreateFontIndirect(&lf);
    }

    SelectObject(hdc, font);

    sprintf(text, "%6.2lfHz  ", display.frequency);
    TextOut(hdc, x, y, text, strlen(text));

    y += height / 2;

    sprintf(text, "%+6.2lfdB  ", display.decibels);
    TextOut(hdc, x, y, text, strlen(text));

    return true;
}

// Draw Knob

BOOL DrawKnob(HDC hdc, RECT rect, UINT state)
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
VOID CharPressed(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    default:
	return;
    }
}

// Key pressed
VOID KeyDown(WPARAM wParam, LPARAM lParam)
{
    int value;

    switch(wParam)
    {
	// Right, increase frequency
    case VK_RIGHT:
        knob.value = round(knob.value + 1.0);
	if (knob.value > FREQ_MAX)
	    knob.value = FREQ_MAX;
        scale.value = knob.value;
	break;

	// Left, decrease frequency
    case VK_LEFT:
        knob.value = round(knob.value - 1.0);
	if (knob.value < FREQ_MIN)
	    knob.value = FREQ_MIN;
        scale.value = knob.value;
	break;

	// Up, increase fine frequency (sliders are backwards)
    case VK_UP:
	value = SendMessage(fine.hwnd, TBM_GETPOS, 0, 0);
	SendMessage(fine.hwnd, TBM_SETPOS, true, --value);
	break;

	// Down, decrease fine frequency
    case VK_DOWN:
	value = SendMessage(fine.hwnd, TBM_GETPOS, 0, 0);
	SendMessage(fine.hwnd, TBM_SETPOS, true, ++value);
	break;

	// Page up, increase level (sliders are backwards)
    case VK_PRIOR:
	value = SendMessage(level.hwnd, TBM_GETPOS, 0, 0);
	SendMessage(level.hwnd, TBM_SETPOS, true, --value);
	break;

	// Page down, decrease level
    case VK_NEXT:
	value = SendMessage(level.hwnd, TBM_GETPOS, 0, 0);
	SendMessage(level.hwnd, TBM_SETPOS, true, ++value);
	break;

    default:
	return;
    }

    UpdateValues();
}

// Knob clicked
VOID KnobClicked(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    switch(HIWORD(wParam))
    {
    case STN_CLICKED:
	break;
    }
}

// Mouse move
VOID MouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
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
	    UpdateValues();
	}

	// Remember angle
	last = theta;
    }

    // Button not down
    else
	if (move)
	    move = false;
}

// Slider change
BOOL SliderChange(WPARAM wParam, LPARAM lParam)
{
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
	return false;
    }

    // Update
    UpdateValues();
    return true;
}

// Display exact
VOID DisplayExact(WPARAM wParam, LPARAM lParam)
{
    WNDCLASS wc =
        {CS_HREDRAW | CS_VREDRAW, ExactWndProc,
         0, 0, hInst,
         LoadIcon(hInst, "SigGen"),
         LoadCursor(NULL, IDC_ARROW),
         GetSysColorBrush(COLOR_WINDOW),
         NULL, ECLASS};

    // Register the window class.
    RegisterClass(&wc);

    // Get the main window rect
    GetWindowRect(window.hwnd, &window.wind);

    // Create the window, offset right
    exact.hwnd =
        CreateWindow(ECLASS, "Exact frequency",
                     WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION,
                     window.wind.left + OFFSET,
                     window.wind.top + OFFSET,
                     EXACT_WIDTH, EXACT_HEIGHT,
                     window.hwnd, (HMENU)NULL, hInst, NULL);
}

// Exact Procedure
LRESULT CALLBACK ExactWndProc(HWND hWnd, UINT uMsg,
                              WPARAM wParam, LPARAM lParam)
{
    // Switch on message
    switch (uMsg)
    {
    case WM_CREATE:
        {
            static TCHAR s[64];

            // Get the window and client dimensions
            GetWindowRect(hWnd, &exact.wind);
            GetClientRect(hWnd, &exact.rect);

            // Calculate desired window width and height
            int border = (exact.wind.right - exact.wind.left) -
                exact.rect.right;
            int header = (exact.wind.bottom - exact.wind.top) -
                exact.rect.bottom;
            int width  = EXACT_WIDTH + border;
            int height = EXACT_HEIGHT + header;

            // Set new dimensions
            SetWindowPos(hWnd, NULL, 0, 0,
                         width, height,
                         SWP_NOMOVE | SWP_NOZORDER);

            // Get client dimensions
            GetWindowRect(hWnd, &exact.wind);
            GetClientRect(hWnd, &exact.rect);

            width = exact.rect.right;
            height = exact.rect.bottom;

            // Create text
            widgets.text.hwnd =
                CreateWindow(WC_STATIC, "Enter exact frequency",
                             WS_VISIBLE | WS_CHILD | SS_LEFT,
                             exact.rect.left + MARGIN,
                             exact.rect.top + MARGIN,
                             TEXT_WIDTH, TEXT_HEIGHT, hWnd,
                             (HMENU)TEXT_ID, hInst, NULL);

            GetWindowRect(widgets.text.hwnd, &widgets.text.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&widgets.text.rect, 2);

            // Create edit control
            sprintf(s, " %6.2lf", display.frequency);

            widgets.edit.hwnd =
                CreateWindow(WC_EDIT, s,
                             WS_VISIBLE | WS_CHILD | WS_BORDER,
                             widgets.text.rect.left,
                             widgets.text.rect.bottom + SPACING,
                             TEXT_WIDTH, TEXT_HEIGHT, hWnd,
                             (HMENU)EDIT_ID, hInst, NULL);

            GetWindowRect(widgets.edit.hwnd, &widgets.edit.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&widgets.edit.rect, 2);

            // Add edit to tooltip
            tooltip.info.uId = (UINT_PTR)widgets.edit.hwnd;
            tooltip.info.lpszText = (LPSTR)"Exact frequency";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create cancel button
            widgets.cancel.hwnd =
                CreateWindow(WC_BUTTON, "Cancel",
                             WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                             widgets.edit.rect.left,
                             widgets.edit.rect.bottom + SPACING,
                             BUTTON_WIDTH, BUTTON_HEIGHT,
                             hWnd, (HMENU)CANCEL_ID, hInst, NULL);

            GetWindowRect(widgets.cancel.hwnd, &widgets.cancel.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&widgets.cancel.rect, 2);

            // Create OK button
            widgets.ok.hwnd =
                CreateWindow(WC_BUTTON, " OK ",
                             WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                             widgets.cancel.rect.right + SPACING,
                             widgets.edit.rect.bottom + SPACING,
                             BUTTON_WIDTH, BUTTON_HEIGHT,
                             hWnd, (HMENU)OK_ID, hInst, NULL);
        }
        break;

    // Colour static text
    case WM_CTLCOLORSTATIC:
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
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
	    // Cancel
	case CANCEL_ID:
            DestroyWindow(hWnd);
	    break;

	    // OK
	case OK_ID:
            ExactFrequency(wParam, lParam);
            DestroyWindow(hWnd);
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

        // Process other messages.
    case WM_DESTROY:
        exact.hwnd = NULL;
	break;

	// Everything else
    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Exact frequency
VOID ExactFrequency(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    GetWindowText(widgets.edit.hwnd, s, sizeof(s));
    float frequency = atof(s);
    knob.value = log10(frequency / 10) * FREQ_SCALE;
    if (knob.value < FREQ_MIN)
        knob.value = FREQ_MIN;
    if (knob.value > FREQ_MAX)
        knob.value = FREQ_MAX;
    scale.value = knob.value;
    SendMessage(fine.hwnd, TBM_SETPOS, true, FINE_REF);
    UpdateValues();
}

// Update values
VOID UpdateValues()
{
    HKEY hkey;

    // Update frequency
    int value = SendMessage(fine.hwnd, TBM_GETPOS, 0, 0);
    double df = -(double)(value - FINE_REF) / 10000.0;

    double frequency = pow(10.0, (double)scale.value / FREQ_SCALE) * 10.0;
    display.frequency = frequency + (frequency * df);

    // Save values
    LONG error = RegCreateKeyEx(HKEY_CURRENT_USER,
                                "SOFTWARE\\Audiotools\\SigGen", 0,
                                NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
        RegSetValueEx(hkey, FINE, 0, REG_DWORD,
                      (LPBYTE)&value, sizeof(value));
        value = round(scale.value);
        RegSetValueEx(hkey, FREQ, 0, REG_DWORD,
                      (LPBYTE)&value, sizeof(value));
    }

    // Update level
    value = SendMessage(level.hwnd, TBM_GETPOS, 0, 0);
    display.decibels = -80.0 * value / LEVEL_MAX;
    audio.level = MAX_LEVEL * pow(10.0, display.decibels / 20.0);

    if (error == ERROR_SUCCESS)
    {
        RegSetValueEx(hkey, LEVEL, 0, REG_DWORD,
                      (LPBYTE)&value, sizeof(value));
        RegCloseKey(hkey);
    }

    else
    {
        static TCHAR s[64];

        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
                      0, s, sizeof(s), NULL);
        MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    InvalidateRgn(display.hwnd, NULL, true);
    InvalidateRgn(scale.hwnd, NULL, true);
    InvalidateRgn(knob.hwnd, NULL, true);
}

// GetSavedStatus
VOID GetSavedStatus()
{
    HKEY hkey;
    int value;
    int size = sizeof(value);

    // Initial values
    audio.mute = false;
    audio.wave = SINE;
    level.value = LEVEL_REF;
    fine.value = FINE_REF;
    scale.value = FREQ_SCALE * 2;
    knob.value = FREQ_SCALE * 2;

    // Open user key
    LONG error = RegOpenKeyEx(HKEY_CURRENT_USER,
                              "SOFTWARE\\Audiotools\\SigGen", 0,
                              KEY_READ, &hkey);

    if (error == ERROR_SUCCESS)
    {
        // Mute
        error = RegQueryValueEx(hkey, MUTE, NULL, NULL,
                                (LPBYTE)&value, (LPDWORD)&size);
        // Update value
        if (error == ERROR_SUCCESS)
            audio.mute = value;

        // Waveform
        error = RegQueryValueEx(hkey, WAVE, NULL, NULL,
                                (LPBYTE)&value, (LPDWORD)&size);
        // Update value
        if (error == ERROR_SUCCESS)
            audio.wave = value;

        // Freq
        error = RegQueryValueEx(hkey, FREQ, NULL, NULL,
                                (LPBYTE)&value, (LPDWORD)&size);
        // Update value
        if (error == ERROR_SUCCESS)
        {
            knob.value = value;
            scale.value = value;
        }

        // Fine
        error = RegQueryValueEx(hkey, FINE, NULL, NULL,
                                (LPBYTE)&value, (LPDWORD)&size);
        // Update value
        if (error == ERROR_SUCCESS)
            fine.value = value;

        // Update frequency
        double df = -(double)(value - FINE_REF) / 10000.0;
        double frequency = pow(10.0, (double)scale.value / FREQ_SCALE) * 10.0;
        display.frequency = frequency + (frequency * df);

        // Level
        error = RegQueryValueEx(hkey, LEVEL, NULL, NULL,
                                (LPBYTE)&value, (LPDWORD)&size);
        // Update value
        if (error == ERROR_SUCCESS)
            level.value = value;

        // Update level
        display.decibels = -80.0 * value / LEVEL_MAX;
        audio.level = MAX_LEVEL * pow(10.0, display.decibels / 20.0);

        // Close key
        RegCloseKey(hkey);
    }
}

// Tooltip show
VOID TooltipShow(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    switch (GetDlgCtrlID((HWND)pnmh->idFrom))
    {
    case DISPLAY_ID:
	SetWindowText(status.hwnd, " Frequency and level display");
	break;

    case SCALE_ID:
	SetWindowText(status.hwnd, " Frequency scale");
	break;

    case KNOB_ID:
	SetWindowText(status.hwnd, " Frequency adjustment knob");
	break;

    case FINE_ID:
	SetWindowText(status.hwnd, " Fine frequency adjustment");
	break;

    case LEVEL_ID:
	SetWindowText(status.hwnd, " Level adjustment");
	break;
    }
}

// Tooltip pop
VOID TooltipPop(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    SetWindowText(status.hwnd, " Turn knob to adjust frequency,"
		  " fine left, level right slider");
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

    // Open a waveform audio output device
    mmr = waveOutOpen(&audio.hwo, WAVE_MAPPER | WAVE_FORMAT_DIRECT, &wf,
		     (DWORD_PTR)audio.id, (DWORD_PTR)NULL, CALLBACK_THREAD);
    if (mmr != MMSYSERR_NOERROR)
    {
	char text[64];

	waveOutGetErrorText(mmr, text, sizeof(text));
	MessageBox(window.hwnd, text, "WaveOutOpen", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Create the waveform audio output buffers and structures
    static short data[2][SAMPLES];
    static WAVEHDR hdrs[2] =
	{{(LPSTR)data[0], sizeof(data[0]), 0, 0, 0, 0},
	 {(LPSTR)data[1], sizeof(data[1]), 0, 0, 0, 0}};

    for (int i = 0; i < Length(hdrs); i++)
    {
	// Prepare a waveform audio input header
	mmr = waveOutPrepareHeader(audio.hwo, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static char s[64];

	    waveOutGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, "WaveOutPrepareHeader",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}
    }

    // Create a message loop for processing thread messages
    MSG msg;
    BOOL flag;

    while ((flag = GetMessage(&msg, (HWND)-1, 0, 0)) != 0 && !audio.done)
    {
	static double K = 2.0 * M_PI / SAMPLE_RATE;
	static double q = 0.0;
	static double f;
	static double l;

	WAVEHDR *hdrp;
	short *datap;

	if (flag == -1)
	    break;

	// Process messages
	switch (msg.message)
	{
	    // Audio input opened
	case MM_WOM_OPEN:

	    f = display.frequency;

	    // Fill the buffers
	    for (int i = 0; i < Length(data); i++)
	    {
		for (int j = 0; j < SAMPLES; j++)
		{
		    f += ((display.frequency - f) / (double)SAMPLES);
		    l += ((audio.level - l) / (double)SAMPLES);

		    q += (q < M_PI)? f * K: (f * K) - (2.0 * M_PI);

		    data[i][j] = round(sin(q) * l);
		}

		// Write the current buffer
		mmr = waveOutWrite(audio.hwo, &hdrs[i], sizeof(WAVEHDR));

		if (mmr != MMSYSERR_NOERROR)
		{
		    static char s[64];

		    waveOutGetErrorText(mmr, s, sizeof(s));
		    MessageBox(window.hwnd, s, "WaveOutWrite",
			       MB_OK | MB_ICONERROR);
		    return mmr;
		}
	    }
	    break;

	    // Audio input data
	case MM_WOM_DONE:

	    hdrp = (WAVEHDR *) msg.lParam;
	    datap = (short *)hdrp->lpData;

	    // Fill the current buffer
	    for (int i = 0; i < SAMPLES; i++)
	    {
		f += ((display.frequency - f) / (double)SAMPLES);
		l += audio.mute? -l / (double)SAMPLES: 
		    (audio.level - l) / (double)SAMPLES;

		q += (q < M_PI)? f * K: (f * K) - (2.0 * M_PI);

		switch (audio.wave)
		{
		case SINE:
		    datap[i] = round(sin(q) * l);
		    break;

		case SQUARE:
		    datap[i] = (q > 0.0)? l: -l;
		    break;

		case SAWTOOTH:
		    datap[i] = round((q / M_PI) * l);
		    break;
		}
	    }

	    // Write the buffer
	    mmr =  waveOutWrite(audio.hwo, hdrp, sizeof(WAVEHDR));

	    if (mmr != MMSYSERR_NOERROR)
	    {
		static char s[64];

		waveOutGetErrorText(mmr, s, sizeof(s));
		MessageBox(window.hwnd, s, "WaveOutWrite",
			   MB_OK | MB_ICONERROR);
		return mmr;
	    }

	    break;

	    // Audio input closed
	case MM_WOM_CLOSE:
            // Not used
	    break;
	}
    }

    waveOutReset(audio.hwo);
    waveOutClose(audio.hwo);
    return 0;
}
