
# Message Distributor
In MMITSS, Roadside Units (RSUs) and On-board Units (OBUs) perform the function of wireless broadcast of generated messages, where an individual RSU is mounted on intersections and individual OBU is installed in vehicles. However, while **simulating** connected vehicles and connected intersections, it is often infeasible to have multiple RSUs or OBUs on bench that represent each simulated intersection or simulated vehicle. Therefore, in the simulated environment, it is necessary to have a wired-route mechanism that replaces the wireless broadcasting functions of RSUs and OBUs. Further, wireless devices have range limitations, where they may not be able to receive a message broadcasted from outside their range. Message-Distributor application is designed to address the range-limited message distribution requirements in the simulation environment. This application is intended to run in a separate container which has network connectivity to both, Simulated-Bsm-Blob-Processor application and other containers that represent individual intersections. 

## Work-flow
Message Distributor receives Basic safety Messages (BSMs) from Simulated-Bsm-Blob-Processor, SRMs from Priority-Request-Generator(s), Signal Status Messages (SSMs) from Priority-Request-Server(s), Signal Phase and Timing (SPaT) and MAP messages from the Map-Spat-Broadcaster(s). It distributes the received BSMs and SRMs to each of the configured intersections if the [haversine](https://en.wikipedia.org/wiki/Haversine_formula) distance between the location specified in the message and the configured location of the intersection is less than the configured DSRC range of that intersection.

## Console output and logging
Message-Distributor does not produce any console output. However, if the `["raw_bsm_logging"]` field in the `mmitss-message-distributor-config.json` is set to `true` then a file containing the information from the received BSMs is generated in the working directory.

## Configuration
The message distributor requires two configuration files placed in the `/nojournal/bin/` directory: (1) `mmitss-phase3-master-config.json`, and (2)`mmitss-message-distributor-config.json`. The required fields from these configuration files are described below:
1. `mmitss-phase3-master-config.json`: 
    - `["MessageDistributorIP"]`: a `string` specifying IPv4 address of the machine (or container) that hosts the Message-Distributor application.
    - `["PortNumber"]["MessageDistributor"]`: an `int` specifying the UDP port used by the Message-Distributor to receive messages.
2. `mmitss-message-distributor-config.json`:
    - `["package_drop_probability"]`: a `float` in the range [0,1] that specifies the probability of dropping a received message.
    - `["raw_bsm_logging"]`: a `bool` indicating whether or not to log the data from received messages to a file. If this field is set to `true`, a `*.csv` file is generated in the working directory that contains the data from received BSMs.
    - `["intersections"]`: a `list` containing intersection related information. For each intersection in the list, the following information is required:
        - `["intersections"][n]["name"]`: a `string` specifying the name of the intersection
        - `["intersections"][n]["ip_address"]`: a `string` specifying the IPv4 address of the machine or container representing the intersection
        - `["intersections"][n]["bsm_client_port"]`: an `int` specifying the UDP port of the primary BSM client of the intersection (for example, V2X-Data-Collector)
        - `["intersections"][n]["srm_client_port"]`: an `int` specifying the UDP port of the primary SRM client of the intersection (for example, Priority-Request-Server)
        - `["intersections"][n]["dsrc_range_Meter"]`: an `int` specifying the DSRC range of the intersection in Meter
        - `["intersections"][n]["position"]["latitude_DecimalDegree"]`: a `float` specifying the latitude of the center of the intersection, in Decimal Degree
        - `["intersections"][n]["position"]["longitude_DecimalDegree"]`: a `float` specifying the longitude of the center of the intersection, in Decimal Degree
        - `["intersections"][n]["position"]["elevation_Meter"]`: a `float` specifying the elevation of the center of the intersection, in Meter
    - `["bsm_clients"]`: a `list` containing additional clients that need the BSMs from specific vehicle type. MMITSS supports 4 vehicle types: Passenger, Transit, Truck, and EmergencyVehicle. For each vehicle type, following information is required.
        - `["bsm_clients"]["Passenger"]`: a `list` containing the network information about additional clients that require BSMs from Passenger vehicles. In this list, following information is required for each client:
            - `["bsm_clients"]["Passenger"][n]["ip_address"]`: a `string` specifying the IPv4 address of the client
            - `["bsm_clients"]["Passenger"][n]["port"]`: an `int` specifying the UDP port of the client
         - `["bsm_clients"]["Transit"]`: a `list` containing the network information about additional clients that require BSMs from Transit vehicles. In this list, following information is required for each client:
            - `["bsm_clients"]["Transit"][n]["ip_address"]`: a `string` specifying the IPv4 address of the client
            - `["bsm_clients"]["Transit"][n]["port"]`: an `int` specifying the UDP port of the client
         - `["bsm_clients"]["Truck"]`: a `list` containing the network information about additional clients that require BSMs from Truck vehicles. In this list, following information is required for each client:
            - `["bsm_clients"]["Truck"][n]["ip_address"]`: a `string` specifying the IPv4 address of the client
            - `["bsm_clients"]["Truck"][n]["port"]`: an `int` specifying the UDP port of the client
          - `["bsm_clients"]["EmergencyVehicle"]`: a `list` containing the network information about additional clients that require BSMs from EmergencyVehicle vehicles. In this list, following information is required for each client:
            - `["bsm_clients"]["EmergencyVehicle"][n]["ip_address"]`: a `string` specifying the IPv4 address of the client
            - `["bsm_clients"]["EmergencyVehicle"][n]["port"]`: an `int` specifying the UDP port of the client
          - `["bsm_clients"]["other"]`: a `list` containing the network information about additional clients that require BSMs from other vehicles (unknown vehicle type). In this list, following information is required for each client:
            - `["bsm_clients"]["other"][n]["ip_address"]`: a `string` specifying the IPv4 address of the client
            - `["bsm_clients"]["other"][n]["port"]`: an `int` specifying the UDP port of the client
    - `["map_clients"]`: a `list` containing additional clients that need the received MAP messages. In this list, following information is required for each client:
            - `["map_clients"][n]["ip_address"]`: a `string` specifying the IPv4 address of the client
            - `["map_clients"][n]["port"]`: an `int` specifying the UDP port of the client
    - `["ssm_clients"]`: a `list` containing additional clients that need the received SSM messages. In this list, following information is required for each client:
            - `["ssm_clients"][n]["ip_address"]`: a `string` specifying the IPv4 address of the client
            - `["ssm_clients"][n]["port"]`: an `int` specifying the UDP port of the client

## Requirements
1. Network connectivity with containers representing each intersection and the container hosting Simulated-Bsm-Blob-Processor application.
2. Python packages: [haversine](https://pypi.org/project/haversine/)

## Known issues/limitations
Message-Distributor can distribute messages only if they are in the specified JSON format, and it is not designed to be able to encode or decode messages to/from UPER Hex format.

