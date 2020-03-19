//
//  Audio.m
//  Oscilloscope
//
//  Created by Bill Farmer on 08/04/2018.
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

#import "Audio.h"

// Setup audio
OSStatus SetupAudio()
{
    // Specify an output unit
    AudioComponentDescription dc =
        {kAudioUnitType_Output,
         kAudioUnitSubType_HALOutput,
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

    UInt32 enable;
    UInt32 size;

    // Enable input
    enable = true;
    status = AudioUnitSetProperty(audio.output,
                                  kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Input,
                                  1, &enable, sizeof(enable));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioOutputUnitProperty_EnableIO %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Disable output
    enable = false;
    status = AudioUnitSetProperty(audio.output,
                                  kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Output,
                                  0, &enable, sizeof(enable));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioOutputUnitProperty_EnableIO %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    AudioDeviceID id;
    size = sizeof(id);

    // Get the default input device
    AudioObjectPropertyAddress inputDeviceAOPA =
        {kAudioHardwarePropertyDefaultInputDevice,
         kAudioObjectPropertyScopeGlobal,
         kAudioObjectPropertyElementMaster};

    // Get device
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                        &inputDeviceAOPA,
                                        0, nil, &size, &id);
    if (status != noErr)
    {
        // AudioObjectGetPropertyData
        NSLog(@"Error in AudioObjectGetPropertyData: " 
                    "kAudioHardwarePropertyDefaultInputDevice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Set the audio unit device
    status = AudioUnitSetProperty(audio.output,
                                  kAudioOutputUnitProperty_CurrentDevice,
                                  kAudioUnitScope_Global, 0, &id, sizeof(id));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioOutputUnitProperty_CurrentDevice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    Float64 rate = kSampleRate;
    size = sizeof(rate);

    // Get nominal sample rates
    AudioObjectPropertyAddress audioDeviceAOPA =
        {kAudioDevicePropertyNominalSampleRate,
         kAudioObjectPropertyScopeGlobal,
         kAudioObjectPropertyElementMaster};

    // Set the sample rate, will probably fail
    status = AudioObjectSetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                        size, &rate);
    // Get the sample rate
    status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                        &size, &rate);
    if (status != noErr)
    {
        // AudioObjectGetPropertyData
        NSLog(@"Error in AudioObjectGetPropertyData: " 
                    "kAudioDevicePropertyNominalSampleRate %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Set the rate
    audio.sample = rate;

    UInt32 frames = kFrames;

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

    // Get the frames
    status = AudioUnitGetProperty(audio.output,
                                  kAudioUnitProperty_MaximumFramesPerSlice,
                                  kAudioUnitScope_Global, 0, &frames, &size);
    if (status != noErr)
    {
        // AudioUnitGetProperty
        NSLog(@"Error in AudioUnitGetProperty: " 
                    "kAudioUnitProperty_MaximumFramesPerSlice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    audio.frames = frames;

    AudioStreamBasicDescription format;
    size = sizeof(format);

    // Get stream format
    status = AudioUnitGetProperty(audio.output,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input, 1,
                                  &format, &size);
    if (status != noErr)
    {
        // AudioUnitGetProperty
        NSLog(@"Error in AudioUnitGetProperty: " 
                    "kAudioUnitProperty_StreamFormat %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    format.mSampleRate = rate;
    format.mBytesPerPacket = kBytesPerPacket;
    format.mBytesPerFrame = kBytesPerFrame;
    format.mChannelsPerFrame = kChannelsPerFrame;

    // Set stream format
    status = AudioUnitSetProperty(audio.output,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Output, 1,
                                  &format, sizeof(format));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioUnitProperty_StreamFormat %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    AURenderCallbackStruct input =
        {InputProc, &audio.output};

    // Set callback
    status = AudioUnitSetProperty(audio.output,
                                  kAudioOutputUnitProperty_SetInputCallback,
                                  kAudioUnitScope_Global, 0,
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

    return status;
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
    static AudioBufferList abl =
        {1, {1, 0, nil}};

    // Initialise data structs
    if (audio.ablp == nil)
        audio.ablp = &abl;

    // Render data
    OSStatus status
        = AudioUnitRender(*(AudioUnit *)inRefCon, ioActionFlags,
                          inTimeStamp, inBusNumber,
                          inNumberFrames, &abl);
    if (status != noErr)
        return status;

    // Run in main queue
    dispatch_async(dispatch_get_main_queue(), ProcessAudio);

    return noErr;
}

// Process audio
void (^ProcessAudio)() = ^
{
    static int count;
    static int index;
    static int state;
    static float last;

    // Create buffers for processing the audio data
    static float buffer[kSamples];

    // Initialise data structs
    if (scope.data == nil)
    {
        scope.data = buffer;
        scope.length = scope.count;
    }

    // Copy the input data
    float *data = audio.ablp->mBuffers[0].mData;

    // State machine for sync and copying data to display buffer
    switch (state)
    {
        // INIT: waiting for sync
    case INIT:

        index = 0;

        if (scope.bright)
            state++;

        else
        {
            if (scope.single && !scope.trigger)
                break;

            // Calculate sync level
            float level = -yscale.index * scope.yscale;

            // Initialise sync
            float dx = 0.0;

            // Sync polarity
            if (level < 0.0)
            {
                for (int i = 0; i < audio.frames; i++)
                {
                    dx = data[i] - last;

                    if (dx < 0.0 && last > level && data[i] < level)
                    {
                        index = i;
                        state++;
                        break;
                    }

                    last = data[i];
                }
            }

            else
            {
                for (int i = 0; i < audio.frames; i++)
                {
                    dx = data[i] - last;

                    if (dx > 0.0 && last < level && data[i] > level)
                    {
                        index = i;
                        state++;
                        break;
                    }

                    last = data[i];
                }
            }
        }

        // No sync, try next time
        if (state == 0)
            break;

        // Reset trigger
        if (scope.single && scope.trigger)
            scope.trigger = false;

        // FIRST: First chunk of data
    case FIRST:

        // Update count
        count = scope.count;
        scope.length = count;

        // Copy data
        memmove(buffer, data + index, (audio.frames - index) * sizeof(float));
        index = audio.frames - index;

        // If done, wait for sync again
        if (index >= count)
            state = INIT;

        else
    
            // Else get some more data next time
            state++;
        break;

        // NEXT: Subsequent chunks of data
    case NEXT:

        // Copy data
        memmove(buffer + index, data, audio.frames * sizeof(float));
        index += audio.frames;

        // Done, wait for sync again
        if (index >= count)
            state = INIT;

        // Else if last but one chunk, get last chunk next time
        else if (index + audio.frames >= count)
            state++;
        break;

        // LAST: Last chunk of data
    case LAST:

        // Copy data
        memmove(buffer + index, data, (count - index) * sizeof(float));

        // Wait for sync next time
        state = INIT;
        break;
    }

    // Update display
    scopeView.needsDisplay = true;
};

OSStatus DisplayAlert(CFStringRef error, CFStringRef explanation)
{
    return noErr;
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
