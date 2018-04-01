//
//  KnobView.swift
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

class KnobView: NSControl
{
    var value: CGFloat = 0
    {
        didSet
        {
            needsDisplay = true
        }
    }

    // mouseDragged
    override func mouseDragged(with event: NSEvent)
    {
        if (event.type == .leftMouseDragged)
        {
            let position = event.locationInWindow

            // Convert point
            let location = convert(position, from: nil)

            // Get centre
            let centre = NSMakePoint(NSMidX(bounds), NSMidY(bounds))

            // Calculate previous location
            let prevX = location.x - event.deltaX
            let prevY = location.y - event.deltaY

            // Previous offset from centre of knob
            var x = prevX - centre.x
            var y = prevY - centre.y

            // Angle
            let theta = atan2(x, y)

            // Current offset from centre
            x = location.x - centre.x
            y = location.y - centre.y

            // Change in angle
            var change = atan2(x, y) - theta

            if (change > .pi)
            {
	        change -= 2.0 * .pi
            }

            if (change < -.pi)
            {
	        change += 2.0 * .pi
            }

            value += change / .pi
            sendAction(action, to: target)

            print("Change", change / .pi)
            print("Value", value)
        }
    }

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        let gradient = NSGradient(colors: [NSColor.white, NSColor.gray])!
        let shade = NSBezierPath(ovalIn: dirtyRect)
        gradient.draw(in: shade, angle: 315) 
        NSColor.lightGray.set()
        let inset = NSInsetRect(dirtyRect, 4, 4)
        let path = NSBezierPath(ovalIn: inset)
        path.fill()

        let centre = AffineTransform(translationByX: NSMidX(dirtyRect),
                                     byY: NSMidY(dirtyRect))
        (centre as NSAffineTransform).concat()

        let indentSize = NSMaxX(dirtyRect) / 32
        let indent = NSMakeRect(-indentSize / 2, -indentSize / 2,
                                indentSize, indentSize)
        let indentPath = NSBezierPath(ovalIn: indent)

        let y = NSMidY(inset) - NSMidY(inset) / 4
        let transform = AffineTransform(translationByX: 0, byY: y)
        (transform as NSAffineTransform).concat()
        gradient.draw(in: indentPath, angle: 135)
    }
    
}
