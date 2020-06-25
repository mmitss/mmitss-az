/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   

/* ReqEntry.cpp
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

#include "ReqEntry.h"
#include <string.h>
ReqEntry::ReqEntry(void)
{
	VehID=0;//!!! B.H : strcpy(VehID,"");
	VehClass=10;  // very low priority
	ETA=0.0;
	Phase=0;       
	fSetRequestTime=0.0;
	MinGreen =0.0;
	Split_Phase=-10; // means not a split phase
	 iInLane=0;iOutLane=0; iStrHour=0; iStrMinute=0; iStrSecond=0; iEndHour=0;iEndMinute=0; iEndSecond=0; iVehState=0; iMsgCnt=0; 
	 
}


ReqEntry::ReqEntry(int vehID, int vehClass, double eta, int phase,float mgreen, int fsetRequestTime, int split_phase, int iinLane,int ioutLane,int istrHour,int istrMinute,int istrSecond,int iendHour,int iendMinute,int iendSecond,int ivehState, int imsgcnt)  
{ //!!! : ReqEntry(char * vehID, int vehClass, float eta, int phase,float mgreen, int absTime, int split_phase, int iinLane,int ioutLane,int istrHour,int istrMinute,int istrSecond,int iendHour,int iendMinute,int iendSecond,int ivehState, int imsgcnt) 
	VehID=vehID; //!!! B.H : strcpy(VehID,vehID);
	VehClass=vehClass;
	ETA=eta;
	Phase=phase;
	MinGreen = mgreen;
    fSetRequestTime=fsetRequestTime;
	Split_Phase=split_phase;
	iInLane=iinLane;
	iOutLane=ioutLane;
	iStrHour=istrHour;
	iStrMinute=istrMinute;
	iStrSecond=istrSecond;
	iEndHour=iendHour;
	iEndMinute=iendMinute;
	iEndSecond=iendSecond;
	iVehState=ivehState;
	iMsgCnt=imsgcnt;	 
}


ReqEntry::ReqEntry(ReqEntry& Req)  
{
	VehID = Req.VehID; //!!! strcpy(VehID,Req.VehID)
	VehClass=Req.VehClass;
	ETA=Req.ETA;
	Phase=Req.Phase;
	MinGreen = Req.MinGreen;
    fSetRequestTime=Req.fSetRequestTime;
	Split_Phase=Req.Split_Phase;
	iInLane=Req.iInLane;
	iOutLane=Req.iOutLane;
	iStrHour=Req.iStrHour;
	iStrMinute=Req.iStrMinute;
	iStrSecond=Req.iStrSecond;
	iEndHour=Req.iEndHour;
	iEndMinute=Req.iEndMinute;
	iEndSecond=Req.iEndSecond;
	iVehState=Req.iVehState;
	iMsgCnt=Req.iMsgCnt;	
	
}

ReqEntry& ReqEntry::operator=(ReqEntry& Req)
{
	VehID = Req.VehID; //!!! strcpy(VehID,Req.VehID);
	VehClass=Req.VehClass;
	ETA=Req.ETA;
	Phase=Req.Phase;
	MinGreen = Req.MinGreen;
    fSetRequestTime=Req.fSetRequestTime;
	Split_Phase=Req.Split_Phase;
	iInLane=Req.iInLane;
	iOutLane=Req.iOutLane;
	iStrHour=Req.iStrHour;
	iStrMinute=Req.iStrMinute;
	iStrSecond=Req.iStrSecond;
	iEndHour=Req.iEndHour;
	iEndMinute=Req.iEndMinute;
	iEndSecond=Req.iEndSecond;
	iVehState=Req.iVehState;
	iMsgCnt=Req.iMsgCnt;	

	return *this;
	
}

int ReqEntry::Display(char *logfilename)
{//BUG here,only display one entry
	char output[64];
	//sprintf(output," VehClass:%d, VehID:%s, Phase:%d, Arrival Time:%d, MiniGreen:%d\n",VehClass,VehID,Phase,ETA,MinGreen);
    sprintf(output," VehClass:%d, VehID:%d, Phase:%d, Arrival Time:%f  At time: [%f]\n",VehClass,VehID,Phase,ETA,fSetRequestTime); //!!! sprintf(output," VehClass:%d, VehID:%s, Phase:%d, Arrival Time:%f  At time: [%ld]\n",VehClass,VehID,Phase,ETA,AbsTime);
	cout<<output<<endl;

	fstream fs;
	fs.open(logfilename, ios::out | ios::app);
	if (!fs || !fs.good())
	{
		cout << "could not open file!\n";
		return -1;
	}
	fs << output << endl;

	if (fs.fail())
	{
		cout << "failed to append to file!\n";
		return -1;
	}

}


ReqEntry::~ReqEntry(void)
{}
