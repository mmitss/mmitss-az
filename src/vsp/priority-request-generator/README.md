# Software Component Description: Priority-Request-Generator 
The Priority-Request-Generator (PRG) software component is responsible for generating signal request messages (SRMs) for the priority eligible connected vehicles. SRMs are generated based on the gps coordinates and received maps of the intersection geometry. It also generates updated priority request for a change in vehicle status (speed drops, passed the intersection etc.). The PRG receives the signal status messages (SSMs) from the roadside as an acknowledgement message. It stores the SSM in the active request table (ART). The PRG can send information about the priority request status priority eligible vehicles, and presence of other connected vehicles and available map status to the HMI controller software component. 

## Work-flow
The Priority-Request-Generator receives basic safety messages (BSMs) at the frequency of 10 hz. It also receives MAPs, SSMs from the intersection that are within the range of the wireless device. The PRG composed of three class- (1) MapManager (2) PriorityRequestGeneratorStatus, and (3) PriorityRequestGenerator.

### MapManager Class
The Mapmanager contains API to manage the available map list (contain all the received maps) and active map list (contain only the map which is active based on BSM data). It can also delete the old maps if map message for an intersection is not received for more than 5 minutes.
#### Get Active Map
A function/method called createActiveMapList(BasicVehicle basicVehicle) is responsible for obtaining the active map from the available map list. The method use BSM data (vehiclesGPS coordinates,  speed, heading) and map-engine library (The locateVehicleInMap(const GeoUtils::connectedVehicle_t& cv, GeoUtils::vehicleTracking_t& cvTrackingState) function ) to locate the vehicle on a map. 

### PriorityRequestGeneratorStatus Class
The responsibility of PriorityRequestGeneratorStatus class is to formulate a JSON string for HMI controller. The JSON string contains the priority of all the priority eligible vehicles based on the ART, remote connected vehicles information and all the received map status. An example of such JSON formatted messages is as follows:
```
{
	"PriorityRequestGeneratorStatus" : 
	{
		"hostVehicle" : 
		{
			"heading_Degree" : 18.47,
			"laneID" : 2,
			"position" : 
			{
				"elevation_Meter" : 540.70902794320136,
				"latitude_DecimalDegree" : 33.861352171815518,
				"longitude_DecimalDegree" : -112.11117735626212
			},
			"priorityStatus" : 
			{
				"OnMAP" : "False",
				"requestSent" : "False"
			},
			"secMark_Second" : 59500.0,
			"signalGroup" : 4,
			"speed_MeterPerSecond" : 15.662807475285053,
			"vehicleID" : 26412641,
			"vehicleType" : 6
		},
		"infrastructure" : 
		{
			"activeRequestTable" :
			[
				{
					"vehicleID": 26412641,
					"requestID”: 5
                    "msgCount”: 98
                    "basicVehicleRole”: 16
                    "inBoundLane”: 2
                    "inBoundApproach”: 1
                    “vehicleETA”: 28.0
                    "duration: 4.0
                    "priorityRequestStatus": 4
				}
			],
			"availableMaps" : 
			[
				{
					"DescriptiveName" : "Map22391",
					"IntersectionID" : 22391,
					"active" : "False",
					"age" : 2.0
				}
			]
		}
	}
}
```

### PriorityRequestGenerator Class
PriorityRequestGenerator has the functionality to generate signal request messages(SRMs) for the priority eligible vehicles and store the signal status messages (SSMs) in the active request table (ART). It call an instance of MapManager class to locate the vehicle on a Map. 
#### SRM Generating Condition
If there is an active map,  SRM can be generated for following reasons-
- ART is empty (requestType:PriorityRequest)
- Vehicles requested signalGroup is changed
- Vehicles speed is changed by a threshold value (4 m/s) compare to the ART (requestType: UpdateRequest)
- Vehicles ETA is changed by a threshold value (6 seconds) compare to the ART (requestType: UpdateRequest)
- msgCount does not match on the ART (requestType: UpdateRequest)
- To avoid PRS timed out (requestType: UpdateRequest)
- Vehicle is either leaving or not in inBoundlane of the Intersection (requestType :CanellationRequest)
- Vehicle is out off the map (requestType: CanellationRequest)
Truck, Transit vehicle (Bus), Emergency vehicle can send SRMs. Truck sends SRM if it satisfies any of the above conditions. Transit vehicle sends SRM if it satisfies any of the above conditions and passes the bus-stop (if available on that particular active map route). Emergency vehicle sends SRM if it satisfies any of the above conditions and light-siren is active. An example of such JSON formatted SRMs is as follows:
```
{   
    "MsgType":"SRM",
    "SignalRequest":
    {
        "msgCount": 6,
        "minuteOfYear":345239,
        "msOfMinute":54000,
        "regionalID":0,
        "intersectionID":26379,
        "priorityRequestType": 1,
        "vehicleID": 610,
        "basicVehicleRole": 9,
        "vehicleType": 9,
        "inBoundLane":
        {
            "LaneID": 12,
            "ApproachID": 1
        },
        "expectedTimeOfArrival":
        {
            "ETA_Minute": 0,
            "ETA_Second" : 38.0,
            "ETA_Duration": 4.0
        },
        "position":
        {
            "latitude_DecimalDegree": 32.1256713,
            "longitude_DecimalDegree": -110.15623892,
            "elevation_Meter": 712.0
        },
        "heading_Degree": 212.28,
        "speed_MeterPerSecond": 16.78
    }
}
```

#### Managing SSM
The PRG accpts the SSM, if the intersectionID and regionalID of the vehicle and intersectionID and regionalID of the SSM match. The PRG clears the ART when it received new ssm and create a new ART. An example of such JSON formatted SSM is as follows:
```
{
	"MessageType" : "SSM",
	"noOfRequest" : 2,
	"SignalStatus" : 
	{
		"minuteOfYear" : 345239,
		"msOfMinute" : 54000,
        "intersectionID" : 26379,
		"regionalID" : 0,
		"sequenceNumber" : 4,
		"updateCount" : 4,
		"requestorInfo" : 
		[
			{
				"ETA_Duration" : 4.0,
				"ETA_Minute" : 0,
				"ETA_Second" : 20.0,
				"basicVehicleRole" : 16,
				"inBoundLaneID" : 8,
				"msgCount" : 12,
				"priorityRequestStatus" : 4,
				"requestID" : 5,
				"vehicleID" : 601
			},
			{
				"ETA_Duration" : 4.0,
				"ETA_Minute" : 0,
				"ETA_Second" : 38.0,
				"basicVehicleRole" : 9,
				"inBoundLaneID" : 12,
				"msgCount" : 6,
				"priorityRequestStatus" : 4,
				"requestID" : 5,
				"vehicleID" : 610
			}
		]
	}
}
```

## Console output and logging
The PRG can store important information like- SRMs, SSMs etc. in the time-stamped log files. The log file name is a function of the date and time. For example, if date is April 28,2021 and time is 01:14:27 pm the logfile name will be vehicle_prgLog_04292021_131427.log. The log file stored in nojournal/bin/log directory. It is expensive process to write in a file or display output in the console. Therefore, logging and displaying console output are turned off by default. It can be turned on for debugging or analyzing purpose. Logging and displaying console output can be turned on by setting the variable "Logging" as true, "ConsoleOutput" as true (instead of false) in the 'mmitss-phase3-master-config.json' configuration file. The console output console output can be redirected to a file using supervisor, if mmitss is running inside the container. The following information is displayed in the console:
- The cause of sending SRM
- SSM received status

## Requirements
- None

## Configuration
In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["PortNumber"]["PriorityRequestGenerator"]`:  UDP port number (integer) 
- `config["PortNumber"]["HMIController"]`:  UDP port number (integer)
- `config["SRMTimedOutTime"]`: time-out period to delete priority request from the Active Request Table, if infrastructure doesn't receive SRM.

## Test
A basic test of the PRG software can be done by using a tool (bsmSender.py, mapSender.py scripts) reside on mmitss/src/vsp/priority-request-generator/test directory. The bsmSender.py and msgSender.py can send BSM MAP JSON string respectively to the PRG over the UDP socket. The PRG will sent SRM if the received map becomes active map.

## Known issues/limitations
- None
