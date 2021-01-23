//
//  YScaleView.swift
//  Oscilloscope
//
//  Created by Bill Farmer on 09/04/2018.
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

// YScaleView
class YScaleView: NSView
{
    var size = NSZeroSize

    // intrinsicContentSize
    override var intrinsicContentSize: NSSize
    {
        get
        {
            return NSSize(width: CGFloat(kScaleWidth),
                          height: super.intrinsicContentSize.height)
        }
    }

    @objc var index = CGFloat(0)
    {
        didSet
        {
            needsDisplay = true
        }
    }

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            let location = event.locationInWindow
            let point = convert(location, from: nil)
            index = point.y - (size.height / 2)
        }
    }

    // draw
    override func draw(_ rect: NSRect)
    {
        super.draw(rect)

        // Drawing code here.
        size = rect.size

        // Move the origin
        let context = NSGraphicsContext.current!
        context.cgContext.translateBy(x: 0, y: rect.midY)
        context.shouldAntialias = false

        // Draw scale
        for y in stride(from: 0, to: rect.height / 2, by: 10)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(rect.maxX * 2 / 3, y),
                                    to: NSMakePoint(rect.maxX, y))
            NSBezierPath.strokeLine(from: NSMakePoint(rect.maxX * 2 / 3, -y),
                                    to: NSMakePoint(rect.maxX, -y))
        }

        for y in stride(from: 0, to: rect.height / 2, by: 50)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(rect.midX, y),
                                    to: NSMakePoint(rect.maxX, y))
            NSBezierPath.strokeLine(from: NSMakePoint(rect.midX, -y),
                                    to: NSMakePoint(rect.maxX, -y))
        }

        // Thumb
        context.shouldAntialias = true
        let thumb = NSBezierPath()
        thumb.move(to: NSMakePoint(-1, 1))
        thumb.line(to: NSMakePoint(1, 1))
        thumb.line(to: NSMakePoint(2, 0))
        thumb.line(to: NSMakePoint(1, -1))
        thumb.line(to: NSMakePoint(-1, -1))
        thumb.close()

        // Transform thumb
        let scale =
          AffineTransform(scale: rect.width / (thumb.bounds.width * 1.5))
        let translate =
          AffineTransform(translationByX: rect.width / 2.5,
                          byY: index)
        thumb.transform(using: scale)
        thumb.transform(using: translate)

        // Draw thumb
        if (index != 0)
        {
            thumb.fill()
        }
    }    
}
