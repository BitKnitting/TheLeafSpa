//
//  ViewController.swift
//  BLELeafSpaTest
//
//  Created by Margaret Johnson on 4/23/17.
//  Copyright © 2017 Margaret Johnson. All rights reserved.
//

import UIKit

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        BleManager.sharedInstance.startScan()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

