//
//  ScopeView.swift
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

class ScopeView: NSView
{
    override func draw(_ rect: NSRect)
    {
        super.draw(rect)

        // Drawing code here.
        NSBezierPath.fill(rect)

        // Dark green graticule
        let darkGreen = NSColor(red: 0, green: 0.125, blue: 0, alpha: 1.0)
        darkGreen.set()

        // Move the origin
        let transform = AffineTransform(translationByX: 0, byY: NSMidY(rect))
        (transform as NSAffineTransform).concat()
        let context = NSGraphicsContext.current!
        context.shouldAntialias = false;

        // Draw graticule
        for x in stride(from: NSMinX(rect), to: NSMaxX(rect), by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(x, NSMaxY(rect) / 2),
                                    to: NSMakePoint(x, -NSMaxY(rect) / 2))
        }

        for y in stride(from: 0, to: NSHeight(rect) / 2, by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(NSMinX(rect), y),
                                    to: NSMakePoint(NSMaxX(rect), y))
            NSBezierPath.strokeLine(from: NSMakePoint(NSMinX(rect), -y),
                                    to: NSMakePoint(NSMaxX(rect), -y))
        }

        if (scope.data == nil)
        {
            return
        }
    }

}
