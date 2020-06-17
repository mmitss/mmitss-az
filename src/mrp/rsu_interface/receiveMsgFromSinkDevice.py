'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  receiveMsgFromSinkDevice.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision developed for testing the RSU4.1 interface on the available RSUs and OBUs.
  -> While using this sender, verify the following
    -> IP and port of the server receiving messages from the sink dsrc device (Lines 28 and 29)
'''

import socket
import datetime

def Main():
    # This is the IP address and port of the server receiving messages from sink dsrc device (called 'server')
	serverIP = '10.12.6.108'
	serverPort = 10002 # This is what Kun should tell us. Which port on the remote computer will the messages be forwarded.
	server = (serverIP, serverPort)

    # Create a socket
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the created socket to the server information.
	s.bind(server)
	
	# Create a timestamp which will be appended at the end of name of a file storing received data.
	timestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))
	messagePath = 'received_messages/'
	fileName = messagePath + "msgLog_" + timestamp
	# Open a file to store the received data. (Message log file).
	f = open((fileName), "w")

    # Define a counter variable to track the number of messages received from the sender
	i = 1

	while True:
		receivedData, addr = s.recvfrom(1024) # Receive the data from the sender, and store its identification.
		f.write(receivedData.hex() + '\n') # Write the received data to a message log file.
		print("Received message# " + str(i) + " from " + str(addr[0]) + ":" + str(addr[1])) # Print the number of messages received and the identification of the sender.
		i = i + 1 # Increment the counter variable
	s.close() # Close the socket
	f.close() # Close the file containing received messages.

if __name__ == '__main__':
	Main()
