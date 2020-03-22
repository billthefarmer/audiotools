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

let kKnobMax: CGFloat = 6.8
let kKnobRef: CGFloat = 4.0
let kKnobMin: CGFloat = 0.0

class KnobView: NSControl
{
    var value = kKnobRef
    {
        didSet
        {
            if (value > kKnobMax)
            {
                value = kKnobMax
            }

            if (value < kKnobMin)
            {
                value = kKnobMin
            }

            sendAction(action, to: target)
            needsDisplay = true
        }
    }

    // mouseDragged
    override func mouseDragged(with event: NSEvent)
    {
        switch event.type
        {
        case .leftMouseDragged:

            // Convert point
            let location = convert(event.locationInWindow, from: nil)

            // Get centre
            let centre = NSMakePoint(NSMidX(bounds), NSMidY(bounds))

            // Current offset from centre
            let x = location.x - centre.x
            let y = location.y - centre.y

            // Angle
            let theta = atan2(x, y)

            // Calculate previous location
            let prevX = x - event.deltaX
            let prevY = y + event.deltaY

            // Change in angle
            var delta = theta - atan2(prevX, prevY)

            if (delta > .pi)
            {
	        delta -= 2.0 * .pi
            }

            if (delta < -.pi)
            {
	        delta += 2.0 * .pi
            }

            value += delta / .pi

            NSLog("Change %f", delta / .pi)
            NSLog("Value %f", value)

        default:
            break
        }
    }

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        let gradient = NSGradient(colors: [NSColor.white, NSColor.gray])!

        // Draw bezel
        let shade = NSBezierPath(ovalIn: dirtyRect)
        gradient.draw(in: shade, angle: 315)

        // Draw knob
        NSColor.lightGray.set()
        let inset = NSInsetRect(dirtyRect, 4, 4)
        let path = NSBezierPath(ovalIn: inset)
        path.fill()

        // Translate to centre
        let context = NSGraphicsContext.current!
        context.cgContext.translateBy(x: dirtyRect.midX, y: dirtyRect.midY)

        // Path for indent
        let indentSize = dirtyRect.maxX / 32
        let indent = NSMakeRect(-indentSize / 2, -indentSize / 2,
                                indentSize, indentSize)
        let indentPath = NSBezierPath(ovalIn: indent)

        // Translate for indent
        let indentRadius = inset.midY * 0.8
        let x = sin(value * .pi) * indentRadius
        let y = cos(value * .pi) * indentRadius
        let translate = AffineTransform(translationByX: x, byY: y)
        indentPath.translate(using: transform)

        // Draw indent
        gradient.draw(in: indentPath, angle: 135)
    }
}
