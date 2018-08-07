#pragma once
class Brakes
{
public:
		bool rf_BrakesApplied ;
		bool lf_BrakesApplied ;
		bool rr_BrakesApplied ; 
		bool lr_BrakesApplied ;
		bool wheelBrakesUnavialable ;
		bool spareBit ; //set to 0

		bool traction_1 ;
		bool traction_2 ;
		bool anitLock_1 ;
		bool antiLock_2 ;
		bool stability_1 ;
		bool stability_2 ;
		bool boost_1 ;
		bool boost_2 ;
		bool auxBrakes_1 ;
		bool auxBrakes_2 ;
	Brakes(void);
	~Brakes(void);
};

