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

    let kMaxMax: Float = 0.125

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            let location = event.locationInWindow
            let point = convert(location, from: nil)
            scope.index = Int32(point.x)
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
            yscale.index += 1
            if yscale.index > Int32(size.height / 2)
            {
                yscale.index = Int32(size.height / 2)
            }
            yScaleView.needsDisplay = true;
            break
        case UInt16(kKeyboardDownKey):
            yscale.index -= 1
            if yscale.index < -Int32(size.height / 2)
            {
                yscale.index = -Int32(size.height / 2)
            }
            yScaleView.needsDisplay = true;
            break
        case UInt16(kKeyboardRightKey):
            scope.index += 1
            if scope.index > Int32(size.width)
            {
                scope.index = Int32(size.width)
            }
            needsDisplay = true;
            break
        case UInt16(kKeyboardLeftKey):
            scope.index -= 1
            if scope.index < 0
            {
                scope.index = 0
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
        let width = NSWidth(rect)
        let height = NSHeight(rect)

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
            graticule.translateBy(x: 0, y: NSMidY(rect))
            graticule.setShouldAntialias(false)

            // Draw graticule
            graticule.beginPath()
            for x in stride(from: 0, to: NSWidth(rect), by: 10)
            {
                graticule.move(to: NSMakePoint(x, NSMaxY(rect) / 2))
                graticule.addLine(to: NSMakePoint(x, -NSMaxY(rect) / 2))
            }

            for y in stride(from: 0, to: NSHeight(rect) / 2, by: 10)
            {
                graticule.move(to: NSMakePoint(NSMinX(rect), y))
                graticule.addLine(to: NSMakePoint(NSMaxX(rect), y))
                graticule.move(to: NSMakePoint(NSMinX(rect), -y))
                graticule.addLine(to: NSMakePoint(NSMaxX(rect), -y))
            }

            graticule.strokePath()
            image = graticule.makeImage()
            scope.clear = true
        }
 
        // No trace if no data
        if (scope.data == nil)
        {
	    context.cgContext.draw(image, in: rect)
	    return;
        }

        // Erase background
        if (!scope.storage || scope.clear)
        {
	    // Draw graticule image
	    bitmap.draw(image, in: rect)

	    scope.clear = false
        }

        // Calculate scale etc
        let xscale = 1.0 / ((audio.sample / 100000.0) * scope.scale)
        let xstart = Int(scope.start)
        let xstep = Int(1.0 / Float(xscale))
        var xstop = Int(Float(xstart) + (Float(width) / xscale))

        if (xstop > Int(scope.length))
        {
	    xstop = Int(scope.length)
        }

        // Calculate scale
        if (scope.max < kMaxMax)
        {
	    scope.max = kMaxMax
        }

        scope.yscale = scope.max / Float(height / 2)
        scope.max = 0

        // Move the origin
        bitmap.translateBy(x: 0, y: NSMidY(rect))

        // Green trace
        bitmap.setStrokeColor(CGColor(red: 0, green: 1, blue: 0, alpha: 1))

        // Draw the trace
        bitmap.beginPath()
        bitmap.move(to: .zero)

        if (xscale < 1.0)
        {
            for i in stride(from: 0, to: xstop, by: xstep)
            {
                if (scope.max < abs(scope.data![xstart + i]))
                {
                    scope.max = abs(scope.data![xstart + i])
                }

                let x = CGFloat(Float(i) * xscale)
                let y = -CGFloat(scope.data![xstart + i] / scope.yscale)

                bitmap.addLine(to: NSMakePoint(x, y))
            }

            bitmap.strokePath()
        }

        else
        {
            for i in 0 ... xstop - xstart
            {
                if (scope.max < abs(scope.data[xstart + i]))
                {
                    scope.max = abs(scope.data[xstart + i])
                }

                let x = CGFloat(Float(i) * xscale)
                let y = -CGFloat(scope.data[xstart + i] / scope.yscale)

                bitmap.addLine(to: NSMakePoint(x, y))
            }

            bitmap.strokePath()
        }

	// Draw points at maximum resolution
	if (timebase.index == 0)
	{
	    for i in 0 ...  xstop - xstart
	    {
                let x = CGFloat(Float(i) * xscale)
                let y = -CGFloat(scope.data[xstart + i] / scope.yscale)

		bitmap.stroke(NSMakeRect(x - 2, y - 2, 4, 4))
	    }
	}

        if (scope.index > 0)
        {

            // Yellow trace
            bitmap.setStrokeColor(CGColor(red: 1, green: 1, blue: 0, alpha: 1))

            // Draw cursor
            bitmap.strokeLineSegments(between:
                                        [NSMakePoint(CGFloat(scope.index),
                                                     -height / 2),
                                         NSMakePoint(CGFloat(scope.index),
                                                     height / 2)])
        }                                     

        // Move the origin
        bitmap.translateBy(x: 0, y: -NSMidY(rect))
        let content = bitmap.makeImage()!
        context.cgContext.draw(content, in: rect)
        /*
        NSBezierPath.fill(rect)

        // Dark green graticule
        let darkGreen = NSColor(red: 0, green: 0.125, blue: 0, alpha: 1.0)
        darkGreen.set()

        // Move the origin
        let transform = AffineTransform(translationByX: 0, byY: NSMidY(rect))
        (transform as NSAffineTransform).concat()
        context.shouldAntialias = false

        // Draw graticule
        for x in stride(from: 0, to: NSWidth(rect), by: 10)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(x, NSMaxY(rect) / 2),
                                    to: NSMakePoint(x, -NSMaxY(rect) / 2))
        }

        for y in stride(from: 0, to: NSHeight(rect) / 2, by: 10)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(NSMinX(rect), y),
                                    to: NSMakePoint(NSMaxX(rect), y))
            NSBezierPath.strokeLine(from: NSMakePoint(NSMinX(rect), -y),
                                    to: NSMakePoint(NSMaxX(rect), -y))
        }

        NSColor.green.set()
        NSBezierPath.strokeLine(from: NSMakePoint(0, 0),
                                to: NSMakePoint(NSWidth(rect), 0))

        NSColor.yellow.set()
        if scope.index > 0
        {
            NSBezierPath.strokeLine(from: NSMakePoint(CGFloat(scope.index),
                                                      -NSMaxY(rect) / 2),
                                    to: NSMakePoint(CGFloat(scope.index),
                                                    NSMaxY(rect) / 2))
        }

        if (scope.data == nil)
        {
            return
        }
         */
    }

}
