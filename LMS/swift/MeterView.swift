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
    let textSize = height / 2

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        NSEraseRect(rect)

        let as: [CGFloat] =
          [-40, -20, -14, -10, -8, -7, -6, -5,
           -4, -3, -2, -1, 0, 1, 2, 3]

        let context = NSGraphicsContext.default!

        let font = NSFont.systemFont(ofSize: textSize)
        let attribs: [NSAttributedString.Key: Any] = [.font: font]
        for v in as
        {
            let s = String(format: "%d", v)
            var x = pow(10, (v + 20) / 20) / 10
            x /= pow(10, 23 / 20) / 10
            x *= width - 24
            x += 11
            let dx = s.size(withAttributes: attribs).width
            x -= dx / 2
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
            x *= width - 24
            x += 10
            NSBezierPath.strokeLine(from: NSMakePoint(x, height * 5 / 16),
                                    to: NSMakePoint(x, height / 2))
        }
    }
}
