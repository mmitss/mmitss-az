/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  Motion.cpp  
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


#include "Motion.h"

Motion::Motion(void)
{
	speed                 = 0.0; 
	heading               = 0.0; 
	angle                 = 0.0;  
	accel.longAcceleration= 0.0  ; // -x- Along the Vehicle Longitudinal axis
	accel.latAcceleration = 0.0; //-x- Along the Vehicle Lateral axis
	accel.verticalAcceleration= 0.0 ; // -x- Along the Vehicle Vertical axis
	accel.yawRate             = 0.0; 
}


Motion::~Motion(void)
{
}
