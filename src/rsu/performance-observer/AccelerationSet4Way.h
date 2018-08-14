#pragma once
class AccelerationSet4Way
{
public:
	double longAcceleration; // -x- Along the Vehicle Longitudinal axis
	double latAcceleration ; //-x- Along the Vehicle Lateral axis
	double verticalAcceleration ; // -x- Along the Vehicle Vertical axis
	double yawRate ;
	
	
	AccelerationSet4Way(void);
	~AccelerationSet4Way(void);
};

