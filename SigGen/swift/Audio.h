//
//  Audio.h
//  SigGen
//
//  Created by Bill Farmer on 29/03/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef Audio_h
#define Audio_h

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <AudioUnit/AudioUnit.h>
#import <CoreAudio/CoreAudio.h>

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

// Waveform
enum
    {kSine     = 'Sine',
     kSquare   = 'Squa',
     kSawtooth = 'Sawt'};

// Tags
enum
    {kTagFreq   = 'Freq',
     kTagFine   = 'Fine',
     kTagLevel  = 'Levl',
     kTagSine   = 'Sine',
     kTagSquare = 'Squa',
     kTagSaw    = 'Sawt',
     kTagMute   = 'Mute'};

// Keycodes
enum
    {kUpKey    = 0x7e,
     kDownKey  = 0x7d,
     kLeftKey  = 0x7b,
     kRightKey = 0x7c,
     kPriorKey = 0x74,
     kNextKey  = 0x79};

// Global data
typedef struct
{
    AudioUnit output;
    double frequency;
    double level;
    float rate;
    bool mute;
    int waveform;
} Audio;
Audio audio;

// Functions
OSStatus SetupAudio(void);
OSStatus ShutdownAudio(void);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);
char *AudioUnitErrString(OSStatus);
void setVertical(NSSlider *, bool);

#endif /* Audio_h */
