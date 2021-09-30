# TRAFFIC-CONTROLLER-INTERFACE

The Traffic Controller Interface (TCI) component is the interfacing component that mediates the communication between the Snmp-Engine and other MMITSS applications that desire to communicate with the signal controller via Snmp-Engine. The TCI is reponsible for the following tasks:
1. Listening to various types of requests from other MMITSS components
2. Managing a background scheduler to schedule events that fulfill received requests.

## Work-flow
The TCI processes following types of requests:
1. Execution of phase control schedule:
  - To execute a phase control schedule, a message similar to the following is expected:
  ```
  {
    "MsgType": "Schedule",
    "Schedule": [
        {
            "commandEndTime": 0.0,
            "commandPhase": 2,
            "commandStartTime": 0.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 24.0,
            "commandPhase": 4,
            "commandStartTime": 5.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 44.0,
            "commandPhase": 2,
            "commandStartTime": 29.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 64.0,
            "commandPhase": 4,
            "commandStartTime": 49.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 12.0,
            "commandPhase": 2,
            "commandStartTime": 11.0,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 69.5,
            "commandPhase": 4,
            "commandStartTime": 68.5,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 109.5,
            "commandPhase": 2,
            "commandStartTime": 108.5,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 129.5,
            "commandPhase": 4,
            "commandStartTime": 128.5,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 23.999767065048218,
            "commandPhase": 4,
            "commandStartTime": 0.0,
            "commandType": "call_veh"
        },
        {
            "commandEndTime": 133.5,
            "commandPhase": 1,
            "commandStartTime": 0.0,
            "commandType": "omit_veh"
        },
        {
            "commandEndTime": 133.5,
            "commandPhase": 3,
            "commandStartTime": 0.0,
            "commandType": "omit_veh"
        },
        {
            "commandEndTime": 0.0,
            "commandPhase": 6,
            "commandStartTime": 0.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 24.0,
            "commandPhase": 7,
            "commandStartTime": 5.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 44.0,
            "commandPhase": 6,
            "commandStartTime": 29.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 64.0,
            "commandPhase": 7,
            "commandStartTime": 49.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 12.0,
            "commandPhase": 6,
            "commandStartTime": 11.0,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 69.5,
            "commandPhase": 7,
            "commandStartTime": 68.5,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 109.5,
            "commandPhase": 6,
            "commandStartTime": 108.5,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 129.5,
            "commandPhase": 7,
            "commandStartTime": 128.5,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 23.999767065048218,
            "commandPhase": 7,
            "commandStartTime": 0.0,
            "commandType": "call_veh"
        },
        {
            "commandEndTime": 133.5,
            "commandPhase": 5,
            "commandStartTime": 0.0,
            "commandType": "omit_veh"
        },
        {
            "commandEndTime": 133.5,
            "commandPhase": 8,
            "commandStartTime": 0.0,
            "commandType": "omit_veh"
        }
    ]
}
  ```
  - Number and type of commands in the `["Schedule"]` key may vary based on the requirement. Supported values in the `["commandType"]` key are the following:
    - `"omit_veh"`: omit a vehicle phase
    - `"omit_ped"`: omit a pedestrian phase
    - `"hold"`: hold a vehicle phase
    - `"forceoff"`: forceoff a vehicle phase
    - `"call_veh"`: call a vehicle phase
    - `"call_ped"`: call a pedestrian phase
  - When a new schedule is received while executing another schedule, and if new and old schedules have the same `commandType` for the same `commandPhase`, the `commandType`   state of the `commandPhase` is maintained while executing the next schedule.

2. Clearing a currently executing schedule:
  - When the following message is received, TCI clears all scheduled jobs from the current phase control schedule:
    ```
    {
      "MsgType": "Schedule",
      "Schedule": "Clear"
    }
    ```    
3. Current and next phase status:
  - In association with the Map-Spat-Broadcaster component, the TCI provides the current and next phase status when requested. When the following message is received, TCI sends the current and next phase status to the requester:
  ```
  {
	"MsgType": "CurrNextPhaseRequest"
  }
  ```
5. Timing plan request:
  - TCI maintains the current timing plan set in the signal controller, through a series of NTCIP commands executed at the configured interval. WHen the following message is received, the TCI sends the current signal timing plan to the requester:
  ```
  {
  	"MsgType": "TimingPlanRequest"
  }
  ```
7. Special functions:
   - The TCI allows to set a requested special function to `true` for the request time interval. To set any special function to `true`, a message similar to the following can be sent to the TCI:
   ```
   {
	"MsgType": "SpecialFunction",
	"Status": true,
	"Id": 8,
	"StartTime": 2,
	"EndTime" : 60
   }
   ```
   - Note that in the above message, `StartTime` and `EndTime` will be seconds after the message was received.
   - To set any special function to `false`, a message similar to the following can be sent to TCI:
   ```
   {
	"MsgType": "SpecialFunction",
	"Status": false,
	"Id": 8,
   }
   ```
## Console output and logging
If `["Logging"]` key is set to `true` in the `mmitss-phase3-master-config.json` configuration file, the TCI generates log file in that stores the information about received schedules, timely execution of events, and translation of schedule into an initial SPAT table. This information is also displayed on console if `["Console"]` key is set to `true` in the `mmitss-phase3-master-config.json` configuration file

## Requirements
- The Snmp-Engine component of MMITSS forwards the `get` or `set` requests formulated by the Traffic-Controller-Interface to the signal controller. Therefore, for correct functioning of the Traffic-Controller-Interface, Snmp-Engine needs to be started before the Traffic-Controller-Interface.
- Required python packages: APScheduler, bitstring, numpy. These can be installed by running `pip3 install apscheduler bitstring numpy`

## Configuration
1. In the signal controller:
  - NTCIP backup time must be > 0

2. In the `mmitss-phase3-master-config.json` (config) file, following keys need to be assigned with appropriate values:
  - `["HostIp"]`: a `string` specifying the IP address of the host processor
  - `["PortNumber"]["TrafficControllerInterface"]`: an `int` specifying the UDP port to be used by the Traffic COntroller Interface
  - `["PortNumber"]["TrafficControllerCurrPhaseListener"]`: an `int` specifying the UDP port where Traffic Controller Interface listens for current phase information sent by the Map Spat Broadcaster
  - `["PortNumber"]["TrafficControllerTimingPlanSender"]`: an `int` specifying the UDP port using which the traffic controller sends the current timing plan to requester
  - `["SignalController"]["IpAddress"]`: a `string` specifying the IP address of the signal controller
  - `["SignalController"]["NtcipPort"]`: an `int` specifying the UDP port where the signal controller listens for the NTCIP `get` and `set` requests
  - `["SignalController"]["TimingPlanUpdateInterval_sec"]`: an `int` specifying the seconds after which the traffic controller interface attempts to see if the timing plan has changed in the signal controller 
  - `["SignalController"]["Vendor"]`: a `string` specifying the vendor of the signal controller. Currently Traffic Controller Interface is tested with Econolite and MaxTime signal controllers
  - `["SignalController"]["TimingPlanMib"]`: a `string` specifying the location of the `*.py` file that stores the timing plan object indentifiers.


## Known issues/limitations
1. Special Functions:
If the special function is already active (and being maintained), and another request for activating the "same" special function is received during this time (probably to extend the activation time), currently this request is discarded.
