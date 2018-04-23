//
//  Audio.m
//  Oscilloscope
//
//  Created by Bill Farmer on 08/04/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//

#import "Audio.h"

// Setup audio
OSStatus SetupAudio()
{
    // Specify an output unit
    ComponentDescription dc =
        {kAudioUnitType_Output,
         kAudioUnitSubType_HALOutput,
         kAudioUnitManufacturer_Apple,
         0, 0};

    // Find an output unit
    Component cp
        = FindNextComponent(NULL, &dc);

    if (cp == NULL)
    {
        DisplayAlert(CFSTR("FindNextComponent"),
                     CFSTR("Can't find an output audio unit"));
        return -1;
    }

    // Open it
    OSStatus status = OpenAComponent(cp, &audio.output);

    if (status != noErr)
    {
        DisplayAlert(CFSTR("OpenAComponent"),
                     CFSTR("Can't open an output audio unit"));
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
        DisplayAlert(CFSTR("AudioUnitSetProperty"),
                     CFSTR("Can't set an output audio unit property"));
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
        DisplayAlert(CFSTR("AudioUnitSetProperty"),
                     CFSTR("Can't set an output audio unit property"));
        return status;
    }

    AudioDeviceID id;
    size = sizeof(id);

    // Get the default input device

    status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
                                      &size, &id);
    if (status != noErr)
    {
        DisplayAlert(CFSTR("AudioHardwareGetProperty"),
                     CFSTR("Can't get the default input device"));
        return status;
    }

    // Set the audio unit device

    status = AudioUnitSetProperty(audio.output,
                                  kAudioOutputUnitProperty_CurrentDevice,
                                  kAudioUnitScope_Global, 0, &id, sizeof(id));
    if (status != noErr)
    {
        DisplayAlert(CFSTR("AudioUnitSetProperty"),
                     CFSTR("Can't set output audio unit current device"));
        return status;
    }

    Float64 rate = kSampleRate;
    size = sizeof(rate);

    // Set the sample rate, will probably fail

    status = AudioDeviceSetProperty(id, NULL, 0, true,
                                    kAudioDevicePropertyNominalSampleRate,
                                    sizeof(rate), &rate);
    // Get the sample rate

    status = AudioDeviceGetProperty(id, 0, true,
                                    kAudioDevicePropertyNominalSampleRate,
                                    &size, &rate);
    if (status != noErr)
    {
        DisplayAlert(CFSTR("AudioDeviceGetProperty"),
                     CFSTR("Can't get audio device nominal sample rate"));
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
        DisplayAlert(CFSTR("AudioUnitSetProperty"),
                     CFSTR("Can't set output audio unit maximum frames"));
        return status;
    }

    // Set the buffer size

    status = AudioUnitSetProperty(audio.output,
                                  kAudioDevicePropertyBufferFrameSize,
                                  kAudioUnitScope_Global, 0,
                                  &frames, sizeof(frames));
    if (status != noErr)
    {
        DisplayAlert(CFSTR("AudioUnitSetProperty"),
                     CFSTR("Can't set output audio unit buffer size"));
        return status;
    }

    // Get the frames

    status = AudioUnitGetProperty(audio.output,
                                  kAudioUnitProperty_MaximumFramesPerSlice,
                                  kAudioUnitScope_Global, 0, &frames, &size);
    if (status != noErr)
    {
        DisplayAlert(CFSTR("AudioUnitGetProperty"),
                     CFSTR("Can't get output audio unit maximum frames"));
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
        DisplayAlert(CFSTR("AudioUnitGetProperty"),
                     CFSTR("Can't get output audio unit stream format"));
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
        DisplayAlert(CFSTR("AudioUnitSetProperty"),
                     CFSTR("Can't set output audio unit stream format"));
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
        DisplayAlert(CFSTR("AudioUnitSetProperty"),
                     CFSTR("Can't set output audio unit input callback"));
        return status;
    }

    // Start the audio unit

    status = AudioUnitInitialize(audio.output);

    if (status != noErr)
    {
        DisplayAlert(CFSTR("AudioUnitInitialize"),
                     CFSTR("Can't initialise output audio unit"));
        return status;
    }

    AudioOutputUnitStart(audio.output);

    if (status != noErr)
    {
        DisplayAlert(CFSTR("AudioOutputUnitStart"),
                     CFSTR("Can't start output audio unit"));
        return status;
    }

    return status;
}

// Input proc

OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
                   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
                   UInt32 inNumberFrames, AudioBufferList *ioData)
{
    static AudioBufferList abl =
        {1, {1, 0, NULL}};

    // Initialise data structs

    if (audio.ablp == NULL)
        audio.ablp = &abl;

    // Render data

    OSStatus status
        = AudioUnitRender(*(AudioUnit *)inRefCon, ioActionFlags,
                          inTimeStamp, inBusNumber,
                          inNumberFrames, &abl);
    if (status != noErr)
        return status;

    // Create an event to post to the main event queue

    return noErr;
}

// Audio event handler

OSStatus AudioEventHandler(/*EventHandlerCallRef next,
                             EventRef event, void *v*/)
{
    static int count;
    static int index;
    static int state;
    static float last;

    // Create buffers for processing the audio data

    static float buffer[kSamples];

    // Initialise data structs

    if (scope.data == NULL)
    {
        scope.data = buffer;
        scope.length = timebase.counts[timebase.index];
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
        
        count = timebase.counts[timebase.index];
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

    // Check timebase

    if (scope.scale != timebase.values[timebase.index])
    {
        scope.scale = timebase.values[timebase.index];
        xscale.scale = timebase.values[timebase.index];
        xscale.step = 500 * xscale.scale;
    
        // Update display
    
        // HIViewSetNeedsDisplay(xscale.view, true);
    }

    // Update display

    // HIViewSetNeedsDisplay(scope.view, true);

    return noErr;
}

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
