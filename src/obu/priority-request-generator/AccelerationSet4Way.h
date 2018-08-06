/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  AccelerationSet4Way.h  
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


#ifndef _AccelerationSet4Way_h
#define _AccelerationSet4Way_h

class AccelerationSet4Way
{
	public:
		double longAcceleration;        // -x- Along the Vehicle Longitutal axis
		double latAcceleration ;        //-x- Along the Vehicle Lateral axis
		double verticalAcceleration ;  // -x- Along the Vehicle Vertical axis
		double yawRate ;
		AccelerationSet4Way(void);
		~AccelerationSet4Way(void);
};

#endif
