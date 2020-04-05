//
//  SLMSView.swift
//  SLMS
//
//  Created by Bill Farmer on 03/07/2018.
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
//

import Cocoa

class SLMSView: NSView
{
    var rect: NSRect = .zero
    var width: CGFloat = 0
    var height: CGFloat = 0

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        rect = DrawEdge(dirtyRect)
        width = rect.width
        height = rect.height
    }

    // DrawEdge
    func DrawEdge(_ rect: NSRect) -> NSRect
    {
        // Save context
        NSGraphicsContext.saveGraphicsState()

        // Set colour
        NSColor.lightGray.set()

        // Draw edge
        var path = NSBezierPath(roundedRect: rect, xRadius: 8, yRadius: 8)
        path.fill()

        // Restore context before clip
        NSGraphicsContext.restoreGraphicsState()

        // Create inset
        let inset = NSInsetRect(rect, 2, 2)
        path = NSBezierPath(roundedRect: inset, xRadius: 8, yRadius: 8)
        path.setClip()

        return inset
    }

    // keyDown
    override func keyDown(with event: NSEvent)
    {
        let code = Int(event.keyCode)
        switch code
        {
        case kLeftKey:
            knobView.value = round((knobView.value * 100) - 1) / 100

        case kRightKey:
            knobView.value = round((knobView.value * 100) + 1) / 100

        case kHomeKey:
            knobView.value = round((knobView.value * 1000) - 1) / 1000

        case kEndKey:
            knobView.value = round((knobView.value * 1000) + 1) / 1000

        default:
            NSLog("Key %x", code)
            
        }
    }
}
