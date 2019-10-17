/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  IntersectionAccessPoint.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is setters and getters method for obtaining laneID, approachID, and laneConnectionID
*/

#include "IntersectionAccessPoint.h"

IntersectionAccessPoint::IntersectionAccessPoint()
{
}

//Setters
void IntersectionAccessPoint::setLaneID(int vehLaneID)
{
    laneID = vehLaneID;
}

void IntersectionAccessPoint::setApproachID(int vehApproachID)
{
    approachID = vehApproachID;
}

void IntersectionAccessPoint::setLaneConnectionID(int vehLaneConnectionID)
{
    laneConnectionID = vehLaneConnectionID;
}

//Getter
int IntersectionAccessPoint::getLaneID()
{
    return laneID;
}

int IntersectionAccessPoint::getApproachID()
{
    return approachID;
}

int IntersectionAccessPoint::getLaneConnectionID()
{
    return laneConnectionID;
}

//Destructor:
IntersectionAccessPoint::~IntersectionAccessPoint()
{
}
