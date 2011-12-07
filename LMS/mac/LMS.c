////////////////////////////////////////////////////////////////////////////////
//
//  LMS - A Level Measuring Set written in C.
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
//  Bill Farmer  william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#include <Carbon/Carbon.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <Accelerate/Accelerate.h>

// Macros

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define kMin        0.5
#define kScale   1024.0
#define kTimerDelay 0.1

// Slider values

enum
    {kMeterMax   = 200,
     kMeterValue = 0,
     kMeterMin   = 0};

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

// Audio event constants

enum
    {kEventAudioUpdate = 'Updt'};

// Global data

typedef struct
{
    HIViewRef view;
    int length;
    float *data;
} Spectrum;

Spectrum spectrum;

typedef struct
{
    HIViewRef view;
    float f;
    float l;
} Display;

Display display;

typedef struct
{
    HIViewRef view;
    HIViewRef slider; 
    EventLoopTimerRef timer;
    float l;
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

// Function prototypes.

OSStatus SpectrumDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus DisplayDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus MeterDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus WindowEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus CommandEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus AudioEventHandler(EventHandlerCallRef, EventRef, void *);

OSStatus SetupAudio(void);
OSStatus DisplayAlert(CFStringRef, CFStringRef);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);

OSStatus PostCommandEvent(HIViewRef);

OSStatus StrokeRoundRect(CGContextRef, CGRect, float);
OSStatus CentreTextAtPoint(CGContextRef, float, float, const char *, size_t);
HIRect DrawEdge(CGContextRef, HIRect);

void TimerProc(EventLoopTimerRef, void *);

// Function main

int main(int argc, char *argv[])
{
    WindowRef window;
    HIViewRef content;

    MenuRef menu;

    // Window bounds

    Rect bounds = {0, 0, 206, 320};

    // Create window

    CreateNewWindow(kDocumentWindowClass,
		    kWindowStandardFloatingAttributes |
		    kWindowFrameworkScaledAttribute |
		    kWindowStandardHandlerAttribute |
		    kWindowCompositingAttribute,
		    &bounds, &window);

    // Set the title

    SetWindowTitleWithCFString(window, CFSTR("Level Measuring Set"));

    // Create an application menu

    CreateNewMenu(0, 0, &menu);

    // Set menu title

    CFStringRef apple =
	CFStringCreateWithPascalString(kCFAllocatorDefault,
				       "\p\024",
				       kCFStringEncodingMacRoman);

    SetMenuTitleWithCFString(menu, apple);
    CFRelease(apple);

    // Create an about item

    AppendMenuItemTextWithCFString(menu, CFSTR("About Level Measuring Set"),
                                   0, kHICommandAbout, NULL);

    // Insert the menu

    InsertMenu(menu, 0);
    ReleaseMenu(menu);

    // Create a standard window menu

    CreateStandardWindowMenu(0, &menu);

    // Insert the menu

    InsertMenu(menu, 0);
    ReleaseMenu(menu);

    // Show and position the window

    ShowWindow(window);
    RepositionWindow(window, NULL, kWindowAlertPositionOnMainScreen);

    // Find the window content

    HIViewFindByID(HIViewGetRoot(window),
                   kHIViewWindowContentID,
                   &content);

    // Bounds of display

    bounds.bottom = 50;
    bounds.right  = 280;

    // Create display pane

    CreateUserPaneControl(window, &bounds, 0, &display.view);

    // Set help tag

    HMHelpContentRec help =
	{kMacHelpVersion,
	 {0, 0, 0, 0},
	 kHMInsideLeftCenterAligned,
	 {{kHMCFStringContent,
	   CFSTR("Display")},
	  {kHMNoContent, NULL}}};

    HMSetControlHelpContent(display.view, &help);

    // Place in the window

    HIViewAddSubview(content, display.view);
    HIViewPlaceInSuperviewAt(display.view, 20, 20);

    // Bounds of meter

    bounds.bottom = 50;
    bounds.right  = 280;

    // Create meter pane

    CreateUserPaneControl(window, &bounds,
			  kHIViewFeatureAllowsSubviews, &meter.view);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Level");
    HMSetControlHelpContent(meter.view, &help);

    // Place in the window

    HIViewAddSubview(content, meter.view);
    HIViewPlaceInSuperviewAt(meter.view, 20, 78);

    // Bounds of slider

    bounds.bottom = 16;
    bounds.right  = 264;

    // Create meter slider

    CreateSliderControl(window, &bounds, kMeterValue, kMeterMin, kMeterMax,
                        kControlSliderPointsUpOrLeft, 0, false,
			NULL, &meter.slider);
    // Control size

    ControlSize small = kControlSizeSmall;

    // Set control size

    SetControlData(meter.slider, kControlEntireControl, kControlSizeTag,
		   sizeof(ControlSize), &small);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Level");
    HMSetControlHelpContent(meter.slider, &help);

    // Place in the pane

    HIViewAddSubview(meter.view, meter.slider);
    HIViewPlaceInSuperviewAt(meter.slider, 8, 28);

    // Bounds of spectrum

    bounds.bottom = 50;
    bounds.right  = 280;

    // Create spectrum pane

    CreateUserPaneControl(window, &bounds, 0, &spectrum.view);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Spectrum");
    HMSetControlHelpContent(spectrum.view, &help);

    // Place in the window

    HIViewAddSubview(content, spectrum.view);
    HIViewPlaceInSuperviewAt(spectrum.view, 20, 136);

    // Window events type spec

    EventTypeSpec windowEvents[] =
        {{kEventClassWindow, kEventWindowClose},
	 {kEventClassWindow, kEventWindowZoomed}};

    // Install event handler

    InstallWindowEventHandler(window, NewEventHandlerUPP(WindowEventHandler),
                              LENGTH(windowEvents), windowEvents,
                              NULL, NULL);

    // Audio events type spec

    EventTypeSpec audioEvents[] =
        {{kEventClassApplication, kEventAudioUpdate}};

    // Install event handler

    InstallApplicationEventHandler(NewEventHandlerUPP(AudioEventHandler),
                                   LENGTH(audioEvents), audioEvents,
                                   window, NULL);
    // Draw events type spec

    EventTypeSpec drawEvents[] =
	{{kEventClassControl, kEventControlDraw}};

    // Install event handlers

    InstallControlEventHandler(spectrum.view,
			       NewEventHandlerUPP(SpectrumDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       spectrum.view, NULL);

    InstallControlEventHandler(display.view,
			       NewEventHandlerUPP(DisplayDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       display.view, NULL);

    InstallControlEventHandler(meter.view,
			       NewEventHandlerUPP(MeterDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       meter.view, NULL);

    // Set up timer

    InstallEventLoopTimer(GetMainEventLoop(), kTimerDelay, kTimerDelay,
			  NewEventLoopTimerUPP(TimerProc),
			  NULL, &meter.timer);
    // Set up audio

    SetupAudio();

    // Run the application event loop

    RunApplicationEventLoop();

    return 0;
}

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

    UInt32 frames = kStep;
    size = sizeof(frames);

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

// Display alert

OSStatus DisplayAlert(CFStringRef error, CFStringRef explanation)
{
    DialogRef dialog;

    CreateStandardAlert(kAlertStopAlert, error, explanation, NULL, &dialog);
    SetWindowTitleWithCFString(GetDialogWindow(dialog), CFSTR("Tuner"));
    RunStandardAlert(dialog, NULL, NULL);

    return noErr;
}

// Input proc

OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
		   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
		   UInt32 inNumberFrames, AudioBufferList *ioData)
{
    static AudioBufferList abl =
	{1, {1, 0, NULL}};

    // Initialise data structs

    static Float32 buffer[kSamples];

    if (audio.buffer == NULL)
	audio.buffer = buffer;

    // Render data

    OSStatus status
	= AudioUnitRender(*(AudioUnit *)inRefCon, ioActionFlags,
			  inTimeStamp, inBusNumber,
			  inNumberFrames, &abl);
    if (status != noErr)
	return status;

    // Copy the input data

    memmove(buffer, buffer + audio.frames,
	    (kSamples - audio.frames) * sizeof(float));

    Float32 *data = abl.mBuffers[0].mData;

    memmove(buffer + kSamples - audio.frames, data,
	    audio.frames * sizeof(float));

    // Create an event to post to the main event queue

    EventRef event;

    CreateEvent(kCFAllocatorDefault, kEventClassApplication,
		kEventAudioUpdate, 0,
		kEventAttributeUserEvent, &event);

    PostEventToQueue(GetMainEventQueue(), event,
		     kEventPriorityHigh);

    ReleaseEvent(event);

    return noErr;
}

// Audio event handler

OSStatus AudioEventHandler(EventHandlerCallRef next,
			   EventRef event, void *data)
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

    if (spectrum.data == NULL)
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
    UInt32 imax;

    vDSP_maxmgvi(xa, 1, &max, &imax, kRange);

    float f = xf[imax];

    static long n1;

    if (max > kMin)
    {
	display.f = f;
	n1 = 0;
    }

    else
    {
	if (n1 == 64)
	    display.f = 0.0;
    }

    n1++;

    float dB = log10f(level * 3.0) * 20.0;

    if (dB < -80.0)
	dB = -80.0;

    display.l = dB;

    meter.l = level * 3.0 / powf(10.0, 0.15);

    static long n2;

    // Update display

    if ((n2 % 4) == 0)
	HIViewSetNeedsDisplay(spectrum.view, true);

    if ((n2 % 16) == 0)
	HIViewSetNeedsDisplay(display.view, true);

    n2++;

    return noErr;
}

// Timer proc

void TimerProc(EventLoopTimerRef timer, void *data)
{
    static float ml;

    // Do meter calculation

    ml = ((ml * 7.0) + meter.l) / 8.0;

    int value = round(ml * kMeterMax) + kMeterValue;

    // Update meter

    HIViewSetValue(meter.slider, value);
}

// Draw edge

HIRect DrawEdge(CGContextRef context, HIRect bounds)
{
    CGContextSetShouldAntialias(context, true);
    CGContextSetLineWidth(context, 3);
    CGContextSetGrayStrokeColor(context, 0.8, 1);

    // Draw edge

    StrokeRoundRect(context, bounds, 7);

    // Create inset

    CGRect inset = CGRectInset(bounds, 2, 2);
    CGContextClipToRect(context, inset);

    return inset;
}

OSStatus StrokeRoundRect(CGContextRef context, CGRect rect, float radius)
{
    CGPoint point = rect.origin;
    CGSize size = rect.size;

    CGContextBeginPath(context);

    CGContextMoveToPoint(context, point.x + radius, point.y);
    CGContextAddLineToPoint(context, point.x + size.width - radius, point.y);
    CGContextAddArcToPoint(context, point.x + size.width, point.y,
			   point.x + size.width, point.y + radius, radius);
    CGContextAddLineToPoint(context, point.x + size.width,
			    point.y + size.height - radius);
    CGContextAddArcToPoint(context, point.x + size.width,
			   point.y + size.height,
			   point.x + size.width - radius,
			   point.y + size.height, radius);
    CGContextAddLineToPoint(context, point.x + radius, point.y + size.height);
    CGContextAddArcToPoint(context, point.x, point.y + size.height,
			   point.x, point.y + size.height - radius, radius);
    CGContextAddLineToPoint(context, point.x, point.y + radius);
    CGContextAddArcToPoint(context, point.x, point.y, point.x + radius,
			   point.x, radius);

    CGContextStrokePath(context);

    return noErr;
}

OSStatus SpectrumDrawEventHandler(EventHandlerCallRef next,
				  EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

    // Get context

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(context), NULL,
		      &context);
    // Get view

    GetEventParameter(event, kEventParamDirectObject,
		      typeControlRef, NULL,
		      sizeof(view), NULL,
		      &view);
    // Get bounds

    HIViewGetBounds(view, &bounds);

    inset = DrawEdge(context, bounds);

    int width = inset.size.width;
    int height = inset.size.height;

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    // Black background

    CGContextSetGrayFillColor(context, 0, 1);
    CGContextFillRect(context, inset);

    CGContextTranslateCTM(context, 2, 3);

    // Dark green graticule

    CGContextSetRGBStrokeColor(context, 0, 0.6, 0, 1);

    // Draw graticule

    CGContextBeginPath(context);

    for (int i = 0; i <= width; i += 6)
    {
	CGContextMoveToPoint(context, i, 0);
	CGContextAddLineToPoint(context, i, height - 1);
    }

    for (int i = 5; i <= height; i += 6)
    {
	CGContextMoveToPoint(context, 0, i);
	CGContextAddLineToPoint(context, width, i);
    }

    CGContextStrokePath(context);

    // Don't attempt the trace until there's a buffer

    if (spectrum.data == NULL)
	return noErr;

    // Move the origin

    CGContextTranslateCTM(context, 0, height - 1);

    static float max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling

    float yscale = (float)height / max;

    max = 0.0;

    // Green pen for spectrum trace

    CGContextSetRGBStrokeColor(context, 0, 1, 0, 1);

    // Draw the spectrum

    CGContextBeginPath(context);
    CGContextMoveToPoint(context, 0, 0);

    float xscale = (float)spectrum.length / (float)width;

    for (int x = 0; x < width; x++)
    {
	float value = 0.0;

	// Don't show DC component

	if (x > 0)
	{
	    for (int j = 0; j < xscale; j++)
	    {
		int n = x * xscale + j;

		if (value < spectrum.data[n])
		    value = spectrum.data[n];
	    }
	}

	if (max < value)
	    max = value;

	float y = -value * yscale;

	CGContextAddLineToPoint(context, x, y);
    }

    CGContextStrokePath(context);

    return noErr;
}

OSStatus DisplayDrawEventHandler(EventHandlerCallRef next,
				 EventRef event, void *data)
{
    enum
    {kTextSize = 24};

    static char s[16];

    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

    // Get context

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(context), NULL,
		      &context);
    // Get view

    GetEventParameter(event, kEventParamDirectObject,
		      typeControlRef, NULL,
		      sizeof(view), NULL,
		      &view);
    // Get bounds

    HIViewGetBounds(view, &bounds);

    inset = DrawEdge(context, bounds);

    int width = inset.size.width;

    CGContextTranslateCTM(context, 2, 4);
    CGContextSetLineWidth(context, 1);

    // Black text

    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetGrayFillColor(context, 0, 1);

    CGContextSetShouldAntialias(context, true);
    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);

    // Select font

    CGContextSelectFont(context, "Arial Bold", kTextSize,
			kCGEncodingMacRoman);

    sprintf(s, "%7.1lfHz    ", display.f);
    CGContextShowTextAtPoint(context, 4, 28, s, strlen(s));

    sprintf(s, "%5.1lfdB  ", display.l);
    CGContextShowTextAtPoint(context, width / 2, 28, s, strlen(s));

    return noErr;
}

OSStatus MeterDrawEventHandler(EventHandlerCallRef next,
			       EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

    // Get context

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(context), NULL,
		      &context);
    // Get view

    GetEventParameter(event, kEventParamDirectObject,
		      typeControlRef, NULL,
		      sizeof(view), NULL,
		      &view);
    // Get bounds

    HIViewGetBounds(view, &bounds);

    inset = DrawEdge(context, bounds);

    int width = inset.size.width;

    CGContextTranslateCTM(context, 2, 3);
    CGContextSetLineWidth(context, 1);

    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetGrayFillColor(context, 0, 1);

    CGContextSetShouldAntialias(context, true);

    // Select font

    CGContextSelectFont(context, "Arial", 14, kCGEncodingMacRoman);
    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);

    // Draw the meter scale

    static int as[] =
	{-40, -20, -14, -10, -8, -7, -6, -5,
	 -4, -3, -2, -1, 0, 1, 2, 3};

    for (int i = 0; i < LENGTH(as); i++)
    {
	static char s[16];

	sprintf(s, "%d", abs(as[i]));

	float x = pow(10.0, (as[i] + 20.0) / 20.0) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;

	x *= (width - 24);

	x += 11;

	CentreTextAtPoint(context, x, 14, s, strlen(s));
    }

    CGContextSetShouldAntialias(context, false);
    CGContextBeginPath(context);

    CGContextMoveToPoint(context, 3, 20);
    CGContextAddLineToPoint(context, 7, 20);

    CGContextMoveToPoint(context, width - 4, 20);
    CGContextAddLineToPoint(context, width - 8, 20);
    CGContextMoveToPoint(context, width - 6, 18);
    CGContextAddLineToPoint(context, width - 6, 22);

    static int at[] =
	{-10, -5, 0, 1, 2, 3, 4, 5,
	 6, 7, 8, 9, 10, 11, 12, 13};

    for (int i = 0; i < LENGTH(at); i++)
    {
	float x = pow(10.0, at[i] / 10.0) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;

	x *= (width - 24);

	x += 11;

	CGContextMoveToPoint(context, x, 16);
	CGContextAddLineToPoint(context, x, 24);
    }

    for (int i = 1; i < 26; i += 2)
    {
	float x = pow(10.0, (i / 20.0)) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;

	x *= (width - 24);

	x += 11;

	CGContextMoveToPoint(context, x, 18);
	CGContextAddLineToPoint(context, x, 24);
    }

    for (int i = 17; i < 46; i += 2)
    {
	float x = pow(10.0, (i / 40.0)) / 10.0;

	x /= pow(10.0, 23.0 / 20.0) / 10.0;

	x *= (width - 24);

	x += 11;

	CGContextMoveToPoint(context, x, 20);
	CGContextAddLineToPoint(context, x, 24);
    }

    CGContextStrokePath(context);

    return noErr;
}

// Centre text at point

OSStatus CentreTextAtPoint(CGContextRef context, float x, float y,
			   const char * bytes, size_t length)
{
    CGContextSetTextDrawingMode(context, kCGTextInvisible);
    CGContextShowTextAtPoint(context, x, y, bytes, length);

    CGPoint point = CGContextGetTextPosition(context);

    float dx = (point.x - x) / 2.0;
    float dy = (point.y - y) / 2.0;

    CGContextSetTextDrawingMode(context, kCGTextFill);
    CGContextShowTextAtPoint(context, x - dx, y - dy, bytes, length);

    return noErr;
}

// Window event handler

OSStatus WindowEventHandler(EventHandlerCallRef next,
			    EventRef event, void *data)
{
    UInt32 kind;

    // Get the event kind

    kind = GetEventKind(event);

    // Switch on event kind

    switch (kind)
    {
        // Window close event

    case kEventWindowClose:

	// Close audio unit

	AudioOutputUnitStop(audio.output);
	AudioUnitUninitialize(audio.output);

	// Flush preferences

	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

        // Quit the application

        QuitApplicationEventLoop();
        break;

    default:
        return eventNotHandledErr;
    }

    // Return ok

    return noErr;
}
