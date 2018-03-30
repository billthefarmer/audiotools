//
//  AppDelegate.swift
//  SigGen
//
//  Created by Bill Farmer on 29/03/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//

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

        knobView.toolTip = "Frequency knob"
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
        setVertical(fine, true)
        let level = NSSlider()
        level.target = self
        level.action = #selector(sliderChange)
        level.toolTip = "Level"
        level.doubleValue = 0.2
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
        window.makeMain()
    }

    // sliderChange
    @objc func sliderChange(sender: NSSlider)
    {
        print("Slider", sender, sender.doubleValue)
    }

    // buttonClicked
    @objc func buttonClicked(sender: NSButton)
    {
        print("Button", sender, sender.state)
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
    }


}

