#pragma once

#include <string>
#include <cstdint>
#include "json.h"
#include "Position3D.h"


class BasicVehicle
{
    private:
        int temporaryID;
        double secMark_Second; // seconds
        Position3D position;
        double speed_MeterPerSecond; // Meter per second
        double heading_Degree; // Degree
        int type; // Enumeration

    public:
        //Constructor:
        BasicVehicle();
        
        //Setters:
        void setTemporaryID(int vehTemporaryID);
        void setSecMark_Second(double vehSecMark_Second);
        void setPosition(double vehLatitude_DecimalDegree, double vehLongitude_DecimalDegree, double vehElevation_Meter);
        void setSpeed_MeterPerSecond(double vehSpeed);
        void setHeading_Degree(double vehHeading_Degree);
        void setType(int vehType);
        
        //Getters:
        int getTemporaryID();
        double getSecMark_Second();
        Position3D getPosition();
        double getLatitude_DecimalDegree();
        double getLongitude_DecimalDegree();
        double getElevation_Meter();
        double getSpeed_MeterPerSecond();
        double getHeading_Degree();
        int getType();

        //JSON Handlers:
        std::string basicVehicle2Json();
        void json2BasicVehicle(std::string jsonString);

        //Destructor:
        ~BasicVehicle();
};