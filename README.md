#Problem
When arriving at home, the garage door is still locked.  I have to stop in front of the closed door, take of my backpack, find the keyfob and press the key on it.  Waiting then a few seconds before the door is fully open and I can enter.

#Wish
While riding and coming near to my house, press a button on my bike so that the door opens.  By the time I arrive at the door, it's fully open and I can ride in the garage without stopping.

#Issues
* What transmitter has enough range so that its signal is received by the receiver inside the garage?
* When there's a button mounted on my bike, everyone can push it when my bike is parked outside.  How do I prevent these non-authenticated requests?
* How to prevent that someone can open the garage door when he steals the bike and reverse engineers its circuitry?
* How to prevent that someone can bond a foreign transmitter to the receiver in the garage and make the receiver think that it's a legitimate device?
* How to prevent that someone captures the RF-signal and launches a replay-attack?
* How to prevent that someone captures the RF-signal and predicts the next valid message?

#Solution
The system will consist of three parts:  

* door controller in the garage  
* key fob in my back pack 
* LF wake up initializer on my bike  
* optional NFC card  

The LF wake up initializer on the bike can be considered as a remote button on the key fob.   If the button on the LF wake up initializer is pushed, it's as if the button on the key fob is pushed.  Technically speaking: the key fob will be in sleep mode for most of the time.  Pushing the button on the LF wake up initializer will generate a low frequency field which will be used to send a wake up code to the key fob.  The key fob has a specialized circuit that waits for LF-events.  Once that circuit detects the wake up code, it will bring the key fob in active mode.  When the key fob is awake, it will wirelessly issue a command to the door controller and go to sleep again.  The key fob should have a physical button too so that it can be used when the bike is not around.
During system setup, the key fob will have to be securely bound to the garage controller.  For ease of use, compliance with future addon, prevention of man-in-the-middle (MITM) attacks, industry standard NFC will be used.  The user will use a button to put the door controller in learn mode, swipe the key fob along the controller and that's it.  
The key fob will also have a button, so that it can be used without the LF wake up initializer.  There are cases known where people leave their keyfob in their car.  Car thieves force the car door open, use the key fob to enter the house and steal the car keys there.  To prevent that situation, the Aiakos key fob will only work in close proximity with an NFC-card (which you keep in your wallet).  If you use that button a lot when you're in the car, then it's probably better to install an LF wake up receiver in your car and always take the key fob with you, instead of using the keyfob button.

#Future addon
The garage controller can be equipped with a bluetooth low energy peripheral IC.  The user could then operate the door using an application on a smartphone.  To securely authenticate the smartphone to the garage controller using simple secure pairing (SSP), some out of band information (OOB) would be needed: NFC, (IrDa).  Also numeric comparison (NC) or pass key (PK) could be used to authenticate the garage controller, but it would require that the garage controller has a 6 digit display and/or a key pad.

* [Garage controller](../../wiki/Master:-garage-controller)


