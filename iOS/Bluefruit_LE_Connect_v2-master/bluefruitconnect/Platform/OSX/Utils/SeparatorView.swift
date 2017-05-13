//
//  SeparatorView.swift
//  Bluefruit Connect
//
//  Created by Antonio García on 01/10/15.
//  Copyright © 2015 Adafruit. All rights reserved.
//

import Cocoa

class SeparatorView: NSView {
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonSetup()
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonSetup()
    }
    
    private func commonSetup() {
        wantsLayer = true
        layer?.backgroundColor = NSColor.blackColor().colorWithAlphaComponent(0.2).CGColor
    }
    
    /*
    override func drawRect(dirtyRect: NSRect) {
        super.drawRect(dirtyRect)

        self.wantsLayer = true
        self.layer?.backgroundColor = NSColor.blackColor().colorWithAlphaComponent(0.2).CGColor
    }
*/
    
}
