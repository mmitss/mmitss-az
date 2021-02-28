"""
**********************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  simulated-bsm-blob-processor.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

**********************************************************************************
"""
import json
import socket
import time, datetime
from SimulatedBsmBlobProcessor import SimulatedBsmBlobProcessor

# Load the information from the config file:
with open("/nojournal/bin/mmitss-phase3-master-config.json", 'r') as configFile:
    config = json.load(configFile)

host = (config["HostIp"], config["PortNumber"]["HostBsmDecoder"]) # The port is reused here as in simulation environment, SimulatedBsmBlobProcessor application replaces HostBsmDecoder.
messageDistributor = (config["HostIp"], config["PortNumber"]["MessageDistributor"])

# Create an object of SimulatedBsmBlobProcessor class:
processor = SimulatedBsmBlobProcessor()

# Create the socket and bind it to the appropriate address:
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(host)

while True:
    # Receive blob data (from VISSIM)
    data, address = s.recvfrom(128)
    
    # Try to unpack the blob data
    try:
        bsmJson = processor.unpack_blob_to_json(data)
        # Forward the unpacked data (JSON) to the message distributor
        s.sendto(bsmJson.encode(), messageDistributor)
    except: pass
    
# Close the socket before exiting
s.close()

    
