# MAP-SPAT-BROADCASTER

A SPaT (Signal Phase and Timing) message describes the current status of each vehicle and pedestrian phase of a signalized intersection along with the minimum and maximum residual time of the status of each vehicle and pedestrian phase with a frequency of 10 Hz. The information required to formulate a SPaT message can be obtained by placing SnmpGetRequest(s) to the signal controller with the same frequency. However, this generates an undesirable additional overhead on the signal controller to handle such high frequency SNMP calls.  

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

As it can be seen from the above JSON string that in addition to the information required in the SPaT message, the Map-Spat-Broadcaster also maintains the elapsed time since each phase had entered its current status, as this information is required by other MMITSS components. For each received blob, the Map-Spat-Broadcaster generates a JSON string containing the information about current phases and their elapsed time in the current status. This information is streamed to Traffic-Controller-Interface for further handling and distribution. An example of such JSON string is as follows:
```
{
    "currentPhases":
        [ 
            {   
                "Phase": 2,
                "State": "yellow",
                "ElapsedTime": 30.0
            },
            {   
                "Phase": 6,
                "State": "yellow",
                "ElapsedTime": 30.0 
            }
        ]
}
```
### Permissive yellow
### Inactive phases


## Console output and logging
Map-Spat-Broadcaster component does not generate any log files. The console output displays if the broadcast of the Map and SPaT messages has started successfully.

## Requirements
- Physical network connection between the host machine (hosting the Map-Spat-Broadcaster) and the target [NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controller.
- In case of Econolite signal controller, MapSpatBroadcaster sends an SnmpSetRequest to the MMITSS component Snmp-Engine that enables the streaming of raw SPaT blob from the signal controller. For this feature to function correctly, the MMITSS component Snmp-Engine needs to be running prior to the starting of Map-Spat-Broadcaster.

## Configuration
1. In the signal controller:
- The server IP to which the signal controller streams the raw SPaT blob must be set to the IP address of the host where Map-Spat-Broadcaster is hosted.
- Port where the signal controller streams the raw SPAT blob must match in both, the Map-Spat-Broadcaster configuration and the corresponding configuration in the signal controller.

2. In the mmitss-phase3-master-config.json (config) file, following keys need to be assigned with appropriate values:
- `config["SignalController"]["IpAddress"]`: IPv4 address of the signal controller (string)
- `config["PortNumber"]["MapSPaTBroadcaster"]`:  UDP port number on the host (integer) where the signal controller pushes the raw SPaT blob. Note: Change only if the default (recommended) port number is already occupied on the host machine. In case of a change, corresponding change in the signal controller needs to be ensured. 
- `config["SignalController"]["Vendor"]`: vendor of the signal-controller (string). (tested on Econolite Cobalt, Intellight MaxTime). For Econolite signal controllers, use "Econolite" here.
- `config["signalController"]["InactiveVehPhases"]`
- `config["signalController"]["InactivePedPhases"]`
- `config["signalController"]["SplitPhases"]["1"]`
- `config["SignalController"]["SplitPhases"]["3"]`
- `config["SignalController"]["SplitPhases"]["5"]`
- `config["SignalController"]["SplitPhases"]["7"]`
- `config["SignalController"]["PermissiveEnabled"]["1"]`
- `config["SignalController"]["PermissiveEnabled"]["3"]`
- `config["SignalController"]["PermissiveEnabled"]["5"]`
- `config["SignalController"]["PermissiveEnabled"]["7"]`

3. In the mmitss-data-external-clients.json (clients) file, external network clients that need either the raw spat blob or the SPaT JSON string generated by the Map-Spat-Broadcaster can be specified by adding appropriate values to the following keys:
- `clients["spat"]["blob"]`
- `clients["spat"]["json"]`

## Known issues
some issues
