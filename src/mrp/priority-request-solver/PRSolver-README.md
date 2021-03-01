# Software Component Description: Priority-Request-Solver (PRSolver)
The Priority-Request-Solver software component is responsible for formulating the optimization model based on priority request list, current traffic signal plan, current traffic signal status. It generates optimal signal timing schedule as a solution of the optimization problem. A GNU GLPK package (version 4.55) is used to solve the optimization model.The optimal solution is written on Results.txt file The optimal schedule is send to the TrafficControllerStatus(TCI) to serve the priority requests.
## Work-flow
The PRSolver receives priority requests list from the PriorityRequestServer(PRS). It receives current signal timing plan(static) from the TCI. It also receives split data from SignalCoordinationRequestGenerator if there is an active coordination plan. The PRSolver requests for current signal status to TCI. It writes the optimization model in the NewModel.mod (for tranit, truck or coordination priority requests) or NewModel_EV.mod(for emergency vehicle prioriry requests) file. The input for the optimization model is written in the NewModelData.dat file. A JSON formatted optimal signal timing schedule is sent to the TCI over the UDP socket. The PRSolver composed of three class- (1) SolverDataManager (2) Schedulemanager, and (3) PriorityRequestSolver.

### SolverDataManager Class
The SolverDataManager class manages the data required for solving the optimization model. It writes the dat file (NewModelData.dat) in the /nojournal/bin directory, required for solving the optimization model. The dat file contains the  information about current signal status (starting phases, time required to start next phase if current phase is on clearance interval, elapsed green time if current phase is on green), static signal timing plan (yellow change interval, red clearance interval, minimum green time and maximum green time for all the active phases), priority type (transit, truck, EV etc.), priority weights, ETA (earliest arrival time and latest arrival time) of each priorrity requests. An example of such NewModelData.dat file is as follows:

```
data;
param SP1:=4;
param SP2:=8;
param init1:=0;
param init2:=0;
param Grn1 :=15;
param Grn2 :=13;
param y          	:=	1	3	2	4	3	3	4	3.6	5	3	6	4	7	3	8	3.6;
param red          	:=	1	1	2	2.5	3	1	4	3.4	5	1	6	2.5	7	1	8	3.4;
param gmin      	:=	1	4	2	15	3	4	4	15	5	4	6	15	7	4	8	15;
param gmax      	:=	1	13	2	35.075	3	8	4	17	5	13	6	35.075	7	10	8	15;
param priorityType:= 1 2 2 5 3 5 4 0 5 0 6 0 7 0 8 0 9 0 10 0  ;  
param PrioWeight:=  1 0 2 0.9 3 0 4 0 5 0.2 6 0 7 0 8 0 9 0 10 0 ; 
param Rl (tr): 1 2 3 4 5 6 7 8:=
1  .	20.4804	.	.	.	.	.	.	
2  .	29.9206	.	.	.	.	.	.	
3  .	.	.	.	.	29.9211	.	.	
;
param Ru (tr): 1 2 3 4 5 6 7 8:=
1  .	28.4804	.	.	.	.	.	.	
2  .	49.9206	.	.	.	.	.	.	
3  .	.	.	.	.	49.9211	.	.	
;
end;
```

### Schedulemanager Class
The Schedulemanager class can read the Results.txt file and develop optimal schedule in a JSON formatted message.
An example of such JSON formatted messages is as follows:
```
{
    "MsgType": "Schedule",
    "Schedule": [
        {
            "commandEndTime": 2.0,
            "commandPhase": 4,
            "commandStartTime": 0.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 13.0,
            "commandPhase": 1,
            "commandStartTime": 9.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 49.920000000000002,
            "commandPhase": 2,
            "commandStartTime": 17.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 60.420000000000002,
            "commandPhase": 3,
            "commandStartTime": 56.420000000000002,
            "commandType": "hold"
        },
        {
            "commandEndTime": 79.420000000000002,
            "commandPhase": 4,
            "commandStartTime": 64.420000000000002,
            "commandType": "hold"
        },
        {
            "commandEndTime": 90.420000000000002,
            "commandPhase": 1,
            "commandStartTime": 86.420000000000002,
            "commandType": "hold"
        },
        {
            "commandEndTime": 109.42,
            "commandPhase": 2,
            "commandStartTime": 94.420000000000002,
            "commandType": "hold"
        },
        {
            "commandEndTime": 119.92,
            "commandPhase": 3,
            "commandStartTime": 115.92,
            "commandType": "hold"
        },
        {
            "commandEndTime": 3.0,
            "commandPhase": 4,
            "commandStartTime": 2.0,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 17.48,
            "commandPhase": 1,
            "commandStartTime": 16.48,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 56.560000000000002,
            "commandPhase": 2,
            "commandStartTime": 55.560000000000002,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 71.060000000000002,
            "commandPhase": 3,
            "commandStartTime": 70.060000000000002,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 92.060000000000002,
            "commandPhase": 4,
            "commandStartTime": 91.060000000000002,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 112.06,
            "commandPhase": 1,
            "commandStartTime": 111.06,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 151.13999999999999,
            "commandPhase": 2,
            "commandStartTime": 150.13999999999999,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 161.63999999999999,
            "commandPhase": 3,
            "commandStartTime": 160.63999999999999,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 28.480413895048045,
            "commandPhase": 2,
            "commandStartTime": 0.0,
            "commandType": "call_veh"
        },
        {
            "commandEndTime": 49.920605659484863,
            "commandPhase": 2,
            "commandStartTime": 0.0,
            "commandType": "call_veh"
        },
        {
            "commandEndTime": 2.0,
            "commandPhase": 8,
            "commandStartTime": 0.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 13.0,
            "commandPhase": 5,
            "commandStartTime": 9.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 49.920000000000002,
            "commandPhase": 6,
            "commandStartTime": 17.0,
            "commandType": "hold"
        },
        {
            "commandEndTime": 60.420000000000002,
            "commandPhase": 7,
            "commandStartTime": 56.420000000000002,
            "commandType": "hold"
        },
        {
            "commandEndTime": 79.420000000000002,
            "commandPhase": 8,
            "commandStartTime": 64.420000000000002,
            "commandType": "hold"
        },
        {
            "commandEndTime": 90.420000000000002,
            "commandPhase": 5,
            "commandStartTime": 86.420000000000002,
            "commandType": "hold"
        },
        {
            "commandEndTime": 109.42,
            "commandPhase": 6,
            "commandStartTime": 94.420000000000002,
            "commandType": "hold"
        },
        {
            "commandEndTime": 119.92,
            "commandPhase": 7,
            "commandStartTime": 115.92,
            "commandType": "hold"
        },
        {
            "commandEndTime": 3.0,
            "commandPhase": 8,
            "commandStartTime": 2.0,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 23.0,
            "commandPhase": 5,
            "commandStartTime": 22.0,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 56.560000000000002,
            "commandPhase": 6,
            "commandStartTime": 55.560000000000002,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 73.060000000000002,
            "commandPhase": 7,
            "commandStartTime": 72.060000000000002,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 92.060000000000002,
            "commandPhase": 8,
            "commandStartTime": 91.060000000000002,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 112.06,
            "commandPhase": 5,
            "commandStartTime": 111.06,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 151.13999999999999,
            "commandPhase": 6,
            "commandStartTime": 150.13999999999999,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 161.63999999999999,
            "commandPhase": 7,
            "commandStartTime": 160.63999999999999,
            "commandType": "forceoff"
        },
        {
            "commandEndTime": 49.921069622039795,
            "commandPhase": 6,
            "commandStartTime": 0.0,
            "commandType": "call_veh"
        }
    ]
}
```
The PRS sends clear request message to the PRSolver when all the priority requests are served (ART is empty). An example of such clear request messages is as follows:
```
{
    "MsgType": "ClearRequest"
}
```
### Managing system performance Data
The PRS store the information of number of SRMs received, and number of SRMs granted or rejected for specified time period. It formulates a JSON formatted message sends it to the Data-Collector. The Data-Collector stores the information in a csv file in /nojournal/bin/v2x-data directory. An example of such JSON formatted system performance data messages is as follows:
```
{
    "MsgType": "MsgCount",
    "PriorityRequestList": {
        "MsgSource": "speedway-mountain",
        "MsgCountType" :"SRM",
        "MsgCount": 186,
        "MsgServed": 154
        "MsgRejected": 32,
        "TimeInterval": 300,
        "Timestamp_posix": 1613780524.06905,
        "Timestamp_verbose": 2021-02-20 00:22:04.68
    }
}
```
## Console output and logging
The PRS can store important information like- SRMs, SSMs etc. in the log files. The log file name depends on the intersection name (specified in the 'mmitss-phase3-master-config.json' configuration file). For example, if intersection name is daisy-gavilan, the logfile name will be PRSLog-daisy-davilan.txt. It is expensive process to write in a file. Therefore, logging is turned off by default. It can be turned on for debugging or analyzing purpose. Logging can be turned on by setting the variable "Logging" as "True" (instead of "False") in the 'mmitss-phase3-master-config.json' configuration file.
The console output also provides some information about the status of the component. The console ouput can be redirected to a file using supervisor if mmitsss is running inside container. The following information is displayed in the console:
- Messages sent or received status
- List of available priority requests in the active request table (ART)

## Requirements
- Install GNU GLPK package (version 4.55).The GLPK (https://www.gnu.org/software/glpk/) provides open-source C/C++ packages that has packages to solve large-scale linear programming (LP), mixed integer programming (MIP), and other related problems.
## Configuration
In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["PortNumber"]["PriorityRequestServer"]`:  UDP port number (integer). 
- `config["SRMTimedOutTime"]`: time-out period to delete priority request from Active Request Table if Infrustracture doesn't receive srm. 
- `config["SystemPerformanceTimeInterval"]`: time interval to log the cumulative system performance data

## Test
A basic test of the PRS software can be done by using a tool (msgSender.py script) reside on mmitss/src/mrp/priority-request-server/Test directory. The msgSender.py can send SRM JSON string to the PRS over the UDP socket. The PRS will populate the ART and sent SSM. 

## Known issues/limitations
- None 