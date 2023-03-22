//
//  AppDelegate.swift
//  LMS
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

// Apparently these aren't declared anywhere
@objc protocol EditMenuActions
{
    func redo(_ sender:AnyObject)
    func undo(_ sender:AnyObject)
}

class AppDelegate: NSObject, NSApplicationDelegate
{
    var window: NSWindow!

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

	return NSLocalizedString("Level Measuring Set",
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

	title = NSLocalizedString("Bring All to Front",
                                  comment: "Bring All to Front menu item")
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
                                                  CGFloat(kMinWidth),
                                                  CGFloat(kMinHeight)),
                          styleMask: [.titled, .closable,
                                      .resizable, .miniaturizable],
                          backing: .buffered,
                          defer: true)
        window.contentMinSize = NSMakeSize(CGFloat(kMinWidth),
                                           CGFloat(kMinHeight))
        window.contentAspectRatio = NSMakeSize(CGFloat(kMinWidth),
                                               CGFloat(kMinHeight))
        window.collectionBehavior.insert(.fullScreenNone)
        window.title = applicationName
        window.center()

        // Views
        displayView = DisplayView()
        meterView = MeterView()
        spectrumView = SpectrumView()

        displayView.toolTip = "Frequency and level display"
        meterView.toolTip = "Level meter"
        spectrumView.toolTip = "Spectrum"

        // Stack
        let stack = NSStackView(views: [displayView, meterView, spectrumView])
        stack.orientation = .vertical
        stack.spacing = 8
        stack.edgeInsets = NSEdgeInsetsMake(20, 20, 20, 20)
        let spectrumHeight = NSLayoutConstraint(item: spectrumView as Any,
                                                attribute: .height,
                                                relatedBy: .equal,
                                                toItem: displayView as Any,
                                                attribute: .height,
                                                multiplier: 1,
                                                constant: 0)
        stack.addConstraint(spectrumHeight)
        let meterHeight = NSLayoutConstraint(item: meterView as Any,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: displayView as Any,
                                             attribute: .height,
                                             multiplier: 1,
                                             constant: 0)
        stack.addConstraint(meterHeight)

        // Window
        window.contentView = stack
        window.makeKeyAndOrderFront(self)
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

        let error = (status > 0) ? NSFileTypeForHFSTypeCode(OSType(status))! :
          String(utf8String: AudioUnitErrString(status))!

        alert.informativeText = String(format: "%@: %@ (%x)", informativeText,
                                       error, status)
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
}

