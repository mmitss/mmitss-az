#include <iostream>
#include "ExpectedTimeOfArrival.h"
using namespace std;

ETA::ETA()
{

}


void ETA::setETA_Minute(int vehExpectedTimeOfArrival_Minute)
{
    expectedTimeOfArrival_Minute = vehExpectedTimeOfArrival_Minute; 
}

bool ETA::setETA_Second(double vehExpectedTimeOfArrival_Second)
{
	bool bvehExpectedTimeOfArrival_Second = true;
    
    if(vehExpectedTimeOfArrival_Second >= second_MinLimit && vehExpectedTimeOfArrival_Second <= second_MaxLimit)
        expectedTimeOfArrival_Second = vehExpectedTimeOfArrival_Second;
    else
        bvehExpectedTimeOfArrival_Second = false;

    return bvehExpectedTimeOfArrival_Second;
}

bool ETA::setETA_Duration(double vehDuration)
{
	bool bvehDuration = true;
    
    if(vehDuration >= second_MinLimit && vehDuration <= second_MaxLimit)
        expectedTimeOfArrival_Duration = vehDuration;
    else
        bvehDuration = false;
    
    return bvehDuration;
}

int ETA::getETA_Minute()
{
	return  expectedTimeOfArrival_Minute;
}

double ETA::getETA_Second()
{
	return expectedTimeOfArrival_Second;
}   

double ETA::getETA_Duration()
{
	return	expectedTimeOfArrival_Duration;
}

ETA::~ETA()
{
}