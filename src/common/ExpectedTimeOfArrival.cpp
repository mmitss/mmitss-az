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
#include "ExpectedTimeOfArrival.h"

const double second_MinLimit = 0.0;
const double second_MaxLimit = 59.99; //This value comes frrom J2735 standard

ETA::ETA()
{
}

void ETA::setETA_Minute(int vehExpectedTimeOfArrival_Minute)
{
    expectedTimeOfArrival_Minute = vehExpectedTimeOfArrival_Minute;
}

bool ETA::setETA_Second(double vehExpectedTimeOfArrival_Second)
{
    bool bvehExpectedTimeOfArrival_Second = true;

    if (vehExpectedTimeOfArrival_Second >= second_MinLimit && vehExpectedTimeOfArrival_Second <= second_MaxLimit)
        expectedTimeOfArrival_Second = vehExpectedTimeOfArrival_Second;
    else
        bvehExpectedTimeOfArrival_Second = false;

    return bvehExpectedTimeOfArrival_Second;
}

bool ETA::setETA_Duration(double vehDuration)
{
    bool bvehDuration = true;

    if (vehDuration >= second_MinLimit && vehDuration <= second_MaxLimit)
        expectedTimeOfArrival_Duration = vehDuration;
    else
        bvehDuration = false;

    return bvehDuration;
}

int ETA::getETA_Minute()
{
    return expectedTimeOfArrival_Minute;
}

double ETA::getETA_Second()
{
    return expectedTimeOfArrival_Second;
}

double ETA::getETA_Duration()
{
    return expectedTimeOfArrival_Duration;
}

ETA::~ETA()
{
}