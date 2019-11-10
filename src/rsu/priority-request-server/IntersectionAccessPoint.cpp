#include "IntersectionAccessPoint.h"

IntersectionAccessPoint::IntersectionAccessPoint()
{
}

//Setters
void IntersectionAccessPoint::setLaneID(int vehLaneID)
{
    laneID = vehLaneID;
}

void IntersectionAccessPoint::setApproachID(int vehApproachID)
{
    approachID = vehApproachID;
}

void IntersectionAccessPoint::setLaneConnectionID(int vehLaneConnectionID)
{
    laneConnectionID = vehLaneConnectionID;
}

//Getter
int IntersectionAccessPoint::getLaneID()
{
    return laneID;
}

int IntersectionAccessPoint::getApproachID()
{
    return approachID;
}

int IntersectionAccessPoint::getLaneConnectionID()
{
    return laneConnectionID;
}

//Destructor:
IntersectionAccessPoint::~IntersectionAccessPoint()
{
}
