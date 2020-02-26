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
    var bitmap = NSGraphicsContext()
    var image = NSImage()

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

        if size.width != width || size.height != height
        {
            size = rect.size

            let colour = CGColorSpace(name: CGColorSpace.sRGB)!
            bitmap = NSGraphicsContext(
              cgContext:
                CGContext(data: nil,
                          width: Int(width),
                          height: Int(height),
                          bitsPerComponent: 8,
                          bytesPerRow: Int(width * 4),
                          space: colour,
                          bitmapInfo:
                            CGImageAlphaInfo.premultipliedLast.rawValue)!,
              flipped: false)
            let graticule = NSGraphicsContext(
              cgContext:
                CGContext(data: nil,
                          width: Int(width),
                          height: Int(height),
                          bitsPerComponent: 8,
                          bytesPerRow: Int(width * 4),
                          space: colour,
                          bitmapInfo:
                            CGImageAlphaInfo.premultipliedLast.rawValue)!,
              flipped: false)
        }

        NSBezierPath.fill(rect)

        // Dark green graticule
        let darkGreen = NSColor(red: 0, green: 0.125, blue: 0, alpha: 1.0)
        darkGreen.set()

        // Move the origin
        let transform = AffineTransform(translationByX: 0, byY: NSMidY(rect))
        (transform as NSAffineTransform).concat()
        context.shouldAntialias = false

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

        NSColor.green.set()
        NSBezierPath.strokeLine(from: NSMakePoint(NSMinX(rect), 0),
                                to: NSMakePoint(NSMaxX(rect), 0))

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
    }

}
