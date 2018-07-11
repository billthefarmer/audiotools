//
//  Audio.h
//  LMS
//
//  Created by Bill Farmer on 03/07/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
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

#ifndef Audio_h
#define Audio_h

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Accelerate/Accelerate.h>
#import <AudioUnit/AudioUnit.h>
#import <CoreAudio/CoreAudio.h>

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

#define kMin        0.5
#define kScale   1024.0
#define kTimerDelay 0.1

// Audio in values
enum
    {kSampleRate       = 44100,
     kBytesPerPacket   = 4,
     kBytesPerFrame    = 4,
     kChannelsPerFrame = 1};

// Audio processing values
enum
    {kOversample = 4,
     kSamples = 4096,
     kLog2Samples = 12,
     kSamples2 = kSamples / 2,
     kRange = kSamples * 7 / 16,
     kStep = kSamples / kOversample};

// Global data
typedef struct
{
    int length;
    float *data;
} Spectrum;
Spectrum spectrum;

typedef struct
{
    float frequency;
    float level;
} Display;
Display display;

typedef struct
{
    float level;
} Meter;
Meter meter;

typedef struct
{
    AudioUnit output;
    float *buffer;
    int frames;
    float sample;
} Audio;
Audio audio;

DisplayView *displayView;
MeterView *meterView;
SpectrumView *spectrumView;

OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
                   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
                   UInt32 inNumberFrames, AudioBufferList *ioData);
void (^ProcessAudio)();
char *AudioUnitErrString(OSStatus);

#endif /* Audio_h */
