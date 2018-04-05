//
//  ScaleView.swift
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

let kScaleRef: CGFloat = 2.0

class ScaleView: SigGenView
{

    var value = kScaleRef
    {
        didSet
        {
            needsDisplay = true
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        let kTextSize = height / 3
        let kFreqScale = width
        let context = NSGraphicsContext.current!

        let font = NSFont.systemFont(ofSize: kTextSize)
        var attribs: [NSAttributedStringKey: Any] = [.font: font]

        // Scale text if necessary
        let dx = "80".size(withAttributes: attribs).width
        if (dx > width / 8)
        {
            let expansion = log((width / 8) / dx)
            attribs = [.font: font, .expansion: expansion]
        }

        // Drawing code here.
        NSEraseRect(rect)

        let transform = AffineTransform(translationByX: NSMidX(rect),
                                        byY: NSMidY(rect))
        (transform as NSAffineTransform).concat()

        let a = [1, 2, 3, 4, 6, 8]
        for i in a
        {
            var x = (kFreqScale * log10(CGFloat(i))) -
              value * kFreqScale

            for _ in 0 ... 1
            {
                var s = String(format: "%d", i)
                var offset = s.size(withAttributes: attribs).width / 2
                s.draw(at: NSMakePoint(x - offset, 0),
                       withAttributes: attribs)

                s = String(format: "%d", i * 10)
                offset = s.size(withAttributes: attribs).width / 2
                s.draw(at: NSMakePoint(x + kFreqScale - offset, 0),
                       withAttributes: attribs)
                x += 2 * kFreqScale
            }
        }

        context.shouldAntialias = false;

        for i in 1 ... 10
        {
            var x = (kFreqScale * log10(CGFloat(i))) -
              value * kFreqScale

            for _ in 0 ... 3
            {
                NSBezierPath.strokeLine(from: NSMakePoint(x, 0),
                                        to: NSMakePoint(x, -NSMidY(rect) / 2))
                x += kFreqScale
            }
        }

        for i in stride(from: 3, through: 19, by: 2)
        {
            var x = (kFreqScale * log10(CGFloat(i) / 2.0)) -
              value * kFreqScale

            for _ in 0 ... 3
            {
                NSBezierPath.strokeLine(from: NSMakePoint(x, -NSMidY(rect) / 6),
                                        to: NSMakePoint(x, -NSMidY(rect) / 2))
 	        x += kFreqScale;
            }
        }

        NSBezierPath.strokeLine(from: NSMakePoint(0, NSMidY(rect)),
                                to: NSMakePoint(0, -NSMidY(rect)))
    }
}
