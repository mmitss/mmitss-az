/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  BasicVehicle.cpp  
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
#include "stdafx.h"
#include "BasicVehicle.h"


BasicVehicle::BasicVehicle()
{
	MsgCount            = 0;    
	TemporaryID         = 0; 
	DSecond             = 0;
	pos.elevation       = 0.0;
	pos.latitude        = 0.0;
	pos.longitude       = 0.0;
	pos.positionAccuracy= 0.0;
	motion.speed        = 0.0;
	motion.heading      = 0.0;
	motion.angle        = 0.0;
	motion.accel.latAcceleration      = 0.0 ;
	motion.accel.longAcceleration     = 0.0 ;
	motion.accel.verticalAcceleration = 0.0 ;
	motion.accel.yawRate              = 0.0 ;

	brakes.brackAppPres           =unavailable;    
	brakes.wheelBrakesUnavialable =false ;
	brakes.spareBit               =false ;
	brakes.tractionCntrStat       =TracCntrlunavailable;   
	brakes.antiLckBrkStat         =AntiLockunavailable;  
	brakes.stabilityCtrlStat      =Stabilitycntlunavailable;
	brakes.brakeBstAppld          =BrakeBoostunavailable;
	brakes.ausciliaryBrkStat      =AuxiliaryBrkunavailable;

	length = 0.0;
	width = 0.0;
}

BasicVehicle::~BasicVehicle()
{
}

// Create an empty valid buffer for BSM blob and copy it to the
// inported buffer . This function would be called before creating BSM
void BasicVehicle::CreateEmptyBSMblob(char* theTarget)
{
	// Build up the default BSM msg buffer to be used later
	char emptyMsg [] = { 

		0x00,   // start of blob data  // Msg Count
		0x00, 0x00,  0x00, 0x00,  // Temp Id 4Byte
		0x00, 0x01,               // Dsecond 2Byte
		0x00, 0x00, 0x00, 0x00,   // Lat 4Byte
		0x00, 0x00, 0x00, 0x00,   // Lon 4Byte
		0xF0, 0x00,               // Elevation  2Byte
		0xFF, 0xFF, 0xFF, 0xFF,   // Accuracy 4Byte
		0x40, 0x00,               // Speed 2Byte
		0x00, 0x7F,               // heading 2Byte
		0xFF,                     // angle 1Byte 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, //AccelSet 7Byte
		0x00, 0x00,               // brakes 2Byte
		0x0F, 0xA2, 0x58          // size 3Byte for vehicle size 
	}; 
	// Test to confirm BSM has the corect expected size 
	if (sizeof(emptyMsg) != BSM_BlOB_SIZE) 
	{
		cout << "ERROR: Length of the blank message is not correct.!" << endl;
		cout << "Expected to find " << BSM_BlOB_SIZE << " but found " << \
			sizeof(emptyMsg) << " bytes." << endl;
	}

	// copy each char of the empty msg to the passed in pointer
	if (theTarget != NULL)
	{
		for(int i=0; i<BSM_BlOB_SIZE; i++)
		{
			theTarget[i] = emptyMsg[i];
		}
	}
	else 
		cout << "ERROR: passed a null pointer to the function CreateEmptyBSMblob." << endl;
}


// This function unpack the BSM blob stream and populates the vehicle attributes (the class data)
void BasicVehicle::BSMToVehicle(char* ablob)
{

	if (ablob != NULL)
	{

		int offset;         // a counter into the blob bytes
		unsigned short   tempUShort; // temp values to hold data in final format
		long    lTemp;
		unsigned char byteA,byteB,byteC;

		// the code is written to be compatibele with Intel platforms (which are all little endian)  
		// while in Moto machines, or an ARM, or in ASN.1 the ordere is reversed (big endian)

		offset = 0; // set to start of blob inside message

		// do msgCnt   (1 char)
		MsgCount = (char)ablob[offset];
		offset = offset + 1; // move past to next item

		// do temp ID  (4 bytes)
		lTemp=UnpackBlob(offset,ablob,4);
		TemporaryID = (long)lTemp;
		offset = offset + 4;

		// do secMark  (2 bytes)
		lTemp=UnpackBlob(offset,ablob,2);
		DSecond = (int)(lTemp); // in fact unsigned
		offset = offset + 2;

		// do the latitude data element (4 bytes)
		lTemp=UnpackBlob(offset,ablob,4);
		lTemp = (unsigned long)lTemp;
		pos.latitude = (lTemp /  DEG2mDEG); // convert and store as float
		offset = offset + 4;

		// do the longitude data element (4 bytes)
		lTemp=UnpackBlob(offset,ablob,4);
		pos.longitude = (lTemp /  DEG2mDEG); // convert and store as float
		offset = offset + 4;

		// There is no elevation data element in VISSIM, so this may just be a default constant each time we use 
		lTemp=UnpackBlob(offset,ablob,2);
		lTemp = (unsigned long)(lTemp);
		pos.elevation = (lTemp / 10.0); // in 0.1 m steps
		offset = offset + 2;

		// do the accuracy data frame  set to the default
		offset = offset + 4;

		// do the Speed data element 
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		// remove the three upper transmission value bits from 1st char
		byteA = byteA & 0x1F; 
		tempUShort  = (unsigned short)((byteA << 8) + (byteB));
		motion.speed = (tempUShort / 50.0); 
		offset = offset + 2;

		// do the m_heading data element   should be in (0 ,360) 
		lTemp=UnpackBlob(offset,ablob,2);
		tempUShort = (short)(lTemp);
		motion.heading = (tempUShort / 80.0);  
		offset = offset + 2;

		// do the m_steeringWheelAngle data element 

		offset = offset + 1;

		// Do the vehicle long accel 
		// convert from xx.xx to LSB units are 0.01 m/s^2
		lTemp=UnpackBlob(offset,ablob,2);
		tempUShort = (unsigned short)(lTemp);
		motion.accel.longAcceleration  = ((short)tempUShort / 50.0);
		offset = offset + 2;

		// See Note about accel data blob on the pack bytes side. 
		//None of this are in fact used by VISSIM style, so skipped
		// Do the vehicle lat accel 
		offset = offset + 2;
		// Do the vehicle vert accel 
		offset = offset + 1;
		// Do the vehicle yaw rate
		offset = offset + 2;
		// do the m_brakeSet data element 
		offset = offset + 2;

		// do the m_width and m_length data elements 

		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		tempUShort = (unsigned short)((byteA << 2) + ((byteB & 0xC0) >> 6));
		width  = (float)(tempUShort / 100.0);
		tempUShort = (unsigned short)(((byteB & 0x3F) << 8) + (byteC));
		length = (float)(tempUShort / 100.0);
		offset = offset + 3;

		assert(offset == BSM_BlOB_SIZE);
	}
	else // gripe
		cout << "Passed in an empty pointer for BSM char char array, no BSMToVehicle decode." << endl; 
}



// This function creates a BSM payload from a vehicle data come from VISSIM, 
// BSM blob payload would be located in a octet string stream

void BasicVehicle::VehicleToBSM(char* ablob)
{
	if (ablob != NULL)
	{
		int offset;         // a counter 
		char*   pByte;      // pointer used (by cast)to get at each char 
		unsigned short   usTemp;
		long    lTemp;
		double  dTemp;

		// Create an "empty" but valid starting point with a char copy
		CreateEmptyBSMblob(ablob);
		// the code is written to be compatibele with Intel platforms (which are all little endian)

		offset = 0; // set to start of blob inside message

		// do msgCnt   (1 char)
		ablob[offset] = (char) MsgCount;

		offset = offset + 1; // move past to next item

		// do temp ID  (4 bytes)
		lTemp = (long)TemporaryID;  
		pByte = (char* ) &lTemp;
		FillupBlob(offset, ablob, pByte,4);
		offset = offset + 4;

		// do secMark  (2 bytes)
		usTemp = (unsigned short)DSecond; 
		pByte = (char* ) &usTemp;
		FillupBlob(offset, ablob, pByte,2);
		offset = offset + 2;

		// do the latitude data element (4 bytes)
		lTemp = (long)(pos.latitude * DEG2mDEG); 
		pByte = (char* ) &lTemp;
		FillupBlob(offset, ablob, pByte,4);
		offset = offset + 4;

		// do the longitude data element (4 bytes)
		lTemp = (long)(pos.longitude * DEG2mDEG); 
		pByte = (char* ) &lTemp;
		FillupBlob(offset, ablob, pByte,4);
		offset = offset + 4;

		// do elevation data element
		usTemp = (unsigned short)(pos.elevation * 10.0); // to 0.1 meter steps
		pByte = (char* ) &usTemp;
		FillupBlob(offset, ablob, pByte,2);
		offset = offset + 2;

		// do the m_accuracy data frame    set to the default value in CreateEmptyBSM
		offset = offset + 4;

		// do the Speed data element 
		// get it into the right units first 
		usTemp = (unsigned short)(motion.speed * 50.0);
		// need the current trans value from the array and re-use the upper
		// three bits we find as the trans state
		char theTrans = ablob[offset+0] & 0xE0; 

		pByte = (char* ) &usTemp;
		ablob[offset+0] = (char) ( *(pByte + 1) | theTrans ); 
		ablob[offset+1] = (char) *(pByte + 0); 
		offset = offset + 2;

		// do the m_heading data element   , assume that heading is in degree not in radian
		lTemp=(int) motion.heading;
		dTemp=motion.heading-lTemp;
		lTemp=lTemp%360;
		if (lTemp < 0) // is neg, make positive
			lTemp = lTemp + 360.0;
		dTemp = lTemp+dTemp;
		usTemp = (unsigned short)(dTemp * 80.0);  // convert to units and pos range
		pByte = (char* ) &usTemp;
		FillupBlob(offset, ablob, pByte,2);
		offset = offset + 2;

		// do the m_steeringWheelAngle data element   set to the default value in CreateEmptyBSM
		offset = offset + 1;

		// Do the vehicle long accel 
		// convert from xx.xx to LSB units are 0.01 m/s^2
		usTemp = (unsigned short)(motion.accel.longAcceleration * 50.0); // convert units
		lTemp = (long)(motion.accel.longAcceleration * 50.0); // convert units
		pByte = (char* ) &usTemp;
		FillupBlob(offset, ablob, pByte,2); 
		offset = offset + 2;

		// None of this are in fact used by VISSIM style, so skipped
		// Do the vehicle lat accel   set to the default value in CreateEmptyBSM
		offset = offset + 2;
		// Do the vehicle vert accel   set to the default value in CreateEmptyBSM
		offset = offset + 1;
		// Do the vehicle yaw rate    set to the default value in CreateEmptyBSM
		offset = offset + 2;

		// do the m_brakeSet data element    set to the default value in CreateEmptyBSM
		offset = offset + 2;

		// do the width and length data elements, these values share
		// a 'middle' char so we have to combine them and then convert
		// the class values from meters to cm as well
		// we set the width to get the upper 10 bits and the length to get the bottom 14 bits
		lTemp = ((((long)((width  + 0.005) * 100.0)) &  0x03FF) << 14) + (((long)((length + 0.005) * 100.0)) &  0x3FFF);
		pByte = (char* ) &lTemp;
		FillupBlob(offset, ablob, pByte,3);
		offset = offset + 3;


		assert(offset == BSM_BlOB_SIZE); // check if we exactlly used 38 byte of blob
	}
	else 
		cout << "Passed in an empty pointer for BSM to the VehicleToBSM function" << endl; 

}

void BasicVehicle::FillupBlob( int offset,char * blob, char* p, int size){
	static int i;
	for (i=0;i<size;i++)
	{
		blob[offset+i]= *(p+(size-i-1));
	}
}


long BasicVehicle::UnpackBlob(int offset,char * blob,int size){
	long l=0;
	unsigned char ucTemp;
	for (int i=size-1;i>=0;i--){
		ucTemp=blob[offset+i];
		l+= (ucTemp <<  (size-1-i)*8) ;
	}
	return(l);
}



// end of file BasicVehicle.cpp
