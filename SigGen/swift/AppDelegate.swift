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

var scaleView: ScaleView!
var displayView: DisplayView!
var knobView: KnobView!

var fineSlider: NSSlider!
var levelSlider: NSSlider!

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{
    let kFineRef  = 0.5
    let kLevelRef = 0.1

    let kFreqVal = 1000.0
    let kLevelVal = -20.0

    @IBOutlet weak var window: NSWindow!

    var menu: NSMenu!

    var stack: NSStackView!

    // applicationDidFinishLaunching
    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application

        window.contentMinSize = NSMakeSize(340, 260)
        window.contentAspectRatio = NSMakeSize(340, 260)
        window.collectionBehavior.insert(.fullScreenNone)

        window.title = "Signal Generator"

        // Find the menu
        menu = NSApp.mainMenu

        knobView = KnobView()
        scaleView = ScaleView()
        displayView = DisplayView()

        knobView.target = self
        knobView.action = #selector(knobChange)
        knobView.toolTip = "Frequency knob"
        knobView.tag = kTagFreq

        scaleView.toolTip = "Frequency scale"
        displayView.toolTip = "Frequency and level display"

        knobView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        scaleView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        displayView.layerContentsRedrawPolicy = .onSetNeedsDisplay

        let lStack = NSStackView(views: [scaleView, knobView])

        let knobHeight = NSLayoutConstraint(item: knobView as Any,
                                            attribute: .height,
                                            relatedBy: .equal,
                                            toItem: knobView as Any,
                                            attribute: .width,
                                            multiplier: 1,
                                            constant: 0)
        lStack.addConstraint(knobHeight)
        lStack.orientation = .vertical
        lStack.spacing = 8

        fineSlider = NSSlider()
        fineSlider.target = self
        fineSlider.action = #selector(sliderChange)
        fineSlider.toolTip = "Fine frequency"
        fineSlider.doubleValue = 0.5
        fineSlider.tag = kTagFine
        setVertical(fineSlider, true)

        levelSlider = NSSlider()
        levelSlider.target = self
        levelSlider.action = #selector(sliderChange)
        levelSlider.toolTip = "Level"
        levelSlider.doubleValue = kLevelRef
        levelSlider.tag = kTagLevel
        setVertical(levelSlider, true)

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
        let exact = NSButton()
        exact.target = self
        exact.action = #selector(showExact)
        exact.setButtonType(.momentaryPushIn)
        exact.bezelStyle = .rounded
        exact.title = "Exact…"
        let quit = NSButton()
        quit.target = NSApp
        quit.action = #selector(NSApp.terminate)
        quit.setButtonType(.momentaryPushIn)
        quit.bezelStyle = .rounded
        quit.title = "Quit"

        let vStack = NSStackView(views: [sine, square, sawtooth,
                                         mute, exact, quit])
        vStack.orientation = .vertical
        vStack.alignment = .left
        vStack.spacing = 8
        let hStack = NSStackView(views: [fineSlider, levelSlider, vStack])
        hStack.orientation = .horizontal
        hStack.alignment = .top
        hStack.spacing = 16
        let rStack = NSStackView(views: [displayView, hStack])
        rStack.orientation = .vertical
        rStack.alignment = .left
        rStack.spacing = 8
        let displayHeight = NSLayoutConstraint(item: displayView as Any,
                                               attribute: .height,
                                               relatedBy: .equal,
                                               toItem: hStack,
                                               attribute: .height,
                                               multiplier: 0.46,
                                               constant: 0)
        rStack.addConstraint(displayHeight)
        stack = NSStackView(views: [lStack, rStack])
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
        let scaleHeight = NSLayoutConstraint(item: scaleView as Any,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: displayView as Any,
                                             attribute: .height,
                                             multiplier: 1,
                                             constant: 0)
        stack.addConstraint(scaleHeight)
        stack.edgeInsets = NSEdgeInsetsMake(20, 20, 20, 20)
        window.contentView = stack

        window.makeKeyAndOrderFront(self)
        window.makeFirstResponder(displayView)
        window.makeMain()

        displayView.frequency = kFreqVal
        displayView.decibels = kLevelVal

        audio.frequency = kFreqVal
        audio.waveform = Int32(kSine)
        audio.level = kLevelRef

        let status = SetupAudio()
        if (status != noErr)
        {
            displayAlert("Tuner", "Audio initialisation failed", status)
        }
    }

    // knobChange
    @objc func knobChange(sender: KnobView)
    {
        let value = Double(sender.value)

        switch sender.tag
        {
        case kTagFreq:
            scaleView.value = sender.value / 2.0
            if (fineSlider != nil)
            {
                frequencyChange(value, fineSlider.doubleValue)
            }

        default:
            break
        }
    }

    // sliderChange
    @objc func sliderChange(sender: NSSlider)
    {
        let value = sender.doubleValue
        switch sender.tag
        {
        case kTagLevel:
            var decibels = log10(value) * 20.0
            if (decibels < -80.0)
            {
                decibels = -80.0
            }
            displayView.decibels = decibels
            audio.level = value

        case kTagFine:
            if (knobView != nil)
            {
                frequencyChange(Double(knobView.value), value)
            }

        default:
            break
        }
    }

    //frequencyChange
    func frequencyChange(_ value: Double, _ fine: Double)
    {
        var frequency = pow(10.0, value / 2.0) * 10.0
        let fine = (fine  - kFineRef) / 100.0
        frequency += (frequency * fine)

        displayView.frequency = frequency
        audio.frequency = frequency
    }

    // setFrequency
    func setFrequency(_ value: Double)
    {
        fineSlider.doubleValue = kFineRef
        knobView.value = log10(CGFloat(value / 10)) * 2
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

    // showExact
    @objc func showExact(sender: Any)
    {
        let alert = NSAlert()
        alert.alertStyle = .informational
        alert.messageText = "Please enter exact frequency"
        let accessory = NSTextView(frame: NSMakeRect(0, 0, 180, 18))
        accessory.isFieldEditor = true
        accessory.font = NSFont.boldSystemFont(ofSize: 16)
        accessory.string = String(format: "%1.2f", audio.frequency)
        alert.accessoryView = accessory
        alert.addButton(withTitle: "OK")
        alert.addButton(withTitle: "Cancel")

        let result = alert.runModal()
        if (result == .alertFirstButtonReturn)
        {
            let value = Double(accessory.string)
            if (value != nil)
            {
                setFrequency(value!)
            }
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

