/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  ExpectedTimeOfArrival.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script will validate the attributes of ETA and set the values to all fields of ETA
*/

#include <iostream>
#include <time.h>
#include "ExpectedTimeOfArrival.h"

//These values come from J2735 2016 standard
const int ETA_Minute_MinLimit = 0;
const int ETA_Minute_MaxLimit = 527040;
const int ETA_Second_MinLimit = 0;
const int ETA_Second_MaxLimit = 60999; 
const int ETA_Second_Unavailable = 65595; 
const int ETA_Duration_MinLimit = 0;
const int ETA_Duration_MaxLimit = 60999; 
const int ETA_Duration_Unavailable = 65595; 
#define HOURSINADAY 24
#define MINUTESINAHOUR 60
#define SECONDTOMILISECOND 1000

ETA::ETA()
{
}

void ETA::setETA_Minute(int vehicleExpectedTimeOfArrival_Minute)
{
    
    if (vehicleExpectedTimeOfArrival_Minute >= ETA_Minute_MinLimit && vehicleExpectedTimeOfArrival_Minute <= ETA_Minute_MaxLimit)    
        expectedTimeOfArrival_Minute = vehicleExpectedTimeOfArrival_Minute; 

    else 
        expectedTimeOfArrival_Minute = 1;
}

void ETA::setETA_Second(int vehicleExpectedTimeOfArrival_Second)
{
    if (vehicleExpectedTimeOfArrival_Second >= ETA_Second_MinLimit && vehicleExpectedTimeOfArrival_Second <= ETA_Second_MaxLimit)
        expectedTimeOfArrival_Second = static_cast<int>(vehicleExpectedTimeOfArrival_Second);
    
    else
        expectedTimeOfArrival_Second = ETA_Second_Unavailable;
}

void ETA::setETA_Duration(int vehicleExpectedTimeOfArrival_Duration)
{
    if (vehicleExpectedTimeOfArrival_Duration >= ETA_Duration_MinLimit && vehicleExpectedTimeOfArrival_Duration <= ETA_Duration_MaxLimit)
        expectedTimeOfArrival_Duration = static_cast<int>(vehicleExpectedTimeOfArrival_Duration);
    
    else
        expectedTimeOfArrival_Duration = ETA_Duration_Unavailable;
}

int ETA::getETA_Minute()
{
    return expectedTimeOfArrival_Minute;
}

int ETA::getETA_Second()
{
    return expectedTimeOfArrival_Second;
}

int ETA::getETA_Duration()
{
    return expectedTimeOfArrival_Duration;
}

ETA::~ETA()
{
}
