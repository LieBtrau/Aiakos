# This project will solve the following problem
When arriving at home  with my bike, I find the garage door still locked.  I have to stop in front of the closed door, take of my backpack, find the keyfob and activate it.  It takes a few seconds then before the door is fully open and I can enter.  When it's raining heavily, every second outside is a second too much.

# Wish
While riding and coming near to my house, press a button on my bike so that the door opens.  By the time I arrive at the door, it's fully open and I can ride in the garage without having to stop on the drive way.

# Proposed solutions
## Not implemented: Using smartphone
An off-the-shelf bluetooth tracker is attached to the bike.  The user has a smartphone with a wireless data connection and bluetooth low energy support.  The garage is equipped with an internet connected controller.  After binding the tracker with the smartphone, the system is ready to use.  
Pressing the button on the bluetooth tracker, wakes up the service running on the smartphone.  The smartphone uses geo-location service to check if the smartphone is in the neighbourhood of the garage.  If so, then a command is sent through the cloud to instruct the garage controller to open the door.  

## Without smartphone
The system will consist of three parts:  

* door controller in the garage  
* Wake up initializer
* BLE-enabled key fob in my back pack  

### Door controller in the garage
The door controller in the garage is connected to the motor that can open and close the gate.  It will only do this when an authenticated request from a paired keyfob has been received by its wireless UHF-receiver.
During system setup, the key fob will have to be securely bound to the garage controller.  For ease of use, compliance with future addon, and prevention of man-in-the-middle (MITM) attacks, serial communication will be used.  The user will use a button to put the door controller in learn mode.  To pair the keyfob, the user connects keyfob and the controller with an audio jack cable and that's it.  Audio jack cables and connectors are cheaper and less power hungry than NFC readers and tags.

### Wake up initializer

### Key fob
After the key fob has been paired with the controller in the garage it will return to a low power sleep mode.  Reception of the correct code on the LF-receiver will wake up the key fob.  The UHF-transmitter in the key fob will then automatically send a request to the garage controller to open the gate.

### Used technologies
* 434MHz LoRa wireless communication
* Bluetooth Low Energy
* ...

# Future addon
The garage controller can be equipped with a bluetooth low energy peripheral IC.  The user could then operate the door using an application on a smartphone.  

* [Garage controller](https://github.com/LieBtrau/Aiakos/wiki/Garage-controller)
