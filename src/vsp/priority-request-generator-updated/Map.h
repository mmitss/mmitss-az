/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  Map.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is contains the data structure for MapManager
*/

#pragma once
#include "string"

namespace Map
{
    struct AvailableMap
    {
        std::string availableMapPayload;
        std::string availableMapFileName;
        std::string availableMapFileDirectory;
        int mapIntersectionID;
        double mapAge;
        int minuteOfYear;
        int secondOfMinute;
        std::string activeMapStatus;
        void reset() 
        {
            availableMapPayload = "";
            availableMapFileName = "";
            availableMapFileDirectory = "";
            mapIntersectionID = 0;
            mapAge = 0; 
            minuteOfYear = 0;
            secondOfMinute = 0;
            activeMapStatus = "False";
        }
        
    };
    struct ActiveMap
    {
        std::string activeMapFileName;
        std::string activeMapFileDirectory;
        void reset()
        {
            activeMapFileName = "";
            activeMapFileDirectory = "";
        }

    };
}; 