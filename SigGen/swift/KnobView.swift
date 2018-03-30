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

class KnobView: NSControl
{

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        let indexSize = width / 32
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

        let indent = NSMakeRect(-size / 2, -size / 2, size, size)
        let indentPath = NSBezierPath(ovalIn: indent)

        let y = NSMidY(inset) - NSMidY(inset) / 4
        let transform = AffineTransform(translationByX: 0, byY: y)
        (transform as NSAffineTransform).concat()
        gradient.draw(in: indentPath, angle: 135)
    }
    
}
