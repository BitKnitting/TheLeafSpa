//
//  MqttSettingsHeaderCell.swift
//  Adafruit Bluefruit LE Connect
//
//  Created by Antonio García on 30/07/15.
//  Copyright (c) 2015 Adafruit Industries. All rights reserved.
//


import UIKit


class MqttSettingsHeaderCell: UITableViewCell {
    
    // UI
    @IBOutlet weak var nameLabel: UILabel!
    @IBOutlet weak var isOnSwitch: UISwitch!
    
    // Data
    var isOnChanged : ((Bool) -> ())?
    
    @IBAction func isOnValueChanged(sender: UISwitch) {
        self.isOnChanged?(sender.on)
    }
 
    override func awakeFromNib() {
        super.awakeFromNib()
  
        // Make switch smaller
        isOnSwitch.transform = CGAffineTransformMakeScale(0.6, 0.6)
    }
}


