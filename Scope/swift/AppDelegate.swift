//
//  AppDelegate.swift
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

import Cocoa

struct Timebase: Equatable, Hashable
{
    var index: Int
    let values: [Float]
    let strings: [String]
    let counts: [Int]

    init(_ index: Int,_ values: [Float],_ strings: [String],_ counts: [Int])
    {
        self.index = index
        self.values = values
        self.strings = strings
        self.counts = counts
    }
}

var timebase =
  Timebase(3,
           [0.1, 0.2, 0.5, 1.0,
            2.0, 5.0, 10.0, 20.0,
            50.0, 100.0, 200.0, 500.0],
           ["0.1 ms", "0.2 ms", "0.5 ms",
            "1.0 ms", "2.0 ms", "5.0 ms",
            "10 ms", "20 ms", "50 ms",
            "0.1 sec", "0.2 sec", "0.5 sec"],
           [128, 256, 512, 1024,
            2048, 4096, 8192, 16384,
            32768, 65536, 131072, 262144])
  
// AppDelegate
@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{
    let kToolbar = "toolbar"

    @IBOutlet weak var window: NSWindow!

    var menu: NSMenu!
    var toolbar: NSToolbar!
    var delegate: ToolbarDelegate!

    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application
        window.setContentSize(NSMakeSize(CGFloat(kMinimumWidth),
                                         CGFloat(kMinimumHeight)))
        window.contentMinSize = NSMakeSize(CGFloat(kMinimumWidth),
                                           CGFloat(kMinimumHeight))
        window.contentMaxSize = NSMakeSize(CGFloat(kMaximumWidth),
                                           CGFloat(kMaximumHeight))
        window.collectionBehavior.insert(.fullScreenNone)

        // Find the menu
        menu = NSApp.mainMenu

        // Toolbar
        toolbar = NSToolbar(identifier: NSToolbar.Identifier(kToolbar))
        delegate = ToolbarDelegate()
        toolbar.delegate = delegate
        toolbar.displayMode = .iconOnly
        window.toolbar = toolbar
        NSLog("Items", toolbar.items);

        // Views
        scopeView = ScopeView()
        xScaleView = XScaleView()
        yScaleView = YScaleView()

        xScaleView.autoresizingMask = .width
        yScaleView.autoresizingMask = .height

        let hStack = NSStackView(views: [yScaleView, scopeView])
        hStack.spacing = 0
        let stack = NSStackView(views: [hStack, xScaleView])
        stack.orientation = .vertical
        stack.spacing = 0
        window.contentView = stack
        window.makeKeyAndOrderFront(self)
        window.makeFirstResponder(scopeView)
        window.makeMain()

        SetupAudio()
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

    // applicationWillTerminate
    func applicationWillTerminate(_ aNotification: Notification)
    {
        // Insert code here to tear down your application
    }

    // ToolbarDelegate
    class ToolbarDelegate: NSObject, NSToolbarDelegate
    {
        let bright = NSToolbarItem.Identifier("bright")
        let single = NSToolbarItem.Identifier("single")
        let trigger = NSToolbarItem.Identifier("trigger")
        let time = NSToolbarItem.Identifier("timebase")
        let storage = NSToolbarItem.Identifier("storage")
        let clear = NSToolbarItem.Identifier("clear")
        let left = NSToolbarItem.Identifier("left")
        let right = NSToolbarItem.Identifier("right")
        let start = NSToolbarItem.Identifier("start")
        let end = NSToolbarItem.Identifier("end")
        let reset = NSToolbarItem.Identifier("reset")

        // toolbar
        func toolbar(_ toolbar: NSToolbar, 
                     itemForItemIdentifier itemIdentifier:
                       NSToolbarItem.Identifier, 
                     willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem?
        {
            let item = NSToolbarItem(itemIdentifier: itemIdentifier)
            // NSLog("Toolbar item")
            switch itemIdentifier
            {
            case bright:
                item.label = "Bright line"
                item.toolTip = "Bright line"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_important")
                item.image = image
                return item

            case single:
                item.label = "Single shot"
                item.toolTip = "Single shot"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_flash_on")
                item.image = image
                return item

            case trigger:
                item.label = "Trigger"
                item.toolTip = "Trigger"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_repeat")
                item.image = image
                return item

            case time:
                item.label = "Timebase"
                item.toolTip = "Timebase"
                item.target = self
                item.action = #selector(timebaseClicked)
                let image = getImage("ic_action_time")
                item.image = image
                return item

            case storage:
                item.label = "Storage"
                item.toolTip = "Storage"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_storage")
                item.image = image
                return item

            case clear:
                item.label = "Clear"
                item.toolTip = "Clear"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_refresh")
                item.image = image
                return item

            case left:
                item.label = "Left"
                item.toolTip = "Left"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_previous_item")
                item.image = image
                return item

            case right:
                item.label = "Right"
                item.toolTip = "Right"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_next_item")
                item.image = image
                return item

            case start:
                item.label = "Start"
                item.toolTip = "Start"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_back")
                item.image = image
                return item

            case end:
                item.label = "End"
                item.toolTip = "End"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_forward")
                item.image = image
                return item

            case reset:
                item.label = "Reset"
                item.toolTip = "Reset"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_action_undo")
                item.image = image
                return item

            default:
                break
            }

            return nil
        }

        // toolbarAllowedItemIdentifiers
        func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) ->
          [NSToolbarItem.Identifier]
        {
            NSLog("Allowed")
            return [bright, single, trigger, time,
                    storage, clear, left, right, start,
                    end, reset]
        }

        // toolbarDefaultItemIdentifiers
        func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) ->
          [NSToolbarItem.Identifier]
        {
            NSLog("Default")
            return [bright, single, trigger, time,
                    storage, clear, left, right, start,
                    end, reset]
        }

        // toolbarSelectableItemIdentifiers
        func toolbarSelectableItemIdentifiers(_ toolbar: NSToolbar) ->
          [NSToolbarItem.Identifier]
        {
            NSLog("Selectable")
            return [bright, single, storage]
        }

        // getImage
        func getImage(_ image: String) -> NSImage?
        {
            let main = Bundle.main
            let url = main.url(forResource: image,
                               withExtension: "png",
                               subdirectory: "Icons")
            return NSImage(contentsOf: url ??
                             URL(fileReferenceLiteralResourceName: ""))
        }

        // toolbarClicked
        @objc func toolbarClicked(sender: NSToolbarItem)
        {
            switch sender.itemIdentifier
            {
            case bright:
                scope.bright = !scope.bright
                break
            case single:
                scope.single = !scope.single
                break
            case trigger:
                scope.trigger = true
                break
            case storage:
                scope.storage = !scope.storage
                break
            case clear:
                scope.clear = true
                break

            case left:
                scope.start -= xscale.step
                if (scope.start < 0)
                {
                    scope.start = 0
                }
                xscale.start = scope.start
                xScaleView.needsDisplay = true
                break

            case right:
                scope.start += xscale.step
                if (scope.start >= scope.length)
                {
                    scope.start -= xscale.step
                }
                xscale.start = scope.start
                xScaleView.needsDisplay = true
                break

            case start:
	        scope.start = 0
	        scope.index = 0
	        xscale.start = 0
	        yscale.index = 0

                xScaleView.needsDisplay = true
                yScaleView.needsDisplay = true
                break

            case end:
                while (scope.start < scope.length)
                {
		    scope.start += xscale.step
                }
	        scope.start -= xscale.step
	        xscale.start = scope.start
                xScaleView.needsDisplay = true
                break

            case reset:
                scope.index = 0
	        scope.start = 0

	        scope.bright = false
	        scope.single = false
	        scope.storage = false

	        xscale.start = 0
	        yscale.index = 0

                xScaleView.needsDisplay = true
                yScaleView.needsDisplay = true
                break

            default:
                break
            }
        }

        // timebaseClicked
        @objc func timebaseClicked(sender: NSToolbarItem)
        {
            let menu = NSMenu(title: "Timebase")
            for string in timebase.strings
            {
                let item = menu.addItem(withTitle: string,
                                        action: #selector(timebaseChanged),
                                        keyEquivalent: "")
                item.target = self
                item.isEnabled = true
            }

            let location = window.mouseLocationOutsideOfEventStream
            menu.popUp(positioning: nil, at: location, in: window.contentView!)
        }

        // timebaseChanged
        @objc func timebaseChanged(sender: NSMenuItem)
        {
            timebase.index = Int(sender.menu!.index(of: sender))
            scope.scale = timebase.values[timebase.index]
	    xscale.scale = timebase.values[timebase.index]
	    xscale.step = Int32(500 * xscale.scale)
            xScaleView.needsDisplay = true
        }
    }
}

