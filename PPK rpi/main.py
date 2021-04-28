import paho.mqtt.client as paho
import ssl
import json
import time
from src.ppk2_api import PPK2_API
import sys
import time
import board
import neopixel

EndPoint = "XXXXXXXXXXXXXXX.iot.us-east-1.amazonaws.com"
Client = "account-XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
sub_topic = 'prod/XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX/a/gateways'

pixel_pin = board.D18
num_pixels = 16
ORDER = neopixel.GRB

pixels = neopixel.NeoPixel(
    pixel_pin, num_pixels, brightness=0.1, auto_write=False, pixel_order=ORDER
)

class Error(Exception):
    """Base class for other exceptions"""
    pass


class RESET(Error):
    """Raised when Reset"""
    pass

def wheel(pos):
    # Input a value 0 to 255 to get a color value.
    # The colours are a transition r - g - b - back to r.
    if pos < 0 or pos > 255:
        r = g = b = 0
    elif pos < 85:
        r = int(pos * 3)
        g = int(255 - pos * 3)
        b = 0
    elif pos < 170:
        pos -= 85
        r = int(255 - pos * 3)
        g = 0
        b = int(pos * 3)
    else:
        pos -= 170
        r = 0
        g = int(pos * 3)
        b = int(255 - pos * 3)
    return (r, g, b) if ORDER in (neopixel.RGB, neopixel.GRB) else (r, g, b, 0)

def rainbow_cycle(wait):
    for j in range(255):
        for i in range(num_pixels):
            pixel_index = (i * 256 // num_pixels) + j
            pixels[i] = wheel(pixel_index & 255)
        pixels.show()
        time.sleep(wait) 

while 1:
    try:
        rainbow_cycle(0.01)
        ppk2s_connected = PPK2_API.list_devices()
        ppk2_test = PPK2_API("/dev/ttyACM0")
        ppk2_test.get_modifiers()
        ppk2_test.use_ampere_meter()

        # Change this value for your ENDPOINT
        

        caPath = "Certs/ca.cert"
        certPath = "Certs/client.cert"
        keyPath = "Certs/priv.cert"

        flag = False

        def on_connect(client, userdata, flags, rc):
            print("Connection returned result: " + str(rc))
            # client.subscribe("#" , 1 ) # Wild Card

        # This function trigger every time we receive a message from the platform

        def on_message(client, userdata, msg):
            global flag
            global ppk2s_connected
            global ppk2_test
            
            myjson = json.loads(msg.payload.decode())
            result = myjson['message']["event"]["characteristic"]["value"][1]
            first = myjson['message']["event"]["characteristic"]["value"][0]
            if(first == 1):
                ...
            else:
                print("topic: "+msg.topic)
                if(result == 0):
                    print("Start")
                    flag = True
                    ppk2_test.start_measuring()
                elif(result == 1):
                    print("Stop")
                    ppk2_test.stop_measuring()
                    flag = False
                    pixels.fill((255, 255, 255))
                    pixels.show()
                elif(result == 2):
                    print("Set Amp")
                    ppk2_test.stop_measuring()
                    ppk2_test.get_modifiers()
                    ppk2_test.use_ampere_meter()  # set ampere meter mode
                    ppk2_test.stop_measuring()
                    flag = False
                elif(result == 3):
                    print("Set Vol")
                    ppk2_test.stop_measuring()
                    ppk2_test.get_modifiers()
                    ppk2_test.use_source_meter()  # set ampere meter mode
                    ppk2_test.set_source_voltage(mV=3300)
                    ppk2_test.toggle_DUT_power("ON")  # disable DUT power
                    flag = False
                elif(result == 4):
                    raise RESET

        # This function trigger when we publish


        def on_publish(client, obj, mid):
            print("Data Sent")

        # This function trigger when we subscribe to a new topic


        def on_subscribe(client, obj, mid, granted_qos):
            print("Subscribed: " + str(mid) + " " + str(granted_qos))


        mqttc = paho.Client(client_id=Client")

        mqttc.on_connect = on_connect
        mqttc.on_message = on_message
        mqttc.on_publish = on_publish
        mqttc.on_subscribe = on_subscribe
        mqttc.tls_set(caPath, certfile=certPath, keyfile=keyPath,
                    cert_reqs=ssl.CERT_REQUIRED, tls_version=ssl.PROTOCOL_TLSv1_2, ciphers=None)
        mqttc.connect(EndPoint, 8883, keepalive=60)
        rc = 0

        mqttc.subscribe(sub_topic)

        print("Mqtt :Ok")

        pixels.fill((255, 255, 255))
        pixels.show()

        while rc == 0:
            rc = mqttc.loop()
            if(flag):
                for i in range(0, 1000):
                    rc = mqttc.loop()
                    if(not(flag)):
                        break
                    read_data = ppk2_test.get_data()
                    if read_data != b'':
                        samples = ppk2_test.get_samples(read_data)
                        print(f"Average of {len(samples)} samples is: {sum(samples)/len(samples)}uA")
                        current = sum(samples)/len(samples)
                        if(current>10000):
                            pixels.fill((255, 0, 0))
                            pixels.show()
                            print("MACRO SHOCK DANGER")
                        elif(current>10):
                            pixels.fill((0, 0, 255))
                            pixels.show()
                            print("MICRO SHOCK DANGER")
                        else:
                            pixels.fill((0, 255, 0))
                            pixels.show()
                            print("OK")
                        
                    time.sleep(0.001)
                flag = False
                ppk2_test.stop_measuring()

    except RESET:
        print("Reset All")
        
    except KeyboardInterrupt:
        sys.exit()
    except:
        ...