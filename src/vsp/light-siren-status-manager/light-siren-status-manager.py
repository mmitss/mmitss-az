#!/usr/bin/env python3

import socket
import json
import ST7735 as ST7735
import sys
import time

import automationhat
time.sleep(0.1)  # Short pause after ads1015 class creation recommended

try:
    from PIL import Image, ImageDraw
except ImportError:
    print("""This example requires PIL.
Install with: sudo apt install python{v}-pil
""".format(v="" if sys.version_info.major == 2 else sys.version_info.major))
    sys.exit(1)


# print("""input.py

# This Automation HAT Mini example displays the status of
# the three 24V-tolerant digital inputs.

# Press CTRL+C to exit.
# """)

# Create ST7735 LCD display class.
disp = ST7735.ST7735(
    port=0,
    cs=ST7735.BG_SPI_CS_FRONT,
    dc=9,
    backlight=25,
    rotation=270,
    spi_speed_hz=4000000
)

# Initialize display.
disp.begin()

on_colour = (99, 225, 162)
off_colour = (235, 102, 121)

# Values to keep everything aligned nicely.
on_x = 115
on_y = 35

off_x = 46
off_y = on_y

dia = 10

def lightSirenStatusMsg(lightSirenStatus):
    lightSirenStatusDictionary = {"MsgType":"LightSirenStatusMessage", "LightSirenStatus": lightSirenStatus}
    jsonString = json.dumps(lightSirenStatusDictionary)
    return jsonString

def main():
    # Read the config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    # configFile = open("/home/vsp-config/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()
    # read the ip address and port number from the config file
    hostIP = config["HostIp"]
    port = config["PortNumber"]["LightSirenStatusManager"]
    priorityRequestGeneratorAddress = (config["HostIp"], config["PortNumber"]["PriorityRequestGenerator"])
    communicationInfo = (hostIP, port)
    # Open a socket and bind it to the IP and port dedicated for this application:
    lightSirenStatusManagerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    lightSirenStatusManagerSocket.bind(communicationInfo)
        
    vehicleType = config["VehicleType"]
    lightSirenStatus = "OFF"
    temporaryLightSirenStatus = "OFF"

    while True:
        if vehicleType == "EmergencyVehicle":
            # Value to increment for spacing circles vertically.
            offset = 0

            # Open our background image.
            image = Image.open("inputs-blank.jpg")
            draw = ImageDraw.Draw(image)
            
            if automationhat.input[1].is_on():
                draw.ellipse((on_x, on_y + offset, on_x + dia, on_y + dia + offset), on_colour)
                temporaryLightSirenStatus = "ON"
            elif automationhat.input[2].is_off():
                draw.ellipse((off_x, off_y + offset, off_x + dia, off_y + dia + offset), off_colour)
                temporaryLightSirenStatus = "OFF"     
            offset += 14
            
                
            if temporaryLightSirenStatus != lightSirenStatus:
                #print("\nLightSirenStatus is: ", lightSirenStatus," at time ", time.time())
                #print("TemporaryLightSirenStatus is: ", temporaryLightSirenStatus," at time ", time.time())  
                lightSirenStatus = temporaryLightSirenStatus
                lightSirenStatusJsonString = lightSirenStatusMsg(lightSirenStatus)
                print('\n',lightSirenStatusJsonString)
                lightSirenStatusManagerSocket.sendto(lightSirenStatusJsonString.encode(),priorityRequestGeneratorAddress)
            
                
            # Draw the image to the display
            disp.display(image)

            time.sleep(0.5)
        else:
            break
    lightSirenStatusManagerSocket.close()
    
if __name__ == "__main__":
    main()
