/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    MapEngine.cpp
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:

*/

#pragma once
#include <string>
#include <json/json.h>
#include "locAware.h"
/* This class uses the local copy of locAware.h, as getControlPhaseByLaneId is provate in the global locAware.h file. It needs to be made sure that this class uses the global locAware.h  file once the 3rdparty changes are made and are in the main codeline */

class MapEngine
{
    private:
        // fmap variable stores the name of the map.payload file
        std::string fmap{};
        std::string intersectionName{};
        uint16_t intersectionId{};
        uint16_t regionalId{};

        /* plocAwareLib is a pointer that points to a variable of the type LocAware. This variable is be created in the constructor of this class, as it requires some other parameters that are available in the constructor.*/
        LocAware *plocAwareLib;   

    public:
        /*The MapEngine class reads the map paylod from the mmitss-master-config file, and stores it into a file. Using this file, an instance of the LocAware is created in the constructor, and is made available for rest of member functions, by pointing the class attribute pointer to this memory location. This instance of LocAware is reused for all LocateVehicleOnMapRequests.*/
        MapEngine(std::string configFilename);

        /*The destructor of the MapEngine cllass deletes the instance of LocAware class that was created in the constructor.*/
        ~MapEngine();
        
        /*This function reads the intersection map payload from the mmitss-phase3-master-config.json file, and stores it into a *.map.payload file. The name of the *.map.payload file is returned from this function.
        This function also reads the lla coordinates of the intersection from the configuration file and stores them into the class attributes.*/
        std::string readIntersectionMapConfig(std::string configFilename);

        bool isVehicleOnMap(double latitude, double longitude, double elevation, double speed, double heading);

        /*This function uses the instance of the LocAware object to locate a vehicle on the map. The vehicle information needs to provided as an argument to this function as a JSON string of LocateVehicleOnMapRequest message. An example for the JSON string is provided in the ./test/LocateVehicleOnMapRequest.json file. This function returns a JSON string containing the LocateVehicleOnMapRequest message. Examples of the LocateVehicleOnMapStatus is provided in the ./test/LocateVehicleOnMapStatus*.json.*/
        std::string getOnMapCsv(double latitude, double longitude, double elevation, double speed, double heading);
};