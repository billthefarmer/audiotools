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

// Apparently these aren't declared anywhere
@objc protocol EditMenuActions
{
    func redo(_ sender:AnyObject)
    func undo(_ sender:AnyObject)
}

// AppDelegate
class AppDelegate: NSObject, NSApplicationDelegate
{
    let kWidth = CGFloat(360)
    let kHeight = CGFloat(280)

    let kFineRef  = 0.5
    let kLevelRef = 0.75

    let kFreqVal = 1000.0
    let kLevelVal = -20.0

    var window: NSWindow!
    var stack: NSStackView!

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

	return NSLocalizedString("Signal Generator",
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

        window = NSWindow(contentRect: NSMakeRect(0, 0, kWidth, kHeight),
                          styleMask: [.titled, .closable,
                                      .resizable, .miniaturizable],
                          backing: .buffered,
                          defer: true)
        window.contentMinSize = NSMakeSize(kWidth, kHeight)
        window.contentAspectRatio = NSMakeSize(kWidth, kHeight)
        window.collectionBehavior.insert(.fullScreenNone)

        window.title = applicationName
        window.center()

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
        fineSlider.doubleValue = kFineRef
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
        audio.level = pow(10.0, kLevelVal / 20.0)

        let status = SetupAudio()
        if (status != noErr)
        {
            displayAlert("SigGen", "Audio initialisation failed", status)
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
            var decibels = (value - 1.0) * 80.0
            if (decibels < -80.0)
            {
                decibels = -80.0
            }
            displayView.decibels = decibels
            audio.level = pow(10.0, decibels / 20.0)

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

    // print
    @objc func print(sender: Any)
    {
        window.printWindow(sender)
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
}

