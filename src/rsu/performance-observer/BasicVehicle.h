#pragma once


#ifndef byte
    #define byte  char  // needed if byte not defined
#endif
#ifndef BSM_BlOB_SIZE
    #define BSM_BlOB_SIZE  (38)
#endif
#ifndef BSM_BlOB_INDEX
    #define BSM_BlOB_INDEX  (07)  // where the blob data starts in the message itself
#endif
#ifndef BSM_MSG_SIZE
    #define BSM_MSG_SIZE  (45)
#endif
#ifndef DEG2ASNunits
    #define DEG2ASNunits  (10000000.0)  // used for ASN 1/10 MICRO deg to unit converts
#endif

#include "PositionLocal3D.h" 
#include "AccelerationSet4Way.h"
#include "Brakes.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>	
#include <assert.h>
using namespace std;    // access to cout stream

class BasicVehicle
{
public:
	/* Attributes of the SAE J2735 Basic Safety Message
	Referenced to the J2735 Ballot Ready Version 35 */
	// Not Used (is a constant): int DSRCmsgID ;   // 1 byte
	int MsgCount ;    //	1 byte // value is 0-127 
	long TemporaryID ; // id TemporaryID, -x- 4 bytes // call it vehicle ID (from VISSIM for simulation)
	int DSecond ;     //secMark DSecond, -x- 2 bytes - Time in milliseconds within a minute
	
	PositionLocal3D  pos;
	
	// motion Motion,
	double speed ;    // TransmissionAndSpeed, speed in meters per second
	double heading ;  // Heading, -x- 2 byte compass heading in **degrees** 
	/* 9052 Use: The current heading of the sending device, expressed in unsigned units of 0.0125 degrees from North
	   9053 (such that 28799 such degrees represent 359.9875 degrees). North shall be defined as the axis defined by
	   9054 the WSG-84 coordinate system and its reference ellipsoid. Headings "to the east" are defined as the
	   9055 positive direction. A 2 byte value when sent, a value of 28800 shall be used when unavailable. When sent
	   9056 by a vehicle, this element indicates the orientation of the front of the vehicle. */
	double angle ;	  // SteeringWheelAngle -x- 1 bytes //VISSIM this is lane heading angle
	AccelerationSet4Way accel ;
    
	// control Control,
	Brakes brakes ; // BrakeSystemStatus

	//-- basic VehicleBasic,
	float length ;	 // combined into VehicleSize, 3 bytes 
	float width ;
	float weight ; //mass

/*	This section contains data that would be populated a call the intersection 
    class with current MAP and SPaT data for the purposes of traffic signal control 
    (passing the TemporaryID to it to idenify this instacne of this class)
    */

	int desiredPhase ; //traffic signal phase associated with a lane (MAP + phase association)
	double stopBarDistance ; //distance to the stop bar from current location (MAP)
	double estArrivalTime ; // estimated arrival time at the stop bar given current speed

/* ---------------------------------------Methods---------------------------------------*/

	BasicVehicle(void);
	~BasicVehicle(void);
	void Init(); 

	void Vehicle2BSM(byte* bytes2Send) ; //this function creates a BSM payload from a vehicle
	void BSM2Vehicle(byte* bytesReceived) ; //this function populates the vehicle attributes from a BSM payload 

    /**  Moved to another class //from Larry - we do this, but if we keep them here, we can wrap the other call in this method.
	int DeterminePhase() ; // this function requires the MAP and vehicle position and heading
	double EstArrivalTime() ; // this function requires the MAP and vehicle position 
    ***/

    // Added for debug use
    // When called, the current position is moved by appliing the current heading and speed 
    // values for a one second constant rate. Accleration is not used and thse values are 
    // not set, only the lat-long is changed, and the header is adjuts a small amount to 
    // give some "curve" to the movement. To be removed when code works
    // returns -1 on failure, otherwise zero. 
    int MoveAhead();

    // Output current class values to console for debug use
    void Out();

    // Copy bytes of an empty message to the passed in pointer
    // used to reset a message to a known starting point
    // typically called before creating msg value content
    void CopyMessageFromTemplate(byte* theTarget);

    // Write the passed bytes, using the passed count, to the passed file name
    // return number of bytes written on sucess or -1 on error
    // NOTE: There is no reano to have this as part of the class, 
    // I just put it here to have a file write call. 
  //  size_t WriteBytesToFile(char* fileName, byte* pData, int dataCnt);

    // Returns true if the passed BasicVehicle object matches this one
    // else false. Compares each variable in the class to determine this.
    // if verbose is set true any item in the class that is not alike 
    // will be noted in the cout stream. Otherwise operation is silent.
    bool  IsEqual(BasicVehicle* pTest, bool verbose);

    int WriteMessageToFile(char *filename);



    // Given an angle value (in degrees) return it in the range (0~360)
    inline double CorrectAngleRange(double anAngle);

};



