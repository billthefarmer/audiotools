////////////////////////////////////////////////////////////////////////////////
//
//  Scope - An audio oscilloscope written in C.
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
// #include <Accelerate/Accelerate.h>

// Macros

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define kToolbarID    CFSTR("com.billthefarmer.toolbar")
#define kItemVolume   CFSTR("com.billthefarmer.toolbar.volume")
#define kItemBright   CFSTR("com.billthefarmer.toolbar.bright")
#define kItemSingle   CFSTR("com.billthefarmer.toolbar.single")
#define kItemTrigger  CFSTR("com.billthefarmer.toolbar.trigger")
#define kItemSync     CFSTR("com.billthefarmer.toolbar.sync")
#define kItemTimebase CFSTR("com.billthefarmer.toolbar.timebase")
#define kItemStorage  CFSTR("com.billthefarmer.toolbar.storage")
#define kItemClear    CFSTR("com.billthefarmer.toolbar.clear")
#define kItemLeft     CFSTR("com.billthefarmer.toolbar.left")
#define kItemRight    CFSTR("com.billthefarmer.toolbar.right")
#define kItemStart    CFSTR("com.billthefarmer.toolbar.start")
#define kItemEnd      CFSTR("com.billthefarmer.toolbar.end")
#define kItemReset    CFSTR("com.billthefarmer.toolbar.reset")

// Audio in values

enum
    {kSampleRate       = 44100,
     kBytesPerPacket   = 4,
     kBytesPerFrame    = 4,
     kChannelsPerFrame = 1};

// Audio processing values

enum
    {kSamples = 262144,
     kFrames = 4096};

// Slider values

enum
    {kVolumeMax  = 100,
     kVolumeMin  = 0,
     kVolumeStep = 10};


// Window dimensions

enum
    {kMinimumHeight = 480,
     kMaximumHeight = 768,
     kMinimumWidth  = 640,
     kMaximumWidth  = 1024};

// Scale dimensions

enum
    {kScaleHeight = 24,
     kScaleWidth  = 16};

// Command IDs

enum
    {kCommandVolume   = 'Volm',
     kCommandBright   = 'Brgt',
     kCommandSingle   = 'Sngl',
     kCommandTrigger  = 'Trig',
     kCommandSync     = 'Sync',
     kCommandTimebase = 'Time',
     kCommandStorage  = 'Stor',
     kCommandClear    = 'Clr ',
     kCommandLeft     = 'Left',
     kCommandRight    = 'Rght',
     kCommandStart    = 'Strt',
     kCommandEnd      = 'End ',
     kCommandReset    = 'Rst '};

// Keycodes

enum
    {kKeyboardUpKey    = 0x7e,
     kKeyboardDownKey  = 0x7d,
     kKeyboardLeftKey  = 0x7b,
     kKeyboardRightKey = 0x7c};

// Audio event constants

enum
    {kEventAudioUpdate = 'Updt'};

// Scope id

HIViewID ScopeID =
    {'Scop', 1};

// Global data

typedef struct
{
    HIViewRef view;
    float *data;
    float scale;
    int index;
    int start;
    int length;
    Boolean bright;
    Boolean single;
    Boolean trigger;
    Boolean polarity;
    Boolean storage;
    Boolean clear;
} Scope;

Scope scope;

typedef struct
{
    HIViewRef view;
    float scale;
    float start;
    float step;
} XScale;

XScale xscale;

typedef struct
{
    HIViewRef view;
} Tool;

Tool yscale;

typedef struct
{
    HIViewRef pane;
    HIViewRef view;
} Volume;

Volume volume;

typedef struct
{
    CFStringRef id;
    CFStringRef name;
    CFStringRef label;
    CFStringRef help;
    MenuCommand command;
} Item;

Item items[] =
    {{kItemVolume, CFSTR("volumedrop"), CFSTR("Volume"),
      CFSTR("Volume, click to pop up slider"), kCommandVolume},
     {kItemBright, CFSTR("bright"), CFSTR("Bright line"),
      CFSTR("Bright line, click to turn off sync"), kCommandBright},
     {kItemSingle, CFSTR("single"), CFSTR("Single shot"),
      CFSTR("Single shot, click to enable"), kCommandSingle},
     {kItemTrigger, CFSTR("trigger"), CFSTR("Trigger"),
      CFSTR("Trigger, click to trigger trace"), kCommandTrigger},
     {kItemSync, CFSTR("positive"), CFSTR("Sync"),
      CFSTR("Sync, click to change sync polarity"), kCommandSync},
     {kItemTimebase, CFSTR("timebasedrop"), CFSTR("Timebase"),
      CFSTR("Timebase, click to pop up menu"), kCommandTimebase},
     {kItemStorage, CFSTR("storage"), CFSTR("Storage"),
      CFSTR("Storage, click to enable"), kCommandStorage},
     {kItemClear, CFSTR("clear"), CFSTR("Clear"),
      CFSTR("Clear, click to clear trace"), kCommandClear},
     {kItemLeft, CFSTR("left"), CFSTR("Left"),
      CFSTR("Left, click to shift trace left"), kCommandLeft},
     {kItemRight, CFSTR("right"), CFSTR("Right"),
      CFSTR("Right, click to shift trace right"), kCommandRight},
     {kItemStart, CFSTR("start"), CFSTR("Start"),
      CFSTR("Start, click to shift trace to start"), kCommandStart},
     {kItemEnd, CFSTR("end"), CFSTR("End"),
      CFSTR("End, click to shift trace to end"), kCommandEnd},
     {kItemReset, CFSTR("reset"), CFSTR("Reset"),
      CFSTR("Reset, click to reset"), kCommandReset}};

typedef struct
{
    HIToolbarRef toolbar;
    HIToolbarItemRef items[LENGTH(items)];
} Toolbar;

Toolbar toolbar;

typedef struct
{
    AudioUnit output;
    AudioBufferList *ablp;
    int frames;
    float sample;
} Audio;

Audio audio;

typedef struct
{
    int index;
    float values[12];
    char *strings[12];
    int counts[12];
} Timebase;

Timebase timebase =
    {3,
     {0.1, 0.2, 0.5, 1.0,
      2.0, 5.0, 10.0, 20.0,
      50.0, 100.0, 200.0, 500.0},
     {"0.1 ms", "0.2 ms", "0.5 ms",
      "1.0 ms", "2.0 ms", "5.0 ms",
      "10 ms", "20 ms", "50 ms",
      "0.1 sec", "0.2 sec", "0.5 sec"},
     {128, 256, 512, 1024,
      2048, 4096, 8192, 16384,
      32768, 65536, 131072, 262144}};

// Function prototypes.

OSStatus XScaleDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus YScaleDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus CentreTextAtPoint(CGContextRef, float, float, const char *, size_t);
OSStatus ScopeDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus PaneDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus KeyboardEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus WindowEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus CommandEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus MouseEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus AudioEventHandler(EventHandlerCallRef, EventRef, void *);

OSStatus SetupAudio(void);
OSStatus DisplayAlert(CFStringRef, CFStringRef);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);

OSStatus DisplayPopupMenu(WindowRef, HICommandExtended, HIRect);
OSStatus DisplaySlider(WindowRef, HICommandExtended, HIRect);
OSStatus ChangeVolume(EventRef, HICommandExtended, UInt32);
void VolumeActionProc(HIViewRef, ControlPartCode);
CGImageRef GetToolbarImage(CFStringRef);

// Function main

int main(int argc, char *argv[])
{
    WindowRef window;
    HIViewRef content;

    // Window bounds

    Rect bounds = {0, 0, kMinimumHeight, kMinimumWidth};

    // Create window

    CreateNewWindow(kDocumentWindowClass,
		    kWindowStandardFloatingAttributes |
		    kWindowFullZoomAttribute |
		    kWindowFrameworkScaledAttribute |
		    kWindowStandardHandlerAttribute |
		    kWindowResizableAttribute |
		    kWindowLiveResizeAttribute |
		    kWindowToolbarButtonAttribute |
		    kWindowCompositingAttribute,
		    &bounds, &window);

    // Set the title

    SetWindowTitleWithCFString(window, CFSTR("Audio Oscilloscope"));

    // Set resize limits

    HISize min = {kMinimumWidth, kMinimumHeight};
    HISize max = {kMaximumWidth, kMaximumHeight};

    SetWindowResizeLimits(window, &min, &max);

    MenuRef menu;

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

    AppendMenuItemTextWithCFString(menu, CFSTR("About Scope"),
                                   0, kHICommandAbout, NULL);
    // Create a separator

    AppendMenuItemTextWithCFString(menu, NULL,
				   kMenuItemAttrSeparator, 0, NULL);
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

    // Create toolbar

    HIToolbarCreate(kToolbarID,
		    kHIToolbarIsConfigurable |
		    kHIToolbarAutoSavesConfig,
		    &toolbar.toolbar);

    // Add toolbar items

    for (int i = 0; i < LENGTH(items); i++)
    {
	HIToolbarItemCreate(items[i].id,
			    kHIToolbarItemNoAttributes,
			    &toolbar.items[i]);

	HIToolbarItemSetCommandID(toolbar.items[i], items[i].command);
	HIToolbarItemSetLabel(toolbar.items[i], items[i].label);
	HIToolbarItemSetHelpText(toolbar.items[i], items[i].help, NULL);

	// Get image

	CGImageRef image = GetToolbarImage(items[i].name);

	HIToolbarItemSetImage(toolbar.items[i], image);
	CGImageRelease(image);

	HIToolbarAppendItem(toolbar.toolbar, toolbar.items[i]);
    }

    HIToolbarSetDisplayMode(toolbar.toolbar, kHIToolbarDisplayModeIconOnly);

    SetWindowToolbar(window, toolbar.toolbar);
    ShowHideWindowToolbar(window, true, false);

    // Find the window content

    HIViewFindByID(HIViewGetRoot(window),
                   kHIViewWindowContentID,
                   &content);
    HIRect rect;

    // Get bounds

    HIViewGetBounds(content, &rect);

    // Bounds of X scale

    bounds.bottom = kScaleHeight;
    bounds.right  = rect.size.width;

    // Create X scale

    CreateUserPaneControl(window, &bounds, 0, &xscale.view);

    // Set help tag

    HMHelpContentRec help =
	{kMacHelpVersion,
	 {0, 0, 0, 0},
	 kHMInsideLeftCenterAligned,
	 {{kHMCFStringContent,
	   CFSTR("X scale")},
	  {kHMNoContent, NULL}}};

    HMSetControlHelpContent(xscale.view, &help);

    // Place in the window

    HIViewAddSubview(content, xscale.view);
    HIViewPlaceInSuperviewAt(xscale.view, 0, rect.size.height - kScaleHeight);

    // Get layout info

    HILayoutInfo info = 
	{kHILayoutInfoVersionZero};

    HIViewGetLayoutInfo(xscale.view, &info);

    info.binding.left.toView = content;
    info.binding.left.kind = kHILayoutBindLeft;

    info.binding.right.toView = content;
    info.binding.right.kind = kHILayoutBindRight;

    info.binding.bottom.toView = content;
    info.binding.bottom.kind = kHILayoutBindBottom;

    HIViewSetLayoutInfo(xscale.view, &info);

    // Bounds of Y scale

    bounds.bottom = rect.size.height - kScaleHeight;
    bounds.right  = kScaleWidth;

    // Create Y scale

    CreateUserPaneControl(window, &bounds, 0, &yscale.view);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Y scale");
    HMSetControlHelpContent(yscale.view, &help);

    // Place in the window

    HIViewAddSubview(content, yscale.view);
    HIViewPlaceInSuperviewAt(yscale.view, 0, 0);

    // Get layout info

    HIViewGetLayoutInfo(yscale.view, &info);

    info.binding.top.toView = content;
    info.binding.top.kind = kHILayoutBindTop;

    info.binding.bottom.toView = xscale.view;
    info.binding.bottom.kind = kHILayoutBindTop;

    HIViewSetLayoutInfo(yscale.view, &info);

    // Bounds of scope

    bounds.bottom = rect.size.height - kScaleHeight;
    bounds.right  = rect.size.width - kScaleWidth;

    // Create scope

    CreateUserPaneControl(window, &bounds, 0, &scope.view);

    // Set id

    HIViewSetID(scope.view, ScopeID);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Oscilloscope display");
    HMSetControlHelpContent(scope.view, &help);

    // Place in the window

    HIViewAddSubview(content, scope.view);
    HIViewPlaceInSuperviewAt(scope.view, kScaleWidth, 0);

    // Get layout info

    HIViewGetLayoutInfo(scope.view, &info);

    info.binding.top.toView = content;
    info.binding.top.kind = kHILayoutBindTop;

    info.binding.left.toView = yscale.view;
    info.binding.left.kind = kHILayoutBindRight;

    info.binding.right.toView = content;
    info.binding.right.kind = kHILayoutBindRight;

    info.binding.bottom.toView = xscale.view;
    info.binding.bottom.kind = kHILayoutBindTop;

    HIViewSetLayoutInfo(scope.view, &info);

    // Window events type spec

    EventTypeSpec windowEvents[] =
        {{kEventClassWindow, kEventWindowClose},
	 {kEventClassWindow, kEventWindowZoomed}};

    // Install event handler

    InstallWindowEventHandler(window, NewEventHandlerUPP(WindowEventHandler),
                              LENGTH(windowEvents), windowEvents,
                              NULL, NULL);

    // Command events type spec

    EventTypeSpec commandEvents[] =
        {{kEventClassCommand, kEventCommandProcess}};

    // Install event handler

    InstallApplicationEventHandler(NewEventHandlerUPP(CommandEventHandler),
                                   LENGTH(commandEvents), commandEvents,
                                   window, NULL);
    // Mouse events type spec

    EventTypeSpec mouseEvents[] =
        {{kEventClassMouse, kEventMouseDown}};

    // Install event handler

    InstallWindowEventHandler(window, NewEventHandlerUPP(MouseEventHandler),
			      LENGTH(mouseEvents), mouseEvents,
			      window, NULL);

    // Keyboard events type spec

    EventTypeSpec keyboardEvents[] =
        {{kEventClassKeyboard, kEventRawKeyDown},
	 {kEventClassKeyboard, kEventRawKeyRepeat}};

    // Install event handler

    InstallApplicationEventHandler(NewEventHandlerUPP(KeyboardEventHandler),
                                   LENGTH(keyboardEvents), keyboardEvents,
                                   window, NULL);
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

    InstallControlEventHandler(scope.view,
			       NewEventHandlerUPP(ScopeDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       scope.view, NULL);

    // Install event handlers

    InstallControlEventHandler(xscale.view,
			       NewEventHandlerUPP(XScaleDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       xscale.view, NULL);

    // Install event handlers

    InstallControlEventHandler(yscale.view,
			       NewEventHandlerUPP(YScaleDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       yscale.view, NULL);
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

// Display alert

OSStatus DisplayAlert(CFStringRef error, CFStringRef explanation)
{
    DialogRef dialog;

    CreateStandardAlert(kAlertStopAlert, error, explanation, NULL, &dialog);
    SetWindowTitleWithCFString(GetDialogWindow(dialog),
			       CFSTR("Audio Oscilloscope"));
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
			   EventRef event, void *v)
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
	// 0: waiting for sync

    case 0:

	index = 0;

	if (scope.bright)
	    state++;

	else
	{
	    if (scope.single && !scope.trigger)
		break;

	    // Initialise sync

	    float dx = 0.0;

	    // Sync polarity

	    if (scope.polarity)
	    {
		for (int i = 0; i < audio.frames; i++)
		{
		    dx = data[i] - last;

		    if (dx < 0.0 && last > 0.0 && data[i] < 0.0)
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

		    if (dx > 0.0 && last < 0.0 && data[i] > 0.0)
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

	// 1: First chunk of data

    case 1:

	// Update count

	count = timebase.counts[timebase.index];
	scope.length = count;

	// Copy data

	memmove(buffer, data + index, (audio.frames - index) * sizeof(float));
	index = audio.frames - index;

	// If done, wait for sync again

	if (index >= count)
	    state = 0;

	else

	// Else get some more data next time

	    state++;
	break;

	// 2: Subsequent chunks of data

    case 2:

	// Copy data

	memmove(buffer + index, data, audio.frames * sizeof(float));
	index += audio.frames;

	// Done, wait for sync again

	if (index >= count)
	    state = 0;

	// Else if last but one chunk, get last chunk next time

	else if (index + audio.frames >= count)
	    state++;
	break;

	// Last chunk of data

    case 3:

	// Copy data

	memmove(buffer + index, data, (count - index) * sizeof(float));

	// Wait for sync next time

	state = 0;
	break;
    }

    // Check timebase

    if (scope.scale != timebase.values[timebase.index])
    {
	scope.scale = timebase.values[timebase.index];
	xscale.scale = timebase.values[timebase.index];
	xscale.step = 500 * xscale.scale;

	// Update display

	HIViewSetNeedsDisplay(xscale.view, true);
    }

    // Update display

    HIViewSetNeedsDisplay(scope.view, true);

    return noErr;
}

// Get toolbar image

CGImageRef GetToolbarImage(CFStringRef name)
{
    // Get url of image

    CFURLRef url =
	CFBundleCopyResourceURL(CFBundleGetMainBundle(),
				name, CFSTR("png"),
				CFSTR("images"));

    // Create data provider

    CGDataProviderRef data = CGDataProviderCreateWithURL(url);

    // Release the url

    CFRelease(url);

    // Create the image

    CGImageRef image =
	CGImageCreateWithPNGDataProvider(data, NULL, true,
					 kCGRenderingIntentDefault);
    // Release the data

    CGDataProviderRelease(data);

    return image;
}

// Display slider

OSStatus DisplaySlider(WindowRef window, HICommandExtended command, HIRect rect)
{
    HIViewRef content;

    if (volume.pane == NULL)
    {
	// Find the window content

	HIViewFindByID(HIViewGetRoot(window),
		       kHIViewWindowContentID,
		       &content);

	// Bounds of pane

	Rect bounds =
	    {0, 0, 112, 55};

	// Create display pane

	CreateUserPaneControl(window, &bounds,
			      kControlSupportsEmbedding, &volume.pane);

	// Place in the window
    
	HIViewAddSubview(content, volume.pane);
	HIViewPlaceInSuperviewAt(volume.pane, rect.origin.x, 0);
    
	// Bounds of slider

	bounds.bottom = 72;
	bounds.right  = 15;

	// Create slider

	CreateSliderControl(window, &bounds, kVolumeMax, kVolumeMin, kVolumeMax,
			    kControlSliderPointsDownOrRight,
			    0, true, VolumeActionProc, &volume.view);
	// Control size

	ControlSize small = kControlSizeSmall;

	// Set control size

	SetControlData(volume.view, kControlEntireControl, kControlSizeTag,
		       sizeof(small), &small);

	// Set command ID

	HIViewSetCommandID(volume.view, kCommandVolume);

	// Set help tag

	HMHelpContentRec help =
	    {kMacHelpVersion,
	     {0, 0, 0, 0},
	     kHMInsideLeftCenterAligned,
	     {{kHMCFStringContent,
	       CFSTR("Volume, slide to adjust")},
	      {kHMNoContent, NULL}}};

	HMSetControlHelpContent(volume.view, &help);

	// Place in the window

	HIViewAddSubview(volume.pane, volume.view);
	HIViewPlaceInSuperviewAt(volume.view, 20, 20);

	// Draw events type spec

	EventTypeSpec drawEvents[] =
	    {{kEventClassControl, kEventControlDraw}};

	// Install event handler

	InstallControlEventHandler(volume.pane,
				   NewEventHandlerUPP(PaneDrawEventHandler),
				   LENGTH(drawEvents), drawEvents,
				   volume.pane, NULL);
    }

    else if (HIViewIsVisible(volume.pane))
	HIViewSetVisible(volume.pane, false);

    else
	HIViewSetVisible(volume.pane, true);

    return noErr;
}

// Display popup menu

OSStatus DisplayPopupMenu(WindowRef window, HICommandExtended command,
			  HIRect rect)
{
    MenuRef menu;
    MenuItemIndex item;

    // Create menu

    CreateNewMenu(0, 0, &menu);

    // Create items

    for (int i = 0; i < LENGTH(timebase.strings); i++)
    {
	CFStringRef s =
	    CFStringCreateWithCString(kCFAllocatorDefault,
				      timebase.strings[i],
				      kCFStringEncodingMacRoman);

	AppendMenuItemTextWithCFString(menu, s, 0, kCommandTimebase, &item);
	CFRelease(s);

	if (timebase.index == i)
	    CheckMenuItem(menu, item, true);
    }

    Rect bounds;

    // Get bounds

    GetWindowBounds(window, kWindowContentRgn, &bounds);

    // Show the menu

    PopUpMenuSelect(menu, bounds.top + rect.origin.y + 4,
		    bounds.left + rect.origin.x, 0);

    // Release menu

    ReleaseMenu(menu);

    return noErr;
}

// X scale draw event handler

OSStatus XScaleDrawEventHandler(EventHandlerCallRef next,
				EventRef event, void *data)
{
    CGContextRef context;
    HIViewRef view;
    HIRect bounds;

    enum
    {kTextSize = 14};

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

    float width = bounds.size.width;
    float height = bounds.size.height;

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    CGContextTranslateCTM(context, kScaleWidth, 0);

    CGContextBeginPath(context);

    // Draw the scale

    for (int x = 0; x < width - kScaleWidth; x += 50)
    {
	CGContextMoveToPoint(context, x, 0);
	CGContextAddLineToPoint(context, x, height / 3);
   }

    for (int x = 0; x < width - kScaleWidth; x += 10)
    {
	CGContextMoveToPoint(context, x, 0);
	CGContextAddLineToPoint(context, x, height / 4);
   }

    CGContextStrokePath(context);

    CGContextSetShouldAntialias(context, true);

    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);

    // Select font

    CGContextSelectFont(context, "Arial Bold", kTextSize,
			kCGEncodingMacRoman);

    // Draw legend

    if (xscale.scale < 100.0)
    {
	static char s[16];

	CentreTextAtPoint(context, 0, height - 4, "ms", 2);

	for (int x = 100; x < width - kScaleWidth; x += 100)
	{
	    sprintf(s, "%0.1f", (xscale.start + (x * xscale.scale)) / 100.0);
	    CentreTextAtPoint(context, x, height - 4, s, strlen(s));
	}
    }

    else
    {
	static char s[16];

	CentreTextAtPoint(context, 0, height - 4, "sec", 3);

	for (int x = 100; x < width - kScaleWidth; x += 100)
	{
	    sprintf(s, "%0.1f", (xscale.start + (x * xscale.scale)) / 100000.0);
	    CentreTextAtPoint(context, x, height - 4, s, strlen(s));
	}
    }

    return noErr;
}

// Centre text at point

OSStatus CentreTextAtPoint(CGContextRef context, float x, float y,
                           const char * bytes, size_t length)
{
    // Draw invisible text

    CGContextSetTextDrawingMode(context, kCGTextInvisible);
    CGContextShowTextAtPoint(context, x, y, bytes, length);

    // Get new text position

    CGPoint point = CGContextGetTextPosition(context);

    // Calculate differences

    float dx = (point.x - x) / 2.0;
    float dy = (point.y - y) / 2.0;

    // Draw it again in the right place

    CGContextSetTextDrawingMode(context, kCGTextFill);
    CGContextShowTextAtPoint(context, x - dx, y - dy, bytes, length);

    return noErr;
}

// Y scale draw event handler

OSStatus YScaleDrawEventHandler(EventHandlerCallRef next,
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

    float width = bounds.size.width;
    float height = bounds.size.height;

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    CGContextTranslateCTM(context, width / 2, height / 2);

    CGContextBeginPath(context);

    // Draw scale

    for (int y = 0; y < height / 2; y += 50)
    {
	CGContextMoveToPoint(context, 0, y);
	CGContextAddLineToPoint(context, width / 2, y);

	CGContextMoveToPoint(context, 0, -y);
	CGContextAddLineToPoint(context, width / 2, -y);
    }

    for (int y = 0; y < height / 2; y += 10)
    {
	CGContextMoveToPoint(context, width / 8, y);
	CGContextAddLineToPoint(context, width / 2, y);

	CGContextMoveToPoint(context, width / 8, -y);
	CGContextAddLineToPoint(context, width / 2, -y);
    }

    CGContextStrokePath(context);

    return noErr;
}

// Scope draw event handler

OSStatus ScopeDrawEventHandler(EventHandlerCallRef next,
			       EventRef event, void *data)
{
    // Font height

    enum
    {kTextSize = 12};

    static CGContextRef bitmap;
    static CGImageRef image;
    static CGSize size;

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

    float width = bounds.size.width;
    float height = bounds.size.height;

    if (size.width != width || size.height != height)
    {
	if (bitmap != NULL)
	{
	    free(CGBitmapContextGetData(bitmap));
	    CGContextRelease(bitmap);
	    CGImageRelease(image);
	}

	CGColorSpaceRef colour =
	    CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

	bitmap =
	    CGBitmapContextCreate(malloc(width * height * 4), width, height,
				  8, width * 4, colour,
				  kCGImageAlphaPremultipliedLast);

	CGContextRef graticule =
	    CGBitmapContextCreate(malloc(width * height * 4), width, height,
				  8, width * 4, colour,
				  kCGImageAlphaPremultipliedLast);

	CGColorSpaceRelease(colour);

	size = bounds.size;

	CGContextSetShouldAntialias(graticule, false);
	CGContextSetLineWidth(graticule, 1);

	// Black background

	CGContextSetGrayFillColor(graticule, 0, 1);
	CGContextFillRect(graticule, bounds);

	// Dark green graticule

	CGContextSetRGBStrokeColor(graticule, 0, 0.6, 0, 1);

	// Draw graticule

	CGContextBeginPath(graticule);

	for (int i = 0; i <= width; i += 10)
	{
	    CGContextMoveToPoint(graticule, i, 0);
	    CGContextAddLineToPoint(graticule, i, height);
	}

	// Move the origin

	CGContextTranslateCTM(graticule, 0, height / 2);

	for (int i = 0; i <= height / 2; i += 10)
	{
	    CGContextMoveToPoint(graticule, 0, i);
	    CGContextAddLineToPoint(graticule, width, i);

	    CGContextMoveToPoint(graticule, 0, -i);
	    CGContextAddLineToPoint(graticule, width, -i);
	}

	CGContextStrokePath(graticule);

	image = CGBitmapContextCreateImage(graticule);

	free(CGBitmapContextGetData(graticule));
	CGContextRelease(graticule);

	scope.clear = true;
    }

    // No trace if no data

    if (scope.data == NULL)
    {
	CGContextDrawImage(context, bounds, image);
	return noErr;
    }

    // Erase background

    if (!scope.storage || scope.clear)
    {
	// Draw graticule image

	CGContextDrawImage(bitmap, bounds, image);

	scope.clear = false;
    }

    // Calculate scale etc

    float xscale = 1.0 / (((float)audio.sample / 100000.0) * scope.scale);
    int xstart = round(scope.start);
    int xstep = round(1.0 / xscale);
    int xstop = round(xstart + ((float)width / xscale));

    if (xstop > scope.length)
	xstop = scope.length;

    // Calculate scale

    static float max;

    if (max < 0.125)
	max = 0.125;

    float yscale = max / ((float)height / 2.0);

    max = 0.0;

    // Move the origin

    CGContextTranslateCTM(bitmap, 0, height / 2);
    CGContextSetShouldAntialias(bitmap, false);
    CGContextSetLineWidth(bitmap, 1);

    // Green trace

    CGContextSetRGBStrokeColor(bitmap, 0, 1, 0, 1);

    // Draw the trace

    CGContextBeginPath(bitmap);
    CGContextMoveToPoint(bitmap, 0, 0);

    if (xscale < 1.0)
    {
	for (int i = 0; i <= xstop - xstart; i += xstep)
	{
	    if (max < fabs(scope.data[i + xstart]))
		max = fabs(scope.data[i + xstart]);

	    float x = (float)i * xscale;
	    float y = -scope.data[i + xstart] / yscale;

	    CGContextAddLineToPoint(bitmap, x, y);
	}

	CGContextStrokePath(bitmap);
    }

    else
    {
	for (int i = 0; i <= xstop - xstart; i++)
	{
	    if (max < fabs(scope.data[i + xstart]))
		max = fabs(scope.data[i + xstart]);

	    float x = (float)i * xscale;
	    float y = -scope.data[i + xstart] / yscale;

	    CGContextAddLineToPoint(bitmap, x, y);
	}

	CGContextStrokePath(bitmap);

	// Draw points at maximum resolution

	if (timebase.index == 0)
	{
	    for (int i = 0; i <= xstop - xstart; i++)
	    {
		float x = (float)i * xscale;
		float y = -scope.data[i + xstart] / yscale;

		CGContextStrokeRect(bitmap, CGRectMake(x - 2, y - 2, 4, 4));
	    }
	}
    }

    // Select font

    CGContextSelectFont(bitmap, "Arial Bold", kTextSize,
			kCGEncodingMacRoman);

    CGContextSetTextMatrix(bitmap, CGAffineTransformMakeScale(1, -1));
    CGContextSetRGBFillColor(bitmap, 0, 1, 0, 1);

    // Draw cursor

    if (scope.index > 0)
    {
	static char s[16];

	CGContextBeginPath(bitmap);
	CGContextMoveToPoint(bitmap, scope.index, -height / 2);
	CGContextAddLineToPoint(bitmap, scope.index, height / 2);
	CGContextStrokePath(bitmap);

	CGContextSetShouldAntialias(bitmap, true);

	int i = round((float)scope.index / xscale);
	float y = -scope.data[i + xstart] / yscale;

	sprintf(s, "%0.2f", scope.data[i + xstart]);
	CGContextShowTextAtPoint(bitmap, scope.index, y, s, strlen(s));

	if (scope.scale < 100.0)
	{
	    sprintf(s, (scope.scale < 1.0)? "%0.3f": 
		    (scope.scale < 10.0)? "%0.2f": "%0.1f",
		    (scope.start + (scope.index * scope.scale)) / 100.0);
	    CentreTextAtPoint(bitmap, scope.index, height / 2,
			      s, strlen(s));
	}

	else
	{
	    sprintf(s, "%0.3f", (scope.start + (scope.index *
						scope.scale)) / 100000.0);
	    CentreTextAtPoint(bitmap, scope.index, height / 2,
			      s, strlen(s));
	}
    }

    CGContextTranslateCTM(bitmap, 0, -height / 2);
    CGImageRef content = CGBitmapContextCreateImage(bitmap);
    CGContextDrawImage(context, bounds, content);
    CGImageRelease(content);

    return noErr;
}

// Pane draw event handler

OSStatus PaneDrawEventHandler(EventHandlerCallRef next,
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
    CGContextSetGrayFillColor(context, 1, 0.9);
    CGContextFillRect(context, bounds);

    CGContextSetGrayStrokeColor(context, 0.8, 0.9);
    CGContextSetLineWidth(context, 3);
    CGContextStrokeRect(context, bounds);

    return noErr;
}

// Command event handler

OSStatus CommandEventHandler(EventHandlerCallRef next, EventRef event,
			     void *data)
{
    enum
    {kControlKindToolbarItemView = kEventClassToolbarItemView};

    HICommandExtended command;
    WindowRef window;
    HIViewKind kind;
    HIRect bounds;
    UInt32 value;

    // Get the command

    GetEventParameter(event, kEventParamDirectObject,
                      typeHICommand, NULL, sizeof(command),
                      NULL, &command);

    switch (command.attributes)
    {
    case kHICommandFromControl:

	// Get the bounds

	HIViewGetFrame(command.source.control, &bounds);

	// Get the window
    
	window = HIViewGetWindow(command.source.control);

	// Get the value

	value = HIViewGetValue(command.source.control);

	// Switch on the command ID

	switch (command.commandID)
	{
	    // Volume

	case kCommandVolume:
	    HIViewGetKind(command.source.control, &kind);

	    switch (kind.kind)
	    {
		// Toolbar button

            case kControlKindToolbarItemView:
                DisplaySlider(window, command, bounds);
                break;

		// Slider

            case kControlKindSlider:
		ChangeVolume(event, command, value);
		break;
	    }
	    break;

	    // Bright line

	case kCommandBright:
	    scope.bright = !scope.bright;

	    // Get image

	    CGImageRef image =
		GetToolbarImage(scope.bright?
				CFSTR("brightsel"):
				CFSTR("bright"));

	    HIToolbarItemSetImage(toolbar.items[1], image);
	    CGImageRelease(image);
	    break;

	    // Single shot

	case kCommandSingle:
	    scope.single = !scope.single;

	    // Get image

	    image =
		GetToolbarImage(scope.single?
				CFSTR("singlesel"):
				CFSTR("single"));

	    HIToolbarItemSetImage(toolbar.items[2], image);
	    CGImageRelease(image);
	    break;

	    // Trigger

	case kCommandTrigger:
	    scope.trigger = true;
	    break;

	    // Sync polarity

	case kCommandSync:
	    scope.polarity = !scope.polarity;

	    // Get image

	    image =
		GetToolbarImage(scope.polarity?
				CFSTR("negative"):
				CFSTR("positive"));

	    HIToolbarItemSetImage(toolbar.items[4], image);
	    CGImageRelease(image);
	    break;

	    // Timebase

	case kCommandTimebase:
	    DisplayPopupMenu(window, command, bounds);
	    break;

	    // Storage

	case kCommandStorage:
	    scope.storage = !scope.storage;

	    // Get image

	    image =
		GetToolbarImage(scope.storage?
				CFSTR("storagesel"):
				CFSTR("storage"));

	    HIToolbarItemSetImage(toolbar.items[6], image);
	    CGImageRelease(image);
	    break;

	    // Clear

	case kCommandClear:
	    scope.clear = true;
	    break;

	    // Left

	case kCommandLeft:
	    scope.start -= xscale.step;
	    if (scope.start < 0)
		scope.start = 0;
	    xscale.start = scope.start;
	    HIViewSetNeedsDisplay(xscale.view, true);
	    break;

	    // Right

	case kCommandRight:
	    scope.start += xscale.step;
	    if (scope.start >= scope.length)
		scope.start -= xscale.step;
	    xscale.start = scope.start;
	    HIViewSetNeedsDisplay(xscale.view, true);
	    break;

	    // Start

	case kCommandStart:
	    scope.start = 0;
	    scope.index = 0;
	    xscale.start = 0;
	    HIViewSetNeedsDisplay(xscale.view, true);
	    break;

	    // End

	case kCommandEnd:
	    while (scope.start < scope.length)
		scope.start += xscale.step;
	    scope.start -= xscale.step;
	    xscale.start = scope.start;
	    HIViewSetNeedsDisplay(xscale.view, true);
	    break;

	    // Reset

	case kCommandReset:
	    scope.index = 0;
	    scope.bright = false;
	    scope.single = false;
	    scope.polarity = false;
	    scope.storage = false;


	    // Get image

	    image = GetToolbarImage(CFSTR("bright"));
	    HIToolbarItemSetImage(toolbar.items[1], image);
	    CGImageRelease(image);

	    // Get image

	    image = GetToolbarImage(CFSTR("single"));
	    HIToolbarItemSetImage(toolbar.items[2], image);
	    CGImageRelease(image);

	    // Get image

	    image = GetToolbarImage(CFSTR("positive"));
	    HIToolbarItemSetImage(toolbar.items[4], image);
	    CGImageRelease(image);

	    // Get image

	    image = GetToolbarImage(CFSTR("storage"));
	    HIToolbarItemSetImage(toolbar.items[6], image);
	    CGImageRelease(image);
	    break;

	    // Quit

	case kHICommandQuit:

	    // Close audio unit

	    AudioOutputUnitStop(audio.output);
	    AudioUnitUninitialize(audio.output);

	    // Let the default handler handle it

	default:
	    return eventNotHandledErr;
	}
	break;

	// Menu

    case kHICommandFromMenu:

	// Switch on the command ID

	switch (command.commandID)
	{
	    // Timebase

	case kCommandTimebase:
	    timebase.index = command.source.menu.menuItemIndex - 1;
	    break;

	default:
	    return eventNotHandledErr;
	}
	break;

    default:
	return eventNotHandledErr;
    }

    return noErr;
}

// Window event handler

OSStatus WindowEventHandler(EventHandlerCallRef next,
			    EventRef event, void *data)
{
    // Get the event kind

    UInt32 kind = GetEventKind(event);

    // Switch on event kind

    switch (kind)
    {
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
    WindowRef window;

    GetEventParameter(event, kEventParamMouseButton,
		      typeMouseButton, NULL, sizeof(button),
		      NULL, &button);

    switch (button)
    {
    case kEventMouseButtonPrimary:

	GetEventParameter(event, kEventParamWindowRef,
			  typeWindowRef, NULL, sizeof(window),
			  NULL, &window);
	HIPoint location;
	HIViewRef view;
	HIViewID id;

	HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &view);
	HIViewGetID(view, &id);

	switch (id.id)
	{
	case 1:
	    GetEventParameter(event, kEventParamWindowMouseLocation,
			      typeHIPoint, NULL, sizeof(location),
			      NULL, &location);
	    HIViewConvertPoint(&location, NULL, view);
	    scope.index = location.x;
	    break;

	default:
	    return eventNotHandledErr;
	}

    default:
	return eventNotHandledErr;
    }

    return noErr;
}

// Keyboard event handler

OSStatus KeyboardEventHandler(EventHandlerCallRef next,
			      EventRef event, void *data)
{
    UInt32 code;
    HIRect bounds;

    // Get key code

    GetEventParameter(event, kEventParamKeyCode, typeUInt32,
		      NULL, sizeof(code), NULL, &code);

    HIViewGetBounds(scope.view, &bounds);

    switch (code)
    {
	// Left

    case kKeyboardLeftKey:
	scope.index--;

	if (scope.index < 0)
	    scope.index = 0;
	break;

	// Right

    case kKeyboardRightKey:
	scope.index++;

	if (scope.index >= bounds.size.width)
	    scope.index = 0;
	break;

    default:
	return eventNotHandledErr;
    }

    return noErr;
}

// Change volume

OSStatus ChangeVolume(EventRef event,
		      HICommandExtended command,
		      UInt32 value)
{
    Float32 vol = (float)value / (float)kVolumeMax;

    AudioUnitSetParameter(audio.output, kHALOutputParam_Volume,
			  kAudioUnitScope_Input, 1, vol, 0);

    HIViewSetVisible(volume.pane, false);
    return noErr;
}

// Volume action proc

void VolumeActionProc(HIViewRef view, ControlPartCode part)
{
    // Get the slider value

    UInt32 value = HIViewGetValue(view);

    Float32 volume = (float)value / (float)kVolumeMax;

    AudioUnitSetParameter(audio.output, kHALOutputParam_Volume,
			  kAudioUnitScope_Input, 1, volume, 0);
}
