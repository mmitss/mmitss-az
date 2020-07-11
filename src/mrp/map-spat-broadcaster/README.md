# MAP-SPAT-BROADCASTER

A SPaT (Signal Phase and Timing) message describes the current status of each vehicle and pedestrian phase of a signalized intersection along with the minimum and maximum residual time of the status of each vehicle and pedestrian phase with a frequency of 10 Hz. The information required to formulate a SPaT message can be obtained by placing SnmpGetRequest(s) to the signal controller with the same frequency. However, this approach generates an undesirable additional overhead on the signal controller to handle such high frequency SNMP calls.  

To avoid this additional overhead, [NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controllers stream a blob containing the information required in the SPaT message with a frequency of 10 Hz. This blob is streamed to a network node (UDP socket) defined in the signal controller. 

The MMITSS component **Map-Spat-Broadcaster** is responsible for receiving and processing the SPaT blobs streamed by a signal controller.

## Work-flow

The Map-Spat-Broadcaster receives the SPaT blobs streamed by the signal controller, extracts the required information, stores it in a JSON string, and forwards this JSON string to the MMITSS component Message-Encoder for UPER-encoding. An example of the JSON formatted SPaT message is as follows:

```
{
    "MsgType": "SPaT",
    "Timestamp_verbose": "2020-03-26 04:20:09.898820",
    "Timestamp_posix": 1585221609.898842,
    "Spat": {
        "IntersectionState": {
            "regionalID": 0,
            "intersectionID": 44383
        },
        "msgCnt": 10,
        "minuteOfYear": 122660,
        "msOfMinute": 9897,
        "status": "0000000000000000",
        "phaseState": [
            {
                "phaseNo": 1,
                "currState": "red",
                "startTime": 0,
                "minEndTime": 220,
                "maxEndTime": 820,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 2,
                "currState": "yellow",
                "startTime": 0,
                "minEndTime": 30,
                "maxEndTime": 30,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 3,
                "currState": "red",
                "startTime": 0,
                "minEndTime": 40,
                "maxEndTime": 40,
                "elapsedTime": 8
            },
            {
                "phaseNo": 4,
                "currState": "red",
                "startTime": 0,
                "minEndTime": 130,
                "maxEndTime": 430,
                "elapsedTime": 8
            },
            {
                "phaseNo": 5,
                "currState": "red",
                "startTime": 0,
                "minEndTime": 220,
                "maxEndTime": 820,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 6,
                "currState": "yellow",
                "startTime": 0,
                "minEndTime": 30,
                "maxEndTime": 30,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 7,
                "currState": "red",
                "startTime": 0,
                "minEndTime": 40,
                "maxEndTime": 40,
                "elapsedTime": 8
            },
            {
                "phaseNo": 8,
                "currState": "red",
                "startTime": 0,
                "minEndTime": 130,
                "maxEndTime": 430,
                "elapsedTime": 8
            }
        ],
        "pedPhaseState": [
            {
                "phaseNo": 1,
                "currState": "do_not_walk",
                "startTime": 0,
                "minEndTime": 0,
                "maxEndTime": 0,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 2,
                "currState": "do_not_walk",
                "startTime": 0,
                "minEndTime": 0,
                "maxEndTime": 0,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 3,
                "currState": "do_not_walk",
                "startTime": 0,
                "minEndTime": 0,
                "maxEndTime": 0,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 4,
                "currState": "do_not_walk",
                "startTime": 0,
                "minEndTime": 0,
                "maxEndTime": 0,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 5,
                "currState": "do_not_walk",
                "startTime": 0,
                "minEndTime": 0,
                "maxEndTime": 0,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 6,
                "currState": "do_not_walk",
                "startTime": 0,
                "minEndTime": 0,
                "maxEndTime": 0,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 7,
                "currState": "do_not_walk",
                "startTime": 0,
                "minEndTime": 0,
                "maxEndTime": 0,
                "elapsedTime": 0.0
            },
            {
                "phaseNo": 8,
                "currState": "do_not_walk",
                "startTime": 0,
                "minEndTime": 0,
                "maxEndTime": 0,
                "elapsedTime": 0.0
            }
        ]
    }
}
```
### Current phases and elapsed times
As it can be seen from the above JSON string that in addition to the information required in the SPaT message, the Map-Spat-Broadcaster also maintains the elapsed time since each phase had entered its current status, as this information is required by other MMITSS components. For each received blob, the Map-Spat-Broadcaster generates a JSON string containing the information about current phases and their elapsed time in the current status. This information is streamed to Traffic-Controller-Interface for further handling and distribution. An example of such JSON string is as follows:
```
{
    "currentPhases":
        [ 
            {   
                "Phase": 2,
                "State": "yellow",
                "ElapsedTime": 0.0
            },
            {   
                "Phase": 6,
                "State": "yellow",
                "ElapsedTime": 0.0 
            }
        ]
}
```
### Elapsed times of inactive phases
The calculation of the elapsed time is based on the current state of each phase in the blob. For intersections that have one or more inactive phases (for example T-intersections), the raw spat blob from the signal controller indicates the current status of the inactive phases as "red", at all times. Due to this, the elapsed time for such phases in the Map-Spat-Broadcaster can build up to really large numbers over the period of time, and may result into performance degradation. To deal with this, Map-Spat-Broadcaster allows to configure the list of inactive phases in the `mmitss-phase3-master-config.json` file. For such configured inactive phases, the Map-Spat-Broadcaster does not calculate or increment the elapsed time. The configuration is further explained in the configuration section of this document.

### Permissive-yellow left-turn phases
The raw SPaT blob from the signal controller does not contain any information about the status of permissive-left turn phases. This is generally configured by wiring in the intersection's signal controller cabinet. Map-Spat-Broadcaster allows to configure some or all left-turn phases to be in permissive-yellow state if the corresponding *through* phase is in the "green" interval. Permissive-yellow left-turn phases and their corresponding through phases can be configured in the `mmitss-phase3-master-config.json` file.

## Console output and logging
Map-Spat-Broadcaster component does not generate any log files. The console output displays if the broadcast of the Map and SPaT messages has started successfully.

## Requirements
- Physical network connection between the host machine (hosting the Map-Spat-Broadcaster) and the target [NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controller.
- In case of Econolite signal controller, MapSpatBroadcaster sends an SnmpSetRequest to the MMITSS component Snmp-Engine that enables the streaming of raw SPaT blob from the signal controller. For this feature to function correctly, the MMITSS component Snmp-Engine needs to be in a running state before starting the Map-Spat-Broadcaster.

## Configuration
1. In the signal controller:
    - The *server IP* to which the signal controller streams the raw SPaT blob must be set to the IP address of the host where Map-Spat-Broadcaster is hosted (that is MMITSS-MRP).
    - *UDP Port* where the signal controller streams the raw SPAT blob must match in both, the Map-Spat-Broadcaster configuration and the corresponding configuration in the signal controller.

2. In the `mmitss-phase3-master-config.json` (config) file, following keys need to be assigned with appropriate values:
    - `config["SignalController"]["IpAddress"]`: IPv4 address of the signal controller (string)
    - `config["SignalController"]["NtcipPort"]`: NTCIP port of the signal controller (integer)
    - `config["PortNumber"]["MapSPaTBroadcaster"]`:  UDP port number on the host (integer) where the signal controller pushes the raw SPaT blob. Note: Change only if the default (6053) port number is already occupied on the host machine. In case of a change, corresponding change in the signal controller needs to be ensured.
    - `config["PortNumber"]["TrafficControllerCurrPhaseListener"]`: UDP port where the MMITSS component Traffic-Controller_interface listens for the CurrentPhaseStatus.
    - `config["SignalController"]["Vendor"]`: vendor of the signal-controller (string).   
    NOTE: This software is tested to work on Econolite (ASC3 and Cobalt) and Intellight (MaxTime) made signal controllers. For Econolite signal controllers, use "Econolite" in this key.
    - `config["signalController"]["InactiveVehPhases"]`: A list of vehicle phases that are inactive for the given intersection (list)
    - `config["signalController"]["InactivePedPhases"]`: A list of pedestrian phases that are inactive for the given intersection (list)
    - `config["SignalController"]["PermissiveEnabled"]["1"]`: Flag indicating if left-turn phase 1 is enabled for permissive-yellow left-turn (boolean)
    - `config["SignalController"]["PermissiveEnabled"]["3"]`: Flag indicating if left-turn phase 3 is enabled for permissive-yellow left-turn (boolean)
    - `config["SignalController"]["PermissiveEnabled"]["5"]`: Flag indicating if left-turn phase 5 is enabled for permissive-yellow left-turn (boolean)
    - `config["SignalController"]["PermissiveEnabled"]["7"]`: Flag indicating if left-turn phase 7 is enabled for permissive-yellow left-turn (boolean)
    - `config["signalController"]["SplitPhases"]["1"]`: Through phase corresponding to the left-turn phase 1 (integer)
    - `config["SignalController"]["SplitPhases"]["3"]`: Through phase corresponding to the left-turn phase 3 (integer)
    - `config["SignalController"]["SplitPhases"]["5"]`: Through phase corresponding to the left-turn phase 5 (integer)
    - `config["SignalController"]["SplitPhases"]["7"]` Through phase corresponding to the left-turn phase 7 (integer)
    NOTE: SplitPhases (through phases corresponding to the left-turn phases) MUST be configured if the PermissiveEnabled flag is set to True for a particular phase.
3. In the `mmitss-data-external-clients.json` (clients) file, external network clients that need either the raw spat blob or the SPaT JSON string generated by the Map-Spat-Broadcaster can be specified by adding appropriate values to the following keys:
    - `clients["spat"]["blob"]`: A list of JSON strings of network addresses (IP Address and Port) of clients that require raw SPaT blob (example shown below)
    - `clients["spat"]["json"]`: A list of JSON strings of network addresses (IP Address and Port) of clients that require JSON string of the SPaT message (example shown below)
    An example of client-network address JSON string is as follows:
    ```
    {
        "IP": "127.0.0.1",
        "Port": 9
    }
    ```

## Known issues/limitations
There are following known limitations in the Map-Spat-Broadcaster:
1. It works with intersections having upto 8 phases.
2. "Overlaps" related information is available in the raw SPaT blob streamed by the signal controller. However, the Map-Spat-Broadcaster does not decode and process this information. Current application is limited to vehicle and pedestrian phases.
