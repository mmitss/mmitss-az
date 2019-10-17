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
  1. This script is the header file for IntersectionAccessPoint.cpp
*/

#pragma once

class IntersectionAccessPoint
{
private:
    int laneID{};
    int approachID{};
    int laneConnectionID{};

public:
    //Constructor:
    IntersectionAccessPoint();

    //Setters:
    void setLaneID(int vehLaneID);
    void setApproachID(int vehApproachID);
    void setLaneConnectionID(int vehLaneConnectionID);

    //Getters:
    int getLaneID();
    int getApproachID();
    int getLaneConnectionID();

    //Destructor:
    ~IntersectionAccessPoint();
};
