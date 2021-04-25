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
    int vehicleType;
    int vehicleLaneID;
    int vehicleApproachID;
    int prsStatus;
    int minuteOfYear;
    int secondOfMinute;
    int signalGroup;
    double vehicleETA;
    double vehicleETADuration;
    double vehicleLatitude;
    double vehicleLongitude;
    double vehicleElevation;
    double vehicleHeading;
    double vehicleSpeed;
    double msgReceivedTime;
    double etaUpdateTime; 
    void reset()
    {
        vehicleID = 0;
        requestID = 0;
        msgCount = 0;
        basicVehicleRole = 0;
        vehicleType  = 0;
        vehicleLaneID = 0;
        vehicleApproachID = 0;
        prsStatus = 0;
        minuteOfYear = 0;
        secondOfMinute = 0;
        signalGroup = 0;
        vehicleETA = 0.0;
        vehicleETADuration = 0.0;
        vehicleLatitude = 0.0;
        vehicleLongitude = 0.0;
        vehicleElevation = 0.0;
        vehicleHeading = 0.0;
        vehicleSpeed = 0.0;
        msgReceivedTime = 0.0;
        etaUpdateTime = 0.0;
    }
};
