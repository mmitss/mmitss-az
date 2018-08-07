/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  PositionLocal3D.h  
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

#ifndef _PositionLocal3D_h
#define _PositionLocal3D_h

class PositionLocal3D
{
public:
	PositionLocal3D(void);
	~PositionLocal3D(void);

	public:
	double latitude ;         // 4 bytes in degrees
	double longitude ;        // 4 bytes in degrees
	double elevation ;        // 2 bytes in meters
	double positionAccuracy ; // 4 bytes 
};

#endif