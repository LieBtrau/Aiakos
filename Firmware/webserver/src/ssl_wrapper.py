# Checking for man-in-the-middle attacks.  Comparing certificates:
# Show fingerprint of certificate on server side:
#   CLI:  openssl x509 -in cert.pem -noout -sha256 -fingerprint
#   Output: SHA256 Fingerprint=15:F5:7E:70:86:B3:CC:45:8F:A0:48:0F:20:3F:15:BA:9A:35:1E:BC:21:D8:8A:CB:AE:74:6D:9D:38:A0:07:0C
# Show fingerprint of certificate on client side:
#   CLI:  echo -n | openssl s_client -connect esp32_wifi.local:443 2>/dev/null | openssl x509  -noout -fingerprint -sha256
#   Output: SHA256 Fingerprint=15:F5:7E:70:86:B3:CC:45:8F:A0:48:0F:20:3F:15:BA:9A:35:1E:BC:21:D8:8A:CB:AE:74:6D:9D:38:A0:07:0C
# Fingerprints match: OK!

try:
    import usocket as socket
except:
    import socket
import ussl as ssl


# Generating private key & self-signed certificate in linux using: 
#   openssl req -x509 -newkey rsa:512 -keyout key.pem -out cert.pem -days 3650 -nodes -subj '/CN=esp32_wifi.local'
#   
#   Also tried ECDSA keys, but that didn't work.
# 
# Converting certificate to a format suitable for android: (https://coderwall.com/p/wv6fpq/add-self-signed-ssl-certificate-to-android-for-browsing)
#   openssl x509 -inform PEM -outform DM -in cert.pem -out cert.crt
with open('key.pem', 'rb') as f:
    key = f.read()
with open('cert.pem', 'rb') as f:
    cert = f.read()


def wrap_in_ssl_socket(s):
    # CPython uses key keyfile/certfile arguments, but MicroPython uses key/cert
    try:
        s = ssl.wrap_socket(s, server_side=True, key=key, cert=cert)
    except OSError as e:
        #   When client sees this self-signed certificate for the first time, the web browser will cause the following error message on server side
        #       OSError: (-30592, 'MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE')
        print(str(e))
        s.close()
        return s, False
    return s, True
