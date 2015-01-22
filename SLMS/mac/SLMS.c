////////////////////////////////////////////////////////////////////////////////
//
//  SLMS - A Selective Level Measuring Set written in C.
//
//  Copyright (C) 2011  Bill Farmer
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

#define Length(a) (sizeof(a) / sizeof(a[0]))

#define kMin        0.5
#define kScale    256.0
#define kTimerDelay 0.1

// Frequency scale

enum
    {kFrequencyScale = 250,
     kFrequencyMax   = 850,
     kFrequencyMin   = 0};

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
     kLog2Step = 10,
     kRange = kSamples * 7 / 64,
     kStep = kSamples / kOversample};

// Audio event constants

enum
    {kEventAudioUpdate = 'Updt'};

// Command IDs

enum
    {kCommandFrequency = 'Freq'};

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
} Knob;

Knob knob;

typedef struct
{
    HIViewRef view;
    float frequency;
    int length;
    float *data;
} Spectrum;

Spectrum spectrum;

typedef struct
{
    HIViewRef view;
    float frequency;
    float level;
} Display;

Display display;

typedef struct
{
    HIViewRef view;
    HIViewRef slider; 
    EventLoopTimerRef timer;
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

// Function prototypes.

OSStatus ScaleDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus SpectrumDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus DisplayDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus KnobDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus MeterDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus KeyboardEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus WindowEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus MouseEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus AudioEventHandler(EventHandlerCallRef, EventRef, void *);

OSStatus SetupAudio(void);
OSStatus DisplayAlert(CFStringRef, CFStringRef);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);

OSStatus UpdateFrequency(void);

OSStatus StrokeRoundRect(CGContextRef, CGRect, float);
OSStatus CentreTextAtPoint(CGContextRef, float, float, const char *, size_t);
HIRect DrawEdge(CGContextRef, HIRect);

void TimerProc(EventLoopTimerRef, void *);

// Function main

int main(int argc, char *argv[])
{
    WindowRef window;
    HIViewRef content;
    HIViewRef button;

    MenuRef menu;

    // Window bounds

    Rect bounds = {0, 0, 248, 488};

    // Create window

    CreateNewWindow(kDocumentWindowClass,
		    kWindowStandardFloatingAttributes |
		    kWindowFrameworkScaledAttribute |
		    kWindowStandardHandlerAttribute |
		    kWindowCompositingAttribute,
		    &bounds, &window);

    // Set the title

    SetWindowTitleWithCFString(window, CFSTR("Selective Level Measuring Set"));

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

    AppendMenuItemTextWithCFString(menu,
				   CFSTR("About Selective Level Measuring Set"),
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

    // Bounds of scale

    bounds.bottom = 40;
    bounds.right  = 160;

    // Create scale pane

    CreateUserPaneControl(window, &bounds, 0, &scale.view);

    // Set help tag

    HMHelpContentRec help =
	{kMacHelpVersion,
	 {0, 0, 0, 0},
	 kHMInsideLeftCenterAligned,
	 {{kHMCFStringContent,
	   CFSTR("Frequency scale")},
	  {kHMNoContent, NULL}}};

    HMSetControlHelpContent(scale.view, &help);

    // Place in the window

    HIViewAddSubview(content, scale.view);
    HIViewPlaceInSuperviewAt(scale.view, 20, 20);

    // Bounds of display

    bounds.bottom = 50;
    bounds.right  = 280;

    // Create display pane

    CreateUserPaneControl(window, &bounds, 0, &display.view);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Frequency and level display");
    HMSetControlHelpContent(display.view, &help);

    // Place in the window

    HIViewAddSubview(content, display.view);
    HIViewPlaceInSuperviewAt(display.view, 188, 20);

    // Bounds of knob

    bounds.bottom = 168;
    bounds.right  = 168;

    // Create display pane

    CreateUserPaneControl(window, &bounds, 0, &knob.view);

    // Set command ID

    HIViewSetCommandID(knob.view, kCommandFrequency);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Frequency adjustment knob");
    HMSetControlHelpContent(knob.view, &help);

    // Place in the window

    HIViewAddSubview(content, knob.view);
    HIViewPlaceInSuperviewAt(knob.view, 16, 64);

    // Bounds of meter

    bounds.bottom = 50;
    bounds.right  = 280;

    // Create meter pane

    CreateUserPaneControl(window, &bounds,
			  kHIViewFeatureAllowsSubviews, &meter.view);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Level meter");
    HMSetControlHelpContent(meter.view, &help);

    // Place in the window

    HIViewAddSubview(content, meter.view);
    HIViewPlaceInSuperviewAt(meter.view, 188, 78);

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
	CFSTR("Level meter");
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
    HIViewPlaceInSuperviewAt(spectrum.view, 188, 136);

    // Bounds of button

    bounds.bottom = 20;
    bounds.right  = 72;

    // Create quit button

    CreatePushButtonControl(window, &bounds,  CFSTR("Quit"), &button);

    // Set command ID

    HIViewSetCommandID(button, kHICommandQuit);

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 396, 208);

    // Window events type spec

    EventTypeSpec windowEvents[] =
        {{kEventClassWindow, kEventWindowClose},
	 {kEventClassWindow, kEventWindowZoomed}};

    // Install event handler

    InstallWindowEventHandler(window, NewEventHandlerUPP(WindowEventHandler),
                              Length(windowEvents), windowEvents,
                              NULL, NULL);

    // Audio events type spec

    EventTypeSpec audioEvents[] =
        {{kEventClassApplication, kEventAudioUpdate}};

    // Install event handler

    InstallApplicationEventHandler(NewEventHandlerUPP(AudioEventHandler),
                                   Length(audioEvents), audioEvents,
                                   window, NULL);
    // Draw events type spec

    EventTypeSpec drawEvents[] =
	{{kEventClassControl, kEventControlDraw}};

    // Install event handlers

    InstallControlEventHandler(scale.view,
			       NewEventHandlerUPP(ScaleDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       scale.view, NULL);

    InstallControlEventHandler(knob.view,
			       NewEventHandlerUPP(KnobDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       knob.view, NULL);

    InstallControlEventHandler(spectrum.view,
			       NewEventHandlerUPP(SpectrumDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       spectrum.view, NULL);

    InstallControlEventHandler(display.view,
			       NewEventHandlerUPP(DisplayDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       display.view, NULL);

    InstallControlEventHandler(meter.view,
			       NewEventHandlerUPP(MeterDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       meter.view, NULL);

    // Mouse events type spec

    EventTypeSpec mouseEvents[] =
        {{kEventClassMouse, kEventMouseDragged}};

    // Install event handler

    InstallWindowEventHandler(window, NewEventHandlerUPP(MouseEventHandler),
			      Length(mouseEvents), mouseEvents,
			      window, NULL);

    // Keyboard events type spec

    EventTypeSpec keyboardEvents[] =
        {{kEventClassKeyboard, kEventRawKeyDown},
	 {kEventClassKeyboard, kEventRawKeyRepeat}};

    // Install event handler

    InstallApplicationEventHandler(NewEventHandlerUPP(KeyboardEventHandler),
                                   Length(keyboardEvents), keyboardEvents,
                                   window, NULL);
    // Set up timer

    InstallEventLoopTimer(GetMainEventLoop(), kTimerDelay, kTimerDelay,
			  NewEventLoopTimerUPP(TimerProc),
			  NULL, &meter.timer);
    // Set up audio

    SetupAudio();

    // Update frequency

    UpdateFrequency();

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
    enum
    {kStep2 = kStep / 2};

    // Arrays for processing input

    static float xa[kRange];

    static float window[kStep];
    static float input[kStep];

    static float re[kStep / 2];
    static float im[kStep / 2];

    static DSPSplitComplex x =
	{re, im};

    static FFTSetup setup;

    static float K;

    // Initialise structures

    if (spectrum.data == NULL)
    {
	spectrum.data = xa;
	spectrum.length = kRange;

	K = 2.0 * M_PI / audio.sample;

	// Init Hamming window

	vDSP_hamm_window(window, kStep, 0);

	// Init FFT

	setup = vDSP_create_fftsetup(kLog2Step, kFFTRadix2);
    }

    // Maximum data value

    static float dmax;

    if (dmax < 0.125)
	dmax = 0.125;

    // Calculate normalising value

    float norm = dmax;

    // Get max magitude

    vDSP_maxmgv(audio.buffer, 1, &dmax, kStep);

    // Divide by normalisation

    vDSP_vsdiv(audio.buffer, 1, &norm, input, 1, kStep);

    // Multiply by window

    vDSP_vmul(input, 1, window, 1, input, 1, kStep);

    // Copy input to split complex vector

    vDSP_ctoz((COMPLEX *)input, 2, &x, 1, kStep2);

    // Do FFT

    vDSP_fft_zrip(setup, &x, 1, kLog2Step, kFFTDirection_Forward);

    // Zero the zeroth part

    x.realp[0] = 0.0;
    x.imagp[0] = 0.0;

    // Scale the output

    float scale = kScale;

    vDSP_vsdiv(x.realp, 1, &scale, x.realp, 1, kStep / 2);
    vDSP_vsdiv(x.imagp, 1, &scale, x.imagp, 1, kStep / 2);

    // Magnitude

    vDSP_vdist(x.realp, 1, x.imagp, 1, xa, 1, kRange);

    // Do cross correlation

    float imag = 0.0;
    float real = 0.0;

    for (int i = 0; i < kSamples; i++)
    {
	float window =
	    (0.5 - 0.5 * cosf(2.0 * M_PI * i / kSamples));

	imag += audio.buffer[i] * window * sin(i * display.frequency * K);

	real += audio.buffer[i] * window * cos(i * display.frequency * K);
    }

    float level = hypotf(real, imag);

    level = level / (kSamples / (4.0 * sqrtf(2.0)));

    float dB = log10f(level * 1.5) * 20.0;

    if (dB < -80.0)
	dB = -80.0;

    display.level = dB;

    meter.level = level * 1.5 / powf(10.0, 0.15);

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

    ml = ((ml * 7.0) + meter.level) / 8.0;

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

// Draw scale

OSStatus ScaleDrawEventHandler(EventHandlerCallRef next,
			       EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

    // Text size

    enum
    {kTextSize = 12};

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

    float width = inset.size.width;
    float height = inset.size.height;

    CGContextTranslateCTM(context, 2, 3);

    // Centre the origin

    CGContextTranslateCTM(context,  width / 2, height / 2);

    // Draw scale

    CGContextBeginPath(context);
    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    for (int i = 1; i < 11; i++)
    {
	float x = (kFrequencyScale * log10(i)) - scale.value;

	for (int j = 0; j < 4; j++)
	{
	    CGContextMoveToPoint(context, x, 0);
	    CGContextAddLineToPoint(context, x, height / 2);
	    x += kFrequencyScale;
	}
    }

    for (int i = 3; i < 20; i += 2)
    {
	float x = (kFrequencyScale * log10(i / 2.0)) - scale.value;

	for (int j = 0; j < 4; j++)
	{
	    CGContextMoveToPoint(context, x, 6);
	    CGContextAddLineToPoint(context, x, height / 2);
	    x += kFrequencyScale;
	}
    }

    // Add centre line

    CGContextMoveToPoint(context, 0, -height / 2);
    CGContextAddLineToPoint(context, 0, height / 2);

    CGContextStrokePath(context);

    // Select font

    CGContextSelectFont(context, "Arial Bold", kTextSize,
			kCGEncodingMacRoman);
    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);
    CGContextSetShouldAntialias(context, true);

    int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < Length(a); i++)
    {
    	float x = (kFrequencyScale * log10(a[i])) - scale.value;

    	for (int j = 0; j < 2; j++)
    	{
	    static char s[8];

	    sprintf(s, "%d", a[i]);
	    CentreTextAtPoint(context, x, -6, s, strlen(s));

	    sprintf(s, "%d", a[i] * 10);
	    CentreTextAtPoint(context, x + kFrequencyScale,
				     -6, s, strlen(s));

    	    x += 2 * kFrequencyScale;
    	}
    }

    return noErr;
}

// Draw knob

OSStatus KnobDrawEventHandler(EventHandlerCallRef next,
			      EventRef event, void *data)
{
    CGContextRef context;
    HIViewRef view;
    HIRect bounds;

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

    CGContextSetGrayFillColor(context, 0.9, 1);
    CGContextSetGrayStrokeColor(context, 0.6, 1);

    // Draw filled circle with shadow

    CGSize offset =
	{4, -4};
    CGContextSetShadow(context, offset, 4);

    CGRect inset = CGRectInset(bounds, 4, 4);

    CGContextFillEllipseInRect(context, inset);

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

    float xscale = (float)spectrum.length / (float)width;

    float yscale = (float)height / max;

    max = 0.0;

    // Yellow pen for frequency trace

    CGContextSetRGBStrokeColor(context, 1, 1, 0, 1);

    CGContextBeginPath(context);

    float x = spectrum.frequency / xscale;

    CGContextMoveToPoint(context, x, 0);
    CGContextAddLineToPoint(context, x, -height);
    CGContextStrokePath(context);

    // Green pen for spectrum trace

    CGContextSetRGBStrokeColor(context, 0, 1, 0, 1);

    // Draw the spectrum

    CGContextBeginPath(context);
    CGContextMoveToPoint(context, 0, 0);

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

    sprintf(s, "%7.1lfHz    ", display.frequency);
    CGContextShowTextAtPoint(context, 4, 28, s, strlen(s));

    sprintf(s, "%5.1lfdB  ", display.level);
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

    for (int i = 0; i < Length(as); i++)
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

    for (int i = 0; i < Length(at); i++)
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

        // Quit the application

        QuitApplicationEventLoop();
        break;

    default:
        return eventNotHandledErr;
    }

    // Return ok

    return noErr;
}

// Mouse event handler

OSStatus MouseEventHandler(EventHandlerCallRef next,
			   EventRef event, void *data)
{
    EventMouseButton button;

    // Get button

    GetEventParameter(event, kEventParamMouseButton,
		      typeMouseButton, NULL, sizeof(button),
		      NULL, &button);

    switch (button)
    {
	// Primary button

    case kEventMouseButtonPrimary:
	break;

    default:
	return eventNotHandledErr;
    }

    WindowRef window;
    HIViewRef view;
    UInt32 id;

    // Get window

    GetEventParameter(event, kEventParamWindowRef,
		      typeWindowRef, NULL, sizeof(window),
		      NULL, &window);

    // Get view and id

    HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &view);
    HIViewGetCommandID(view, &id);

    switch (id)
    {
	// Frequency

    case kCommandFrequency:
	break;

    default:
	return eventNotHandledErr;
    }

    HIRect rect;

    // Get bounds

    HIViewGetBounds(view, &rect);

    HIPoint centre =
	{rect.size.width / 2, rect.size.height / 2};

    HIPoint delta;
    HIPoint previous;
    HIPoint location;

    // Get mouse location

    GetEventParameter(event, kEventParamWindowMouseLocation,
		      typeHIPoint, NULL, sizeof(location),
		      NULL, &location);

    // Get delta

    GetEventParameter(event, kEventParamMouseDelta,
		      typeHIPoint, NULL, sizeof(delta),
		      NULL, &delta);

    // Convert point

    HIViewConvertPoint(&location, NULL, view);

    // Calculate previous location

    previous.x = location.x - delta.x;
    previous.y = location.y - delta.y;

    // Previous offset from centre of knob

    float x = previous.x - centre.x;
    float y = previous.y - centre.y;

    // Angle

    float theta = atan2f(x, -y);

    // Current offset from centre

    x = location.x - centre.x;
    y = location.y - centre.y;

    // Change in angle

    float change = atan2f(x, -y) - theta;

    if (change > M_PI)
	change -= 2.0 * M_PI;

    if (change < -M_PI)
	change += 2.0 * M_PI;

    // Change frequency scale

    scale.value += round(change * 100.0 / M_PI);

    if (scale.value < kFrequencyMin)
	scale.value = kFrequencyMin;

    if (scale.value > kFrequencyMax)
	scale.value = kFrequencyMax;

    // Update display

    HIViewSetNeedsDisplay(scale.view, true);
    UpdateFrequency();

    return noErr;
}

// Update frequency

OSStatus UpdateFrequency()
{
    static float fps;

    if (fps == 0.0)
	fps = audio.sample / (float)kStep;

    // Calculate frequency

    float frequency = powf(10.0, (float)scale.value /
			   (float)kFrequencyScale) * 10.0;

    display.frequency = frequency;
    spectrum.frequency = frequency / fps;

    HIViewSetNeedsDisplay(display.view, true);

    return noErr;
}

// Keyboard event handler

OSStatus KeyboardEventHandler(EventHandlerCallRef next,
			      EventRef event, void *data)
{
    UInt32 code;

    // Get key code

    GetEventParameter(event, kEventParamKeyCode, typeUInt32,
		      NULL, sizeof(code), NULL, &code);

    switch (code)
    {
	// Left

    case kKeyboardLeftKey:
	scale.value--;

	if (scale.value < kFrequencyMin)
	    scale.value = kFrequencyMin;

	UpdateFrequency();
	HIViewSetNeedsDisplay(scale.view, true);
	break;

	// Right

    case kKeyboardRightKey:
	scale.value++;

	if (scale.value > kFrequencyMax)
	    scale.value = kFrequencyMax;

	UpdateFrequency();
	HIViewSetNeedsDisplay(scale.view, true);
	break;

    default:
	return eventNotHandledErr;
    }

    return noErr;
}
