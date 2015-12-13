#This project will solve the following problem
When arriving at home  with my bike, I find the garage door still locked.  I have to stop in front of the closed door, take of my backpack, find the keyfob and activate it.  It takes a few seconds then before the door is fully open and I can enter.  When it's raining heavily, every second outside is a second too much.

#Wish
While riding and coming near to my house, press a button on my bike so that the door opens.  By the time I arrive at the door, it's fully open and I can ride in the garage without having to stop on the drive way.

#Solution
The system will consist of three parts:  

* door controller in the garage  
* LF wake up initializer with RFID reader on my bike  
* NFC-enabled key fob in my back pack  

##Door controller in the garage
The door controller in the garage is connected to the motor that can open and close the gate.  It will only do this when an authenticated request from a paired keyfob has been received by its wireless UHF-receiver.
During system setup, the key fob will have to be securely bound to the garage controller.  For ease of use, compliance with future addon, and prevention of man-in-the-middle (MITM) attacks, industry standard NFC will be used.  The user will use a button to put the door controller in learn mode.  To pair the keyfob, the user swipes it keyfob along the controller and that's it.  

#LF wake up initializer with RFID reader
The LF wake up initializer can generate a modulated low frequency field to wake up the keyfob when someone presses the button on the bike.
Some conditions have to be met.  The bike has to be unlocked.  The key of the bike lock is attached to a key chain that also holds an RFID tag.  Checking if the bike is unlocked is done by reading the serial number of the RFID tag on the key chain.  The LF-transmitter sends this serial number to wake up the keyfob.

#NFC enabled key fob
After the key fob has been paired with the controller in the garage it will return to a low power sleep mode.  Reception of the correct code on the LF-receiver will wake up the key fob.  The UHF-transmitter in the key fob will then automatically send a request to the garage controller to open the gate.

#Used technologies
* NFC with NDEF messaging
* 868MHz/915MHz wireless communication
* 125kHz RFID reader
* 125kHz 3D LF wake up receiver with data transmission
* ECMA authentication with ECDH
* ...

#Future addon
The garage controller can be equipped with a bluetooth low energy peripheral IC.  The user could then operate the door using an application on a smartphone.  

* [Garage controller](../../wiki/Master:-garage-controller)


