#pragma once
#include "string"

struct BusStopInformation
{
    std::string intersectionName{};
    int intersectionID{};
    std::string travelDirection{};
    int approachNo{};
    double lattitude_DecimalDegree{};
    double longitude_DecimalDegree{};
    double elevation_Meter{};
    double heading_Degree{};
    
    void reset()
    {
        intersectionName = "";
        intersectionID = 0;
        travelDirection = "";
        approachNo = 0;
        lattitude_DecimalDegree = 0.0;
        longitude_DecimalDegree = 0.0;
        elevation_Meter = 0.0;
        heading_Degree = 0.0;  
    }
};
