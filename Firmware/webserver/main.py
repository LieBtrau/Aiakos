# Complete project details at https://RandomNerdTutorials.com
# https://www.techcoil.com/blog/how-to-setup-micropython-on-your-esp32-development-board-to-run-python-applications/
# https://lemariva.com/blog/2019/08/micropython-vsc-ide-intellisense

from utime import sleep
import machine
import network
import ujson

try:
    import usocket as socket
except:
    import socket

led = machine.Pin(2, machine.Pin.OUT)

with open("wifi-credentials.json", "r") as myfile:
    data = myfile.read()
wificreds = ujson.loads(data)

with open("index.html", "r") as myfile:
    html = myfile.read()

def connect():
    HOSTNAME = "ESP32_WiFi" # So that the server can be found with http://ESP32_WiFi.local

    wlan = network.WLAN(network.STA_IF)
    if wlan.isconnected():
        print("Already connected")
        return

    wlan.active(True)
    sleep(1)
    wlan.config(
        dhcp_hostname=HOSTNAME
    )  
    wlan.connect(wificreds["SSID"], wificreds["PASS"])

    while not wlan.isconnected():
        pass

    print("Connection successful")
    print(wlan.ifconfig())


connect()
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("", 80))
    s.listen(5)
except OSError as e:
    machine.reset()

while True:
    try:
        if gc.mem_free() < 102000:
            gc.collect()
        conn, addr = s.accept()
        #conn.settimeout(3.0)
        print("Got a connection from %s" % str(addr))
        while True:
            h = conn.readline()
            if h == b"" or h == b"\r\n":
                break
            req=str(h)       
            print(req)
            if req.find('GET /?led=on') > 0:
                print("LED ON")
                led.value(1)
            if req.find('GET /?led=off') > 0:
                print("LED OFF")
                led.value(0)

        response = html % ("OFF" if led.value == 0 else "ON")
        # conn.settimeout(None)
        conn.send("HTTP/1.1 200 OK\n")
        conn.send("Content-Type: text/html\n")
        conn.send("Connection: close\n\n")
        conn.sendall(response)
        conn.close()
    except OSError as e:
        conn.close()
        print("Connection closed")
