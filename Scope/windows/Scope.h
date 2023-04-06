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

#ifndef SCOPE_H
#define SCOPE_H

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"

#define BRIGHT  "Bright"
#define SINGLE  "Single"
#define STORAGE "Storage"
#define TIMEBAS "Timebase"

// Tool ids
enum
    {SCOPE_ID = 100,
     XSCALE_ID,
     YSCALE_ID,
     TOOLBAR_ID,
     STATUS_ID,

     BRIGHT_ID,
     SINGLE_ID,
     TRIGGER_ID,
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
    {BRIGHT_BM,
     SINGLE_BM,
     TRIGGER_BM,
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
     MAX_HEIGHT = 1024,
     MAX_WIDTH = 1280};

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

// State machine values
enum
    {INIT,
     FIRST,
     NEXT,
     LAST};

// Timebase default
enum
    {TIMEBASE_DEFAULT = 3};

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
} TOOL, *TOOLP;

TOOL window;
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
    RECT rect;
    int index;
    int length;
    int yscale;
    short *data;
    float scale;
    float start;
    BOOL bright;
    BOOL single;
    BOOL trigger;
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

typedef struct
{
    HWND hwnd;
    RECT rect;
    int height;
    int index;
} YSCALE, *YSCALEP;
YSCALE yscale;

typedef struct
{
    DWORD id;
    HWAVEIN hwi;
    HANDLE thread;
    BOOL done;
} AUDIO, *AUDIOP;
AUDIO audio;

typedef struct
{
    int index;
    const float values[12];
    const char *strings[12];
    const int counts[12];
} TIMEBASE, *TIMEBASEP;
TIMEBASE timebase =
    {TIMEBASE_DEFAULT,
     {0.1, 0.2, 0.5, 1.0,
      2.0, 5.0, 10.0, 20.0,
      50.0, 100.0, 200.0, 500.0},
     {"0.1 ms", "0.2 ms", "0.5 ms",
      "1.0 ms", "2.0 ms", "5.0 ms",
      "10 ms", "20 ms", "50 ms",
      "0.1 sec", "0.2 sec", "0.5 sec"},
     {128, 256, 512, 1024,
      2048, 4096, 8192, 16384,
      32768, 65536, 131072, 393216}};

// Function prototypes.
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
BOOL DisplayTimebaseMenu(HWND, WPARAM, LPARAM);
BOOL CALLBACK EnumChildProc(HWND, LPARAM);
BOOL ScopeClicked(WPARAM, LPARAM);
BOOL XScaleClicked(WPARAM, LPARAM);
BOOL YScaleClicked(WPARAM, LPARAM);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawXScale(HDC, RECT);
BOOL DrawYScale(HDC, RECT);
BOOL DrawScope(HDC, RECT);
BOOL AddToolbarBitmap(HWND, LPCTSTR);
BOOL AddToolbarButtons(HWND);
BOOL WindowResizing(HWND, WPARAM, LPARAM);
DWORD WINAPI AudioThread(LPVOID);
VOID WaveInData(WPARAM, LPARAM);
VOID KeyDown(WPARAM, LPARAM);
BOOL UpdateStatus(VOID);
VOID GetSavedStatus(VOID);

#endif /* SCOPE_H */
