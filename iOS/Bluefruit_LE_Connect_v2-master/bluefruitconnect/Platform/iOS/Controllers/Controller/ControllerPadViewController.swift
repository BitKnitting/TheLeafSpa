//
//  ControllerPadViewController.swift
//  Bluefruit Connect
//
//  Created by Antonio García on 12/02/16.
//  Copyright © 2016 Adafruit. All rights reserved.
//

import UIKit

protocol ControllerPadViewControllerDelegate: class {
    func onSendControllerPadButtonStatus(tag: Int, isPressed: Bool)
}

class ControllerPadViewController: UIViewController {

    //  Constants
    static let prefix = "!B"

    // UI
    @IBOutlet weak var directionsView: UIView!
    @IBOutlet weak var numbersView: UIView!
    
    // Data
    weak var delegate: ControllerPadViewControllerDelegate?
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Setup buttons targets
        for subview in directionsView.subviews {
            if let button = subview as? UIButton {
                setupButton(button)
            }
        }
        
        for subview in numbersView.subviews {
            if let button = subview as? UIButton {
                setupButton(button)
            }
        }
    }
    
    func setupButton(button: UIButton) {
        button.layer.cornerRadius = 8
        button.layer.masksToBounds = true
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.whiteColor().CGColor
        button.layer.masksToBounds = true
        
        button.setTitleColor(UIColor.lightGrayColor(), forState: .Highlighted)
        
        let hightlightedImage = UIImage(color: UIColor.darkGrayColor())
        button.setBackgroundImage(hightlightedImage, forState: .Highlighted)
        
        button.addTarget(self, action: #selector(onTouchDown(_:)), forControlEvents: .TouchDown)
        button.addTarget(self, action: #selector(onTouchUp(_:)), forControlEvents: .TouchUpInside)
        button.addTarget(self, action: #selector(onTouchUp(_:)), forControlEvents: .TouchDragExit)
        button.addTarget(self, action: #selector(onTouchUp(_:)), forControlEvents: .TouchCancel)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        // Fix: remove the UINavigationController pop gesture to avoid problems with the arrows left button
        let delayTime = dispatch_time(DISPATCH_TIME_NOW, Int64(0.1 * Double(NSEC_PER_SEC)))
        dispatch_after(delayTime, dispatch_get_main_queue()) { [unowned self] in
            
            self.navigationController?.interactivePopGestureRecognizer?.delaysTouchesBegan = false
            self.navigationController?.interactivePopGestureRecognizer?.delaysTouchesEnded = false
            self.navigationController?.interactivePopGestureRecognizer?.enabled = false
        }
    }
 

    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        
        
    }
    
    private func sendTouchEvent(tag: Int, isPressed: Bool) {
        if let delegate = delegate {
            delegate.onSendControllerPadButtonStatus(tag, isPressed: isPressed)
        }
    }
    
    // MARK: - Actions
    func onTouchDown(sender: UIButton) {
        sendTouchEvent(sender.tag, isPressed: true)
    }
    
    func onTouchUp(sender: UIButton) {
        sendTouchEvent(sender.tag, isPressed: false)
    }
    
    @IBAction func onClickHelp(sender: UIBarButtonItem) {
        let localizationManager = LocalizationManager.sharedInstance
        let helpViewController = storyboard!.instantiateViewControllerWithIdentifier("HelpViewController") as! HelpViewController
        helpViewController.setHelp(localizationManager.localizedString("controlpad_help_text"), title: localizationManager.localizedString("controlpad_help_title"))
        let helpNavigationController = UINavigationController(rootViewController: helpViewController)
        helpNavigationController.modalPresentationStyle = .Popover
        helpNavigationController.popoverPresentationController?.barButtonItem = sender
        
        presentViewController(helpNavigationController, animated: true, completion: nil)
    }
}
