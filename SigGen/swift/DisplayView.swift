//
//  DisplayView.swift
//  SigGen
//
//  Created by Bill Farmer on 29/03/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//

import Cocoa

class DisplayView: SigGenView
{
    var frequency: double
    {
        set
        {
            frequency = newValue
            needsDisplay = true
        }
    }

    var decibels: double
    {
        set
        {
            decibels = newValue
            needsDisplay = true
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        let textSize = height / 3

        // Drawing code here.
        NSEraseRect(rect)

	// Select font
        let font = NSFont.boldSystemFont(ofSize: textSize)
        let attribs: [NSAttributedStringKey: Any] = [.font: font]

        let x = NSMinX(rect)
        var y = NSMaxY(rect) - textSize - 2

        // Draw frequency
        let freq = String.format("%+5.2fHz", frequency)
        freq.draw(at: NSMakePoint(x, y), withAttributes: attribs)

        y += textSize

        // Draw decibels
        let db = String.format("%+6.1fdB", decibels)
        freq.draw(at: NSMakePoint(x, y), withAttributes: attribs)
    }
}
