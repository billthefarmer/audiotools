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

// Apparently these aren't declared anywhere
@objc protocol EditMenuActions
{
    func redo(_ sender:AnyObject)
    func undo(_ sender:AnyObject)
}

struct Timebase: Equatable, Hashable
{
    let values: [Float]
    let strings: [String]
    let counts: [Int]
    var index: Int!

    init(_ values: [Float],_ strings: [String],_ counts: [Int])
    {
        self.values = values
        self.strings = strings
        self.counts = counts
    }
}

var timebase =
  Timebase([0.1, 0.2, 0.5, 1.0,
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
class AppDelegate: NSObject, NSApplicationDelegate
{
    let kToolbar = "toolbar"
    let kDefaultTimebase = 3

    var window: NSWindow!

    var toolbar: NSToolbar!
    var delegate: ToolbarDelegate!

    lazy var applicationName: String =
    {
	if let bundleName = Bundle.main.object(forInfoDictionaryKey:
                                                 "CFBundleDisplayName")
        {
            if let bundleNameAsString = bundleName as? String
            {
                return bundleNameAsString
	    }

	    else
            {
                Swift.print("CFBundleName not a String!")
	    }
	}

	else
        {
	    Swift.print("CFBundleName nil!")
	}

	return NSLocalizedString("Oscilloscope",
                                 comment: "The name of this application")
    }()

    // applicationWillFinishLaunching
    func applicationWillFinishLaunching(_ notification: Notification)
    {
        populateMainMenu()
    }

    // populateMainMenu
    func populateMainMenu()
    {
        let mainMenu = NSMenu(title: "MainMenu")

	// The titles of the menu items are for identification
	// purposes only and shouldn't be localized.  The strings in
	// the menu bar come from the submenu titles, except for the
	// application menu, whose title is ignored at runtime.
	var menuItem = mainMenu.addItem(withTitle: "Application",
                                        action: nil,
                                        keyEquivalent: "")
	var submenu = NSMenu(title: "Application")
	populateApplicationMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	menuItem = mainMenu.addItem(withTitle: "File", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("File", comment: "File menu"))
	populateFileMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	menuItem = mainMenu.addItem(withTitle: "Edit", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("Edit", comment: "Edit menu"))
	populateEditMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	menuItem = mainMenu.addItem(withTitle: "View", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("View", comment: "View menu"))
	populateViewMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	menuItem = mainMenu.addItem(withTitle: "Window", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("Window", comment: "Window menu"))
	populateWindowMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)
	NSApp.windowsMenu = submenu

	menuItem = mainMenu.addItem(withTitle: "Help", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("Help", comment: "View menu"))
	populateHelpMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	NSApp.mainMenu = mainMenu
    }

    // populateApplicationMenu
    func populateApplicationMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("About", comment: "About menu item") + " " + applicationName
	var menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSApplication.orderFrontStandardAboutPanel(_: )),
                                    keyEquivalent: "")
	menuItem.target = NSApp

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Services", comment: "Services menu item")
	menuItem = menu.addItem(withTitle: title, action: nil, keyEquivalent: "")
	let servicesMenu = NSMenu(title: "Services")
	menu.setSubmenu(servicesMenu, for: menuItem)
	NSApp.servicesMenu = servicesMenu

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Hide", comment: "Hide menu item") + " " + applicationName
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSApplication.hide(_: )),
                                keyEquivalent: "h")
	menuItem.target = NSApp

	title = NSLocalizedString("Hide Others", comment: "Hide Others menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSApplication.hideOtherApplications(_: )),
                                keyEquivalent: "h")
	menuItem.keyEquivalentModifierMask = [.command, .option]
	menuItem.target = NSApp

	title = NSLocalizedString("Show All", comment: "Show All menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSApplication.unhideAllApplications(_: )),
                                keyEquivalent: "")
	menuItem.target = NSApp

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Quit", comment: "Quit menu item") + " " + applicationName
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSApplication.terminate(_: )),
                                keyEquivalent: "q")
	menuItem.target = NSApp
    }

    // populateFileMenu
    func populateFileMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Print…", comment: "Print menu item")
	menu.addItem(withTitle: title,
                     action: #selector(print),
                     keyEquivalent: "p")

	menu.addItem(NSMenuItem.separator())

        title = NSLocalizedString("Close Window", comment: "Close Window menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSWindow.performClose(_: )),
                     keyEquivalent: "w")
    }

    // populateEditMenu
    func populateEditMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Undo", comment: "Undo menu item")
	menu.addItem(withTitle: title,
                     action: #selector(EditMenuActions.undo(_: )),
                     keyEquivalent: "z")

	title = NSLocalizedString("Redo", comment: "Redo menu item")
	menu.addItem(withTitle: title,
                     action: #selector(EditMenuActions.redo(_: )),
                     keyEquivalent: "Z")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Cut", comment: "Cut menu item")
	menu.addItem(withTitle: title, action: #selector(NSText.cut(_: )), keyEquivalent: "x")

	title = NSLocalizedString("Copy", comment: "Copy menu item")
	menu.addItem(withTitle: title, action: #selector(NSText.copy(_: )), keyEquivalent: "c")

	title = NSLocalizedString("Paste", comment: "Paste menu item")
	menu.addItem(withTitle: title, action: #selector(NSText.paste(_: )), keyEquivalent: "v")

	title = NSLocalizedString("Paste and Match Style",
                                  comment: "Paste and Match Style menu item")
	var menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSTextView.pasteAsPlainText(_: )),
                                    keyEquivalent: "V")
	menuItem.keyEquivalentModifierMask = [.command, .option]

	title = NSLocalizedString("Delete", comment: "Delete menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSText.delete(_: )),
                     keyEquivalent: "\u{8}") // backspace

	title = NSLocalizedString("Select All", comment: "Select All menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSText.selectAll(_: )),
                     keyEquivalent: "a")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Find", comment: "Find menu item")
	menuItem = menu.addItem(withTitle: title, action: nil, keyEquivalent: "")
	let findMenu = NSMenu(title: "Find")
	populateFindMenu(findMenu)
	menu.setSubmenu(findMenu, for: menuItem)

	title = NSLocalizedString("Spelling", comment: "Spelling menu item")
	menuItem = menu.addItem(withTitle: title, action: nil, keyEquivalent: "")
	let spellingMenu = NSMenu(title: "Spelling")
	populateSpellingMenu(spellingMenu)
	menu.setSubmenu(spellingMenu, for: menuItem)
    }

    // populateFindMenu
    func populateFindMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Find…", comment: "Find… menu item")
	var menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSResponder.performTextFinderAction(_: )),
                                    keyEquivalent: "f")
	menuItem.tag = NSTextFinder.Action.showFindInterface.rawValue

	title = NSLocalizedString("Find Next", comment: "Find Next menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSResponder.performTextFinderAction(_: )),
                                keyEquivalent: "g")
	menuItem.tag = NSTextFinder.Action.nextMatch.rawValue

	title = NSLocalizedString("Find Previous", comment: "Find Previous menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSResponder.performTextFinderAction(_: )),
                                keyEquivalent: "G")
	menuItem.tag = NSTextFinder.Action.previousMatch.rawValue

	title = NSLocalizedString("Use Selection for Find",
                                  comment: "Use Selection for Find menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSResponder.performTextFinderAction(_: )),
                                keyEquivalent: "e")
	menuItem.tag = NSTextFinder.Action.setSearchString.rawValue

	title = NSLocalizedString("Jump to Selection", comment: "Jump to Selection menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSResponder.centerSelectionInVisibleArea(_: )),
                     keyEquivalent: "j")
    }

    // populateSpellingMenu
    func populateSpellingMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Spelling…", comment: "Spelling… menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSText.showGuessPanel(_: )),
                     keyEquivalent: ": ")

	title = NSLocalizedString("Check Spelling", comment: "Check Spelling menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSText.checkSpelling(_: )),
                     keyEquivalent: ";")

	title = NSLocalizedString("Check Spelling as You Type",
                                  comment: "Check Spelling as You Type menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSTextView.toggleContinuousSpellChecking(_: )),
                     keyEquivalent: "")
    }

    // populateViewMenu
    func populateViewMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Show Toolbar", comment: "Show Toolbar menu item")
	var menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSWindow.toggleToolbarShown(_: )),
                                    keyEquivalent: "t")
	menuItem.keyEquivalentModifierMask = [.command, .option]

	title = NSLocalizedString("Customize Toolbar…", comment: "Customize Toolbar… menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSWindow.runToolbarCustomizationPalette(_: )),
                     keyEquivalent: "")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Enter Full Screen", comment: "Enter Full Screen menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSWindow.toggleFullScreen(_: )),
                                keyEquivalent: "f")
	menuItem.keyEquivalentModifierMask = [.command, .control]
    }

    // populateWindowMenu
    func populateWindowMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Minimize", comment: "Minimize menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSWindow.performMiniaturize(_: )),
                     keyEquivalent: "m")

	title = NSLocalizedString("Zoom", comment: "Zoom menu item")
	menu.addItem(withTitle: title, action: #selector(NSWindow.performZoom(_: )),
                     keyEquivalent: "")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Bring All to Front", comment: "Bring All to Front menu item")
	let menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSApplication.arrangeInFront(_: )),
                                    keyEquivalent: "")
	menuItem.target = NSApp
    }

    // populateHelpMenu
    func populateHelpMenu(_ menu: NSMenu)
    {
        let title = applicationName + " " + NSLocalizedString("Help", comment: "Help menu item") 
	let menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSApplication.showHelp(_: )),
                                    keyEquivalent: "?")
	menuItem.target = NSApp
    }

    // applicationDidFinishLaunching
    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application
        window = NSWindow(contentRect: NSMakeRect(0, 0,
                                                  CGFloat(kMinimumWidth),
                                                  CGFloat(kMinimumHeight)),
                          styleMask: [.titled, .closable,
                                      .resizable, .miniaturizable],
                          backing: .buffered,
                          defer: true)
        window.contentMinSize = NSMakeSize(CGFloat(kMinimumWidth),
                                           CGFloat(kMinimumHeight))
        window.contentMaxSize = NSMakeSize(CGFloat(kMaximumWidth),
                                           CGFloat(kMaximumHeight))
        window.collectionBehavior.insert(.fullScreenNone)
        window.title = applicationName
        window.center()

        // Toolbar
        toolbar = NSToolbar(identifier: NSToolbar.Identifier(kToolbar))
        delegate = ToolbarDelegate(self)
        toolbar.delegate = delegate
        toolbar.displayMode = .iconOnly
        window.toolbar = toolbar

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

        // Timebase
        updateTimebase(kDefaultTimebase)

        // Audio
        let status = SetupAudio()
        if (status != noErr)
        {
            displayAlert("Scope", "Audio initialisation failed", status)
        }
    }

    // DisplayAlert
    func displayAlert(_ message: String, _ informativeText: String,
                      _ status: OSStatus)
    {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = message

        let error = (status > 0) ? NSFileTypeForHFSTypeCode(OSType(status))! :
          String(utf8String: AudioUnitErrString(status))!

        alert.informativeText = String(format: "%@: %@ (%x)", informativeText,
                                       error, status)
        alert.runModal()
    }

    // updateTimebase
    func updateTimebase(_ index: Int)
    {
        timebase.index = index
        scope.length = Int32(timebase.counts[index])
        scopeView.scale = timebase.values[index]
        xScaleView.scale = scopeView.scale
        xScaleView.step = xScaleView.scale * 500
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
        ShutdownAudio()
    }

    // applicationSupportsSecureRestorableState
    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool
    {
        return true
    }

    // ToolbarDelegate
    class ToolbarDelegate: NSObject, NSToolbarDelegate
    {
        let bright = NSToolbarItem.Identifier("bright")
        let single = NSToolbarItem.Identifier("single")
        let trigger = NSToolbarItem.Identifier("trigger")
        let time = NSToolbarItem.Identifier("time")
        let storage = NSToolbarItem.Identifier("storage")
        let clear = NSToolbarItem.Identifier("clear")
        let left = NSToolbarItem.Identifier("left")
        let right = NSToolbarItem.Identifier("right")
        let start = NSToolbarItem.Identifier("start")
        let end = NSToolbarItem.Identifier("end")
        let reset = NSToolbarItem.Identifier("reset")

        var app: AppDelegate!

        // init
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
            switch itemIdentifier
            {
            case bright:
                item.label = "Bright line"
                item.toolTip = "Bright line"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_sub_black_36dp")
                item.image = image
                return item

            case single:
                item.label = "Single shot"
                item.toolTip = "Single shot"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_looks_one_black_36dp")
                item.image = image
                return item

            case trigger:
                item.label = "Trigger"
                item.toolTip = "Trigger"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_repeat_one_black_36dp")
                item.image = image
                return item

            case time:
                item.label = "Timebase"
                item.toolTip = "Timebase"
                item.target = self
                item.action = #selector(timebaseClicked)
                let image = getImage("ic_access_time_black_36dp")
                item.image = image
                return item

            case storage:
                item.label = "Storage"
                item.toolTip = "Storage"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_storage_black_36dp")
                item.image = image
                return item

            case clear:
                item.label = "Clear"
                item.toolTip = "Clear"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_clear_black_36dp")
                item.image = image
                return item

            case left:
                item.label = "Left"
                item.toolTip = "Left"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_keyboard_arrow_left_black_36dp")
                item.image = image
                return item

            case right:
                item.label = "Right"
                item.toolTip = "Right"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_keyboard_arrow_right_black_36dp")
                item.image = image
                return item

            case start:
                item.label = "Start"
                item.toolTip = "Start"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_first_page_black_36dp")
                item.image = image
                return item

            case end:
                item.label = "End"
                item.toolTip = "End"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_last_page_black_36dp")
                item.image = image
                return item

            case reset:
                item.label = "Reset"
                item.toolTip = "Reset"
                item.target = self
                item.action = #selector(toolbarClicked)
                let image = getImage("ic_undo_black_36dp")
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
            return [bright, single, trigger, time,
                    storage, clear, left, right, start,
                    end, reset]
        }

        // toolbarDefaultItemIdentifiers
        func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) ->
          [NSToolbarItem.Identifier]
        {
            return [bright, single, trigger, time,
                    storage, clear, left, right, start,
                    end, reset]
        }

        // toolbarSelectableItemIdentifiers
        func toolbarSelectableItemIdentifiers(_ toolbar: NSToolbar) ->
          [NSToolbarItem.Identifier]
        {
            return []
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
                let image = getImage(scope.bright ? "ic_sub_blue_36dp" :
                                       "ic_sub_black_36dp")
                sender.image = image
                break
            case single:
                scope.single = !scope.single
                let image = getImage(scope.single ? "ic_looks_one_blue_36dp" :
                                       "ic_looks_one_black_36dp")
                sender.image = image
                break
            case trigger:
                scope.trigger = true
                break
            case storage:
                scopeView.storage = !scopeView.storage
                let image = getImage(scopeView.storage ? "ic_storage_blue_36dp" :
                                       "ic_storage_black_36dp")
                sender.image = image
                break
            case clear:
                scopeView.clear = true
                break

            case left:
                scopeView.start -= xScaleView.step
                if (scopeView.start < 0)
                {
                    scopeView.start = 0
                }
                xScaleView.start = scopeView.start
                xScaleView.needsDisplay = true
                break

            case right:
                scopeView.start += xScaleView.step
                if (scopeView.start >= Float(scope.length))
                {
                    scopeView.start -= xScaleView.step
                }
                xScaleView.start = scopeView.start
                xScaleView.needsDisplay = true
                break

            case start:
	        scopeView.start = 0
	        scopeView.index = 0
	        xScaleView.start = 0
	        yScaleView.index = 0

                xScaleView.needsDisplay = true
                break

            case end:
                while (scopeView.start < Float(scope.length))
                {
		    scopeView.start += xScaleView.step
                }
	        scopeView.start -= xScaleView.step
	        xScaleView.start = scopeView.start
                xScaleView.needsDisplay = true
                break

            case reset:
                scopeView.index = 0
	        scopeView.start = 0

	        scope.bright = false
	        scope.single = false
	        scopeView.storage = false

	        xScaleView.start = 0
	        yScaleView.index = 0

                // Reset toolbar icons
                let toolbar = sender.toolbar!
                for item in toolbar.items
                {
                    switch item.itemIdentifier
                    {
                    case bright:
                        let image = getImage("ic_sub_black_36dp")
                        item.image = image
                        break

                    case single:
                        let image = getImage("ic_looks_one_black_36dp")
                        item.image = image
                        break

                    case storage:
                        let image = getImage("ic_storage_black_36dp")
                        item.image = image
                        break

                    default:
                        break
                    }
                }
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
                if (menu.index(of: item) == timebase.index)
                {
                    item.state = .on
                }

                else
                {
                    item.state = .off
                }
            }

            let window = app.window!
            let location = window.mouseLocationOutsideOfEventStream
            let view = window.contentView!
            menu.popUp(positioning: nil, at: location, in: view)
        }

        // timebaseChanged
        @objc func timebaseChanged(sender: NSMenuItem)
        {
            let menu = sender.menu!
            app.updateTimebase(menu.index(of: sender))
        }
    }
}

