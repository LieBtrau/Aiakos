from utime import sleep
import ujson
import network
import machine
try:
    import usocket as socket
except:
    import socket

# Contents of wifi-credentials.json:
# {
#     "SSID" : "SSID_of_home_wifi",
#     "PASS" : "verysafepassword"
# }
with open("wifi-credentials.json", "r") as myfile:
    data = myfile.read()
wificreds = ujson.loads(data)

def station_connect(hostname, static_ip=None):

    wlan = network.WLAN(network.STA_IF)
    if wlan.isconnected():
        print("Already connected")
        return

    wlan.active(True)
    sleep(1)
    wlan.config(
        dhcp_hostname=hostname
    )  
    wlan.connect(wificreds["SSID"], wificreds["PASS"])

    while not wlan.isconnected():
        pass
    if static_ip:
        wlan.ifconfig(static_ip)
    print("Connection successful")
    print(wlan.ifconfig())

def get_socket(port):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # Binding to all interfaces - server will be accessible to other hosts!
        ai = socket.getaddrinfo("0.0.0.0", port)
        print("Bind address info:", ai)
        addr = ai[0][-1]

        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(addr)
        s.listen(5)
        return s
    except OSError as e:
        machine.reset()
