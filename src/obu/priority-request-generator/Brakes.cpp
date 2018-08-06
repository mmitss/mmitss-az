/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  Brakes.cpp
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

#include "Brakes.h"

Brakes::Brakes(void)
{
	brackAppPres=unavailable;                        // 4 bits
	wheelBrakesUnavialable=false ;
	spareBit              =false ;                 
	tractionCntrStat      =TracCntrlunavailable;     // 2 Bits
	antiLckBrkStat         =AntiLockunavailable;     //2 bits
	stabilityCtrlStat      =Stabilitycntlunavailable;
	brakeBstAppld          =BrakeBoostunavailable;
	ausciliaryBrkStat     =AuxiliaryBrkunavailable;
}

Brakes::~Brakes(void)
{
}
