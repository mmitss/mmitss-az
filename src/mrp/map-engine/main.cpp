/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    main.cpp
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:

*/

#include <iostream>
#include <fstream>
#include <string>
#include <UdpSocket.h>
#include <json/json.h>
#include <BasicVehicle.h>
#include "MapEngine.h"

int main()
{
	// Read the mmitss-phase3-master-config.json file and create a JSON object for further use.
	Json::Value configJsonObject;
	Json::Reader configJsonReader;
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");
	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	configJsonReader.parse(configJsonString.c_str(), configJsonObject);

	// Close the file so that it can be oped by other programs if required.
	jsonconfigfile.close();

	// Create JSON object for storing the incoming string (LocateVehicleOnmapStatus).
	Json::Value incomingJsonObject;
	// Create JSON reader for reading the JSON object of the incoming string (LocateVehicleOnmapStatus).
	Json::Reader incomingJsonReader;

	// Stores the outgoingJsonString. The string needs to be obtained using the methods of MapEngine class.
	std::string outgoingJsonString{};

	// Port number for the mapEngine process (as read from the configuration file).
	int mapEnginePort = (configJsonObject["PortNumber"]["MapEngine"]).asInt();
	// Instance of UdpSocket class opened at the correct port number.
	UdpSocket udpSocket(mapEnginePort);

	//  A buffer to temporarily store the incoming message.
	char receiveBuffer[5120];
	// A string to store the data in the receiveBuffer converted into a c++ string.
	std::string receivedMsg{};

	// An object of the MapEngine class to perform the key activities.
	MapEngine mapEngine;

	while (true)
	{
		// Receive new data and store it into the receiveBuffer.
		udpSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));

		// Convert the received message into a c++ string.
		receivedMsg = string(receiveBuffer);

		// Parse the received message considering it is a JSON string.
		incomingJsonReader.parse(receivedMsg.c_str(), incomingJsonObject);

		// Only if the received message is a request to locate the vehicle on the intersection map, then call apropriate methods of the mapEngine and send back the status of the vehicle on the map
		if ((incomingJsonObject["MsgType"]).asString() == "LocateVehicleOnMapRequest")
		{
			outgoingJsonString = mapEngine.getVehicleStatusOnMap(receivedMsg);
			udpSocket.sendData(udpSocket.getSenderIP(), udpSocket.getSenderPort(), outgoingJsonString);
		}
	}

	// Close the socket
	udpSocket.closeSocket();
	return 0;
}