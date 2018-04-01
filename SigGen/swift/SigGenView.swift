//
//  SigGenView.swift
//  SigGen
//
//  Created by Bill Farmer on 29/03/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//

import Cocoa

class SigGenView: NSView
{
    var rect: NSRect = .zero
    var width: CGFloat = 0
    var height: CGFloat = 0

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        rect = DrawEdge(dirtyRect)
        width = NSWidth(rect)
        height = NSHeight(rect)
    }

    // DrawEdge
    func DrawEdge(_ rect: NSRect) -> NSRect
    {
        // Save context
        NSGraphicsContext.saveGraphicsState()

        // Set colour
        NSColor.gray.set()

        // Draw edge
        let path = NSBezierPath(roundedRect: rect, xRadius: 8, yRadius: 8)
        path.lineWidth = 2
        path.stroke()

        // Restore context before clip
        NSGraphicsContext.restoreGraphicsState()

        // Create inset
        let inset = NSInsetRect(rect, 2, 2)
        NSBezierPath.clip(inset)

        return inset
    }

    // keyDown
    func keyDown(with event: NSEvent)
    {
        let key = event.characters
        switch key!.lowercased()
        {

        default:
            NSLog("Key %s", key!)
        }
    }

}
