//
//  DisplayView.swift
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

class DisplayView: SigGenView
{
    var frequency: Double = 0.0
    {
        didSet
        {
            needsDisplay = true
        }
    }

    var decibels: Double = 0.0
    {
        didSet
        {
            needsDisplay = true
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        let textSize = height / 2

        // Drawing code here.
        NSEraseRect(rect)

	// Select font
        let font = NSFont.boldSystemFont(ofSize: textSize)
        var attribs: [NSAttributedStringKey: Any] = [.font: font]

        // Scale text if necessary
        let dx = "25000.00Hz".size(withAttributes: attribs).width
        if (dx >= width)
        {
            let expansion = log((width) / dx)
            attribs = [.font: font, .expansion: expansion]
        }

        let x = NSMinX(rect)
        var y = NSMaxY(rect) - textSize - 2

        // Draw frequency
        let freq = String(format: "%6.2fHz", frequency)
        freq.draw(at: NSMakePoint(x, y), withAttributes: attribs)

        y -= textSize

        // Draw decibels
        let db = String(format: "%+6.2fdB", decibels)
        db.draw(at: NSMakePoint(x, y), withAttributes: attribs)
    }
}
