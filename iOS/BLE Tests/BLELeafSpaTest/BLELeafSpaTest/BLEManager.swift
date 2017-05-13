//
//  BLEManager.swift
//  BLELeafSpaTest
//
//  Created by Margaret Johnson on 4/23/17.
//  Copyright © 2017 Margaret Johnson. All rights reserved.
//  Discover the Adafruit BLE Feather
//  Copy a test log file from the Feather's SD card.
//

import Foundation
import CoreBluetooth

let ADAFRUIT_FEATHER_UUID = CBUUID(string:"4503D07A-6DC9-496E-B6B4-3237A6331ADD")
let ADAFRUIT_FEATHER_UUID_String = "4503D07A-6DC9-496E-B6B4-3237A6331ADD"
let c_centralManagerStateString = ["unknown","resetting","unsupported","unauthorized","poweredOff","poweredOn"]


class BleManager: NSObject, CBCentralManagerDelegate {
    static let sharedInstance = BleManager()
    private var c_centralManager =  CBCentralManager()
    private var isScanning = false
    override init() {
        super.init()
        c_centralManager = CBCentralManager(delegate: self, queue: nil, options:nil)
    }
    func startScan() {
        if isScanning{
            return
        }
        guard c_centralManager.state == .poweredOn else {
            DLog("Scan is not starting because the central manager is not powered on")
            DLog(".....current central manager state is " +  c_centralManagerStateString[c_centralManager.state.rawValue])
            return
        }
        // TODO: Todo -> Peripheral Discovery
        // I expected ADAFRUIT_FEATHER_UUID to return the Feather peripheral.  It is not.  Instead, I am
        // asking for all peripherals then within didDiscover checking if the UUID is the Feather's.
        //c_centralManager.scanForPeripherals(withServices:[ADAFRUIT_FEATHER_UUID], options: [CBCentralManagerScanOptionAllowDuplicatesKey : true])
        c_centralManager.scanForPeripherals(withServices:nil, options: nil)
        isScanning = true
        DLog("...Scanning...")
    }
    func stopScan() {
        isScanning = false
        c_centralManager.stopScan()
    }
    // MARK: centralManager delegate functions
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {

        if (peripheral.identifier.uuidString == ADAFRUIT_FEATHER_UUID_String) {
            DLog("..Found Feather")
            DLog(".......name \(String(describing: peripheral.name))")
            DLog(".......identification UUID: " + peripheral.identifier.uuidString)
        }
    }
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        DLog("---> centralManagerDidUpdateState - " +  c_centralManagerStateString[central.state.rawValue])
        if central.state == .poweredOn && isScanning == false {  //assumes want to scan if state gets into .poweredOn
            startScan()
        }
    }
    
}
