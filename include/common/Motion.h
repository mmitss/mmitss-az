/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  Motion.h  
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
#ifndef _Motion_h
#define _Motion_h


#include "AccelerationSet4Way.h"


class Motion
{
public:
	Motion(void);
	~Motion(void);

	public:
	/* LSB of 0.0125 degrees
     *  A range of 0 to 359.9875 degrees 
	 *  The current heading of the sending device, expressed in unsigned units of 0.0125 degrees from North
     *  (such that 28799 such degrees represent 359.9875 degrees). North shall be defined as the axis defined by
     *  the WSG-84 coordinate system and its reference ellipsoid. Headings "to the east" are defined as the
     *  positive direction. A 2 byte value when sent, a value of 28800 shall be used when unavailable. When sent
     *  by a vehicle, this element indicates the orientation of the front of the vehicle. 
	 *  SAE J2735 standard,(revised in 2009)  page 223
	 */
	double speed ;    // TransmissionAndSpeed, speed in mps (meter per second)


	/* The rate of change of the angle of the steering wheel, expressed in signed units of 3 degrees/second
     * over a range of 381degrees in either direction. To the right being positive. Values beyond this range shall
     * use the last value (-127 or +127).     SAE J2735 standard,(revised in 2009)  page 223
	 */
	double heading ;  // Heading, -x- 2 char compass heading in the format of degrees

	/*The rate of change of the angle of the steering wheel, expressed in signed units of 3 degrees/second
     * over a range of 381degrees in either direction. To the right being positive. Values beyond this range shall
     * use the last value (-127 or +127).     SAE J2735 standard,(revised in 2009)  page 223
	 */
	double angle ;	  // SteeringWheelAngle -x- 1 bytes //in VISSIM this is lane steering angle
	
	
	 /* This data frame is a set of acceleration values in 3 orthogonal directions of the vehicle and with yaw
      * rotation rates, expressed as an octet set. The positive longitudinal axis is to the front of the vehicle. The
      * positive lateral axis is to the right side of the vehicle (facing forward). Positive yaw is to the right
      * (clockwise). A positive vertical "z" axis is upward with the zero point at the bottom of the vehicle's tires.
      * The frame of references and axis of rotation used shall be accordance with that defined in SAE J670, Issued
      * 1976-07 and its successors. Note the definitions provided in Figure 1 (Tire Axis System) and Figure 2
      * (Directional Control Axis Systems).
	  */   // SAE J2735 standard,(revised in 2009)  Page 44
	AccelerationSet4Way accel ;

};

#endif