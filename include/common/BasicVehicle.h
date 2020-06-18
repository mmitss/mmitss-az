/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  BasicVehicle.h
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. 
*/
#pragma once

#include <string>
#include <cstdint>
#include <Position3D.h>

const double VALID_HEADINGMINDEG = 0;
const double VALID_HEADINGMAXDEG = 359.9875;
const double HEADINGUNAVAILABLEDEG = 360.0;

const double VALID_SPEEDMINMPS = 0;
const double VALID_SPEEDMAXMPS = 163.8;
const double UNAVAIL_SPEEDMINMPS = 163.8;
const double UNAVAIL_SPEEDMAXMPS = 163.83;

const double VALID_SECMARKMINSEC = 0.0;
const double VALID_SECMARKMAXSEC = 60.999;
const double RESERVED_SECMARKMINSEC = 61.00;
const double RESERVED_SECMARKMAXSEC = 65.534;

class BasicVehicle
{
    private:
        int temporaryID{};
        double secMark_Second{}; // seconds
        Position3D position;
        double speed_MeterPerSecond{}; // Meter per second
        double heading_Degree{}; // Degree
        std::string type{}; // Enumeration
		int lane{};
        int length_cm{};
        int width_cm{};


    public:
        //Constructor:
        BasicVehicle();
        
        //Setters:
        void setTemporaryID(int vehTemporaryID);
        void setSecMark_Second(double vehSecMark_Second);
        void setPosition(double vehLatitude_DecimalDegree, double vehLongitude_DecimalDegree, double vehElevation_Meter);
        void setSpeed_MeterPerSecond(double vehSpeed);
        void setHeading_Degree(double vehHeading_Degree);
        void setType(std::string vehType);
        void setLength_cm(int vehLength_cm);
        void setWidth_cm(int vehWidth_cm);
        
        //Getters:
        int getTemporaryID();
        double getSecMark_Second();
        Position3D getPosition();
        double getLatitude_DecimalDegree();
        double getLongitude_DecimalDegree();
        double getElevation_Meter();
        double getSpeed_MeterPerSecond();
        double getHeading_Degree();
        std::string getType();
        int getLength_cm();
        int getWidth_cm();

        //JSON Handlers:
        std::string basicVehicle2Json();
        void json2BasicVehicle(std::string jsonString);

        //Destructor:
        ~BasicVehicle();
};
