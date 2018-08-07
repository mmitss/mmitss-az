/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   

/* ReqEntry.h 
*  Created by Mehdi Zamanipour
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
#pragma once

#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

class ReqEntry
{
public:
	char VehID[64];			//  The ID of the vehicle
	int VehClass;		//	The Class of the vehicle
	float ETA;			//	The Estimation Time of Arrival
	int Phase;			//	The phase of the Intersection
	float MinGreen;		// Mini Green time to clear the queue in front of vehicle, only for 0 speed: Queue clear time
    double dSetRequestTime;
	int Split_Phase;    // If it is =0, then no split phase; >0  represents the split phase; =-1, means will not call SplitPhase
	int iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt;	 
	
public:
	ReqEntry();
	ReqEntry(char *vehID, int VehClass, float eta, int phase,float mgreen, double dsetRequestTime, int split_phase,int iInLane,int iOutLane,int iStrHour,int iStrMinute,int iStrSecond,int iEndHour,int iEndMinute,int iEndSecond,int iVehState	, int iMsgCnt);
    ReqEntry(ReqEntry& Req);
	ReqEntry& operator=(ReqEntry& Req);
	int Display(char *logfilename);
public:
	~ReqEntry();
 friend ostream &operator <<(ostream &stream, ReqEntry e)
        {
            stream<<"VehID: "<<e.VehID<<"\tClass: "<<e.VehClass<<"\tETA: "<<e.ETA<<"\tPhase: "<<e.Phase<<"\tSplitPhase: "
                <<e.Split_Phase<<endl;
        return stream;
        }
};
