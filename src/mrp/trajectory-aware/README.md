# Trajectory-Aware

The Trajectory-Aware is a roadside component receives JSON strings of BSMs from remote vehicles from Wireless-Message-Decoder, and 
adds additional information about the vehicle describing it's current relation with the intersection's MAP, to the JSON string.

## Work Flow
The Trajectory-Aware on it's initialization, creates an instance of the MapEngine library, using the MAP payload read from the configuration file. When Trajectory-Aware receives a JSON string of a BSM, it extracts the position and motion information of the vehicle and uses this information to locate the vehicle on intersection's MAP. After locating the vehicle on the MAP, following attributes are extracted for the vehicle:
1. `onMap`: a `bool`indicating whether the vehicle is currently on the intersection's MAP
2. `approachId`: an `int` indicating the ID of the approach on which the vehicle is
3. `laneId`: an `int` indicating the ID of the lane on which the vehicle is
4. `signalGroup`: an `int` indicating the signal group (phase) associated with the vehicle's current lane
5. `distanceToStopbar`: a `float` indicating the distance between vehicle's current location and the stopbar associated with it'n current lane
6. `locationOnMap`: a `string` indicating the associated location of the vehicle on the map. Possible values are: "inbound", "outbound", or "insideIntersectionBox"
NOTE: For the vehicles which are not on the MAP, all of above attributes are set to `false`

Above information is packed and appended in the BSM JSON string under the key `OnmapVehicle`. The new JSON string is then sent to the V2X-Data-Collector for storage. If the `MapBsmFiltering` key in the configuration file is set to `true`, then the Trajectory-Aware filters the datapoints that are not on the MAP.

## Console Output and logging
The Trajectory-Aware does not produce any console output or log files.

## Configuration
In the `mmitss-phase3-master-config.json` file, following fields must be configured:
1. `["HostIp"]`: A `string` specifying the IPv4 address of the host machine or the container
2. `["PortNumber"]["TrajectoryAware"]`: an `int` specifying the UDP port used by the Trajectory-Aware
3. `["MapPayload"]`: a `string` containing the UPER-Hex formatted payload of the intersection's MAP, developed using [USDOT ISD MESSAGE CREATOR](https://webapp.connectedvcs.com/isd/)
4. `["MapBsmFiltering"]`: a `bool` indicating whether or not to filter the datapoints that are not on the MAP
5. `["PortNumber"]["DataCollector"]`: an `int` specifying the UDP port used by the V2X-Data-Collector

## Requirements
- None

## Known issues/limitations
- None
