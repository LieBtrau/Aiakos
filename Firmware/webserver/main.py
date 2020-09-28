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
import ujson


########################################################################""
with open("pin_form.html", "r") as myfile:
    html = myfile.read()
with open("door.html", "r") as myfile:
    door = myfile.read()
with open("config.json", "r") as myfile:
    data = myfile.read()
config = ujson.loads(data)

secure_connection = bool(config['secure-connection'])
HOSTPORT = int(config['secure-tcp-port'] if secure_connection else config['unsecure-tcp-port'])
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

def write_html(s, html):
    s.write(
        "HTTP/1.1 200 OK\n"
        + "Content-Type: text/html\n"
        + "Connection: close\n\n"
        + html
    )

def main():
    wifi_connect.station_connect(config['hostname'], (config['ip-address'], config['netmask'], config['default-route'], config['dns']))
    s = wifi_connect.get_socket(HOSTPORT)
    print(
        "Listening, connect your browser to "
        + ("https://" if secure_connection else "http://")
        + config['hostname']
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
                request_line = client_s.readline()
                request_line = request_line.decode()
                method, path, proto = request_line.split()
                headers = parse_headers(client_s)
                if method=="POST":
                    size = int(headers[b"Content-Length"])
                    h=client_s.read(size)
                    form = utils.parse_qs(h.decode())
                    print("Pincode: "+ form['pincode'])
                    if form['pincode'] in config['pincodes']:
                        write_html(client_s, door % 'open')
                        print('Pin OK')
                    else:
                        write_html(client_s, door % 'blijft toe')
                        print('Pin not OK')
                else:
                    write_html(client_s, html)
            except Exception as e:
                print("Exception serving request:", e)
        client_s.close()
        counter += 1
        print()


main()
