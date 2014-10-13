#Main application
##Remote
###Functionality
* Sleep until a key is pressed
* Start the authentication algorithm to establish the symmetric secret key
* Send the command to the controller to operate the garage door
	* get serial number from ATECC108A
	* get counter value from Arduino EEPROM
	* add command byte to message
	* calculate MAC using ATECC108A
###Needed memory
* message counter
* symmetric secret key
* private keyring: contains private key of the remote
* public keyring: 
	* CA's public key, write protected
	* certificate, signed by CA
		* unique serial number
		* public key of the remote
		* signature (signs all of the forementioned using the private key of CA) 
##Garage Controller
###Functionality
* Always waiting for incoming data
* Command mode
	* Ignore learn mode messages
	* Verify MAC of incoming message
		* Find serial number of the remote in the local list of remotes
		* Get the symmetric secret key belonging to that remote
		* Check in rolling window if message counter is accepted
		* Check validity of the message by checking the MAC
		* Update the list with the message counter for that remote
		* Execute the command requested by the remote
* Learn mode
	* User puts device in learn mode
	* Ignore command mode messages
	* Erase the list of known remotes
	* Authenticate remotes and establish a symmetric secret key with them
	* Add authenticated remotes to the garage controller
	* Update counter of valid keys in EEPROM
	* Leave learn mode by user action or after timeout
###Needed memory
* number of known transmitters
* array with transmitter info
	* serial number of transmitter
	* symmetric secret key with that transmitter
	* message counter associated with that transmitter
* private keyring: contains private key of the controller
* public keyring: 
	* CA's public key, write protected
	* certificate, signed by CA
		* unique serial number
		* public key of the garage controller
		* signature (signs all of the forementioned using the private key of CA) 
#RFID
## General
The key of my bike has an attached EM4100-compatible RFID-tag.
## Hardware
The RDM630 doesn't work on 3V3, 5V is absolutely needed.
## Firmware
Checking validity of RFID: store hashed version of key value in secure EEPROM.  Upon detecting key, calculate hash of the read key
and compare with the hashed version in the EEPROM
## Links
* [Buy link](http://www.banggood.com/125KHz-EM4100-RFID-Card-Read-Module-RDM630-UART-Compatible-Arduino-p-921141.html)
* [Datasheet](http://seeedstudio.com/depot/datasheet/RDM630-Spec..pdf)
* [Source code](http://marioboehmer.blogspot.be/2011/01/rfid-with-arduino.html)

#Authentication
The implementation is based on the [AVR411 application note from Atmel](http://www.atmel.com/Images/Atmel-2600-AVR411-Secure-Rolling-Code-Algorithm-for-Wireless-Link_Application-Note.pdf).  The application note uses symmetric keys for establishing the symmetric secret key.  In this implementation asymmetric keys will be used to establish the symmetric secret key.
##Authentication protocols for establishing the symmetric secret key
###Symmetric key
####AVR-kryptoknight: 2PAP 

	A -> B : ID_A | ID_B | RANDOMNR_A+MSG
	B <- A : ID_B | ID_A | RANDOMBR_B | HMAC(ID_B, RANDOMNR_A+MSG, RANDOMNR_B)
	A -> B : ID_A | ID_B | HMAC(RANDOMNR_A, RANDOMNR_B)

	HMAC(X) = SHA1(X + SECRETKEY);
[AVR Kryptoknight](http://books.google.be/books?id=GEz1sYwz494C&lpg=PA167&ots=PPK7nyTvQf&dq=2PAKDP&pg=PA166#v=onepage&q=2PAKDP&f=false
)

####CHAP
	A -> B : ID_A | ID_B | HELLO?
	B <- A : ID_B | ID_A | RANDOMBR_B
	A -> B : ID_A | ID_B | HMAC(RANDOMNR_B)

###Conclusion
Ok, all of this above is very nice, but before all of this can happen, both devices need to share a common symmetric secret key.
How to get that symmetric secret key in these devices?  If we give all devices the same factory default symmetric secret key, then the compromise of one
device will compromise all the others as well -> low resiliency (In fact, they would only be compromised when a user is in the process of adding a key to the receiver).  This is the only case where the common key is used.

### Asymmetric key
####Algorithm implementation in firmware on AVR microcontrollers.
* [IETF Securing smart objects](http://tools.ietf.org/html/draft-aks-crypto-sensors-01#section-9)
* [AVR Cryptolib](https://trac.cryptolib.org/avr-crypto-lib/browser) Documentation is lacking
* [micro-ecc](https://github.com/myrual/micro-ecc)
* Hardware random number generator (needed to generate symmetric secret keys)
	* [Hardware random number generation on AVR](https://code.google.com/p/avr-hardware-random-number-generation/)
	* [Arduino RNG](https://gitorious.org/benediktkr/ardrand/source/db826463ec1bc96e5d073d2957fac1bc137d5b02:)
	* [Quality of RNG on Arduino](http://www.academia.edu/1161820/Ardrand_The_Arduino_as_a_Hardware_Random-Number_Generator)
* [Relic toolkit](https://github.com/relic-toolkit/relic)
	* Better documentation
	* Build Instructions Relic Toolkit


    sudo apt-get install gcc-avr avr-libc
	unzip relic-master.zip to folder relic-master
	mkdir -p relic-build/
	cd relic-build
	../relic-master/preset/avr-ecc-80k.sh ../relic-master/
	make
	make doc
	sudo make install


####Algorithm implementation on specialized microcontrollers
* ATECC108-SSH Atmel secure EEPROM + CoCPU uses ECC (available at Digikey)
	* Full documentation only available under NDA.  Full documentation is not needed for implementation.
	* EEPROM for 16keys (private key, public key, signature components)
	* possibility to lock sections of EEPROM
	* 72bit unique serial number
	* I²C interface
	* Truly random number generator
	* Lower risk for private key exposure than cryptographic calculation in standard MCU.
	* Generate private key inside the device
	* Challenge response based MAC (with symmetric keys also possible)
	* GenDig -> Generate Digest based on 1 or more keys
	* DeriveKey -> Derives a key from another one: e.g. for rolling code scheme, 
	* CheckMac
	* [Cryptocape Example implementation](https://learn.sparkfun.com/tutorials/cryptocape-hookup-guide/all)
	* [Eclet Linux driver](http://cryptotronix.com/2014/05/26/atecc108_eclet_linux_driver/)
* ATSHA204A 
	* older ATSHA204 not recommended for new designs
	* securely store secrets, 
	* generate true random numbers, 
	* perform SHA-256 cryptographic hashes
	* unique non-modifiable serial number
	* Can be used with a diversified key: diversified key=MAC(rootkey, serialnumber client).  The client needs to store this unique 	diversified key.  Compromise of this key will only affect one unit.  The host needs to store the rootkey.  In the garage system, 		all hosts need to share the same root key.  Only compromising a host device will lead to problems, that's less than 50% of the 		devices.
	* [Hashlet](https://github.com/cryptotronix/hashlet)
* MAXQ1103 Maxim DeepCover Secure Microcontroller with Rapid Zeroization Technology and Cryptography
	Full documentation only available under NDA.
* DS28E35
	Full documentation only available under NDA.

####Authentication protocol
#####Parties & Objects
3 Parties:
* Alice (remote)
* Bob (door)
* CA (Certification authority)

####Protocol
Setting up a session key between Alice & Bob (Computer Networks, §8.7.5):
* Ea, Eb = public key or encryption function of that key with A's or B's public key respectively
* Ra, Rb = nonce (=big random number) generated by A or B respectively
* Ks = shared session key
* cert(x) = certificate of subject x


	A -> B : cert(B)?

A wants to get Eb

	A <- B : cert(B)

A checks with the CA's public key if cert(B) is valid 

	A -> B : Eb(A,Ra), cert(A)

B checks with the CA's public key if cert(A) is valid.

	A <- B : Ea(Ra,Rb,Ks)
	A -> B : Ks(Rb)

B knows now that A also got Ks

#Wireless
##2.4GHz Modules
###NRF24L01+
Probably created by: [Seeedstudio](http://www.seeedstudio.com/depot/s/nRF24L01%252B%2520Module.html?search_in_description=0) and copied by [iteadstudio](http://imall.iteadstudio.com/im120606002.html)
* Chinese module from Banggood has the SI24R1 chip.  Is this NRF24L01 or NRF24L01+? Appears to be NRF24L01+ because it works also on 250Kbps.
* It seems that seeedstudio brought out an improved version with a 3V3-regulator on board.  Probably there had been some
complaints about power integrity for this module.
I added a 100nF and a 10uF elco on the supply pins of the module.
* The range is disappointing.  2m outside of my closed garage door the reception is gone.  Inside in the house, the reception is ok downstairs, but on the first floor, when I move away from the stairs, reception is gone quickly.
* NRF24: 2.4G -> 2.525G in 127 steps of 1MHz
	* Channel 100: OK
	* Channel 127 doesn't work
* Libraries
	* [TMRh20 library](https://github.com/TMRh20/RF24)
	* [Stanleyseow library](https://github.com/stanleyseow/RF24)
	* [Airspayce library](http://www.airspayce.com/mikem/arduino/NRF24/)
* Pin connection: On Arduino Due & Uno: pin 8=CE=pin 3 of RF24-module, pin 10=CSN=pin 4 of RF24-module

Hmm...Maybe 2.4GHz modules are not ideal after all.  Let's try 868MHz modules.  That's what Hoermann uses also.

###XBeePro 
* +10dBm output, -100dBm input
* Range is about 25m from indoors in the garage to the outdoor unit.

###MRF24J40MA 
* 2.4GHz, output 0dBm, input -95dBm, 400ft?
* Distributors
	* RS: €9.70
	* Farnell: €8.43, [Link](http://be.farnell.com/microchip/mrf24j40mat-i-rm/module-rf-module-2-4-ghz-ieee-802/dp/2315747) (at Farnell only available per 10pcs), [Link](http://be.farnell.com/microchip/mrf24j40ma-i-rm/rf-module-txrx-250kbps-pcb-ant/dp/1630202)
	* Mouser: €7.44

###MRF24J40MB
* 2.4GHz, output +20dBm, input -95dBm, 4000ft?
* [RS: €15](http://benl.rs-online.com/web/p/rf-transceivers/6811168/)

###RFM73-S
* Manufacturer: HopeRF
* 2.4GHz, output +3dBm, input -97dBm for 250KSPS, 70m? open air
* [RS: €3.00](http://benl.rs-online.com/web/p/telemetry-modules/7932002/)
* Libraries: 
	* [RadioHead](http://www.airspayce.com/mikem/arduino/RadioHead/index.html)
	* [Voti](http://www.voti.nl/rfm73/)
	* [Tindie](https://www.tindie.com/products/Heye/arduino-clone-with-24ghz-wireless/)

##868MHz Modules
###MRF89XAM8A-I/RM
* RS: €6.88, Farnell: €7.63, Mouser: €7.92, Digikey: €7.95
* 915MHz band version of this module: MRF89XAM9A-I/RM (Farnell: €7.63)
* Output: +10dBm, input: -107dBm
* More results on Google than HopeRF, but HopeRF has more links to libraries etc.  Maybe Microchip is more popular for professionals, 
while HopeRF modules are more used by hobbyists.
* Below is the only useful code that could be found on the internet.  It has to be rewritten from scratch due to the Microchip copyright notice.
	* [MRF89XA.c](https://github.com/x893/Microchip/blob/master/Microchip/Transceivers/MRF89XA/MRF89XA.c)
	* [MRF89XA headers](https://github.com/x893/Microchip/tree/master/Microchip/Include/Transceivers/MRF89XA)
* Hardware
	* Convert the Arduino Uno to 3V3.
	* Reset line of the module must be HiZ, pull up to reset the module.
	* Range of this module is comparable with the original Hörmann remote

###HOPERF RFM69W-868-S2 
* 868MHz, output 13dBm
* also available in 433MHz & 915MHz version, also 20dBm version available
* [RS: €7.5](http://benl.rs-online.com/web/p/telemetry-modules/7931998/)
* Probably compatible with SX1231 (currently in Hoermann remote)

###TRM-868-EUR
* Available at Mouser.be: €28,20 (expensive!)
* The module with professional 868MHz antenna hung upside down in garage (antenna vertical).
* The module in the street has a 82mm wire soldered to it.
* The range is satisfying with the factory default settings.
* Tests have been done with the low power mode (also +13dBm) to see what's the influence on the range.  Effect is considerable.
* The range is comparable to that of the original remote of the door.  In the datasheet it's not clear what's difference in 
current consumption between DTS & LP-mode.

#Miscellaneous
* [AVR protecting the internal code R+W protect](http://electronics.stackexchange.com/questions/106041/avrdude-set-lock-bits-to-avoid-reading-flash-through-spi-atmega328p).  Well, this is open source, so no much use trying to protect the firmware.  For storing keys in program area, this can be useful however.  To store keys more securely, use the ATECC108(A).


