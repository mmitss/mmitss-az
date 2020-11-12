'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  sendMsgToSourceDevice.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision developed for testing the RSU4.1 interface on the available RSUs and OBUs.
  -> While using this sender, verify the following
    -> IP and port of the server streaming messages to the source dsrc device (Lines 30 and 31)
    -> IP and port of the source dsrc device (Lines 35 and 36)
    -> Path and name of the file containing the message. This message must be formatted as specified in RSU4.1 specifications. (Line 45 and 46)
'''

import socket
from time import sleep

def Main():
    # This is the IP address and port of the server sending messages to the source dsrc device (called 'server')
    serverIP = '172.24.5.250'
    serverPort = 5001 # This could be any available port on this computer
    server = (serverIP, serverPort)

    # This is the IP address and port of the source dsrc device
    receiverIP = '172.24.5.254' # 
    receiverPort = 1516 # Kun might give us other port.
    receiver = (receiverIP, receiverPort)

    # Create a socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the created socket to the server information.
    s.bind((server))

    # Open and read the file containing the message for transmission. 
    messagePath = 'messages/' # Type the path of location where message is stored.
    message = 'ssm' # Type the name of file containing the message. This message must be formatted as per RSU4.1 Specifications.
    f = open((messagePath + message), "rb")
    data = f.read(20480)
    print (data) # Print the message being transmitted to the console.
    
    # Define a counter variable to track the number of messages sent to the receiver
    i = 1

    while True:
        s.sendto(data, receiver) # Send the data to receiver
        print("Sent message# " + str(i) + " to " + str(receiverIP) + ":" + str(receiverPort)) # Print number of messages and the receiver identification sent to the console
        i = i + 1 # Increment the counter variable
        sleep(.1) # Pause for 0.1 second -> this field needs to be varied based on the message being transmitted. For example, for MAP message this value should be 1 sec. 
    s.close() # Close the socket
    f.close() # Close the file containing the message.

if __name__ == "__main__":
    Main()
