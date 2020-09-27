# Web server to control garage door
# ---------------------------------
# mDNS is enabled, but it's not supported by Android devices.  A static IP is setup for those devices.

try:
    import usocket as socket
except:
    import socket
from ssl_wrapper import wrap_in_ssl_socket
import wifi_connect
import machine
import utils

secure_connection = True
########################################################################""
with open("pin_form.html", "r") as myfile:
    html = myfile.read()


HOSTNAME = "esp32_wifi"
HOSTPORT = 443 if secure_connection else 80
led = machine.Pin(2, machine.Pin.OUT)

def parse_headers(s):
    headers = {}
    while True:
        l = s.readline()
        if l == b"" or l == b"\r\n":
            break
        k, v = l.split(b":", 1)
        headers[k] = v.strip()
    return headers


def main():
    wifi_connect.station_connect(HOSTNAME, ('192.168.1.254', '255.255.255.0', '192.168.1.1', '8.8.8.8'))
    s = wifi_connect.get_socket(HOSTPORT)
    print(
        "Listening, connect your browser to "
        + "https://" if secure_connection else "http://"
        + HOSTNAME
        + ".local:"
        + str(HOSTPORT)
        + "/"
    )

    counter = 0

    while True:
        if gc.mem_free() < 102000:
            gc.collect()
        client_s, client_addr = s.accept()
        print("Client address:", client_addr)
        print("Client socket:", client_s)

        client_s.settimeout(3.0)
        success = True
        if secure_connection:
            client_s, success = wrap_in_ssl_socket(client_s)
        if success:
            print(client_s)
            try:
                req = str(client_s.readline())
                headers = parse_headers(client_s)
                if req:
                    if req.find("GET /?led=on") > 0:
                        print("LED ON")
                        led.value(1)
                    if req.find("GET /?led=off") > 0:
                        print("LED OFF")
                        led.value(0)
                    if req.find("POST") > 0:
                        print("Reading post variables:")
                        size = int(headers[b"Content-Length"])
                        h=client_s.read(size)
                        form = utils.parse_qs(h.decode())
                        print("Pincode: "+ form['pincode'])
                    # client_s.settimeout(None)
                    client_s.write(
                        "HTTP/1.1 200 OK\n"
                        + "Content-Type: text/html\n"
                        + "Connection: close\n\n"
                        + html #% ("OFF" if led.value == 0 else "ON")
                    )
            except Exception as e:
                print("Exception serving request:", e)
        client_s.close()
        counter += 1
        print()


main()
