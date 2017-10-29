# This project will solve the following problem
When arriving at home  with my bike, I find the garage door still locked.  I have to stop in front of the closed door, take of my backpack, find the keyfob and activate it.  It takes a few seconds then before the door is fully open and I can enter.  When it's raining heavily, every second outside is a second too much.

# Wish
While riding and coming near to my house, press a button on my bike so that the door opens.  By the time I arrive at the door, it's fully open and I can ride in the garage without having to stop on the drive way.

# Solution
The system will consist of three parts:  

* door controller in the garage  
* BLE-enabled key fob in my back pack  
* Wake up initializer  

## Door controller in the garage
The [door controller](https://github.com/LieBtrau/Aiakos/wiki/Garage-controller) is mounted in the garage and connected to the electric motor that can open and close the door.  With its [LoRa](https://www.lora-alliance.org/) module the controller can securely communicate to the key fob.  To make the system operational, the key fob must be paired with the door controller.  The pairing is done by connecting the door controller to the key fob with an audio-jack cable and pressing the button on the key fob.  The serial connection prevents man-in-the-middle (MITM) attacks.  To prevent eavesdropping, the serial connection is secured using [ECDH](https://en.wikipedia.org/wiki/Elliptic-curve_Diffie%E2%80%93Hellman).

## Key fob 
The [key fob](https://github.com/LieBtrau/Aiakos/wiki/Key-fob) is the access token to the system and should be kept safely by the user.  The key fob's Lora module enables communication to the door controller.  The key fob will be asleep most of the time.  When it receives a valid awake command, it will instruct the door controller to open/close the garage door.  The user can wake the key fob in two ways: either by pushing the key fob button, or by pushing the button on the wake up initializer.  
The key fob incorporates a [Bluetooth Low Energy](https://en.wikipedia.org/wiki/Bluetooth_Low_Energy) module to communicate with the wake up initializer.  The key fob will only allow a paired initializer to wake it up.  Pairing involves connecting the fob to the initializer using an audio jack cable and pressing the key fob button.

## Wake up initializer


# Alternatives
## Smartphone and garage controller with WiFi
An off-the-shelf bluetooth tracker is attached to the bike.  The user has a smartphone with a wireless data connection and bluetooth low energy support.  The garage is equipped with an internet connected controller.  After binding the tracker with the smartphone, the system is ready to use.  
Pressing the button on the bluetooth tracker, wakes up the service running on the smartphone.  The smartphone uses geo-location service to check if the smartphone is in the neighbourhood of the garage.  If so, then a command is sent through WiFi to instruct the garage controller to open the door.  
## Smartphone and garage controller with WiFi
The garage controller can be equipped with a bluetooth low energy peripheral IC.  The user could then operate the door using an application on a smartphone.  

* 
