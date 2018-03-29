////////////////////////////////////////////////////////////////////////////////
//
//  SigGen - An audio signal generator written in C.
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

#include <Carbon/Carbon.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

// Audio out values
enum
    {kSampleRate       = 44100,
     kSamples          = 4096,
     kMaxLevel         = 1,
     kBytesPerPacket   = 4,
     kBytesPerFrame    = 4,
     kChannelsPerFrame = 1};

// Frequency scale
enum
    {kFrequencyScale = 250,
     kFrequencyMax   = 850,
     kFrequencyMin   = 0};

// Fine slider
enum
    {kFineMax  = 100,
     kFineRef  = 50,
     kFineMin  = 0};

// Level slider
enum
    {kLevelMax  = 100,
     kLevelRef  = 20,
     kLevelMin  = 0};

// Waveform
enum
    {kSine,
     kSquare,
     kSawtooth};

// Command IDs
enum
    {kCommandFrequency = 'Freq',
     kCommandFine      = 'Fine',
     kCommandLevel     = 'Levl',
     kCommandSine      = 'Sine',
     kCommandSquare    = 'Squa',
     kCommandSawtooth  = 'Sawt',
     kCommandMute      = 'Mute'};

// Keycodes
enum
    {kKeyboardUpKey    = 0x7e,
     kKeyboardDownKey  = 0x7d,
     kKeyboardLeftKey  = 0x7b,
     kKeyboardRightKey = 0x7c,
     kKeyboardPriorKey = 0x74,
     kKeyboardNextKey  = 0x79};

// Global data
typedef struct
{
    HIViewRef view;
    float value;
} Scale;
Scale scale =
    {NULL, kFrequencyScale * 2.0};

typedef struct
{
    HIViewRef view;
    float frequency;
    float decibels;
} Display;
Display display =
    {NULL, 1000.0, -20.0};

typedef struct
{
    HIViewRef fine;
    HIViewRef level;
} Sliders;
Sliders sliders;

typedef struct
{
    HIViewRef sine;
    HIViewRef square;
    HIViewRef sawtooth;
} Buttons;
Buttons buttons;

typedef struct
{
    AudioUnit output;
    Boolean mute;
    Boolean flag;
    int waveform;
    float level;
    float rate;
} Audio;
Audio audio;

// Function prototypes.
OSStatus ScaleDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus DisplayDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus KnobDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus WindowEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus CommandEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus MouseEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus KeyboardEventHandler(EventHandlerCallRef, EventRef, void *);

OSStatus SetupAudio(void);
OSStatus DisplayAlert(CFStringRef, CFStringRef);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);

OSStatus ChangeFrequency(UInt32);
OSStatus CentreTextAtPoint(CGContextRef, float, float, const char *, size_t);
OSStatus ChangeLevel(UInt32);

OSStatus UpdateFrequency(void);
OSStatus UpdateLevel(void);

OSStatus StrokeRoundRect(CGContextRef, CGRect, float);
HIRect DrawEdge(CGContextRef, HIRect);

void FineActionProc(HIViewRef, ControlPartCode);
void LevelActionProc(HIViewRef, ControlPartCode);
