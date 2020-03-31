//
//  SpectrumView.swift
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

class SpectrumView: LMSView
{
    var max: CGFloat = 0

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        NSBezierPath.fill(rect)

        let darkGreen = NSColor(red: 0, green: 0.25, blue: 0, alpha: 1.0)
        darkGreen.set()

        let context = NSGraphicsContext.current!
        context.shouldAntialias = false;

        for x in stride(from: rect.minX, to: rect.maxX, by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(x, rect.minY),
                                    to: NSMakePoint(x, rect.maxY))
        }

        for y in stride(from: rect.minY, to: rect.maxY, by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(rect.minX, y),
                                    to: NSMakePoint(rect.maxX, y))
        }

        if (spectrum.data == nil)
        {
            return
        }

        if (max < 1)
        {
	    max = 1
        }

        // Calculate the scaling
        let yscale = height / max
        let xscale = CGFloat(spectrum.length) / width;

        max = 0

        // Green trace
        NSColor.green.setStroke()
        // Transparent green fill
        NSColor(red: 0, green: 1, blue: 0, alpha: 0.25).setFill()

        var last = 1
        let path = NSBezierPath()
        path.move(to: NSPoint.zero)
        for x in 0 ... width
        {
            value: CGFloat = 0

            let index = Int(round(pow(CGFloat.e, x * xscale)))
            for i in last ... index
            {
                if (i > 0 && i <spectrum.length)
                {
                    if (value < spectrum.data[i])
                    {
                        value = spectrum.data[i]
                    }
                }
            }

	    // Update last index
	    last = index;

	    if (max < value)
	    max = value;

            let y = value * yscale
            path.line(to: NSMakePoint(x, y))
        }

        path.stroke()
        path.line(to: NSMakePoint(width, 0))
        path.close()
        path.fill()
    }
}
