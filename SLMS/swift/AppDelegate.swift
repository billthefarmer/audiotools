//
//  AppDelegate.swift
//  SLMS
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

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{

    @IBOutlet weak var window: NSWindow!

    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application

        window.setContentSize(NSMakeSize(620, 320))
        window.contentMinSize = NSMakeSize(CGFloat(kMinWidth),
                                           CGFloat(kMinHeight))
        window.contentAspectRatio = NSMakeSize(CGFloat(1936),
                                               CGFloat(1000))
        window.collectionBehavior.insert(.fullScreenNone)

        window.title = "Selective Level Measuring Set"

        // Views
        scaleView = ScaleView()
        displayView = DisplayView()
        knobView = KnobView()
        meterView = MeterView()
        spectrumView = SpectrumView()

        knobView.target = self
        knobView.action = #selector(knobChange)

        scaleView.toolTip = "Frequency scale"
        knobView.toolTip = "Frequency knob"
        displayView.toolTip = "Frequency and level display"
        meterView.toolTip = "Level meter"
        spectrumView.toolTip = "Spectrum"

        // Stack
        let lStack = NSStackView(views: [scaleView, knobView])
        let rStack = NSStackView(views: [displayView, meterView, spectrumView])
        let stack = NSStackView(views: [lStack, rStack])

        lStack.orientation = .vertical
        lStack.spacing = 8

        rStack.orientation = .vertical
        rStack.spacing = 8

        stack.orientation = .horizontal
        stack.spacing = 8
        stack.edgeInsets = NSEdgeInsetsMake(20, 20, 20, 20)

        let knobHeight = NSLayoutConstraint(item: knobView,
                                            attribute: .height,
                                            relatedBy: .equal,
                                            toItem: knobView,
                                            attribute: .width,
                                            multiplier: 1,
                                            constant: 0)
        lStack.addConstraint(knobHeight)
        let spectrumHeight = NSLayoutConstraint(item: spectrumView,
                                                attribute: .height,
                                                relatedBy: .equal,
                                                toItem: displayView,
                                                attribute: .height,
                                                multiplier: 1,
                                                constant: 0)
        rStack.addConstraint(spectrumHeight)
        let meterHeight = NSLayoutConstraint(item: meterView,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: displayView,
                                             attribute: .height,
                                             multiplier: 1,
                                             constant: 0)
        rStack.addConstraint(meterHeight)
        let stackWidth = NSLayoutConstraint(item: lStack,
                                            attribute: .width,
                                            relatedBy: .equal,
                                            toItem: rStack,
                                            attribute: .width,
                                            multiplier: 0.5,
                                            constant: 0)
        stack.addConstraint(stackWidth)
        let scaleHeight = NSLayoutConstraint(item: scaleView,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: displayView,
                                             attribute: .height,
                                             multiplier: 1,
                                             constant: 0)
        stack.addConstraint(scaleHeight)

        // Window
        window.contentView = stack
        window.makeKeyAndOrderFront(self)
        window.makeFirstResponder(displayView)
        window.makeMain()

        // Audio
        let status = SetupAudio()
        if (status != noErr)
        {
            displayAlert("Tuner", "Audio initialisation failed", status)
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

    // knobChange
    @objc func knobChange(sender: KnobView)
    {
        let value = sender.value
        scaleView.value = sender.value / 2.0
        let frequency = Float(pow(10, value / 2) * 10)
        audio.frequency = frequency
        disp.frequency = frequency
        let fps = audio.sample / Float(kStep)
        spectrum.slot = frequency / fps
    }

    // print
    @objc func print(sender: Any)
    {
        window.printWindow(sender)
    }

    // applicationShouldTerminateAfterLastWindowClosed
    func
      applicationShouldTerminateAfterLastWindowClosed(_ sender:
                                                        NSApplication) -> Bool
    {
        return true
    }

    func applicationWillTerminate(_ aNotification: Notification)
    {
        // Insert code here to tear down your application
        ShutdownAudio()
    }
}

