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

const int ETA_Minute_MinLimit = 0;
const int ETA_Minute_MaxLimit = 527040;
const int ETA_Second_MinLimit = 0;
const int ETA_Second_MaxLimit = 60999; 
const int ETA_Second_Unavailable = 65595; //These values come from J2735 2016 standard
#define HOURSINADAY 24
#define MINUTESINAHOUR 60
#define SECONDTOMILISECOND 1000

ETA::ETA()
{
}

void ETA::setETA_Minute(int vehExpectedTimeOfArrival_Minute)
{
    
    if (vehExpectedTimeOfArrival_Minute >= ETA_Minute_MinLimit && vehExpectedTimeOfArrival_Minute <= ETA_Minute_MaxLimit)    
        expectedTimeOfArrival_Minute = vehExpectedTimeOfArrival_Minute; 

    else
        expectedTimeOfArrival_Minute = 1;
}

void ETA::setETA_Second(int vehExpectedTimeOfArrival_Second)
{
    if (vehExpectedTimeOfArrival_Second >= ETA_Second_MinLimit && vehExpectedTimeOfArrival_Second <= ETA_Second_MaxLimit)
        expectedTimeOfArrival_Second = static_cast<int>(vehExpectedTimeOfArrival_Second);
    
    else
        expectedTimeOfArrival_Second = ETA_Second_Unavailable;
}

void ETA::setETA_Duration(int vehDuration)
{
    expectedTimeOfArrival_Duration = static_cast<int>(vehDuration);
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
