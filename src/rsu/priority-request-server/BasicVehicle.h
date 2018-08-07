
/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  BasicVehicle.h  
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


#ifndef _BasicVehicle_h
#define _BasicVehicle_h



#ifndef BSM_BlOB_SIZE
#define BSM_BlOB_SIZE  (38)
#endif
#ifndef BSM_BlOB_INDEX
#define BSM_BlOB_INDEX  (07)  // where the blob data starts in the message itself
#endif
#ifndef BSM_MSG_SIZE
#define BSM_MSG_SIZE  (45)
#endif
#ifndef DEG2mDEG
#define DEG2mDEG  (10000000.0)  // used for ASN 1/10 MICRO deg to unit converts
#endif

#include "PositionLocal3D.h" 
#include "AccelerationSet4Way.h"
#include "Brakes.h"
#include "Motion.h"
#include <iostream>
#include <stdlib.h>
#include <assert.h>
using namespace std;

class BasicVehicle
{
	public:
		/*** Attributes of the SAE J2735 standard,(revised in 2009) for Basic Safety Message (BSM)***/

		// Not Used (is a constant):

		/*	 The DSRC Message ID is a data element used in each message to define which type of message
		 *follows from the message set defined by this Standard. This data element is always the first value inside
		 * the message and is used to tell the receiving application how to interpret the remaining bytes (i.e. what
		 * message structure has been used).
		 *SAE J2735 standard,(revised in 2009)   page 154
		 */ 
		int DSRCmsgID ;      // 1 char  Which is 2 for BSM

		/*The MsgCount data element is used to provide a sequence number within a stream of messages with
		 *the same DSRCmsgID and from the same sender. A sender may initialize this element to any value in the
		 *range 0-127 when sending the first message with a given DSRCmsgID, or if the sender has changed
		 *identity (e.g. by changing its TemporaryID) since sending the most recent message with that DSRCmsgID.
		 *Two further use cases exist when the sender has not changed identity: When the rest of the message
		 *content to be sent changes, the MsgCount shall be set equal to one greater than the value used in the most
		 *recent message sent with the same DSRCmsgID
		 *SAE J2735 standard,(revised in 2009)   page 187
		 */	
		int MsgCount ;       //	1 char // value is 0-127 

		/*	This is the 4 byte random device identifier, called the temporary ID. In essence, this value for a
		 * mobile OBU device (unlike a typical wireless or wired 802 device) will periodically change to ensure the
		 * overall anonymity of the vehicle. Because this value is used as a means to identify the local vehicles that
		 * are interacting during an encounter, it is used in the message set
		 *SAE J2735 standard,(revised in 2009)   page 225
		 */
		long TemporaryID ;   // id TemporaryID, -x- 4 bytes // In VISSIM we call it vehicle ID 

		/* The DSRC style second expressed in this data element is a simple value consisting of integer values
		 * from zero to 60999 representing the milliseconds within a minute. A leap second is represented by the
		 * value range 60001 to 60999. The value of 65535 SHALL represent an unavailable value in the range of the
		 * minute, other values from 61000 to 65534 are reserved.
		 * SAE J2735 standard,(revised in 2009)   page 153
		 */

		int DSecond ;         //secMark DSecond, -x- 2 bytes - Time is in milliseconds within a minute (60 sec)

		PositionLocal3D  pos;
		// motion
		Motion motion;

		//Control
		/* The Brake System Status data frame conveys a variety of information about the current brake and
		 * system control activity of the vehicle. Each of the first four bits indicates whether brakes are active for a
		 * given wheel on the vehicle. A value of one shall indicate an active brake. A fifth bit is set to one to
		 * indicate when this data is unavailable. The next bit is reserved at this time (and set to zero). The next five
		 * 2-bit fields indicate the status respectively of the traction control system, the anti-lock brake system, the
		 * stability control system, the brake boost system, and the auxiliary brake system. 
		//  SAE J2735 standard,(revised in 2009)   page 56
		*/
		Brakes brakes ; // BrakeSystemStatus  2Byte

		// Vehicle size,
		/*The VehicleSize is a data frame representing the vehicle length and vehicle width in a three byte value.
		  SAE J2735 standard,(revised in 2009)   page 56 page 128
		  */
		float length ;	 // combined into VehicleSize, 3 bytes 
		float width ;


		/* --------------Methods--------- */

		// Create an empty valid buffer for BSM and copy it to the
		// inported buffer . this functuion would be called before creating BSM
		void CreateEmptyBSMblob(char* theTarget);
		void FillupBlob( int offset,char * blob, char * p, int size);
		long UnpackBlob(int offset,char * blob,int size);
		BasicVehicle(void);
		~BasicVehicle(void);


		void VehicleToBSM(char* bytes2Send) ;   //this function creates a BSM blob payload from a vehicle
		void BSMToVehicle(char* bytesReceived) ; // This function unpack the BSM blob stream and populates the vehicle attributes (the class data)

};



#endif
