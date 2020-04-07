//
//  Audio.m
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

    UInt32 frames = kStep;

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
    static Float32 buffer[kSamples];

    if (audio.buffer == nil)
	audio.buffer = buffer;

    // Render data
    OSStatus status
	= AudioUnitRender(*(AudioUnit *)inRefCon, ioActionFlags,
			  inTimeStamp, inBusNumber,
			  inNumberFrames, &abl);
    if (status != noErr)
	return status;

    // Copy the input data
    memmove(buffer, buffer + inNumberFrames,
	    (kSamples - inNumberFrames) * sizeof(float));

    Float32 *data = abl.mBuffers[0].mData;

    memmove(buffer + kSamples - inNumberFrames, data,
	    inNumberFrames * sizeof(float));

    // Run in main queue
    dispatch_async(dispatch_get_main_queue(), ProcessAudio);

    return noErr;
}

// Process audio
void (^ProcessAudio)() = ^
{
    // Arrays for processing input
    static float xa[kRange];
    static float xp[kRange];
    static float xq[kRange];
    static float xf[kRange];

    static float dxa[kRange];
    static float dxp[kRange];

    static float window[kSamples];
    static float input[kSamples];

    static float re[kSamples2];
    static float im[kSamples2];

    static DSPSplitComplex x =
	{re, im};

    static FFTSetup setup;

    static float fps;
    static float expect;

    // Initialise structures
    if (spectrum.data == nil)
    {
	spectrum.data = xa;
	spectrum.length = kRange;

	fps = audio.sample / (float)kSamples;
	expect = 2.0 * M_PI * (float)kStep / (float)kSamples;

	// Init Hamming window
	vDSP_hamm_window(window, kSamples, 0);

	// Init FFT
	setup = vDSP_create_fftsetup(kLog2Samples, kFFTRadix2);
    }

    // Get RMS
    float level;

    vDSP_rmsqv(audio.buffer, 1, &level, kSamples);

    // Maximum data value
    static float dmax;

    if (dmax < 0.125)
	dmax = 0.125;

    // Calculate normalising value
    float norm = dmax;

    // Get max magitude
    vDSP_maxmgv(audio.buffer, 1, &dmax, kSamples);

    // Divide by normalisation
    vDSP_vsdiv(audio.buffer, 1, &norm, input, 1, kSamples);

    // Multiply by window
    vDSP_vmul(input, 1, window, 1, input, 1, kSamples);

    // Copy input to split complex vector
    vDSP_ctoz((COMPLEX *)input, 2, &x, 1, kSamples2);

    // Do FFT
    vDSP_fft_zrip(setup, &x, 1, kLog2Samples, kFFTDirection_Forward);

    // Zero the zeroth part
    x.realp[0] = 0.0;
    x.imagp[0] = 0.0;

    // Scale the output
    float scale = kScale;

    vDSP_vsdiv(x.realp, 1, &scale, x.realp, 1, kSamples2);
    vDSP_vsdiv(x.imagp, 1, &scale, x.imagp, 1, kSamples2);

    // Magnitude
    vDSP_vdist(x.realp, 1, x.imagp, 1, xa, 1, kRange);

    // Phase
    vDSP_zvphas(&x, 1, xq, 1, kRange);

    // Phase difference
    vDSP_vsub(xp, 1, xq, 1, dxp, 1, kRange);

    for (int i = 1; i < kRange; i++)
    {
	// Do frequency calculation
	float dp = dxp[i];

	// Calculate phase difference
	dp -= (float)i * expect;

	int qpd = dp / M_PI;

	if (qpd >= 0)
	    qpd += qpd & 1;

	else
	    qpd -= qpd & 1;

	dp -=  M_PI * (float)qpd;

	// Calculate frequency difference
	float df = kOversample * dp / (2.0 * M_PI);

	// Calculate actual frequency from slot frequency plus
	// frequency difference
	xf[i] = i * fps + df * fps;

	// Calculate differences for finding maxima
	dxa[i] = xa[i] - xa[i - 1];
    }

    // Copy phase vector
    memmove(xp, xq, kRange * sizeof(float));

    // Maximum FFT output
    float  max;
    vDSP_Length imax;

    vDSP_maxmgvi(xa, 1, &max, &imax, kRange);

    float f = xf[imax];
    float dB = log10f(level * 3.0) * 20.0;

    if (dB < -80.0)
	dB = -80.0;

    static long n;
    static long m;

    // Update display
    if ((m % 16) == 0)
    {
        if (max > kMin)
        {
            displayView.frequency = f;
            spectrumView.slot = f / fps;
            n = 0;
        }
        displayView.level = dB;
    }

    if ((m % 4) == 0)
    {
        spectrumView.needsDisplay = true;
        meterView.level = level * 3.0 / powf(10.0, 0.15);
    }

    if (n == 64)
    {
        displayView.frequency = 0.0;
        spectrumView.slot = 0.0;
    }

    n++;
    m++;
};

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
