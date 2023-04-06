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

#ifndef LMS_H
#define LMS_H

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"

#define MIN 0.5

// Tool ids
enum
    {DISPLAY_ID = 100,
     SPECTRUM_ID,
     METER_ID,
     SLIDER_ID,
     STATUS_ID};

// Timer values
enum
    {METER_DELAY = 100};

// Margins
enum
    {MARGIN = 20,
     SPACING = 8};

// View dimensions
enum
    {DISPLAY_WIDTH = 356,
     DISPLAY_HEIGHT = 60,
     METER_WIDTH = 356,
     METER_HEIGHT = 60,
     SPECTRUM_WIDTH = 356,
     SPECTRUM_HEIGHT = 60};

// Dimensions
enum
    {WIDTH = DISPLAY_WIDTH + MARGIN * 2,
     HEIGHT = DISPLAY_HEIGHT * 3 + MARGIN * 2 + SPACING * 2 + 24};

// Wave in values
enum
    {SAMPLE_RATE = 44100L,
     BITS_PER_SAMPLE = 16,
     BLOCK_ALIGN = 2,
     CHANNELS = 1};

// Audio processing values
enum
    {OVERSAMPLE = 4,
     SAMPLES = 4096,
     RANGE = SAMPLES * 15 / 32,
     STEP = SAMPLES / OVERSAMPLE};

// Global data
HINSTANCE hInst;

// Gdiplus token
ULONG_PTR token;

// Gdiplus input
Gdiplus::GdiplusStartupInput input;

typedef struct
{
    HWND hwnd;
    RECT rect;
    RECT clnt;
} WINDOW, *WINDOWP;

WINDOW window;

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

TOOL status;

typedef struct
{
    HWND hwnd;
    RECT rect;
    double s;
    UINT length;
    double *data;
} SPECTRUM, *SPECTRUMP;

SPECTRUM spectrum;

typedef struct
{
    HWND hwnd;
    RECT rect;
    double f;
    double l;
} DISPLAY, *DISPLAYP;

DISPLAY display;

typedef struct
{
    HWND hwnd;
    RECT rect;
    HANDLE timer;
    double l;
} METER, *METERP;

METER meter;

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
    BOOL done;
} AUDIO, *AUDIOP;

AUDIO audio;

// Function prototypes.
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawSpectrum(HDC, RECT);
BOOL DrawDisplay(HDC, RECT);
BOOL DrawMeter(HDC, RECT);
VOID TooltipShow(WPARAM, LPARAM);
VOID TooltipPop(WPARAM, LPARAM);
DWORD WINAPI AudioThread(LPVOID);
VOID WaveInData(WPARAM, LPARAM);
VOID CALLBACK MeterCallback(PVOID, BOOL);
VOID fftr(complex[], int);

#endif // LMS_H
