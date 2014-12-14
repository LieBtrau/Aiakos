#Main application
##Master
The unique master device is the root of the security system.  If this device has been compromised, then all other devices will be compromised  too.  So it must be kept safely.  It should also have some kind of access control such as a password or fingerprint scanner.
###Generating public/private keypair
The master uses a true random number generator to generate a unique public/private keypair for every device.  Using a truly random number generator instead of a normal one will prevent attackers from guessing private keys of the devices.  The master writes this public/private keypair to the (preferably OTP) memory of the device.
###Generating device signature
The master will read the unique (preferably read-only) serial number from the device.  This serial number and the public key of the device will be hashed, then signed with the private key of the master.
The master then writes the signature and its own public key to (preferably OTP) memory of the device.
###Conclusion
All the devices contain the following before being released to customers:

* a unique serial number
* a unique public/private keypair
* a signature signing the above mentioned data
* the public key of the master
##Remote
The remote is the device that allows owners to gain access to their property.  The device functions as a battery-operated wireless key.  Every key is unique, but multiple keys can be assigned to a single property.
The remote has a number of security features to prevent unauthorized access to the property:

* Every transmitted message should be unique.  This prevents replay-attacks.
* When a transmitted message is known, it should be virtually impossible to predict the next transmitted message.
* The serial number of the device is unique and write protected.  This prevents attackers from making copies of keys by reprogramming other remote devices.
###Functionality
* Sleep until a key is pressed
* Start the authentication algorithm to establish the symmetric secret key
* Send the command to the controller to operate the garage door
	* get serial number from ATECC108A
	* get counter value from Arduino EEPROM
	* add command byte to message
	* calculate MAC using ATECC108A

###Needed memory
* symmetric secret key (256bit = 16bytes)
* private keyring: contains private key of the remote (192bit = 24bytes)
* public keyring: 
	* CA's public key, write protected (=2*24bytes)
	* 1 certificate per verified remotey, signed by CA = 105bytes
		* unique serial number (72bit = 9bytes)
		* public key of the remote (2*24bytes)
		* signature (signs all of the forementioned using the private key of CA)=2*24bytes

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
	* serial number of transmitter (72bit = 9bytes)
	* symmetric secret key associated with that transmitter (256bit = 32bytes)
* private keyring: contains private key of the controller
* public keyring: 
	* CA's public key, write protected
	* certificate, signed by CA
		* unique serial number
		* public key of the garage controller
		* signature (signs all of the forementioned using the private key of CA)
* Info of remotes can be stored in a ["database"](http://playground.arduino.cc/Code/DatabaseLibrary).

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

#Authenticated key agreement protocol
The implementation is loosely based on the [AVR411 application note from Atmel](http://www.atmel.com/Images/Atmel-2600-AVR411-Secure-Rolling-Code-Algorithm-for-Wireless-Link_Application-Note.pdf).  The application note uses symmetric keys for establishing the local secret key.  In this implementation asymmetric keys will be used to establish the local secret key.
In the AVR411, setting up the local secret key goes out from the assumption that both devices already share a secret key.
How to get that shared secret key in these devices?  If we give all devices the same factory default shared secret key, then the compromise of one device will compromise all the others as well.  This is what's called "low resiliency".
The shared secret key is only used in setting up the local secret key between two distinct devices.  The Hoermann implementation circumvents this weakness by limiting the radiated power of the remote when setting up the local secret key.  
The AVR411 application note uses rolling code to prevent replay attacks, but the code scheme is still vulnerable to [jam-and-replay attacks](http://spencerwhyte.blogspot.be/2014/03/delay-attack-jam-intercept-and-replay.html).  In our implementation, the 2PAP authentication scheme below will be used to avoid this vulnerability.  The drawback is that the remote as well as the garage door controllers need to be transponder devices (i.e. capable of receiving and transmitting).  More messages will be sent, which will increase current consumption (especially important for the remote).
##Authentication protocol: ECDSA
![Atmel use of ECDSA](http://atmelcorporation.files.wordpress.com/2013/06/atmelencryptionkeyimage.png)
This algorithm permits to authenticate parties, but it doesn't permit to set up a shared secret key.
There's also a caveat here.  What if someone buys a legitimate key (signature will be ok), and forces a man in the middle attack?
A two factor authentication is needed (e.g. see Bluetooth Secure Simple Pairing):
* Keypad & display -> lot of hardware needed, big size, requires a lot of user interaction.
* speaker & microphone (using DTMF)? -> maybe hard to decode DTMF reliably.
* IR communication (using IrDA)? -> should be easy to implement, cheap, standard hardware, I guess.
    * 4 pin compatible devices found: 
        * [Vishay TFBS4650](http://www.vishay.com/docs/84672/tfbs4650.pdf)
        * [Lite-On HSDL-3208](http://media.digikey.com/pdf/Data%20Sheets/Lite-On%20PDFs/HSDL-3208-021.pdf)
        * [Rohm	rpm841](http://rohmfs.rohm.com/en/products/databook/datasheet/opto/irda_module/rpm841-h16.pdf)
        * [Panasonic CND0214A & CND0215A (Vio only 1.8V)](http://www.semicon.panasonic.co.jp/ds4/CND0215A_BEK_discon.pdf)
    * Implementation
        * [Reading electricity meter with IrDA](http://www.rotwang.co.uk/docs/elec_meter.ino)
        * [AVR Assembler](http://www.duris.de/psh/irdavr/irdavr.htm)
        * [TI MSP430 Implementation](http://www.ti.com/lit/an/slaa202a/slaa202a.pdf)
        * [Microchip IrDA](http://ww1.microchip.com/downloads/en/AppNotes/01071a.pdf)
    * [IrDA standard](http://web.media.mit.edu/~ayb/irx/irda/IrPHY_1_2.PDF)
* UART connection with phone jack connection -> requires a cable and two extra connectors (what about the cable when not pairing?  It will get lost, so not possible to add keys later on).
* NFC, either 13.5MHz or 125kHz. -> 125kHz is already available on one key, but it's not standard to do two way communication.  The coils also take up a lot of space.

##Key agreement protocol
* [ECDH](https://en.wikipedia.org/wiki/Elliptic_curve_Diffie%E2%80%93Hellman)
	Be careful that ECDH doesn't include authentication.  So it's vulnerable to man in the middle attacks.
* [Possibilities](http://dl.acm.org/citation.cfm?id=1303974): B-SPEKE, SRP, AMP, PAK-RY, PAK-X, SKA, LR-AKE and EC-SRP
* [Conference Paper](http://www.researchgate.net/publication/4171674_An_efficient_elliptic_curve_cryptography_based_authenticated_key_agreement_protocol_for_wireless_LAN_security)
###Authenticated key agreement protocol
####Parties & Objects
3 Parties:
* Alice (remote)
* Bob (door)
* CA (Certification authority)

Setting up a session key between Alice & Bob (Computer Networks, §8.7.5):
* Ea, Eb = public key or encryption function of that key with A's or B's public key respectively
* Ra, Rb = nonce (=big random number) generated by A or B respectively
* Ks = shared session key
* cert(x) = certificate of subject x


	A -> B : cert(B)?

A wants to get Eb

	A <- B : cert(B)

A checks with the CA's public key if cert(B) is valid 
A generates a nonce Ra
A encrypts its identity and the nonce using the public key of B.

	A -> B : Eb(A,Ra), cert(A)

B decrypts Eb(A,Ra) with its private key.
B checks with the CA's public key if cert(A) is valid.
If it is, then B generates a message for A that includes the nonce of Ra, a nonce generated by B: Rb and
a truly randomly generated secret symmetric key: Ks.  All of this will be encrypted with A's public key.

	A <- B : Ea(Ra,Rb,Ks)

A decrypts with its private key.
A checks if Ra is the same.
A encrypts Rb with the secret symmetric key.

	A -> B : Ks(Rb)

B knows now that A also got Ks

### Asymmetric key Algorithm implementation in firmware on AVR microcontrollers.
* [IETF Securing smart objects](http://tools.ietf.org/html/draft-aks-crypto-sensors-01#section-9)
* [AVR Cryptolib](https://trac.cryptolib.org/avr-crypto-lib/browser) Documentation is lacking
* [NanoEcc](https://github.com/iSECPartners/nano-ecc)
	* Very simple implementation
	* Included Arduino example
	* Generating ECC-keys offline (on computer, not on the target system): code added for that
	* secp256r1 Curve -> 10s for ECDSA sign or verify on Arduino Uno
	* secp192r1 Curve -> 4s for sign ECDSA, 4.6s for verify ECDSA on Arduino Uno
* [TinyECC for TinyOS](http://discovery.csc.ncsu.edu/software/TinyECC/)
* [µNaCl](http://munacl.cryptojedi.org/atmega.shtml)
	* High level API (hides all of the crypto math)
	* Lacks a good RNG.
* Hardware random number generator (needed to generate symmetric secret keys)
    * The ATSHA204 has a built in hardware random number generator.
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


###Algorithm implementation on specialized microcontrollers
* ATSHA204A 
    * 3V3 or 5V power supply
	* older ATSHA204 not recommended for new designs
	* securely store secrets, 
	* generate true random numbers, 
	* perform SHA-256 cryptographic hashes
	* unique non-modifiable serial number
	* Setting up a shared secret can be done with a diversified key: diversified key=MAC(rootkey, serialnumber client).  The client only stores the unique 	diversified key, not the rootkey.  Compromise of this key will only affect one unit.  The host needs to store the rootkey.  In the garage system, all hosts need to share the same root key.  Only compromising a host device will lead to problems, that's less than 50% of the devices.
	* Setup Procedure:
	   * Write config
	   * Lock configzone
	   * Write data (data zone can be freely written, no limitations apply)
	   * Lock data zone (Slot configuration settings become active)
	* Libraries:
	   * [Nuskunetworks ATSHA204 library](https://github.com/nuskunetworks/arduino_sha204)
	       * Arduino library
	       * also supports I²C-commands (ahem, at least some of them.  The creator had not fixed the 32byte message limit of the Arduino Wire library).
	   * [SparkFun ATSHA204 library](https://github.com/jimblom/sha204-Breakout/tree/master/sha204_library)
	       * Arduino library
	       * only single wire interface (SWI)
	   * [Hashlet](https://github.com/cryptotronix/hashlet)
	       * Linux driver
	       * The only library that describes writing configuration
	   * [Gist ATSHA204 library](https://gist.github.com/ghedo/6751045)
	       * Arduino library
	       * only single wire interface (SWI)
	       * Only low level library functions, no useful examples
* ATECC108-SSH Atmel secure EEPROM + CoCPU uses ECC (available at Digikey)
    * This chip implements all of the ATSHA204 commands including ECDSA commands.  This chip will not be used because we also need to create a shared secret key based on the public/private ECC-keys.  The ECDH-protocol will be used for that.  This chip doesn't support that.  It's by design impossible to read the private key from the ATECC108, which is needed for the ECDH.
	* Backward compatible to the ATSHA204 ([according to Atmel](http://atmelcorporation.wordpress.com/2013/06/27/a-closer-look-at-atmels-atecc108-asymmetric-vs-symmetric/))
	   * Not fully compatible: configuration zone size is 128bytes for ATECC108 and 88bytes for ATSHA204.  For locking the configuration zone, a CRC-calculation of that zone is needed.  This CRC-calculation depends on the size of the configuration zone.
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
	* Cryptocape
		* [Cryptocape Example implementation](https://learn.sparkfun.com/tutorials/cryptocape-hookup-guide/all)
		* [Eclet Linux driver](http://cryptotronix.com/2014/05/26/atecc108_eclet_linux_driver/)
		* [Linux I²C driver](https://github.com/cryptotronix/libcrypti2c/blob/master/crypti2c/i2c.c)
	* [Atmel ATECC108 driver](http://www.atmel.com/tools/cryptoauthentication_atecc108_development_library.aspx)
		* The driver implementation lacks descriptive information
		* Where is the 72bit serial number located and how to read it?
		* How to hash more data than 32 bytes?
		* What does it mean locking config zone and locking data zone?
		* What data is stored in the config zone? What data is stored in the data zone?
* MAXQ1103 Maxim DeepCover Secure Microcontroller with Rapid Zeroization Technology and Cryptography
	Full documentation only available under NDA.
* DS28E35
	Full documentation only available under NDA.

##Symmetric authentication protocols for control of the garage door
###Kryptoknight: 2PAP 

	A -> B : ID_A | ID_B | RANDOMNR_A+MSG
	B <- A : ID_B | ID_A | RANDOMBR_B | HMAC(ID_B, RANDOMNR_A+MSG, RANDOMNR_B)
	A -> B : ID_A | ID_B | HMAC(RANDOMNR_A, RANDOMNR_B)

	HMAC(X) = SHA(X + SECRETKEY);
[IBM Kryptoknight](http://books.google.be/books?id=GEz1sYwz494C&lpg=PA167&ots=PPK7nyTvQf&dq=2PAKDP&pg=PA166#v=onepage&q=2PAKDP&f=false
)

###CHAP
	A -> B : ID_A | ID_B | HELLO?
	B <- A : ID_B | ID_A | RANDOMBR_B
	A -> B : ID_A | ID_B | HMAC(RANDOMNR_B)

I think this is not as secure as 2PAP.

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
* 3V3 only
* RS: €6.88, Farnell: €7.63, Mouser: €7.92, Digikey: €7.95
* 915MHz band version of this module: MRF89XAM9A-I/RM (Farnell: €7.63)
* Output: +10dBm, input: -107dBm
* More results on Google than HopeRF, but HopeRF has more links to libraries etc.  Maybe Microchip is more popular for professionals, 
while HopeRF modules are more used by hobbyists.
* Below is the only useful code that could be found on the internet.  It has to be rewritten from scratch due to the Microchip copyright notice.
	* [MRF89XA.c](https://github.com/x893/Microchip/blob/master/Microchip/Transceivers/MRF89XA/MRF89XA.c)
	* [MRF89XA headers](https://github.com/x893/Microchip/tree/master/Microchip/Include/Transceivers/MRF89XA)
	* [Simple application, incomplete code](http://www.microchip.com/wwwregister/default.aspx?ReturnURL=http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en548019&DownloadDocLink=4F47C92917033199A94C14F24CE299BEDE02EDEC72CBE8C75628CEB0B60C1E3D33CAAEF0AA36F5BF667ABD973B73FF3C74BEDEAD7796DAE57B1363E0D7E380A23B83645C2BBBD0A89BA619CCE50FC019DA40DBBADBE3284C49193F5E4A42BA6FDAFB7FBE16B33D31CACDB09654A8DA367F6181875D2A4FB54C8ECBFEFB32658905B86D7BD1D2DFBAF8EDF86E048E6FDA989786C157D1CBFD2D400C9BAF6AE725)
* Hardware
	* Convert the Arduino Uno to 3V3.  Warning: The Arduino 16MHz clock doesn't run stable on 3V3.  A possibility is to lower the clock to 8MHz.  Don't forget to update your makefile with this value too.
	* Reset line of the module must be HiZ, pull up to reset the module.
	* Range of this module is comparable with the original Hörmann remote

###HOPERF RFM69W-868-S2
* 1V8 or 3V3 operation
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

#Microcontroller modules
* [Moteino](http://lowpowerlab.com/moteino/#specs)
* [Miniwireless](http://www.anarduino.com/miniwireless/)
* [Arduino Mini](http://arduino.cc/en/Main/ArduinoBoardMini)
	* Arduino Pro Mini has a smaller footprint, but a less performant MCU.  Maybe make one myself keeping best of both worlds.
	* No wireless module included.
* [panStamp AVR](http://www.panstamp.com/products/wirelessarduino)

#Miscellaneous
* [AVR protecting the internal code R+W protect](http://electronics.stackexchange.com/questions/106041/avrdude-set-lock-bits-to-avoid-reading-flash-through-spi-atmega328p).  Well, this is open source, so no much use trying to protect the firmware.  For storing keys in program area, this can be useful however.  To store keys more securely, use the ATECC108(A).


