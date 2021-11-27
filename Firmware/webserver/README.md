# What is this all about?
It's an ESP32 running a webserver.  Once you typed in the correct pincode on the ESP32's webpage, a GPIO will be toggled, which will open the garagedoor.
## Security
3 layer security:

 1. The ESP32 only exists as a local device on the Wifi-network.  It's behind the firewall, so it can't be accessed from the internet.  You'll need to know the WiFi-password first.
 2. IP-traffic to the ESP32 is encrypted by TLS1.2, using a self-signed certificate
 3. The user must know the correct pin code.
## Implementation
The code is written in Micropython/CircuitPython.  The source files can be found in this repository.  To make it all work, you'll need to generate your own configuration files.  Examples of these two files follow:
### config.json
    {
	    "secure-connection": "true",
	    "hostname": "esp32_wifi",
	    "secure-tcp-port": "443",
	    "unsecure-tcp-port": "80",
	    "ip-address": "192.168.1.254",
	    "netmask": "255.255.255.0",
	    "default-route": "192.168.1.1",
	    "dns": "8.8.8.8",
	    "pincodes": [
	        "1234",
	        "4321"
	    ]
	}
### wifi-credentials.json
    {
    "SSID" : "MyWifiSSID",
    "PASS" : "MySecretWiFiPassword"
	}


