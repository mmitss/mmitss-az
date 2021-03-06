
# Message Distributor
In MMITSS, Roadside Units (RSUs) and On-board Units (OBUs) perform the function of wireless broadcast of generated messages, where an individual RSU is mounted on intersections and individual OBU is installed in vehicles. However, while **simulating** connected vehicles and connected intersections, it is often infeasible to have multiple RSUs or OBUs on bench that represent each simulated intersection or simulated vehicle. Therefore, in the simulated environment, it is necessary to have a wired-route mechanism that replaces the wireless broadcasting functions of RSUs and OBUs. Further, wireless devices have range limitations, where they may not be able to receive a message broadcasted from outside their range. Message-Distributor application is designed to address the range-limited message distribution requirements in the simulation environment. This application is intended to run in a separate container which has network connectivity to both, Simulated-Bsm-Blob-Processor application and other containers that represent individual intersections. 

## Work-flow
Message Distributor receives Basic safety Messages (BSMs) from Simulated-Bsm-Blob-Processor, SRMs from Priority-Request-Generator(s), Signal Status Messages (SSMs) from Priority-Request-Server(s), Signal Phase and Timing (SPaT) and MAP messages from the Map-Spat-Broadcaster(s). It distributes the received BSMs and SRMs to each of the configured intersections if the [haversine](https://en.wikipedia.org/wiki/Haversine_formula) distance between the location specified in the message and the configured location of the intersection is less than the configured DSRC range of that intersection.

## Configuration
The message distributor requires two configuration files placed in the `/nojournal/bin/` directory: (1) `mmitss-phase3-master-config.json`, and (2)`mmitss-message-distributor-config.json`. The required fields from these configuration files are described below:
1. `mmitss-phase3-master-config.json`: 
    - `["MessageDistributorIP"]`: a `string` specifying IPv4 address of the machine (or container) that hosts the Message-Distributor application.
    - `["PortNumber"]["MessageDistributor"]`: an `int` specifying the UDP port used by the Message-Distributor to receive messages.



## Console output and logging
Message-Distributor component does not generate any log files. 

## Requirements
1. Network connectivity with containers representing each intersection and the container hosting Simulated-Bsm-Blob-Processor application.
2. Python packages: [haversine](https://pypi.org/project/haversine/)

## Known issues/limitations
1. Message-Distributor can distribute messages only if they are in the specified JSON format, and it is not designed to be able to encode or decode messages to/from UPER Hex format.

