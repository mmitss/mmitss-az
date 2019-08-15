/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  Position3D.h  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. 
*/
#pragma once

const double VALID_LATITUDEMINDEG = -90.0;
const double VALID_LATITUDEMAXDEG = 90.0;

const double VALID_LONGITUDEMINDEG = -180.0;
const double VALID_LONGITUDEMAXDEG = 180.0;

const double VALID_ELEVATIONMINMETER = -409.5;
const double VALID_ELEVATIONMAXMETER = 6143.9;
const double UNKNOWN_ELEVATIONMETER = -409.6;

class Position3D
{
    private:
        double latitude_DecimalDegree{};//-90.0 to 90.0 decimalDegree
        double longitude_DecimalDegree{};//-180.0 to 180.0 decimalDegree
        double elevation_Meter{};//-409.5 to +6143.9 meter, -409.6 -> Unknown

    public:
        //Constructor:
        Position3D();

        //Setters:
        void setLatitude_decimalDegree(double vehLatitude_DecimalDegree);
        void setLongitude_decimalDegree(double vehlongitude_DecimalDegree);
        void setElevation_meter(double vehElevation_Meter);

        //Getters:
        double getLatitude_DecimalDegree();
        double getLongitude_DecimalDegree();
        double getElevation_Meter();

        //Destructor:
        ~Position3D();
};

