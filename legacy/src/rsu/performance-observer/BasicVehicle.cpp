
// BasicVehicle.cpp
//
// Contains the implementation of class BasicVehicle which contains all the  
// logic to build and decode a BSM (Part I only) from/to its octets
// DCK Create August 19th 2011
//
#include "BasicVehicle.h"
#include <stdio.h>


inline double Degs2rads(double degs); // forward define


// Given a value in degrees, return its value in radians
inline double Degs2rads(double degs)
{
    return( ( ( degs / 180.0 ) * (3.14159265358979323846) ) );
}


// END Simple stubs for the other class above
// END Simple stubs for the other class above
// END Simple stubs for the other class above



// START Implementation of BasicVehicle code
// START Implementation of BasicVehicle code
// START Implementation of BasicVehicle code


BasicVehicle::BasicVehicle()
{
  Init();
}

BasicVehicle::~BasicVehicle()
{
}

// Set the calls values to a known point
void BasicVehicle::Init()
{
    // DEBUG: cout << " Instance of BasicVehicle class created." << endl;
    MsgCount = 0;    
    TemporaryID  = 0; 
    DSecond = 0;
    pos.elevation = 0.0;
    pos.latitude = 0.0;
    pos.longitude = 0.0;
    speed = 0.0;
    heading = 0.0;
	accel.latAcceleration = 0.0 ;
	accel.longAcceleration = 0.0 ;
	accel.verticalAcceleration = 0.0 ;
	accel.yawRate = 0.0 ;
	brakes.lf_BrakesApplied = false ;
	brakes.rf_BrakesApplied = false ;
	brakes.lr_BrakesApplied = false ;
	brakes.rr_BrakesApplied = false ;
	brakes.wheelBrakesUnavialable = false ;
	brakes.spareBit = false;
	brakes.traction_1 = false ;
	brakes.traction_2 = false ;
	brakes.anitLock_1 = false ;
	brakes.antiLock_2 = false ;
	brakes.stability_1 = false ;
	brakes.stability_2 = false ;
	brakes.boost_1 = false ;
	brakes.boost_2 = false ;
	brakes.auxBrakes_1 = false ;
	brakes.auxBrakes_2 = false ;

    length = -1; //is -1 better than 0 for length? Maybe not. 
    width = -1;
	weight = 0.0 ;

    desiredPhase = -1;
    stopBarDistance = -1.0; // rethink this as null value
    estArrivalTime = -1.0;  // rethink this as null value
}


// This function populates the vehicle attributes (the class data)
// from a BSM payload passed into it, decoding a message for use
void BasicVehicle::BSM2Vehicle(byte* ablob)
{
    // DEBUG: cout << " In method BSM2Vehicle()" << endl;
    if (ablob != NULL)
    {
	    // Note that the mapping to specific bytes is hard coded here
	    int offset;         // a counter into the blob bytes
        unsigned short   tempUShort; // temp values to hold data in final format
        long    tempLong;
        unsigned char   byteA;  // force to unsigned this time,
        unsigned char   byteB;  // we do not want a bunch of sign extension 
        unsigned char   byteC;  // math mucking up our combine logic
        unsigned char   byteD;  
        // byte is often defined signed or unsigned in machines, 
        // so here we are VERY clear what we want (else it will shift in sign bits)

        // Create an "empty" but valid starting point by reseting 
        // the class vars to a known state
        Init();

        // Un-Pack any items we have data for, leave others as is 
        //(the call above has valid default values)

        // CRITICAL DETAIL:
        // This was written for Intel platforms (which are all little endian)
        // this code will not work on any native big-endian machine, in such
        // a case the byte reversals shown below are NOT needed. 
        // ASN.1 is ALWAYS and without exception encoded as big endian

	    offset = BSM_BlOB_INDEX; // set to start of blob inside message

	    // do msgCnt   (1 byte)
        MsgCount = (byte)ablob[offset];
	    offset = offset + 1; // move past to next item

        // do temp ID  (4 bytes)
        byteA = ablob[offset+0];
        byteB = ablob[offset+1];
        byteC = ablob[offset+2];
        byteD = ablob[offset+3];
        TemporaryID = (long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
	    offset = offset + 4;

	    // do secMark  (2 bytes)
        byteA = ablob[offset+0];
        byteB = ablob[offset+1];
        DSecond = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	    offset = offset + 2;

	    // do the latitude data element (4 bytes)
        byteA = ablob[offset+0];
        byteB = ablob[offset+1];
        byteC = ablob[offset+2];
        byteD = ablob[offset+3];
        tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
        pos.latitude = (tempLong /  DEG2ASNunits); // convert and store as float
	    offset = offset + 4;

        // do the longitude data element (4 bytes)
        byteA = ablob[offset+0];
        byteB = ablob[offset+1];
        byteC = ablob[offset+2];
        byteD = ablob[offset+3];
        tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
        pos.longitude = (tempLong /  DEG2ASNunits); // convert and store as float
	    offset = offset + 4;

	    // There is no elevation data element in VISSIM, 
        // so this may just be a default constant each time
        byteA = ablob[offset+0];
        byteB = ablob[offset+1];
        tempLong = (unsigned long)((byteA << 8) + (byteB)); 
        pos.elevation = (tempLong / 10.0); // in 0.1 m steps
	    offset = offset + 2;

	    // do the accuracy data frame 
	    offset = offset + 4;

	    // do the Speed data element 
        byteA = ablob[offset+0];
        byteB = ablob[offset+1];
        // remove 3 upper transmission value bits from 1st byte
        byteA = byteA & 0x1F; 
        tempUShort  = (unsigned short)((byteA << 8) + (byteB));
        speed = (tempUShort / 50.0); // convert units
	    offset = offset + 2;

	    // do the m_heading data element 
        byteA = ablob[offset+0];
        byteB = ablob[offset+1];
        tempUShort = (short)((byteA << 8) + (byteB));
        heading = (tempUShort / 80.0);  // convert units
        // Below not needed unless prefer -180 to +180
        //if (heading > 180.0) // in ASN we use 0~360
        //    heading = heading - 360.0;
	    offset = offset + 2;

	    // do the m_steeringWheelAngle data element 
        // ??? = (byte)ablob[offset];
	    offset = offset + 1;

	    // Do the vehicle long accel 
        // convert from xx.xx to LSB units are 0.01 m/s^2
        byteA = ablob[offset+0];
        byteB = ablob[offset+1];
        tempUShort = (unsigned short)((byteA << 8) + (byteB));
        accel.longAcceleration  = ((short)tempUShort / 50.0);
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

	    assert(offset == BSM_MSG_SIZE); // test for all bytes accounted for
	    // assert(offset == BSM_BlOB_SIZE); // alt form when stuffing blob only


        /** DEBUG Hint - Useful background details
        // If all this big-little stuff confuses you, 
        // play with this code to see the byte order concept
        // On Intel the "little" byte is stored first, on a
        // Moto machines, or an ARM, or in ASN.1 it is reversed
        // The bit order within a byte is the same in all cases
        long value = 0xFFAA8811;
        byte* ptr;
        byte item;
        ptr = (byte* ) &value;
        item = *ptr;    // the LSB is first (11)
        ptr++;
        item = *ptr;    // (88)
        ptr++;
        item = *ptr;    // (AA)
        ptr++;
        item = *ptr;    // the MSB is last (FF)
        **/
    }
    else // gripe
        cout << "Passed in an empty pointer for BSM char byte array, no BSM2Vehicle decode." << endl; 
}





// Added for debug use
// When called, the current position is moved by applying the current heading and speed 
// values for a one second constant rate. Accleration is not used and these values are 
// not set, only the lat-long is changed, and the header is adjusted a small amount to 
// give some "curve" to the movement. To be removed when code works
// return -1 on failure, otherwise zero. 
int BasicVehicle::MoveAhead()
{
    // DEBUG: cout << " In method MoveAhead()" << endl;

    // Move time and count ahead, with modulo logic
    MsgCount++;   
    if (MsgCount > 127) MsgCount = 0;
    DSecond = DSecond + 1000;
    while (DSecond > 60000) DSecond = DSecond - 60000;

    // To make this work I cheat a bit and use local constants to convert 
    // "meters" to micro degree in each axis
    // form:  result = scale * angle of heading * speed * 1 (1 sec so m/s -> m)
    // Below taken from the map work, 
    // this if good to about ~0.5 cm for ~2k distances
    #define m2latScale  (1/ 110920.0)
    #define m2lonScale  (1/ 092554.0)

    double deltaLat = m2latScale * cos(Degs2rads(heading)) * speed;
    double delatLon = m2lonScale * sin(Degs2rads(heading)) * speed;
    pos.latitude    = pos.latitude  + deltaLat; // new position
    pos.longitude   = pos.longitude + delatLon;
    heading = CorrectAngleRange(heading - 6.0); // tweak heading a bit, gives the line some curve
    return(0);
}


// Output current selected key class values to console for debug use
void BasicVehicle::Out()
{
    // cout << " In method Out()" << endl;
    char text[100];
    sprintf(text,"%i , %2.8f , %2.8f , %2.3f , %2.4f \n", 
            TemporaryID,pos.latitude, pos.longitude,speed, heading  );
    cout << text;

}



// This function creates a BSM payload from a vehicle
// It translates the valid values unit to the native units 
// needed in the BSM octer stream and simply stuffs these 
// values into a canned template for use
// presumes that space pointed to can hold the BSM
void BasicVehicle::Vehicle2BSM(byte* ablob)
{
    // DEBUG: cout << " In method Vehicle2BSM()" << endl;
    if (ablob != NULL)
    {
	    // Note that the mapping to specific bytes is hard coded here
	    int offset;         // a counter for the repacking bytes
        byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
        byte    tempByte;   // values to hold data once converted to final format
        unsigned short   tempUShort;
        long    tempLong;
        double  temp;

        // Create an "empty" but valid starting point with a byte copy
        CopyMessageFromTemplate(ablob);

        // Pack any items we have data for, leave others as is 
        // (the blank message above has valid default values)

        // In general we create the final value in the right size data elmement
        // and then set a byte pointer to it and walk over each byte till done.
        // an alternative extract form would be as below (is a bit slower)
	    //ablob[offset+0] = (BYTE) ((m_secMark & 0x0000FF00) >> 8);
	    //ablob[offset+1] = (BYTE) ((m_secMark & 0x000000FF) ); // etc...

        // CRITICAL DETAIL:
        // This was written for Intel platforms (which are all little endian)
        // this code will not work on any native big-endian machine, in such
        // a case the byte reversals shown below are NOT needed. 
        // ASN.1 is ALWAYS and without exception encoded as big endian

	    offset = BSM_BlOB_INDEX; // set to start of blob inside message

	    // do msgCnt   (1 byte)
        tempByte = (byte)MsgCount; 
        ablob[offset] = (byte) tempByte;
	    offset = offset + 1; // move past to next item

        // do temp ID  (4 bytes)
        tempLong = (long)TemporaryID;  
        pByte = (byte* ) &tempLong;
        ablob[offset+0] = (byte) *(pByte + 3); // msb
        ablob[offset+1] = (byte) *(pByte + 2); // Note the bytes swapping 
        ablob[offset+2] = (byte) *(pByte + 1); // used here and below !!
        ablob[offset+3] = (byte) *(pByte + 0); // lsb
	    offset = offset + 4;

	    // do secMark  (2 bytes)
        // for this item DO we need to translate class var units for use???
        // or can we depend on user to set it right??
        tempUShort = (unsigned short)DSecond; 
        pByte = (byte* ) &tempUShort;
        ablob[offset+0] = (byte) *(pByte + 1); 
        ablob[offset+1] = (byte) *(pByte + 0); 
	    offset = offset + 2;

	    // do the latitude data element (4 bytes)
        tempLong = (long)(pos.latitude * DEG2ASNunits); // to 1/10th micro degees units
        pByte = (byte* ) &tempLong;
	    ablob[offset+0] = (byte) *(pByte + 3); 
	    ablob[offset+1] = (byte) *(pByte + 2); 
	    ablob[offset+2] = (byte) *(pByte + 1); 
	    ablob[offset+3] = (byte) *(pByte + 0); 
	    offset = offset + 4;

	    // do the longitude data element (4 bytes)
        tempLong = (long)(pos.longitude * DEG2ASNunits); // to 1/10th micro degees units
        pByte = (byte* ) &tempLong;
	    ablob[offset+0] = (byte) *(pByte + 3); 
	    ablob[offset+1] = (byte) *(pByte + 2); 
	    ablob[offset+2] = (byte) *(pByte + 1); 
	    ablob[offset+3] = (byte) *(pByte + 0); 
	    offset = offset + 4;

	    // do elevation data element
        tempUShort = (unsigned short)(pos.elevation * 10.0); // to 0.1 meter steps
        pByte = (byte* ) &tempUShort;
        ablob[offset+0] = (byte) *(pByte + 1); 
        ablob[offset+1] = (byte) *(pByte + 0); 
	    offset = offset + 2;

	    // do the m_accuracy data frame 
	    offset = offset + 4;

	    // do the Speed data element 
        // get it into the right units first 
        tempUShort = (unsigned short)(speed * 50.0);
        // need the current trans value from the array and re-use the upper
        // three bits we find as the trans state
        byte theTrans = ablob[offset+0] & 0xE0; 
                        // above was: 0xFE;  typo error that got it? DCK Aug 22nd
        pByte = (byte* ) &tempUShort;
	    ablob[offset+0] = (byte) ( *(pByte + 1) | theTrans ); 
	    ablob[offset+1] = (byte) *(pByte + 0); 
	    offset = offset + 2;

	    // do the m_heading data element 
        // CHECK: this code presumes we get degrees not rads
        temp = CorrectAngleRange(heading);
        if (temp < 0.0) 
            temp = 360.0 - temp; // if negative range, fix it
        else if (temp > 360.0)
            temp = temp - 360.0; // if negative range, fix it
        tempUShort = (unsigned short)(temp * 80.0);  // convert to units and pos range
        pByte = (byte* ) &tempUShort;
	    ablob[offset+0] = (byte) *(pByte + 1); 
	    ablob[offset+1] = (byte) *(pByte + 0); 
	    offset = offset + 2;

	    // do the m_steeringWheelAngle data element 
	    offset = offset + 1;

        // NOTE Next four items are all part of a packed data frame / blob
        // called DF_AccelerationSet4Way  - but thanksfully each
        // item lies on a byte boundary so we can just proces them
        // one at a time rather then building another packed array
        // also, all of these use the vehicle frame of reference
        // any lever arm translations should have been done allready

	    // Do the vehicle long accel 
        // convert from xx.xx to LSB units are 0.01 m/s^2
		tempUShort = (unsigned short)(accel.longAcceleration * 50.0); // convert units
        tempLong = (long)(accel.longAcceleration * 50.0); // convert units
        pByte = (byte* ) &tempUShort;
	    ablob[offset+0] = (byte) *(pByte + 1); 
	    ablob[offset+1] = (byte) *(pByte + 0); 
	    offset = offset + 2;
 
        // None of this are in fact used by VISSIM style, so skipped
	    // Do the vehicle lat accel 
	    offset = offset + 2;
	    // Do the vehicle vert accel 
	    offset = offset + 1;
	    // Do the vehicle yaw rate
	    offset = offset + 2;

	    // do the m_brakeSet data element 
	    offset = offset + 2;

	    // do the width and ength data elements, these values share
        // a 'middle' byte so we must combine things and convert
        // class values from meters to cm as well
        // width get upper 10 bits, length get bottom 14 bits
        tempLong = ((((long)((width  + 0.005) * 100.0)) &  0x03FF) << 14) +
                    (((long)((length + 0.005) * 100.0)) &  0x3FFF);
        pByte = (byte* ) &tempLong;
	    ablob[offset+0] = (byte) *(pByte + 2); 
	    ablob[offset+1] = (byte) *(pByte + 1); 
	    ablob[offset+2] = (byte) *(pByte + 0); 
	    offset = offset + 3;


	    assert(offset == BSM_MSG_SIZE); // test for all bytes accounted for
	    // assert(offset == BSM_BlOB_SIZE); // alt form when stuffing blob only
    }
    else // gripe
        cout << "Passed in an empty pointer for BSM char byte array, no Vehicle2BSM encode." << endl; 

}



// Copy bytes of an empty message to the passed in pointer
// used to reset a message to a known starting point
// typically called before creating msg value content
void BasicVehicle::CopyMessageFromTemplate(byte* theTarget)
{
    // Build up the default blob bytes here for use, then copy over
   unsigned byte emptyMsg [] = { 
        0x30, 
        0x2B, 
        0x80,   // msgID tag
        0x01,   // msgID length
        0x02,   // msgID value ==2 for a BSM
        0x81,   // blob tag
        0x26,   // blob length == 0x26 = 38 bytes
        0x00,   // start of blob data
        0x00, 
        0x00, 
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0xF0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x00, 
        0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
        0x00, 0x00, 0x0F, 0xA2, 0x58
    }; // note that there is no termination in a binary file
    // above set to 600cm long, 250cm width, all brakes off, 
    // all safety systems unavailable, accuracy not avail
    // in a forward gear, steering angle not avail
    // elev set to 0xF000 (unavailable)
    // note that the above items wil each be overwritten 
    // when there is any data for it

    // Test to confirm it is the correct and expected size here, 
    // this was added to avoid human edit errors in the above
    if (sizeof(emptyMsg) != BSM_MSG_SIZE)
    {
        cout << "ERROR: Blank message seems to be incorrect size, correct this.!" << endl;
        cout << "ERROR: Expected to find " << BSM_MSG_SIZE << " but found " << \
            sizeof(emptyMsg) << " bytes." << endl;
    }

    // Now do the work
    // copy each byte above over to the passed in pointer
    if (theTarget != NULL)
    {
        for(int i=0; i<BSM_MSG_SIZE; i++)
        {
            theTarget[i] = emptyMsg[i];
        }
    }
    else // gripe
        cout << "ERROR: passed a null pointer to the message copy call." << endl;
}



//int  BasicVehicle::WriteMessageToFile(char *filename)
//{
//    FILE *tempStream=fopen(filename);
//   
//
//
//}


// Returns true if the passed BasicVehicle object matches this one
// else false. Compares each variable in the class to determine this.
// if verbose is set true any item in the class that is not alike 
// will be noted in the cout stream. Otherwise operation is silent.
bool  BasicVehicle::IsEqual(BasicVehicle* pTest, bool vb)
{
    // we test to one LSB for now
    double diff;
    if (pTest != NULL)
    {
        bool isSame = true;
        // Test every item in turn, report any failure and set flag
        if (MsgCount != pTest->MsgCount)
        {
            isSame = false; 
            if (vb) cout << "  MsgCount differs " << endl;
        }
        if (TemporaryID != pTest->TemporaryID)
        {
            isSame = false; 
            if (vb) cout << "  TemporaryID differs " << endl;
        }
        if (DSecond != pTest->DSecond)
        {
            isSame = false; 
            if (vb) cout << "  DSecond differs " << endl;
        }
        // need to break down this item, it would be better if 
        // we had a similar call in that class of IsEqual()
        diff = pos.latitude - pTest->pos.latitude;
        if (fabs(diff) > 0.0000001 ) 
        {
            isSame = false; 
            if (vb) cout << "  Lat  pos (position) differs by " << diff << endl;
        }
        diff = pos.longitude - pTest->pos.longitude;
        if (fabs(diff) > 0.0000001 ) 
        {
            isSame = false; 
            if (vb) cout << "  Lon  pos (position) differs by " << diff << endl;
        }
        diff = pos.elevation - pTest->pos.elevation;
        if (fabs(diff) > 0.1 ) 
        {
            isSame = false; 
            if (vb) cout << "  Elev pos (position) differs by " << diff << endl;
        }

        diff = speed - pTest->speed;
        if (fabs(diff) > 0.04 ) 
        {
            isSame = false; 
            if (vb) cout << "  speed differs " << endl;
        }
        diff = heading - pTest->heading;
        if ((fabs(diff) > 0.0250) && (fabs(diff) != 360.0)) 
        {
            // checks for apodial reflection in above as well
            isSame = false; 
            if (vb) cout << "  heading differs " << endl;
        }
        diff = accel.longAcceleration - pTest->accel.longAcceleration;
        if (fabs(diff) > 0.04 ) 
        {
            isSame = false; 
            if (vb) cout << "  longAccel differs " << endl;
        }
        if (length != pTest->length)
        {
            isSame = false; 
            if (vb) cout << "  length differs " << endl;
        }
        if (width != pTest->width)
        {
            isSame = false; 
            if (vb) cout << "  width differs " << endl;
        }
        // May not want to test these but did so for now. 
        if (desiredPhase != pTest->desiredPhase)
        {
            isSame = false; 
            if (vb) cout << "  desiredPhase differs " << endl;
        }
        if (stopBarDistance != pTest->stopBarDistance)
        {
            isSame = false; 
            if (vb) cout << "  stopBarDistance differs " << endl;
        }
        if (estArrivalTime != pTest->estArrivalTime)
        {
            isSame = false; 
            if (vb) cout << "  estArrivalTime differs " << endl;
        }
        return(isSame);
    }
    else
    {
        if (vb) cout << "ERROR: Passed null pointer to function IsEqual()" << endl;
        return(false);
    }

}


// Given an angle value (in degrees) return it in the range (0~360)
inline double BasicVehicle::CorrectAngleRange(double angle)
{
    if ((angle > 0.0) && (angle < 360.0))
        return (angle); // in range, do nothing
    else if (angle < 0) // is neg, make positive
    {   
        while (angle < 0.0) angle = angle + 360.0;
    }
    else // is > 360, reduce into range
        while (angle > 360.0) angle = angle - 360.0;
    return(angle);
}


// end of file BasicVehicle.cpp



