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
        let toolbar = NSToolbar(identifier: NSToolbar.Identifier(kToolbar))
        delegate = ToolbarDelegate(self)
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
        window.makeMain()
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

    func applicationWillTerminate(_ aNotification: Notification)
    {
        // Insert code here to tear down your application
    }

    class ToolbarDelegate: NSObject, NSToolbarDelegate
    {
        let bright = NSToolbarItem.Identifier("bright")
        let single = NSToolbarItem.Identifier("single")
        let trigger = NSToolbarItem.Identifier("trigger")
        let timebase = NSToolbarItem.Identifier("timebase")
        let storage = NSToolbarItem.Identifier("storage")
        let clear = NSToolbarItem.Identifier("clear")
        let left = NSToolbarItem.Identifier("left")
        let right = NSToolbarItem.Identifier("right")
        let start = NSToolbarItem.Identifier("start")
        let end = NSToolbarItem.Identifier("end")
        let reset = NSToolbarItem.Identifier("reset")

        var app: AppDelegate!

        init(_ app: AppDelegate)
        {
            self.app = app
        }

        // toolbar
        func toolbar(_ toolbar: NSToolbar, 
                     itemForItemIdentifier itemIdentifier:
                       NSToolbarItem.Identifier, 
                     willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem?
        {
            let item = NSToolbarItem(itemIdentifier: itemIdentifier)
            NSLog("Toolbar item")
            switch itemIdentifier
            {
            case bright:
                item.label = "Bright line"
                item.toolTip = "Bright line"
                let image = getImage("ic_action_important")
                item.image = image
                return item

            case single:
                item.label = "Single shot"
                item.toolTip = "Single shot"
                let image = getImage("ic_action_flash_on")
                item.image = image
                return item

            case trigger:
                item.label = "Trigger"
                item.toolTip = "Trigger"
                let image = getImage("ic_action_repeat")
                item.image = image
                return item

            case timebase:
                item.label = "Timebase"
                item.toolTip = "Timebase"
                let image = getImage("ic_action_time")
                item.image = image
                return item

            case storage:
                item.label = "Storage"
                item.toolTip = "Storage"
                let image = getImage("ic_action_storage")
                item.image = image
                return item

            case clear:
                item.label = "Clear"
                item.toolTip = "Clear"
                let image = getImage("ic_action_refresh")
                item.image = image
                return item

            case left:
                item.label = "Left"
                item.toolTip = "Left"
                let image = getImage("ic_action_previous_item")
                item.image = image
                return item

            case right:
                item.label = "Right"
                item.toolTip = "Right"
                let image = getImage("ic_action_next_item")
                item.image = image
                return item

            case start:
                item.label = "Start"
                item.toolTip = "Start"
                let image = getImage("ic_action_back")
                item.image = image
                return item

            case end:
                item.label = "End"
                item.toolTip = "End"
                let image = getImage("ic_action_forward")
                item.image = image
                return item

            case reset:
                item.label = "Reset"
                item.toolTip = "Reset"
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
            return [bright, single, trigger, timebase,
                    storage, clear, left, right, start,
                    end, reset]
        }

        // toolbarDefaultItemIdentifiers
        func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) ->
          [NSToolbarItem.Identifier]
        {
            NSLog("Default")
            return [bright, single, trigger, timebase,
                    storage, clear, left, right, start,
                    end, reset]
        }

        // toolbarSelectableItemIdentifiers
        func toolbarSelectableItemIdentifiers(_ toolbar: NSToolbar) ->
          [NSToolbarItem.Identifier]
        {
            return [bright, single, storage]
        }

        func getImage(_ image: String) -> NSImage?
        {
            let main = Bundle.main
            let url = main.url(forResource: image,
                               withExtension: "png",
                               subdirectory: "Icons")
            return NSImage(contentsOf: url ??
                             URL(fileReferenceLiteralResourceName: ""))
        }
    }
}

