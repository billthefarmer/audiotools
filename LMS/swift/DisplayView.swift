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

        let textSize = height / 2

        // Drawing code here.
        NSEraseRect(rect)

	// Select font, scale text
        let font = NSFont.boldSystemFont(ofSize: textSize)
        var attribs: [NSAttributedString.Key: Any] = [.font: font]

        // Scale text if necessary
        let dx = "25000.00Hz".size(withAttributes: attribs).width
        if (dx >= width)
        {
            let expansion = log((width) / dx)
            attribs = [.font: font, .expansion: expansion]
        }

        var y = NSMaxY(rect) - textSize - 2

        // Draw frequency
        let freq = String(format: "%1.2fHz", disp.frequency)
        var x = rect.maxX - freq.size(withAttributes: attribs).width - 2
        freq.draw(at: NSMakePoint(x, y), withAttributes: attribs)

        y -= textSize

        // Draw decibels
        let db = String(format: "%+1.2fdB", disp.level)
        x = rect.maxX - db.size(withAttributes: attribs).width - 2
        db.draw(at: NSMakePoint(x, y), withAttributes: attribs)
    }
    
}
