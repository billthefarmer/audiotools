//
//  KnobView.swift
//  SigGen
//
//  Created by Bill Farmer on 29/03/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//

import Cocoa

class KnobView: NSView
{

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        let gradient = NSGradient(colors: [NSColor.white, NSColor.darkGray])!
        let shade = NSBezierPath(ovalIn: dirtyRect)
        gradient.draw(in: shade, angle: 315) 
        NSColor.lightGray.set()
        let inset = NSInsetRect(dirtyRect, 4, 4)
        let path = NSBezierPath(ovalIn: inset)
        path.fill()

        let centre = AffineTransform(translationByX: NSMidX(dirtyRect),
                                     byY: NSMidY(dirtyRect))
        (centre as NSAffineTransform).concat()

        let indent = NSMakeRect(-4, -4, 8, 8)
        let indentPath = NSBezierPath(ovalIn: indent)

        let y = NSMidY(inset) - NSMidY(inset) / 4
        let transform = AffineTransform(translationByX: 0, byY: y)
        (transform as NSAffineTransform).concat()
        gradient.draw(in: indentPath, angle: 135)
    }
    
}
