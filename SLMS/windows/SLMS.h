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

#ifndef SLMS_H
#define SLMS_H

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>

// Macros

#define Length(a) (sizeof(a) / sizeof(a[0]))

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

// Margins
enum
    {MARGIN = 20,
     SPACING = 8};

// View dimensions
enum
    {SCALE_WIDTH = 168,
     SCALE_HEIGHT = 60,
     DISPLAY_WIDTH = 356,
     DISPLAY_HEIGHT = 60,
     KNOB_WIDTH = 168,
     KNOB_HEIGHT = 168,
     METER_WIDTH = 356,
     METER_HEIGHT = 60,
     SPECTRUM_WIDTH = 356,
     SPECTRUM_HEIGHT = 60,
     BUTTON_WIDTH = 72,
     BUTTON_HEIGHT = 24};

// Dimensions
enum
    {WIDTH      = SCALE_WIDTH + DISPLAY_WIDTH + MARGIN * 3,
     HEIGHT     = SCALE_HEIGHT + KNOB_HEIGHT + MARGIN * 2 + SPACING + 24};

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
    double r;
    double i;
} complex;

typedef struct
{
    HWND hwnd;
    RECT rect;
    RECT clnt;
} WINDOW, *WINDOWP;

WINDOW window;

typedef struct
{
    HWND hwnd;
    RECT rect;
} TOOL, *TOOLP;

TOOL status;
TOOL quit;

typedef struct
{
    HWND hwnd;
    RECT rect;
    double f;
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
    double level;
    TOOL slider;
} METER, *METERP;

METER meter;

typedef struct
{
    HWND hwnd;
    RECT rect;
    float value;
} SCALE, *SCALEP;

SCALE scale =
    {NULL, {0}, FREQ_SCALE * 2};

typedef struct
{
    HWND hwnd;
    RECT rect;
    float value;
} KNOB, *KNOBP;

KNOB knob;

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
VOID UpdateFrequency(VOID);
VOID fftr(complex *, int);

#endif // SLMS_H
