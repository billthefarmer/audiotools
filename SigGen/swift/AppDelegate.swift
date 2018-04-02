//
//  AppDelegate.swift
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


import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{

    @IBOutlet weak var window: NSWindow!

    var menu: NSMenu!

    var stack: NSStackView!

    var scaleView: ScaleView!
    var displayView: DisplayView!
    var knobView: KnobView!

    // applicationDidFinishLaunching
    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application

        // Find a window
        if (NSApp.mainWindow != nil)
        {
            window = NSApp.mainWindow
        }

        else if (NSApp.keyWindow != nil)
        {
            window = NSApp.keyWindow
        }

        else if (NSApp.windows.count > 0)
        {
            window = NSApp.windows[0]
        }

        if (window == nil)
        {
            return
        }

        window.setContentSize(NSMakeSize(420, 300))
        window.contentMinSize = NSMakeSize(420, 300)
        window.contentAspectRatio = NSMakeSize(1.4, 1.0)
        window.showsResizeIndicator = true

        // Find the menu
        menu = NSApp.mainMenu

        knobView = KnobView()
        scaleView = ScaleView()
        displayView = DisplayView()

        knobView.target = self
        knobView.action = #selector(knobChange)
        knobView.toolTip = "Frequency knob"
        knobView.tag = kTagFreq
        knobView.value = 1.0

        scaleView.toolTip = "Frequency scale"
        displayView.toolTip = "Frequency and level display"

        knobView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        scaleView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        displayView.layerContentsRedrawPolicy = .onSetNeedsDisplay


        let lStack = NSStackView(views: [scaleView, knobView])

        let knobHeight = NSLayoutConstraint(item: knobView,
                                            attribute: .height,
                                            relatedBy: .equal,
                                            toItem: knobView,
                                            attribute: .width,
                                            multiplier: 1,
                                            constant: 0)
        lStack.addConstraint(knobHeight)
        lStack.orientation = .vertical
        lStack.spacing = 8

        let fine = NSSlider()
        fine.target = self
        fine.action = #selector(sliderChange)
        fine.toolTip = "Fine frequency"
        fine.doubleValue = 0.5
        fine.tag = kTagFine
        setVertical(fine, true)
        let level = NSSlider()
        level.target = self
        level.action = #selector(sliderChange)
        level.toolTip = "Level"
        level.doubleValue = 0.2
        level.tag = kTagLevel
        setVertical(level, true)

        let sine = NSButton()
        sine.target = self
        sine.action = #selector(buttonClicked)
        sine.setButtonType(.radio)
        sine.title = "Sine"
        sine.tag = kTagSine
        sine.state = .on
        let square = NSButton()
        square.target = self
        square.action = #selector(buttonClicked)
        square.setButtonType(.radio)
        square.title = "Square"
        square.tag = kTagSquare
        let sawtooth = NSButton()
        sawtooth.target = self
        sawtooth.action = #selector(buttonClicked)
        sawtooth.setButtonType(.radio)
        sawtooth.title = "Sawtooth"
        sawtooth.tag = kTagSaw
        let mute = NSButton()
        mute.target = self
        mute.action = #selector(buttonClicked)
        mute.setButtonType(.switch)
        mute.title = "Mute"
        mute.tag = kTagMute

        let vStack = NSStackView(views: [sine, square, sawtooth, mute])
        vStack.orientation = .vertical
        vStack.alignment = .left
        vStack.spacing = 8
        let hStack = NSStackView(views: [fine, level, vStack])
        hStack.orientation = .horizontal
        hStack.alignment = .top
        hStack.spacing = 16
        let rStack = NSStackView(views: [displayView, hStack])
        rStack.orientation = .vertical
        rStack.alignment = .left
        rStack.spacing = 8
        let displayHeight = NSLayoutConstraint(item: displayView,
                                               attribute: .height,
                                               relatedBy: .equal,
                                               toItem: hStack,
                                               attribute: .height,
                                               multiplier: 0.46,
                                               constant: 0)
        rStack.addConstraint(displayHeight)
        let stack = NSStackView(views: [lStack, rStack])
        stack.orientation = .horizontal
        stack.alignment = .top
        stack.spacing = 8
        let stackWidth = NSLayoutConstraint(item: lStack,
                                            attribute: .width,
                                            relatedBy: .equal,
                                            toItem: rStack,
                                            attribute: .width,
                                            multiplier: 1,
                                            constant: 0)
        stack.addConstraint(stackWidth)
        stack.edgeInsets = NSEdgeInsets(top: 20, left: 20,
                                        bottom: 20, right: 20)

        window.contentView = stack
        window.makeKeyAndOrderFront(self)
        window.makeFirstResponder(knobView)
        window.makeMain()

        audio.frequency = 1000
        audio.level = 0.2

        let status = SetupAudio()
        if (status != noErr)
        {
            displayAlert("Tuner", "Audio initialisation failed", status)
        }
    }

    // knobChange
    @objc func knobChange(sender: KnobView)
    {
        print("Knob", sender, sender.value)
        scaleView.value = sender.value
    }

    // sliderChange
    @objc func sliderChange(sender: NSSlider)
    {
        print("Slider", sender, sender.doubleValue)
        switch sender.tag
        {
        case kTagLevel :
            audio.level = sender.doubleValue
            break

        case kTagFine :
            break

        default:
            break
        }
    }

    // buttonClicked
    @objc func buttonClicked(sender: NSButton)
    {
        print("Button", sender, sender.state)
        switch sender.tag
        {
        case kTagSine :
            audio.waveform = Int32(kSine)

        case kTagSquare :
            audio.waveform = Int32(kSquare)

        case kTagSaw :
            audio.waveform = Int32(kSawtooth)

        case kTagMute :
            audio.mute = !audio.mute

        default :
            break
        }
    }

    // DisplayAlert
    func displayAlert(_ message: String, _ informativeText: String,
                      _ status: OSStatus)
    {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = message

        let error = (status > 0) ? UTCreateStringForOSType(OSType(status))
          .takeRetainedValue() as String :
          String(utf8String: AudioUnitErrString(status))!

        alert.informativeText = informativeText + ": " + error +
          " (" + String(status) + ")"

        alert.runModal()
    }

    // applicationShouldTerminateAfterLastWindowClosed
    func
      applicationShouldTerminateAfterLastWindowClosed(_ sender:
                                                        NSApplication) -> Bool
    {
        return true
    }

    // applicationWillTerminate
    func applicationWillTerminate(_ aNotification: Notification)
    {
        // Insert code here to tear down your application
        ShutdownAudio()
    }


}

