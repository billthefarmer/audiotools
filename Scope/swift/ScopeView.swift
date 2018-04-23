//
//  ScopeView.swift
//  Oscilloscope
//
//  Created by Bill Farmer on 09/04/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//

import Cocoa

class ScopeView: NSView
{

    override func draw(_ rect: NSRect)
    {
        super.draw(rect)

        // Drawing code here.
        NSBezierPath.fill(rect)

        // Dark green graticule
        let darkGreen = NSColor(red: 0, green: 0.25, blue: 0, alpha: 1.0)
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
