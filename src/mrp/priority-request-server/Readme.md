# Software Component Description: Priority-Request-Server (PRS)
The Priority-Request-Server software component is responsible for managing (adding, updating, or deleting) and prioritizing the priority requests received from the multiple priority eligible vehicles (signal request messages (SRMs)) or Signal-Coordination-Request-Generator (coordination requests). The PRS constructs an Active Request Table (ART) to store the information of priority requests. If the PRS receives priority request from an emergency vehicle, it appends a split phase request into the ART. It formulates a JSON formatted message based on the ART. The message is sent over the socket to the Priority-Request-Solver (PRSolver) component. The PRS also generates signal status messages (SSMs). The SSMs are acknowledgement messages for the connected vehicles. The JSON formatted priority requests and SSMs follow ASN1 J2735 2016 standards.

## Work-flow
The PRS receives priority request either from MsgDecoder or Signal-Coordination-Request-Generator. It stores the accepted priority requests information in the ART. It updates ETA in the ART and deletes timed-out requests, if no SRM is received for specified time period (currently 8 seconds but can be changed). It sends message to the PRSolver as an input if there are any significant updates (new requests received, ART is empty) in the ART. The PRS stores the system performance data and sends it to the Data-Collector. All kind of message communication is performed over the UDP socket. 

### Managing received priority requests
The PRS can receive priority requests from all the priority eligible vehicles which are within the range of wireless device. It only accepts those priority requests which are corresponds to its intersection (matches the IntersectionID field in the SRM). The PRS adds the new priority requests in the ART, updates the information if corresponding request is already available in the ART or delete the priority requests which are already served.  An example of such JSON formatted Priority Request is as follows:
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

### Constructing SSMs:
The PRS formulates a JSON formatted SSM as an acknowledgement message whenever it receives priority requests. It updates the ETA in the ART, every second elapsed and formulates SSM. An example of such JSON formatted SSM is as follows:
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

### Creating inputs for PRSolver
The PRSolver requires priority requests list as an input to formulate the optimization problem. The PRS packs the priority list as a JSON formatted message every time it receives priority requests. An example of such JSON formatted messages is as follows:
```
{
    "MsgType": "PriorityRequest",
    "PriorityRequestList": {
        "intersectionID": 26379,
        "minuteOfYear": 345239,
        "msOfMinute": 54000,
        "noOfRequest": 2,
        "regionalID": 0,
        "requestorInfo": [
            {
                "ETA": 20.0,
                "ETA_Duration": 4.0,
                "basicVehicleRole": 16,
                "elevation_Meter": 715.0,
                "heading_Degree": 118.56,
                "inBoundLaneID": 8,
                "latitude_DecimalDegree": 32.1257835,
                "longitude_DecimalDegree": -110.15624931,
                "priorityRequestStatus": 4,
                "requestedSignalGroup": 4,
                "speed_MeterPerSecond": 14.5,
                "vehicleID": 601,
                "vehicleType": 6
            },
            {
                "ETA": 38.0,
                "ETA_Duration": 4.0,
                "basicVehicleRole": 9,
                "elevation_Meter": 712.0,
                "heading_Degree": 212.28,
                "inBoundLaneID": 12,
                "latitude_DecimalDegree": 32.1256713,
                "longitude_DecimalDegree": -110.15623892,
                "priorityRequestStatus": 4,
                "requestedSignalGroup": 2,
                "speed_MeterPerSecond": 16.78,
                "vehicleID": 610,
                "vehicleType": 9
            }
        ]
    }
}
```
The PRS sends clear request message to the PRSolver when all the priority requests are served (ART is empty). An example of such clear request messages is as follows:
```
{
    "MsgType": "ClearRequest"
}
```

### Managing System Performance Data
The PRS store the information of number of SRMs received, and number of SRMs granted or rejected for specified time period. It formulates a JSON formatted message sends it to the Data-Collector. The Data-Collector stores the information in a csv file in /nojournal/bin/v2x-data directory. An example of such JSON formatted system performance data messages is as follows:
```
{
    "MsgType": "MsgCount",
    "PriorityRequestList": {
        "MsgSource": "speedway-mountain",
        "MsgCountType": "SRM",
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
The PRS can store important information like- SRMs, SSMs etc. in the time-stamped log files. The log file name is a function of the intersection name (specified in the 'mmitss-phase3-master-config.json' configuration file), date and time. For example, if intersection name is daisy-gavilan, date is April 28,2021 and time is 01:14:27 pm the logfile name will be daisy-davilan_prsLog_04292021_131427.log. The log file stored in nojournal/bin/log directory. It is expensive process to write in a file or display output in the console. Therefore, logging and displaying console output are turned off by default. It can be turned on for debugging or analyzing purpose. Logging and displaying console output can be turned on by setting the variable "Logging" as true, "ConsoleOutput" as true (instead of false) in the 'mmitss-phase3-master-config.json' configuration file. The console output console output can be redirected to a file using supervisor, if mmitss is running inside the container. The following information is displayed in the console:
- Messages sent or received status
- List of available priority requests in the active request table (ART)

## Requirements
- None

## Configuration
In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["PortNumber"]["PriorityRequestServer"]`:  UDP port number (integer). 
- `config["SRMTimedOutTime"]`: time-out period to delete priority request from Active Request Table if Infrustracture doesn't receive srm. 
- `config["SystemPerformanceTimeInterval"]`: time interval to log the cumulative system performance data

## Test
A basic test of the PRS software can be done by using a tool (msgSender.py script) reside on mmitss/src/mrp/priority-request-server/Test directory. The msgSender.py can send SRM JSON string to the PRS over the UDP socket. The PRS will populate the ART and sent SSM. 

## Known issues/limitations
- None