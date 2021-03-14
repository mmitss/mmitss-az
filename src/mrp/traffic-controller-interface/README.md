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
            "commandEndTime": 8,
            "commandPhase": 4,
            "commandStartTime": 0.0,
            "commandType": "omit_veh"
        },
        {
            "commandEndTime": 19.65,
            "commandPhase": 1,
            "commandStartTime": 9.1,
            "commandType": "omit_veh"
        },
        {
            "commandEndTime": 80.0,
            "commandPhase": 2,
            "commandStartTime": 23.65,
            "commandType": "omit_veh"
        },
        {
            "commandEndTime": 101.35,
            "commandPhase": 3,
            "commandStartTime": 86.69999999999999,
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
    }```    
3. Current and next phase status
4. Timing plan request
5. Special functions


## Console output and logging
Traffic-Controller-Interface component does not generate any log files. The console output displays the execution of the scheduled events.

## Requirements
- The Snmp-Engine component of MMITSS forwards the `get` or `set` requests formulated by the Traffic-Controller-Interface to the signal controller. Therefore, for correct functioning of the Traffic-Controller-Interface, Snmp-Engine needs to be started before the Traffic-Controller-Interface.
- Python package APScheduler needs to be installed. For Python3, it can be installed via `pip3 install apscheduler`.

## Configuration
1. In the signal controller:
  - NTCIP backup time must be > 0

2. In the `mmitss-phase3-master-config.json` (config) file, following keys need to be assigned with appropriate values:


## Known issues/limitations
- None -
