#include <iostream>
#include "Position3D.h"

using std::cout;
using std::endl;
using std::string;

Position3D::Position3D()
{
    latitude_DecimalDegree = 0.0;
    longitude_DecimalDegree = 0.0;
    elevation_Meter = 0.0;
}

//Setters:
void Position3D::setLatitude_decimalDegree(double vehLatitude_DecimalDegree)
{
    if(vehLatitude_DecimalDegree >= -90.0 && vehLatitude_DecimalDegree <= 90.0)
        latitude_DecimalDegree = vehLatitude_DecimalDegree;
    else
        cout << "Latitude out of range!" << endl;
}
void Position3D::setLongitude_decimalDegree(double vehlongitude_DecimalDegree)
{
    if(vehlongitude_DecimalDegree >= -180.0 && vehlongitude_DecimalDegree <= 180.0)
        longitude_DecimalDegree = vehlongitude_DecimalDegree;
    else
        cout << "Longitude out of range!" << endl;
}
void Position3D::setElevation_meter(double vehElevation_Meter)
{
    if(vehElevation_Meter >= -409.5 && vehElevation_Meter <= 6143.9)
        elevation_Meter = vehElevation_Meter;
    else if (vehElevation_Meter == -409.6)
        {
            elevation_Meter = vehElevation_Meter;
            cout << "Elevation unknown!" << endl;
        }
    else if (vehElevation_Meter < -409.6)
        elevation_Meter = -409.5;
    
    else if (vehElevation_Meter >= 6143.9)
        elevation_Meter = 6143.9;
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
