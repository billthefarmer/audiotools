//
//  Audio.m
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

#include "Audio.h"

OSStatus SetupAudio()
{
    // Specify an output unit
    AudioComponentDescription dc =
	{kAudioUnitType_Output,
        kAudioUnitSubType_DefaultOutput,
        kAudioUnitManufacturer_Apple,
        0, 0};

    // Find an output unit
    AudioComponent cp
	= AudioComponentFindNext(nil, &dc);

    if (cp == nil)
    {
        // AudioComponentFindNext
        NSLog(@"Error in AudioComponentFindNext");
	return -1;
    }

    // Open it
    OSStatus status = AudioComponentInstanceNew(cp, &audio.output);
    if (status != noErr)
    {
        // AudioComponentInstanceNew
        NSLog(@"Error in AudioComponentInstanceNew %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    UInt32 frames = kSamples;
    UInt32 size = sizeof(frames);

    // Set the max frames
    status = AudioUnitSetProperty(audio.output,
				  kAudioUnitProperty_MaximumFramesPerSlice,
				  kAudioUnitScope_Global, 0,
				  &frames, sizeof(frames));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioUnitProperty_MaximumFramesPerSlice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Set the buffer size
    status = AudioUnitSetProperty(audio.output,
				  kAudioDevicePropertyBufferFrameSize,
				  kAudioUnitScope_Global, 0,
				  &frames, sizeof(frames));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioDevicePropertyBufferFrameSize %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    AudioStreamBasicDescription format;
    size = sizeof(format);

    // Get stream format
    status = AudioUnitGetProperty(audio.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Input, 0,
				  &format, &size);
    if (status != noErr)
    {
        // AudioUnitGetProperty
        NSLog(@"Error in AudioUnitGetProperty: " 
                    "kAudioUnitProperty_StreamFormat %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    format.mBytesPerPacket = kBytesPerPacket;
    format.mBytesPerFrame = kBytesPerFrame;
    format.mChannelsPerFrame = kChannelsPerFrame;

    // Set stream format
    status = AudioUnitSetProperty(audio.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Input, 0,
				  &format, sizeof(format));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioUnitProperty_StreamFormat %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Save sample rate
    audio.rate = format.mSampleRate;

    AURenderCallbackStruct input =
	{InputProc, &audio.output};

    // Set callback
    status = AudioUnitSetProperty(audio.output,
    				  kAudioUnitProperty_SetRenderCallback,
    				  kAudioUnitScope_Input, 0,
    				  &input, sizeof(input));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioUnitProperty_SetInputCallback %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Init the audio unit
    status = AudioUnitInitialize(audio.output);

    if (status != noErr)
    {
        // AudioUnitInitialize
        NSLog(@"Error in AudioUnitInitialize %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Start the audio unit
    AudioOutputUnitStart(audio.output);

    if (status != noErr)
    {
        // AudioOutputUnitStart
        NSLog(@"Error in AudioOutputUnitStart %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    return noErr;
}

// ShutdownAudio
OSStatus ShutdownAudio()
{
    AudioOutputUnitStop(audio.output);
    AudioUnitUninitialize(audio.output);

    return noErr;
}

// Input proc
OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
		   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
		   UInt32 inNumberFrames, AudioBufferList *ioData)
{
    static double K;
    static double q;
    static double f;
    static double l;

    // Initialise static variables
    if (K == 0)
    {
	K = 2.0 * M_PI / audio.rate;
	f = audio.frequency;
	audio.level = (float)kMaxLevel * 0.2;
    }

    float *buffer = ioData->mBuffers[0].mData;

    // Fill buffer
    for (int i = 0; i < inNumberFrames; i++)
    {
	// Track frequency and level adjustments
	f += ((audio.frequency - f) / (double)kSamples);
	l += audio.mute? -l / (double)kSamples: 
	    (audio.level - l) / (double)kSamples;

	// Advance phase
	q += (q < M_PI)? f * K: (f * K) - (2.0 * M_PI);

	// Waveform
	switch (audio.waveform)
	{
	    // Sine
	case kSine:
	    buffer[i] = sin(q) * l;
	    break;

	    // Square
	case kSquare:
	    buffer[i] = (q > 0.0)? l: -l;
	    break;

	    // Sawtooth
	case kSawtooth:
	    buffer[i] = (q / M_PI) * l;
	    break;
	}
    }

    return noErr;
}

// setVertical
void setVertical(NSSlider *slider, bool value)
{
    slider.vertical = value;
}

// AudioUnitErrString
char *AudioUnitErrString(OSStatus status)
{
    static UInt32 audioUnitErrCodes[] =
        {kAudioUnitErr_CannotDoInCurrentContext,
         kAudioUnitErr_FailedInitialization,
         kAudioUnitErr_FileNotSpecified,
         kAudioUnitErr_FormatNotSupported,
         kAudioUnitErr_Initialized,
         kAudioUnitErr_InvalidElement,
         kAudioUnitErr_InvalidFile,
         kAudioUnitErr_InvalidOfflineRender,
         kAudioUnitErr_InvalidParameter,
         kAudioUnitErr_InvalidProperty,
         kAudioUnitErr_InvalidPropertyValue,
         kAudioUnitErr_InvalidScope,
         kAudioUnitErr_NoConnection,
         kAudioUnitErr_PropertyNotInUse,
         kAudioUnitErr_PropertyNotWritable,
         kAudioUnitErr_TooManyFramesToProcess,
         kAudioUnitErr_Unauthorized,
         kAudioUnitErr_Uninitialized,
         kAudioUnitErr_UnknownFileType,
         kAudioUnitErr_RenderTimeout};

    static char *audioUnitErrStrings[] =
        {"AudioUnitErr_CannotDoInCurrentContext",
         "AudioUnitErr_FailedInitialization",
         "AudioUnitErr_FileNotSpecified",
         "AudioUnitErr_FormatNotSupported",
         "AudioUnitErr_Initialized",
         "AudioUnitErr_InvalidElement",
         "AudioUnitErr_InvalidFile",
         "AudioUnitErr_InvalidOfflineRender",
         "AudioUnitErr_InvalidParameter",
         "AudioUnitErr_InvalidProperty",
         "AudioUnitErr_InvalidPropertyValue",
         "AudioUnitErr_InvalidScope",
         "AudioUnitErr_NoConnection",
         "AudioUnitErr_PropertyNotInUse",
         "AudioUnitErr_PropertyNotWritable",
         "AudioUnitErr_TooManyFramesToProcess",
         "AudioUnitErr_Unauthorized",
         "AudioUnitErr_Uninitialized",
         "AudioUnitErr_UnknownFileType",
         "AudioUnitErr_RenderTimeout"};

    for (int i = 0; i < sizeof(audioUnitErrCodes) / sizeof(UInt32); i++)
        if (audioUnitErrCodes[i] == status)
            return audioUnitErrStrings[i];

    return "UnknownErr";
}
