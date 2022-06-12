/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  ExpectedTimeOfArrival.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the header file for ExpectedTimeOfArrival.cpp
*/

#pragma once

class ETA
{
private:
    int expectedTimeOfArrival_Minute{};
    int expectedTimeOfArrival_Second{};
    int expectedTimeOfArrival_Duration{}; //provide a short interval that extends the ETA

public:
    //Constructor:
    ETA();

    //Setters:
    void setETA_Minute(int ehicleExpectedTimeOfArrival_Minute);
    void setETA_Second(int vehicleExpectedTimeOfArrival_Second);
    void setETA_Duration(int vehicleExpectedTimeOfArrival_Duration);

    //Getters:
    int getETA_Minute();
    int getETA_Second();
    int getETA_Duration();

    //Destructor:
    ~ETA();
};
