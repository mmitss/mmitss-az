#pragma once
class PositionLocal3D
{
public:
	//PositionLocal3D(void);
	//~PositionLocal3D(void);
	double latitude ; // 4 bytes measured in degrees
	double longitude ; // 4 bytes measured in degrees
	double elevation ;  // 2 bytes measured in meters
	double positionAccuracy ; //4 bytes measured in ?? is this J2735 position accurracy??

	void Init();
};

