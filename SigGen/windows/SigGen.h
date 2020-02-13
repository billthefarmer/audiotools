////////////////////////////////////////////////////////////////////////////////
//
//  SigGen - A signal generator written in C++.
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

#ifndef SIGGEN_H
#define SIGGEN_H

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"
#define KCLASS "KnobWClass"
#define ECLASS "ExactWClass"

// Global handle
HINSTANCE hInst;

// Gdiplus token
ULONG_PTR token;

// Gdiplus input
Gdiplus::GdiplusStartupInput input;

// Tool ids
enum
    {SCALE_ID = 100,
     KNOB_ID,
     FINE_ID,
     LEVEL_ID,
     DISPLAY_ID,
     SINE_ID,
     SQUARE_ID,
     SAWTOOTH_ID,
     MUTE_ID,
     EXACT_ID,
     TEXT_ID,
     EDIT_ID,
     CANCEL_ID,
     OK_ID,
     QUIT_ID,
     STATUS_ID};

// Wave out values
enum
    {SAMPLE_RATE = 44100L,
     BITS_PER_SAMPLE = 16,
     MAX_LEVEL = 32767,
     BLOCK_ALIGN = 2,
     CHANNELS = 1};

// Buffer size
enum
    {SAMPLES = 4096};

// Margins
enum
    {MARGIN = 20,
     SPACING = 8,
     OFFSET = 32};

// View dimensions
enum
    {SCALE_WIDTH = 184,
     SCALE_HEIGHT = 60,
     DISPLAY_WIDTH = 184,
     DISPLAY_HEIGHT = 60,
     KNOB_WIDTH = 184,
     KNOB_HEIGHT = 184,
     SLIDER_WIDTH = 36,
     SLIDER_HEIGHT = 184,
     BUTTON_WIDTH = 72,
     BUTTON_HEIGHT = 24,
     STATUS_HEIGHT = 22,
     TEXT_WIDTH = BUTTON_WIDTH * 2 + SPACING,
     TEXT_HEIGHT = BUTTON_HEIGHT};

// Window size
enum
    {WIDTH  = SCALE_WIDTH + DISPLAY_WIDTH + MARGIN * 3,
     HEIGHT = SCALE_HEIGHT + KNOB_HEIGHT + STATUS_HEIGHT +
     MARGIN * 2 + SPACING};

// Exact size
enum
    {EXACT_WIDTH = BUTTON_WIDTH * 2 + MARGIN * 2 + SPACING,
     EXACT_HEIGHT = TEXT_HEIGHT * 2 + MARGIN * 2 + SPACING * 2 + BUTTON_HEIGHT};

// Frequency scale
enum
    {FREQ_SCALE = 250,
     FREQ_MAX   = 850,
     FREQ_MIN   = 0};

// Fine slider
enum
    {FINE_MAX  = 100,
     FINE_MIN  = 0,
     FINE_REF  = 50,
     FINE_STEP = 10};

// Level slider
enum
    {LEVEL_MAX = 100,
     LEVEL_MIN = 0,
     LEVEL_REF = 80,
     LEVEL_STEP = 10};

// Waveform
enum
    {SINE,
     SQUARE,
     SAWTOOTH};

// Global data
typedef struct
{
    HWND hwnd;
    RECT wind;
    RECT rect;
} WINDOW, *WINDOWP;

WINDOW window;
WINDOW exact;

typedef struct
{
    HWND hwnd;
    RECT rect;
} TOOL, *TOOLP;

TOOL fine;
TOOL level;
TOOL group;
TOOL status;

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
    double frequency;
    double decibels;
} DISPLAY, *DISPLAYP;

DISPLAY display =
    {NULL, {0}, 1000.0, -20.0};

typedef struct
{
    HWND hwnd;
    RECT rect;
    float value;
} KNOB, *KNOBP;
KNOB knob =
    {NULL, {0}, FREQ_SCALE * 2};

typedef struct
{
    TOOL sine;
    TOOL square;
    TOOL sawtooth;
    TOOL mute;
    TOOL exact;
    TOOL quit;
} BUTTONS;

BUTTONS buttons;

typedef struct
{
    TOOL text;
    TOOL edit;
    TOOL cancel;
    TOOL ok;
} WIDGETS;

WIDGETS widgets;

typedef struct
{
    HWND hwnd;
    TOOLINFO info;
} TOOLTIP, *TOOLTIPP;

TOOLTIP tooltip;

typedef struct
{
    HWAVEOUT hwo;
    HANDLE thread;
    int waveform;
    int level;
    BOOL mute;
    DWORD id;
} AUDIO, *AUDIOP;

AUDIO audio;

// Function prototypes.
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KnobWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ExactWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
BOOL RegisterKnobClass(HINSTANCE);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawDisplay(HDC, RECT, UINT);
BOOL DrawScale(HDC, RECT, UINT);
BOOL DrawKnob(HDC, RECT, UINT);
VOID DisplayExact(WPARAM, LPARAM);
VOID CharPressed(WPARAM, LPARAM);
VOID KeyDown(WPARAM, LPARAM);
VOID KnobClicked(HWND, WPARAM, LPARAM);
VOID MouseMove(HWND, WPARAM, LPARAM);
BOOL SliderChange(WPARAM, LPARAM);
VOID ExactFrequency(WPARAM, LPARAM);
VOID UpdateValues(VOID);
VOID TooltipShow(WPARAM, LPARAM);
VOID TooltipPop(WPARAM, LPARAM);
DWORD WINAPI AudioThread(LPVOID);

#endif /* SIGGEN_H */
