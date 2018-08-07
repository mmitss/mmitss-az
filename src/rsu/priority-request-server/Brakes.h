

/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  Brakes.h  
 *  Created by Mehdi Zamanipour on 7/9/14.
 *  University of Arizona
 *  ATLAS Research Center
 *  College of Engineering
 *
 *  This code was develop under the supervision of Professor Larry Head
 *  in the ATLAS Research Center.
 *
 *  Revision History:
 *  
 *  
 */


#ifndef _Brakes_h
#define _Brakes_h


/* BrakeSystemStatus ::= OCTET STRING (SIZE(2))
   -- Encoded with the packed content of:
   -- SEQUENCE {
   -- wheelBrakes BrakeAppliedStatus,
   -- -x- 4 bits
   -- wheelBrakesUnavailable BOOL
   -- -x- 1 bit (1=true)
   -- spareBit
   -- -x- 1 bit, set to zero
   -- traction TractionControlState,
   -- -x- 2 bits
   -- abs AntiLockBrakeStatus,
   -- -x- 2 bits
   -- scs StabilityControlStatus,
   -- -x- 2 bits
   -- brakeBoost BrakeBoostApplied,
   -- -x- 2 bits
   -- auxBrakes AuxiliaryBrakeStatus,
   -- -x- 2 bits
   -- }
   SAE J2735 standard,(revised in 2009) page 56
   */




typedef enum BrakeAppliedPressure {
	unavailable=0,// -- B'0000 Not Equipped
	//-- or Brake Pres status is unavailable
	minPressure= 1, // -- B'0001 Minimum Braking Pressure
	bkLvl_2=2,    //-- B'0010
	bkLvl_3=3,    // -- B'0011
	bkLvl_4=4,    //-- B'0100
	bkLvl_5=5,    //  -- B'0101
	bkLvl_6=6,    // -- B'0110
	bkLvl_7=7,    // -- B'0111
	bkLvl_8=8,    //-- B'1000
	bkLvl_9=9,      //-- B'1001
	bkLvl_10=10,    // -- B'1010
	bkLvl_11=11,    //-- B'1011
	bkLvl_12=12,    //-- B'1100
	bkLvl_13=13,    //-- B'1101
	bkLvl_14=14,     //-- B'1110
	maxPressure=15   // -- B'1111 Maximum Braking Pressure
} e_BrakeAppliedPressure;



typedef enum TractionControlState  {
	TracCntrlunavailable=0,// -- B'00 Not Equipped with tracton control
	//-- or tracton control status is unavailable
	TracCntrloff=1,  // -- B'01 tracton control is Off
	TracCntrlon=2,       //-- B'10 tracton control is On (but not Engaged)
	TracCntrlengaged=3 // -- B'11 tracton control is Engaged
} e_TractionControlState;
// -- Encoded as a 2 bit value


typedef enum AntiLockBrakeStatus {
	AntiLockunavailable=0,    // -- B'00 Vehicle Not Equipped with ABS
	//-- or ABS status is unavailable
	AntiLockoff=1,           // -- B'01 Vehicle's ABS is Off
	AntiLockon=2,            // -- B'10 Vehicle's ABS is On (but not engaged)
	AntiLockengaged=3        // -- B'11 Vehicle's ABS is Engaged
}e_AntiLockBrakeStatus;


typedef enum  StabilityControlStatus{
	Stabilitycntlunavailable=0,     // -- B'00 Not Equipped with SC
	//-- or SC status is unavailable
	Stabilitycntloff1,            // -- B'01 Off
	Stabilitycntlon=2               //-- B'10 On or active (engaged)
}e_StabilityControlStatus;


typedef enum BrakeBoostApplied{
	BrakeBoostunavailable=0,  // -- Vehicle not equipped with brake boost
	//  -- or brake boost data is unavailable
	BrakeBoostoff=1,          // -- Vehicle's brake boost is off
	BrakeBooston=2            //  -- Vehicle's brake boost is on (applied)
}e_BrakeBoostApplied;

typedef enum AuxiliaryBrakeStatus{
	AuxiliaryBrkunavailable=0,   //-- B'00 Vehicle Not Equipped with Aux Brakes
	//-- or Aux Brakes status is  unavailable
	AuxiliaryBrkoff=1,           // -- B'01 Vehicle's Aux Brakes are Off
	AuxiliaryBrkon =2,           // -- B'10 Vehicle's Aux Brakes are On ( Engaged )
	AuxiliaryBrkreserved=3       //-- B'11
}e_AuxiliaryBrakeStatus;


class Brakes
{
	public:
		   
			   e_BrakeAppliedPressure brackAppPres;    // 4 bits
		   bool wheelBrakesUnavialable ;
			   bool spareBit ;                       //set to 0
		   e_TractionControlState tractionCntrStat;   // 2 Bits
		   e_AntiLockBrakeStatus antiLckBrkStat;  //2 bits
		   e_StabilityControlStatus stabilityCtrlStat;
			   e_BrakeBoostApplied brakeBstAppld;
			   e_AuxiliaryBrakeStatus ausciliaryBrkStat;
			   Brakes(void);
			   ~Brakes(void);
};

#endif
