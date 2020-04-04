//
//  MeterView.swift
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

class MeterView: LMSView
{
    @objc var level = Float(0)
    var ml = Float(0)

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        let kTextSize = height / 4
        let kMargin = width / 32

        // Drawing code here.
        NSEraseRect(rect)

        let context = NSGraphicsContext.current!

        let font = NSFont.systemFont(ofSize: kTextSize)
        var attribs: [NSAttributedString.Key: Any] = [.font: font]

        // Scale text if necessary
        let dx = "0".size(withAttributes: attribs).width
        if (dx >= (width - (kMargin * 2)) / 32)
        {
            let expansion = log(((width - (kMargin * 2)) / 32)  / dx)
            attribs = [.font: font, .expansion: expansion]
        }

        let av: [CGFloat] =
          [-40, -20, -14, -10, -8, -7, -6, -5,
           -4, -3, -2, -1, 0, 1, 2, 3]

        for v in av
        {
            let s = String(format: "%0.0f", abs(v))
            var x = pow(10, (v + 20) / 20) / 10
            x /= pow(10, 23 / 20) / 10
            x *= width - (kMargin * 2)
            x += kMargin
            let dx = s.size(withAttributes: attribs).width
            x -= dx / 2
            let y = height - kTextSize - height / 16
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
        }

        context.shouldAntialias = false

        let at: [CGFloat] =
          [-10, -5, 0, 1, 2, 3, 4, 5,
	   6, 7, 8, 9, 10, 11, 12, 13]

        for v in at
        {
            var x = pow(10, v / 10) / 10
            x /= pow(10, 23 / 20) / 10
            x *= width - (kMargin * 2)
            x += kMargin
            NSBezierPath.strokeLine(from: NSMakePoint(x, height * 11 / 16),
                                    to: NSMakePoint(x, height / 2))
        }

        for i in stride(from: 1, to: 26, by: 2)
        {
            var x = pow(10, (CGFloat(i) / 20)) / 10
            x /= pow(10, 23 / 20) / 10
            x *= width - (kMargin * 2)
            x += kMargin
            NSBezierPath.strokeLine(from: NSMakePoint(x, height * 10 / 16),
                                    to: NSMakePoint(x, height / 2))
        }

        for i in stride(from: 9, to: 48, by: 2)
        {
            var x = pow(10, (CGFloat(i) / 40)) / 10
            x /= pow(10, 23 / 20) / 10
            x *= width - (kMargin * 2)
            x += kMargin
            NSBezierPath.strokeLine(from: NSMakePoint(x, height * 9 / 16),
                                    to: NSMakePoint(x, height / 2))
        }

        // Move the origin
        context.cgContext.translateBy(x: 0, y: height / 3)
        NSColor.gray.set()

        // Draw bar
        NSBezierPath.fill(NSMakeRect(kMargin, -height / 32,
                                     width - (kMargin * 2), height / 32))
        // Gradient
        let gradient = NSGradient(colors: [NSColor.gray,
                                           NSColor.white,
                                           NSColor.gray])!
        // Thumb
        let thumb = NSBezierPath()
        thumb.move(to: NSMakePoint(0, 2))
        thumb.line(to: NSMakePoint(1, 1))
        thumb.line(to: NSMakePoint(1, -2))
        thumb.line(to: NSMakePoint(-1, -2))
        thumb.line(to: NSMakePoint(-1, 1))
        thumb.close()

        // Do calculation
        ml = ((ml * 7) + level) / 8

        // Transform
        let scale = AffineTransform(scale: height / 16)
        let translate =
          AffineTransform(translationByX: kMargin + CGFloat(ml) *
                            (width - kMargin * 2), byY: 0)
        thumb.transform(using: scale)
        thumb.transform(using: translate)
        context.shouldAntialias = true;
        gradient.draw(in: thumb, angle: 90)
        thumb.stroke()
    }
}
