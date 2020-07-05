
#include "PositionLocal3D.h"

PositionLocal3D::PositionLocal3D(void)
{
	Init() ;
}


PositionLocal3D::~PositionLocal3D(void)
{
}

// Set the calls values to a know point
void PositionLocal3D::Init ()
{
    // DEBUG: cout << " Instance of PositionLocal3D class created." << endl;
    latitude         = 0.0; 
    longitude        = 0.0; 
    elevation        = 0.0;  
    positionAccuracy = 0.0; 
}