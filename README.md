#Problem
When arriving at home, the garage door is still locked.  I have to stop in front of the closed door, take of my backpack, find the keyfob and press the key on it.  Waiting then a few seconds before the door is fully open and I can enter.

#Wish
While riding and coming near to my house, press a button on my bike so that the door opens.  By the time I arrive at the door, it's fully open and I can ride in the garage without stopping.

#Problem
* What transmitter has enough range so that its signal is received by the receiver inside the garage?
* When there's a button mounted on my bike, everyone can push it when my bike is parked outside.  How do I prevent these non-authenticated requests?
* How to prevent that someone can open the garage door when he steals the bike and reverse engineers its circuitry?
* How to prevent that someone can bond a foreign transmitter to the receiver in the garage and make the receiver think that it's a legitimate device?
* How to prevent that someone captures the RF-signal and launches a replay-attack?
* How to prevent that someone captures the RF-signal and predicts the next valid message?

#Solution
The system will consist of three parts: master in the garage, slave on the bike, key fob in my back pack.
When the button on the bike is pushed, it wakes up the slave.  The slave will in turn try to wake up the key fob.  When the key fob is out of range (i.e. not in my backpack), then no further action will be taken and the slave goes to sleep again.  If the key fob however has been found, then it will start communicating with the master, using the slave as communication bridge and range extender.

* [Garage controller](../../wiki/Master:-garage-controller)


