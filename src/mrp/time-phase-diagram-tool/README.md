# Software Component Description: Time-Phase-Diagram-Tool
The Time-Phase-Diagram-Tool software component generates time-phase-diagramfor optimal solution and indicates the timestamp when optimal solution is not achieved. The Time-Phase-Diagram-Tool composed of two class- (1) OptimizationResultsManager (2) TimePhaseDiagramManager.

## Work-flow
The Time-Phase-Diagram-Tool receives a JSON formatted message when the Priority-Request-Solver solves an optimization problem. An example of such JSON formatted messages is as follows:
```
{
    "MsgType": "TimePhaseDiagram",
    "OptimalSolutionStatus": True
}
```
The Time-Phase-Diagram-Tool calls methods from the OptimizationResultsManager class to read the optimal solution from the OptimizationResults.txt file (if there is an optimal solution). Next, methods from the TimePhaseDiagramManager is utilized to generate and store the diagrams. The diagrams are store in the //nojournal/bin/performance-measurement-diagrams/time-phase-diagram directory.

## Requirements
- Matplotlib

## Configuration
In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["PortNumber"]["TimePhaseDiagramTool"]`:  UDP port number (integer) 
- `config["TimePhaseDiagram"]`: boolean to check if the component requires to generate the time-phase diagram.

## Known issues/limitations
- The component stores a specified number of diagrams. It deletes the old diagrams if required to store new diagram.
- The software component consumes lots of cpu if it runs on CVCP.