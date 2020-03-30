//
//  DisplayView.swift
//  LMS
//
//  Created by Bill Farmer on 03/07/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//
//  Created by Bill Farmer on 03/07/2018.
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
//

import Cocoa

class DisplayView: LMSView
{
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        let kTextSize = height / 2
        let kMargin = width / 32

        // Drawing code here.
        NSEraseRect(rect)

	// Select font, scale text
        let font = NSFont.boldSystemFont(ofSize: kTextSize)
        var attribs: [NSAttributedString.Key: Any] = [.font: font]

        // Scale text if necessary
        let dx = "25000.00Hz".size(withAttributes: attribs).width
        if (dx >= width / 2)
        {
            let expansion = log((width / 2)  / dx)
            attribs = [.font: font, .expansion: expansion]
        }

        let y = rect.midY - kTextSize / 2

        // Draw frequency
        let freq = String(format: "%1.2fHz", disp.frequency)
        var x = rect.minX + kMargin
        freq.draw(at: NSMakePoint(x, y), withAttributes: attribs)

        // Draw decibels
        let db = String(format: "%+1.2fdB", disp.level)
        x = rect.maxX - db.size(withAttributes: attribs).width - kMargin
        db.draw(at: NSMakePoint(x, y), withAttributes: attribs)
    }
}
