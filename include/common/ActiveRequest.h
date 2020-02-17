/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  ActiveRequest.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is contains the data structure for Active Request Table
*/

#pragma once

struct ActiveRequest
{
    
    int vehicleID;
    int requestID;
    int msgCount;
    int basicVehicleRole;
    int vehicleLaneID;
    int vehicleApproachID;
    double vehicleETA;
    int prsStatus;
    int minuteOfYear;
    int secondOfMinute;
    int signalGroup; 
    void reset()
    {
        vehicleID = 0;
        requestID = 0;
        msgCount = 0;
        basicVehicleRole = 0;
        vehicleLaneID = 0;
        vehicleApproachID = 0;
        vehicleETA = 0.0;
        prsStatus = 0;
        minuteOfYear = 0;
        secondOfMinute = 0;
        signalGroup = 0;
    }
};