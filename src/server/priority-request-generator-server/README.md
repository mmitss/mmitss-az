# Software Component Description: Priority-Request-Generator Srver
The Priority-Request-Generator-Server (PRG-Server) software component is  responsible for maintaining Priority Request Generator (PRG) list of multiple vehicles. The software is mainly developed for simulating Priority-Request-Generator (PRG) in the simulation environment. It can receive BSMs, Maps and SSMs and generate SRMs for the priority eligible vehicle, if requires.

## Work-flow
The Priority-Request-Generator-Server receives basic safety messages (BSMs) from the simulator (VISSIM). VISSIM use driver-modell.dll to generate BSMs. It also receives MAPs, SSMs from the Message-Distributor. The PRG-Server, use MapManager class to obtain the active map. It has an API to generate SRM JSON string for each vehicle using PriorityRequestGenerator class. The json string is compatible with asn1 j2735 standard. It also manages Active Request Table for each vehicle based on the received ssm.
### Manage BSM of Connected vehicle
The PRG-Server can receive BSMs from connected vehicle. It can either create a new instance of PRG (if BSM is received from a new vehicle) or update the old instance of PRG (if vehicleID is already available in the list) for the priority eligible vehicles and discard other non-priority eligible vehicles. It obtain the vehicle status on the active map (if available). It generates SRMs for all types of priority eligible vehicles (transit, truck, emergency-vehicle), if require. The PRG-Server deletes a timed-out (if it doen't receive BSMs for more than 10 seconds) vehicle information from the list.

## Console output and logging
The PRG-Server does not generate any log files. The console output provides the following important information: 
- The cause of sending SRM and SRM sending status
- SSM received status

## Requirements
- None
## Configuration
In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["PortNumber"]["PriorityRequestGenerator"]`:  UDP port number (integer) 
- `config["PortNumber"]["HMIController"]`:  UDP port number (integer)
- `config["SRMTimedOutTime"]`: time-out period to delete priority request from Active Request Table if Infrustracture doesn't receive srm.

## Test
A basic test of the PRG software can be done by using a tool (bsmSender.py, mapSender.py script) reside on mmitss/src/vsp/priority-request-generator/Test directory. The bsmSender.py and msgSender.py can send BSM MAP JSON string respectively to the PRG over the UDP socket. The PRG will sent SRM if the received map becomes active map.

## Known issues/limitations
- None
