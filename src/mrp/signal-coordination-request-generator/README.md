# Software Component Description: Signal-Coordination-Request-Generator
The Signal-Coordination-Request-Generator software component is responsible for managing (generating, updating, or deleting) coordination priority requests and obtaining active coordination plan. It generates a JSON formatted coordination request message which is analogous to signal request message (SRM). The message is sent over the socket to the Priority-Request-Server (PRS) component. The coordination request is generated in the beginning of each cycle for two cycles to control the future of the traffic signal timing. The first coordination request is generated a cycle length + offset time before the start coordination time. For example, if coordination start time is 7 am and cycle length is 90 seconds, and offset is 10 seconds, then the very first coordination request will be generated 100 (90+10) seconds before the 7 am. The Signal-Coordination-Request-Generator updates the ETA of the coordination requests and deletes the old coordination. It sends the coordination requests after a specified time period (8seconds) to avoid PRS timed-out (PRS receives a request it doesn't receive any updates for specified time period (10 seconds)). The Signal-Coordination-Request-Generator check for active coordination plan if requires. It sends the split data and coordination parameter (signal group of the coordinated phases, coordination start time etc.) whenever it finds a new active coordination plan, or any changes in the currently active coordination plan, or it receives an inquiry from the PRSolver.

## Work-flow
The Signal-Coordination-Request-Generator composed of two class- (1) CoordinationPlanManager (2) CoordinationRequestManager. CoordinationRequestManager can generate virtual coordination requests, update the ETA and Coordination Split for each coordination requests and delete the coordination priority request if coordination split value become zero.

### CoordinationPlanManager Class
CoordinationPlanManager has the functionality to check for a active coordination plan. It can store the coordination parameters and split data for the active coordination Plan in the "coordinationParametersDictionary" and "coordinationSplitDataDictionary" dictionaries, respectively. It can also check and clear old active coordination plan.

#### Get Active Coordination Plan Paramter and Split Data
A function/method called getActiveCoordinationPlan(), computes the start time and end time for each coordination plan. If the current time is in between the start time and end time of a coordination plan or time difference between the coordination start time and current time is less than or equal to the cycle length plus offset, that plan is the active coordination plan. The getSplitData() method can create JSON formatted message to provide coordination plan information to the PRSolver. An example of such JSON formatted messages is as follows:
```
{
    "MsgType": "ActiveCoordinationPlan",
    "CycleLength": 100,
    "Offset": 0,
    "CoordinationStartTime_Hour": 1,
    "CoordinationStartTime_Minute": 10,
    "CoordinatedPhase1": 2,
    "CoordinatedPhase2": 6,
    "TimingPlan": {
        "NoOfPhase": 8,
        "PhaseNumber": [
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        ],
        "SplitData": [
            25,
            37,
            0,
            38,
            24,
            38,
            0,
            38
        ]
    }
}
```

### CoordinationRequestManager Class
CoordinationRequestManager has the functionality to generate virtual coordination requests, update the "ETA" and "Coordination Split" for each coordination requests and delete the coordination priority request if coordination split value become zero.

#### Generate virtual coordination requests
If there is active coordination plan, it is required to generate virtual coordination request- (1) if time gap between the current time and coordination start time is less than equal to the cycle length, or (2) in the beginning of each cycle. There will be four coordination requests, two for each cycle (for two coordinated phase). The ETA of the coordination requests for cycle 1 is offset and for cycle 2 is offset plus cycle length. A JSON formatted message contains these four coordination requests. An example of such JSON formatted coordination priority request message is as follows:
```
{
    "MsgType": "CoordinationRequest",
    "noOfCoordinationRequest": 4,
    "minuteOfYear": 75406.0,
    "msOfMinute": 59000.0,
    "CoordinationRequestList": {
        "requestorInfo": [
            {
                "requestedPhase": 2,
                "vehicleType": 20,
                "vehicleID": 1,
                "basicVehicleRole": 11,
                "ETA": 10,
                "CoordinationSplit": 20.0,
                "priorityRequestType": 1,
            },
            {
                "requestedPhase": 6,
                "vehicleType": 20,
                "vehicleID": 2,
                "basicVehicleRole": 11,
                "ETA": 10,
                "CoordinationSplit": 20.0,
                "priorityRequestType": 1,
            },
            {
                "requestedPhase": 2,
                "vehicleType": 20,
                "vehicleID": 3,
                "basicVehicleRole": 11,
                "ETA": 100,
                "CoordinationSplit": 20.0,
                "priorityRequestType": 1,
            },
            {
                "requestedPhase": 6,
                "vehicleType": 20,
                "vehicleID": 4,
                "basicVehicleRole": 11,
                "ETA": 100,
                "CoordinationSplit": 20.0,
                "priorityRequestType": 1,
            }
        ]
    }
}
```
## Console output and logging
The Signal-Coordination-Request-Generator does not generate any log files. The console output also provides some important information about the status of the component. The console output can be redirected to a file using supervisor if mmitss is running inside container. The following information is displayed in the console:
- Active Coordination plan data
- Coordination priority request messages

## Requirements
- None

## Configuration
In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["PortNumber"]["SignalCoordination"]`:  UDP port number (integer) 
- `config["CoordinationPlanCheckingTimeInterval"]`: time interval to check if there is any update in the active coordination plan

## Known issues/limitations
- None

