#include "BasicVehicle.h"
#include <iostream>

using std::cout;
using std::endl;
using std::string;

//Constructor:
BasicVehicle::BasicVehicle()
{
    temporaryID = 0;
    secMark_Second = 0.0;
    speed_MeterPerSecond = 0.0;
    heading_Degree = 0.0;
    type = 0;
}

//Setters:
void BasicVehicle::setTemporaryID(int vehTemporaryID)
{
    temporaryID = vehTemporaryID;
}
void BasicVehicle::setSecMark_Second(double vehSecMark_Second)
{
    if(vehSecMark_Second>=0 && vehSecMark_Second<=60.999)
        secMark_Second = vehSecMark_Second;
    else if (vehSecMark_Second>=61.000 && vehSecMark_Second<=65.534)
        cout << "secMark in the reserved range!" << endl;
    else
        cout << "secMark out of range!" << endl;
}
void BasicVehicle::setPosition(double vehLatitude_DecimalDegree, double vehLongitude_DecimalDegree, double vehElevation_Meter)
{
    position.setLatitude_decimalDegree(vehLatitude_DecimalDegree);
    position.setLongitude_decimalDegree(vehLongitude_DecimalDegree);
    position.setElevation_meter(vehElevation_Meter);
}
void BasicVehicle::setSpeed_MeterPerSecond(double vehSpeed_MeterPerSecond)
{
    if(vehSpeed_MeterPerSecond >=0 && vehSpeed_MeterPerSecond <=163.8)
        speed_MeterPerSecond = vehSpeed_MeterPerSecond;
    else if (vehSpeed_MeterPerSecond > 163.8 && vehSpeed_MeterPerSecond < 163.83)
        cout << "Speed is unavailable!" << endl; 
    else
        cout << "Speed out of range!" << endl;
}
void BasicVehicle::setHeading_Degree(double vehHeading_Degree)
{
    if(vehHeading_Degree >= 0 && vehHeading_Degree <= 359.9875)
        heading_Degree = vehHeading_Degree;
    else if (vehHeading_Degree == 360.0)
        cout << "Heading unavailable!" << endl;
    else
        cout << "Heading out of range!" << endl;
}
void BasicVehicle::setType(int vehType)
{
    type = vehType;
}

//Getters:
int BasicVehicle::getTemporaryID()
{
    return temporaryID;
}
double BasicVehicle::getSecMark_Second()
{
    return secMark_Second;
}
Position3D BasicVehicle::getPosition()
{
    return position;
}
double BasicVehicle::getLatitude_DecimalDegree()
{
    return position.getLatitude_DecimalDegree();
}
double BasicVehicle::getLongitude_DecimalDegree()
{
    return position.getLongitude_DecimalDegree();
}
double BasicVehicle::getElevation_Meter()
{
    return position.getElevation_Meter();
}
double BasicVehicle::getSpeed_MeterPerSecond()
{
    return speed_MeterPerSecond;
}
double BasicVehicle::getHeading_Degree()
{
    return heading_Degree;
}
int BasicVehicle::getType()
{
    return type;
}

string BasicVehicle::basicVehicle2Json()
{
    Json::Value jsonObject;
    jsonObject["BasicVehicle"]["temporaryID"] = temporaryID;
    jsonObject["BasicVehicle"]["secMark_Second"] = secMark_Second;
    jsonObject["BasicVehicle"]["position"]["latitude_DecimalDegree"] = position.getLatitude_DecimalDegree();
    jsonObject["BasicVehicle"]["position"]["longitude_DecimalDegree"] = position.getLongitude_DecimalDegree();
    jsonObject["BasicVehicle"]["position"]["elevation_Meter"] = position.getElevation_Meter();
    jsonObject["BasicVehicle"]["speed_MeterPerSecond"] = speed_MeterPerSecond;
    jsonObject["BasicVehicle"]["heading_Degree"] = heading_Degree;
    jsonObject["BasicVehicle"]["type"] = type;
    Json::FastWriter fastWriter;
    return fastWriter.write(jsonObject);                                            
}
void BasicVehicle::json2BasicVehicle(string jsonString)
{
    Json::Value jsonObject;
    Json::Reader reader;
    reader.parse(jsonString.c_str(), jsonObject);
    temporaryID = (jsonObject["BasicVehicle"]["temporaryID"]).asInt();
    type = (jsonObject["BasicVehicle"]["type"]).asInt();
    speed_MeterPerSecond = (jsonObject["BasicVehicle"]["speed_MeterPerSecond"]).asDouble();
    secMark_Second = (jsonObject["BasicVehicle"]["secMark_Second"]).asDouble();
    heading_Degree  = (jsonObject["BasicVehicle"]["heading_Degree"]).asDouble();
    position.setLatitude_decimalDegree((jsonObject["BasicVehicle"]["position"]["latitude_DecimalDegree"]).asDouble());
    position.setLongitude_decimalDegree((jsonObject["BasicVehicle"]["position"]["longitude_DecimalDegree"]).asDouble());
    position.setElevation_meter((jsonObject["BasicVehicle"]["position"]["elevation_Meter"]).asDouble());
}

//Destructor:
BasicVehicle::~BasicVehicle()
{

}
