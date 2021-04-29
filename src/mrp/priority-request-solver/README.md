# Software Component Description: Priority-Request-Solver (PRSolver)
The Priority-Request-Solver software component is responsible for formulating the optimization model based on priority request list, current traffic signal plan, current traffic signal status. It generates optimal signal timing schedule as a solution of the optimization problem. A GNU GLPK package (version 4.55) is used to solve the optimization model. The optimal signal timing schedule is sent to the TrafficControllerStatus (TCI) to serve the priority requests.
The GLPK (https://www.gnu.org/software/glpk/) provides open-source C/C++ packages that has packages to solve mixed integer programming (MIP), large-scale linear programming (LP), and other related optimization problems.

## Work-flow
The PRSolver receives priority requests list from the PriorityRequestServer(PRS) and current signal timing plan(static) from the TCI. It also receives split data from the SignalCoordinationRequestGenerator, if there is an active coordination plan. The PRSolver requests for current signal status to the TCI. It writes the optimization model in the OptimizationModel.mod (for transit, truck or coordination priority requests) or OptimizationModel_EV.mod (for emergency vehicle priority requests) file. The input for the optimization model is written in the OptimizationModelData.dat file. A JSON formatted optimal signal timing schedule is sent to the TCI over the UDP socket. The PRSolver composed of five class- (1) TrafficConrtollerStatusManager (2) SolverDataManager  (3) OptimizationModelManager (4)PriorityRequestSolver, and (4) Schedulemanager.

### TrafficConrtollerStatusManager class
TrafficConrtollerStatusManager class has the functionality to manage the current traffic signal status, received from the TCI. It can validate the received signal status. It can modify the received signal status, if requires. For example- if current phase is on green rest, it will set the elapsed time as min green value for transit or truck priority request. An example of such JSON formatted current signal status message is as follows:
```
{
    "currentPhases": [
        {
            "Phase": 2,
            "State": "green",
            "ElapsedTime": 85
        },
        {
            "Phase": 6,
            "State": "green",
            "ElapsedTime": 200
        }
    ],
    "MsgType": "CurrNextPhaseStatus",
    "nextPhases": [
        0
    ]
}
```

### SolverDataManager Class
The SolverDataManager class manages the data required for solving the optimization model. It writes the dat file (OptimizationModelData.dat) in the /nojournal/bin directory, required for solving the optimization model. The dat file contains the  information about current signal status (starting phases, time required to start next phase if current phase is on clearance interval, elapsed green time if current phase is on green), static signal timing plan (yellow change interval, red clearance interval, minimum green time and maximum green time for all the active phases), priority type (transit, truck, EV etc.), priority weights, ETA (earliest arrival time and latest arrival time) of each priority requests. An example of such OptimizationModelData.dat file is as follows:
```
data;
param SP1:=4;
param SP2:=8;
param init1:=0;
param init2:=0;
param Grn1 :=15;
param Grn2 :=13;
param y    :=	1	3	2	4	3	3	4	3.6	5	3	6	4	7	3	8	3.6;
param red  :=	1	1	2	2.5	3	1	4	3.4	5	1	6	2.5	7	1	8	3.4;
param gmin :=	1	4	2	15	3	4	4	15	5	4	6	15	7	4	8	15;
param gmax :=	1	13	2	35.07	3	8	4	17	5	13	6	35.07	7	10	8	15;
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

### OptimizationModelManager Class
OptimizationModelManager class has the functionality to write the optimization model file for the different types of priority requests. The static optimization model file for the truck, transit, or coordination is OptimizationModel.mod. The dynamic optimization model file for the emergency vehicle is OptimizationModel_EV.mod.

### PriorityRequestSolverClass
PriorityRequestSolverClass has the functionality to manage the priority requests in a list, the current signal timing plan, split data and current signal status. It can also manage dilemma zone request list if emergency vehicle is sending priority requests while heavy vehicles are trapped in the dilemma zone on the opposite approaches. It can provide the optimization model and input data to glpk solver package as an input to solve the optimization problem. The optimal solution is written on OptimizationResults.txt file. The optimal soultion can be validated. The first two line of the file contains the information about current signal status (starting phases, time required to start next phase if current phase is on clearance interval, elapsed green time if current phase is on green). The next six lines are the phase duration (first three lines are left critical points of "Hold" points and next three lines are are right critical points or "Force-Off" points) for each phases in cyle 1-3. There are green time for each phases in cycle 1-3 afterwards. The file can contain the information about ETA of the all priority requests and their corresponding delay. An example of such OptimizationResults.txt file is as follows:
```
   4    8 
  0.00   0.00 15.00  13.00 
  0.00   0.00   0.00   9.00   0.00   0.00   0.00   9.00   
  8.00  39.42   8.00  22.00   8.00  39.42   8.00  22.00   
  8.00  21.50   8.00   0.00   8.00  21.50   8.00   0.00   
  0.00   0.00   0.00   9.00   0.00   0.00   0.00   9.00   
 11.48  41.58  12.00  24.00  17.00  36.06  14.00  22.00   
 17.00  41.58   8.00   0.00  17.00  41.58   8.00   0.00   
  0.00   0.00   0.00   2.00   0.00   0.00   0.00   2.00   
  4.00  32.92   4.00  15.00   4.00  32.92   4.00  15.00   
  4.00  15.00   4.00   0.00   4.00  15.00   4.00   0.00   
  0.00   0.00   0.00   2.00   0.00   0.00   0.00   2.00   
  7.48  35.08   8.00  17.00  13.00  29.56  10.00  15.00   
 13.00  35.08   4.00   0.00  13.00  35.08   4.00   0.00   
   3 
 2  20.48  28.48   0.00 2 
 2  29.92  49.92   0.00 5 
 6  29.92  49.92   0.00 5 
  0.00 
 190.97 
```

### ScheduleManager Class
The ScheduleManager class can read the OptimizationResults.txt file and develop optimal schedule in a JSON formatted message. The schedule contains the information about start time, end time of different commands, like- Hold, Force-Off, Omit, Vehicle-Call etc. for each phase. The optimal schedule contains the optimal signal timing plan for two cycles. An example of such JSON formatted optimal schedule is as follows:
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
            "commandEndTime": 49.92,
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
            "commandEndTime": 49.92,
            "commandPhase": 6,
            "commandStartTime": 0.0,
            "commandType": "call_veh"
        }
    ]
}
```

## Console output and logging
The PRSolver can store important information like- priority request list, current signal status, input data for optimization model, optimal schedule etc. in the time-stamped log files. The log file name is a function of the intersection name (specified in the 'mmitss-phase3-master-config.json' configuration file), date and time. For example, if intersection name is daisy-gavilan, date is April 28,2021 and time is 01:14:27 pm the logfile name will be daisy-davilan_prsolverLog_04292021_131427.log. The log file stored in nojournal/bin/log directory. It is expensive process to write in a file or display output in the console. Therefore, logging and displaying console output are turned off by default. It can be turned on for debugging or analyzing purpose. Logging and displaying console output can be turned on by setting the variable "Logging" as true, "ConsoleOutput" as true (instead of false) in the 'mmitss-phase3-master-config.json' configuration file. The console output console output can be redirected to a file using supervisor, if mmitss is running inside the container. The following information is displayed in the console:
- Messages (signal timing plan, split data, current signal status, priority request, or clear request received status, optimal schedule sent status etc.) sent or received status
- GLPK solver output

## Requirements
- The TCI and SNMP-Engine are required to run along with the PRSolver

## Configuration
In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["PortNumber"]["PrioritySolver"]`: UDP port number (integer).
- `config["PortNumber"]["PrioritySolverToTCIInterface"] `: UDP port number (integer). This port is used for obtaining current signal status message from the TCI.

## Test
A basic test of the PRSolver software can be done by using tools reside on mmitss/src/mrp/priority-request-solver/test directory. The priorityRequestSender.py can send priority request list as a JSON string to the PRSolver over the UDP socket. The tciMsgSender.py can send current signal timing plan, and current signal status. It can also receive the optimal schedule. The splitDataSender.py can send split data to the PRSolver. The PRSolver can formulate the optimization model and generate optimal schedule.

## Known issues/limitations
- Optimization model can serve 15 priority requests simultaneously.
