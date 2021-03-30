# Wireless Message Decoder

The Wireless-Message-Decoder application is a common (vehicle and intersection) application that listens for V2X messages broadcasted by surrounding wireless devices (both: OBUs as well as RSUs).

## Work-flow
Upon receipt of a message packet, Wireless-Message-Decoder extracts the payload and inspects the type of the message. If the message is from another peer (that is if RSU receives MAP/SPAT/SSM from another RSU and forwards it to the roadside processor or if OBU receives SRM from another OBU and forwards it to the vehicleside processor), the message is decoded only if the `["PeerDataDecoding"]` key of the `mmitss-phase3-master-config.json` file is set to `true`. Otherwise, such messages are discarded.

After inspecting the message type, the rest of the message is decoded into a JSON string. Depending upon the type of the message, The JSON string is then sent to the different MMITSS components for further processing, and to the V2X-Data-Collector for storage. In addition, the message decoder also keeps a track of received messages and sends the message containing performance data counts to the V2X-Data-Collector at configured intervals.

## Console output and logging
Wireless-Message-Decoder does not produce any console output or log files.

## Requirements
1. The RSU/OBU must be configured to forward the messages received over the wireless network to the Wireless-Message-Decoder of the corresponding coprocessor
2. Undisrupted physical network connection between the RSU/OBU and the corresponding coprocessor (MRP/VSP)

## Configuration
In the `mmitss-phase3-master-config.json` file, following fields need to be configured:
1. Common to both: Intersection and Vehicle
  - `["HostIp"]`: a `string` specifying the IP address of the host processor of the Host-BSM-Decoder (vehicleside coprocessor)
  - `["ApplicationPlatform"]`: a `string` specifying the application platform. Supported application platform are `"vehicle"` and `"roadside"`
  - `["PeerDataDecoding"]`: a `bool` specifying whether or not to decode the messages received from surrounding peers (for vehicles, peers are surrounding vehicles, whereas for intersections peers are surrounding intersections)
  - `["IntersectionName"]`: a `string` specifying the name of the intersection in the roadside deployment or a private vehicle identifier in the vehicle side deployment
  - `["SystemPerformanceTimeInterval"]`: an `int` specifying the time interval after which the system performance data (Message Counts) is sent to the V2X-Data-Collector
  - `["PortNumber"]["MessageTransceiver"]["MessageDecoder"]`:
  - `["PortNumber"]["DataCollector"]`: an `int` specifying the UDP port number used by the V2X-Data-Collector

2. In roadside deployment only:
  - `["MapPayload"]`: a `string` containing the MAP payload created using [USDOT ISD Message Creator](https://webapp.connectedvcs.com/isd/)
  - `["PortNumber"]["PriorityRequestServer"]`: an `int` specifying the UDP port number used by the Priority-Request-Server
  - `["PortNumber"]["OBUBSMReceiver"]`: an `int` specifying the UDP port number used by the Bsm-Locator

3. In vehicle side deployment only:
  - `["HMIControllerIP"]`: a `string` specifying the IP address of the remote machine hosting HMI applications
  - `["PortNumber"]["HMIController"]`: an `int` specifying the UDP port number used by the HMI Controller on the remote machine
  - `["PortNumber"]["PriorityRequestGenerator"]`: an `int` specifying the UDP port number used by the Priority-Request-Generator

  
## Known issues/limitations
- None -
