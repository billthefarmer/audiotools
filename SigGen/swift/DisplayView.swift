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
<<<<<<< HEAD
    var frequency: Double = 0
    {
        didSet
        {
=======
    var frequency: double
    {
        set
        {
            frequency = newValue
>>>>>>> 015a0f93e8b22abb9641e26a0445cb1d616d2724
            needsDisplay = true
        }
    }

<<<<<<< HEAD
    var decibels: Double = 0
    {
        didSet
        {
=======
    var decibels: double
    {
        set
        {
            decibels = newValue
>>>>>>> 015a0f93e8b22abb9641e26a0445cb1d616d2724
            needsDisplay = true
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

<<<<<<< HEAD
        let textSize = height / 2
=======
        let textSize = height / 3
>>>>>>> 015a0f93e8b22abb9641e26a0445cb1d616d2724

        // Drawing code here.
        NSEraseRect(rect)

	// Select font
        let font = NSFont.boldSystemFont(ofSize: textSize)
<<<<<<< HEAD
        var attribs: [NSAttributedStringKey: Any] = [.font: font]

        // Scale text if necessary
        let dx = "25000.00Hz".size(withAttributes: attribs).width
        if (dx >= width)
        {
            let expansion = log((width) / dx)
            attribs = [.font: font, .expansion: expansion]
        }
=======
        let attribs: [NSAttributedStringKey: Any] = [.font: font]
>>>>>>> 015a0f93e8b22abb9641e26a0445cb1d616d2724

        let x = NSMinX(rect)
        var y = NSMaxY(rect) - textSize - 2

        // Draw frequency
<<<<<<< HEAD
        let freq = String(format: "% 5.2fHz", frequency)
        freq.draw(at: NSMakePoint(x, y), withAttributes: attribs)

        y -= textSize

        // Draw decibels
        let db = String(format: "%+6.2fdB", decibels)
        db.draw(at: NSMakePoint(x, y), withAttributes: attribs)
=======
        let freq = String.format("%+5.2fHz", frequency)
        freq.draw(at: NSMakePoint(x, y), withAttributes: attribs)

        y += textSize

        // Draw decibels
        let db = String.format("%+6.1fdB", decibels)
        freq.draw(at: NSMakePoint(x, y), withAttributes: attribs)
>>>>>>> 015a0f93e8b22abb9641e26a0445cb1d616d2724
    }
}
