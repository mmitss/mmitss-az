
# Message Distributor
Message Distributor receives Basic safety Messages (BSMs) from simulation, SRMs from priority-request-generator(s), Signal Status Messages (SSMs) from priority-request-server(s), Signal Phase and Timing (SPaT) and MAP messages from the map-spat-broadcaster(s). It distributes the received BSMs and SRMs to nearby intersections based on the location from where these messages were generated, locations of configured intersections, and configured DSRC range. In addition, it also distributes messages to clients configured for each message type. 

## Configuration
The message distributor requires two configuration files placed in the `/nojournal/bin/` directory: (1) `mmitss-phase3-master-config.json` and `mmitss-message-distributor-config.json`. The required fields from these configuration files are described below:
1. `mmitss-phase3-master-config.json`: 
  - `["MessageDistributorIP"]`: a `string` specifying IPv4 address of the machine (or container) that hosts the Message Distributor application.
  - `["PortNumber"]["MessageDistributor"]`: an `int` specifying the UDP port used by the message distributor to receive messages.



## Console output and logging
Message-Distributor component does not generate any log files. 

## Requirements

## Configuration

## Known issues/limitations


