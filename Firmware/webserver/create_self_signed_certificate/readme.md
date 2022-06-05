# Create a self-signed certificate
The created certificate can be used with the Chrome browser in Android.  It doesn't work for Firefox on Android.  There are [tutorials](https://blog.jeroenhd.nl/article/firefox-for-android-using-a-custom-certificate-authority) that explain how to force Firefox to accept custom CAs, but that didn't help in my case.

    openssl req -x509 -config req.cnf -new -nodes -out ../src/cert.pem -newkey rsa:2048 -days 3650 -extensions req_ext
* **req** : The command primarily creates and processes certificate requests in PKCS#10 format. It can additionally create self signed certificates for use as root CAs for example..
* **-x509** : this option outputs a self signed certificate instead of a certificate request. This is typically used to generate a test certificate or a self signed root CA. The extensions added to the certificate (if any) are specified in the configuration file. Unless specified using the set_serial option, a large random number will be used for the serial number. If existing request is specified with the -in option, it is converted to the self signed certificate otherwise new request is created.
* **-new** : this option generates a new certificate request. It will prompt the user for the relevant field values.The actual fields prompted for and their maximum and minimum sizes are specified in the configuration file and any requested extensions. If the -key option is not used it will generate a new RSA private key using information specified in the configuration file.
* **-nodes** : if this option is specified then if a private key is created it will not be encrypted.
* **-newkey rsa:nbits** : this option creates a new certificate request and a new private key. The argument takes one of several forms.  nbits is the number of bits, generates an RSA key nbits in size. If nbits is omitted, i.e. -newkey rsa specified, the default key size, specified in the configuration file is used.
# To review the certificate
    openssl x509 -text -noout -in ../src/cert.pem 
# Translate to a format that Android understands
    openssl x509 -inform PEM -outform DER -in ../src/cert.pem -out cert.crt
 # Import certificate in Android
 The simplest way is to put `cert.crt` on your Dropbox folder.  Open the Dropbox application in Android and click on the file.  Android will automatically install the certificate for you.
# References
*  [Linux "openssl-req" Command Line Options and Examples](https://zoomadmin.com/HowToLinux/LinuxCommand/openssl-req)
