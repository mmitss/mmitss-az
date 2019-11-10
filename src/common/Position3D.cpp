/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  Position3D.cpp  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. 
*/
#include <iostream>
#include <Position3D.h>

using std::cout;
using std::endl;
using std::string;

// Constructor
Position3D::Position3D()
{

}

//Setters:
void Position3D::setLatitude_decimalDegree(double vehLatitude_DecimalDegree)
{
    if(vehLatitude_DecimalDegree >= VALID_LATITUDEMINDEG && vehLatitude_DecimalDegree <= VALID_LATITUDEMAXDEG)
        latitude_DecimalDegree = vehLatitude_DecimalDegree;
    else
        cout << "Latitude out of range!" << endl;
}
void Position3D::setLongitude_decimalDegree(double vehlongitude_DecimalDegree)
{
    if(vehlongitude_DecimalDegree >= VALID_LONGITUDEMINDEG && vehlongitude_DecimalDegree <= VALID_LONGITUDEMAXDEG)
        longitude_DecimalDegree = vehlongitude_DecimalDegree;
    else
        cout << "Longitude out of range!" << endl;
}
void Position3D::setElevation_meter(double vehElevation_Meter)
{
    if(vehElevation_Meter >= VALID_ELEVATIONMINMETER && vehElevation_Meter <= VALID_ELEVATIONMAXMETER)
        elevation_Meter = vehElevation_Meter;
    else if (vehElevation_Meter == UNKNOWN_ELEVATIONMETER)
        {
            elevation_Meter = vehElevation_Meter;
            cout << "Elevation unknown!" << endl;
        }
    else if (vehElevation_Meter < UNKNOWN_ELEVATIONMETER)
        elevation_Meter = VALID_ELEVATIONMINMETER;
    
    else if (vehElevation_Meter >= VALID_ELEVATIONMAXMETER)
        elevation_Meter = VALID_ELEVATIONMAXMETER;
}

//Getters:
double Position3D::getLatitude_DecimalDegree()
{
    return latitude_DecimalDegree;
}
double Position3D::getLongitude_DecimalDegree()
{
    return longitude_DecimalDegree;
}
double Position3D::getElevation_Meter()
{
    return elevation_Meter;
}

//Destructor:
Position3D::~Position3D()
{

}
