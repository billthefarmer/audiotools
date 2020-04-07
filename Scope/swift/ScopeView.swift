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

// ScopeView
class ScopeView: NSView
{
    var size = NSZeroSize
    var bitmap: CGContext!
    var graticule: CGContext!
    var image: CGImage!

    var index = Float(0)
    {
        didSet
        {
            needsDisplay = true
        }
    }

    @objc var start = Float(0)
    {
        didSet
        {
            needsDisplay = true
        }
    }

    @objc var step = Float(0)
    {
        didSet
        {
            needsDisplay = true
        }
    }

    @objc var scale = Float(0)
    {
        didSet
        {
            needsDisplay = true
        }
    }

    @objc var storage = false
    {
        didSet
        {
            needsDisplay = true
        }
    }

    @objc var clear = false
    {
        didSet
        {
            needsDisplay = true
        }
    }

    var max = Float(0)

    let kMinMax: Float = 0.125
    let kTextSize: CGFloat = 8

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            let location = event.locationInWindow
            let point = convert(location, from: nil)
            index = Float(point.x)
            needsDisplay = true;
        }
    }

    // This, IMHO is a kludge because you ought to be able to do this
    // in AppDelegate rather than one of the views
    // keyDown
    override func keyDown(with event: NSEvent)
    {
        let code = event.keyCode

        switch code
        {
        case UInt16(kKeyboardUpKey):
            yScaleView.index += 1
            if yScaleView.index > (size.height / 2)
            {
                yScaleView.index = (size.height / 2)
            }
            break
        case UInt16(kKeyboardDownKey):
            yScaleView.index -= 1
            if yScaleView.index < -(size.height / 2)
            {
                yScaleView.index = -(size.height / 2)
            }
            break
        case UInt16(kKeyboardRightKey):
            index += 1
            if index > Float(size.width)
            {
                index = Float(size.width)
            }
            needsDisplay = true;
            break
        case UInt16(kKeyboardLeftKey):
            index -= 1
            if index < 0
            {
                index = 0
            }
            needsDisplay = true;
            break
        default:
            NSLog("Code %d", code)
            break
        }
    }

    // draw
    override func draw(_ rect: NSRect)
    {
        super.draw(rect)

        // Drawing code here.
        let width = rect.width
        let height = rect.height

        let context = NSGraphicsContext.current!

        if (size.width != width || size.height != height)
        {
            size = rect.size

            let colour = CGColorSpace(name: CGColorSpace.sRGB)!
            bitmap =
              CGContext(data: nil,
                        width: Int(width),
                        height: Int(height),
                        bitsPerComponent: 8,
                        bytesPerRow: Int(width * 4),
                        space: colour,
                        bitmapInfo:
                          CGImageAlphaInfo.premultipliedLast.rawValue)!

            graticule =
              CGContext(data: nil,
                        width: Int(width),
                        height: Int(height),
                        bitsPerComponent: 8,
                        bytesPerRow: Int(width * 4),
                        space: colour,
                        bitmapInfo:
                          CGImageAlphaInfo.premultipliedLast.rawValue)!

            graticule.setFillColor(CGColor.black)
            graticule.fill(rect)

            // Dark green graticule
            let darkGreen = CGColor(red: 0, green: 0.125, blue: 0, alpha: 1.0)
            graticule.setStrokeColor(darkGreen)

            // Move the origin
            graticule.translateBy(x: 0, y: rect.midY)
            graticule.setShouldAntialias(false)

            // Draw graticule
            graticule.beginPath()
            for x in stride(from: 0, to: rect.width, by: 10)
            {
                graticule.move(to: NSMakePoint(x, rect.maxY / 2))
                graticule.addLine(to: NSMakePoint(x, -rect.maxY / 2))
            }

            for y in stride(from: 0, to: rect.height / 2, by: 10)
            {
                graticule.move(to: NSMakePoint(rect.minX, y))
                graticule.addLine(to: NSMakePoint(rect.maxX, y))
                graticule.move(to: NSMakePoint(rect.minX, -y))
                graticule.addLine(to: NSMakePoint(rect.maxX, -y))
            }

            graticule.strokePath()
            image = graticule.makeImage()
            clear = true
        }
 
        // No trace if no data
        if (scope.data == nil)
        {
	    context.cgContext.draw(image, in: rect)
	    return;
        }

        // Erase background
        if (!storage || clear)
        {
	    // Draw graticule image
	    bitmap.draw(image, in: rect)

	    clear = false
        }

        // Calculate scale etc
        let xscale = 1.0 / ((audio.sample / 100000.0) * scale)
        let xstart = Int(start)
        let xstep = Int(1.0 / Float(xscale))
        var xstop = Int(round(Float(xstart) + (Float(width) / xscale)))

        if (xstop > Int(scope.length))
        {
	    xstop = Int(scope.length)
        }

        // Calculate scale
        if (max < kMinMax)
        {
	    max = kMinMax
        }

        scope.yscale = max / Float(height / 2)
        max = 0

        // Move the origin
        bitmap.translateBy(x: 0, y: rect.midY)

        // Green trace
        bitmap.setStrokeColor(CGColor(red: 0, green: 1, blue: 0, alpha: 1))

        // Draw the trace
        bitmap.beginPath()
        bitmap.move(to: .zero)

        if (xscale < 1.0)
        {
            for i in stride(from: 0, to: xstop, by: xstep)
            {
                if (max < abs(scope.data![xstart + i]))
                {
                    max = abs(scope.data![xstart + i])
                }

                let x = CGFloat(Float(i) * xscale)
                let y = CGFloat(scope.data![xstart + i] / scope.yscale)

                bitmap.addLine(to: NSMakePoint(x, y))
            }

            bitmap.strokePath()
        }

        else
        {
            for i in 0 ... xstop - xstart
            {
                if (max < abs(scope.data[xstart + i]))
                {
                    max = abs(scope.data[xstart + i])
                }

                let x = CGFloat(Float(i) * xscale)
                let y = CGFloat(scope.data[xstart + i] / scope.yscale)

                bitmap.addLine(to: NSMakePoint(x, y))
            }

            bitmap.strokePath()
        }

	// Draw points at maximum resolution
	if (timebase.index == 0)
	{
	    for i in 0 ... xstop - xstart
	    {
                let x = CGFloat(Float(i) * xscale)
                let y = CGFloat(scope.data[xstart + i] / scope.yscale)

		bitmap.stroke(NSMakeRect(x - 2, y - 2, 4, 4))
	    }
	}

        if (index > 0 && !storage)
        {
            let context = NSGraphicsContext(cgContext: bitmap, flipped: false)
            NSGraphicsContext.current = context;
            // Yellow trace
            NSColor.yellow.set()
            // Draw cursor
            NSBezierPath.strokeLine(from: NSMakePoint(CGFloat(index),
                                                      -height / 2),
                                    to: NSMakePoint(CGFloat(index),
                                                    height / 2))
            // Yellow text
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attrs: [NSAttributedString.Key: Any] =
              [.font: font, .foregroundColor: NSColor.yellow]
            let i = Int(Float(index) / xscale)
            let s = String(format: "%0.3f", scope.data[xstart + i])
            let size = s.size(withAttributes: attrs)
            let y = CGFloat(scope.data[xstart + i] / scope.yscale)
            s.draw(at: NSMakePoint(CGFloat(index) - size.width / 2, y),
                   withAttributes: attrs)

            if (scale < 100)
            {
                let s = String(format: (scale < 1) ? "%0.3f": 
		    (scale < 10.0) ? "%0.2f": "%0.1f", (start +
		                  (index * scale)) / 100.0)
                let size = s.size(withAttributes: attrs)
                s.draw(at: NSMakePoint(CGFloat(index) - size.width / 2,
                                       -height / 2),
                       withAttributes: attrs)
            }

            else
            {
                let s = String(format: "%0.3f",
                               (start + (index * scale)) / 100000.0)
                let size = s.size(withAttributes: attrs)
                s.draw(at: NSMakePoint(CGFloat(index) - size.width / 2,
                                       -height / 2),
                       withAttributes: attrs)
            }
        }

        // Move the origin
        bitmap.translateBy(x: 0, y: -rect.midY)
        let content = bitmap.makeImage()!
        context.cgContext.draw(content, in: rect)
    }
}
