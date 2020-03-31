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

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{

    @IBOutlet weak var window: NSWindow!


    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application

        window.contentMinSize = NSMakeSize(CGFloat(kMinWidth),
                                           CGFloat(kMinHeight))
        window.contentAspectRatio = NSMakeSize(CGFloat(kMinWidth),
                                               CGFloat(kMinHeight))
        window.collectionBehavior.insert(.fullScreenNone)

        window.title = "Level Measuring Set"

        // Views
        displayView = DisplayView()
        meterView = MeterView()
        spectrumView = SpectrumView()

        // Stack
        let stack = NSStackView(views: [displayView, meterView, spectrumView])
        stack.orientation = .vertical
        stack.spacing = 8
        stack.edgeInsets = NSEdgeInsetsMake(20, 20, 20, 20)
        let spectrumHeight = NSLayoutConstraint(item: spectrumView,
                                                attribute: .height,
                                                relatedBy: .equal,
                                                toItem: displayView,
                                                attribute: .height,
                                                multiplier: 1,
                                                constant: 0)
        stack.addConstraint(spectrumHeight)
        let meterHeight = NSLayoutConstraint(item: meterView,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: displayView,
                                             attribute: .height,
                                             multiplier: 1,
                                             constant: 0)
        stack.addConstraint(meterHeight)

        // Window
        window.contentView = stack
        window.makeKeyAndOrderFront(self)
        window.makeMain()
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


}

