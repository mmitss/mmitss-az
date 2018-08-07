/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   

/*  MMITSS_MRP_PriorityRequestServer_sim.cpp  
*  Created by Mehdi Zamanipour
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.
*
*  Revision History:
*  1. The app receives priority requests in the format of 2009 SAE j2735 SRM 
*  2. Request_conbimnd txt file is updated. New informaiton such as InLane,OutLane,
*  BeginServiceTime, EndServiceTime and IsCanceld added to each line of request_combined.txt
*  3. ASN1 decoder applied instead of savari decoder
*  4. PRS can integrate with ISIG (COP) with argument -u 2 and can work with priority+actuation by defaule argument -u 1 
*  5. If the application is running in the field, gps time is needed to set coordination priority requests and logging the time of receiving message.
      If the application is running in the simulation, the time is received from VISSIM 
*  6. If we receive a message, we Process the message. If it is SRM, decode it and populate SRM elements, update the request list and update the request 
      files and addd request to Req_List.  If it is VISSIM clock ,update dTime
*  7. The ETA of each request is updated every 1 second for the purpose of showing the ETA of requests inside the vehicle through SSM message		
*  8. If Request list changed because ReqListUpdateFlag becomes positive. Therefore, we write the list into requests.txt and requests_combined.txt with a 
      positive Flag so that Priority_Solver reads it and solve the problem.  ReqListUpdateFlag will be set to "0" after solving the problem in Priority_Solver
*  9. In case the application is being used with actuaion and prioirity method, 
	   sendClearCommandsToInterface function sends a command to the traffic signal interface to clear all commands for all phases. 
	  This happens when ever the last request pass the intersection (no request in request list)
   10. Preprossesor derivatives added to let prs work both for fieldtesting and simulation. In fieldtesing, we need gps time to set coordination request. Therefore, we need savari gps library. 
   11. Econolite proprietory OID is replaced by NITCIP OIDs to make the prs interoperable with other controlers.
* 
* */

#ifdef fieldtesting
#include "savariGPS.h"
#endif


#ifdef simulation
#include "dummySavariGPS.h"
#endif

#ifndef byte
    #define byte  char  
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <string>
#include <math.h>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "ReqEntryListHandle.h"
#include "LinkedList.h"
#include "ReqEntry.h"
#include "IntLanePhase.h"
#include "SRM.h"
#include "BasicVehicle.h"
#include "PriorityConfig.h"


using namespace std;

///* Reading Phase Status through NTCIP: different status has a number.  // FROM "rsu_config" app
//**********asc3PhaseStatusTiming2
// (1) X: XPED timing
// (2) N: Phase Next
// (3) -: Phase Not enabled
// (4) .: Phase Not Timing
// (5) R: Phase Timing RED
// (6) Y: Phase Timing YEL
// (7) G: Phase Timing GREEN
// (8) D: Phase Timing DELAY GREEN
// (9) O: Phase Timing YEL & RED
//(10) g: Phase Timing FLASHING GREEN
//(11) y: Phase Timing FLASHING YELLOW "


// We need to connect to Controller when there is EV in the list. This is required to know the split phase when and EV is among requests. 
// Before 11/3/17, we were using Econolite proprietory PHASE_STA_TIME2_ASC to get signal status,
#define PHASE_STA_TIME2_ASC  "1.3.6.1.4.1.1206.3.5.2.1.18.1.6."  // The OID that is used to get signal status from Controller. NEED last element as "p" for the phase.  
// MZP 11/3/17 we use NTCIP OID to calcute the signal color. This way, we have nteroperability between controllers
#define PHASE_GROUP_STATUS_GREEN   "1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1"  //which phase/phases is green
#define PHASE_GROUP_STATUS_RED     "1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1"  //which phase/phases is red
#define PHASE_GROUP_STATUS_YELLOW  "1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1"  //which phase/phases is yellow
#define PHASE_GROUP_STATUS_NEXT    "1.3.6.1.4.1.1206.4.2.1.1.4.1.11.1"  //which phase/phases is Next
#define MAX_CONTROLLER_OUTPUT_VALUES 50
#define MAX_NO_OF_PHASES 8
#define RED 1
#define GREEN 3
#define YELLOW 4
#define REAL_RED_STATUS 1
#define REAL_YELLOW_STATUS 4
#define REAL_GREEN_STATUS 3


#define MAX_MSG_BUFLEN  195
#define BSM_BLOB_SIZE 38
#define SIM_BROADCAST_ADDR "10.254.56.5"   //is used for getting VISSIM Simulation Time. It is the IP of the machine that VISSIM is running on

// This application can be applied only for pririty eligible vehicles OR can be integrated into COP (To Do)
#define PRIORITY 1
#define COP_AND_PRIORITY 2  

// This application can be used in simulation or in the field
#define FIELD 1
#define SIMULATION 2

#define TIME_INTERVAL_OF_RESOLVING 10  // When there is coordination request in the list, we resolve the optimization problem every 10 seconds

int iPORT=  4444;                  //  Socket Port: For receiving request from PriorityRequestGenerator ( PRG )
int iPRStoInterfacePort=44444;     // PRS sends a clear command to the traffic interface when the last priority vehicle passes the intersection


// for opening the UDP socket connection
int         iSockfd;
struct      sockaddr_in sendaddr;
struct      sockaddr_in recvaddr;
int         addr_length;
struct      timeval tv;             // struct for socket timeout: 1s

char        temp_log[256];
char        INTport[64];            // Port to connect to traffic signal controller e.g. "501"
char        INTip[64];              // IP to connect to traffic signal controller e.g. "150.135.152.23";
int         ReqListUpdateFlag=0;    // When this flag is positive, it will identify the ReqList is updated. Therefore, the Solver needs to resolve the problem IMPORTANT
string      RSUID;                  // will get intersection name from "rsuid.txt"
vector<int> v_PhaseNotEnabled;      // to get the phases from controller that are not active. 

// This file that stores the number of requests and the ReqListUpdateFlag and the requests information
// This file will be modified whenever the request list is updated. The file is being read by Solver. The Sover will make ReqListUpdateFlag flag zero and rewrite the content in request.txt
char   requestfilename[64]  		  = "/nojournal/bin/requests.txt";
// This file that stores the number of requests and the ReqListUpdateFlag and the requests information
// This file will be modified whenever the request list is updated. The file is being read by Solver. The Sover will make ReqListUpdateFlag flag zero and rewrite the content in request.txt
// The difference between requests.txt and requests_combined.txt is that requests_combined includes the requestsd split_phase when there is EV in the request list. 
char   requestfilename_combined[64]   = "/nojournal/bin/requests_combined.txt";
// This filed contains the mode weights, coordination phase split time and the coordination weight.
char   priorityConfiguartionFile[128] = "/nojournal/bin/priorityConfiguration.txt";
char   ConfigInfo[256]	  			  = "/nojournal/bin/ConfigInfo.txt";
char   rsuid_filename[64]   	 	  = "/nojournal/bin/rsuid.txt";
char   logfilename[256]    			  = "/nojournal/bin/log/MMITSS_MRP_PriorityRequestServer_";
char   LanePhaseFile[255]  	 		  = "/nojournal/bin/InLane_OutLane_Phase_Mapping.txt";
char   IPInfo[64]			  	 	  = "/nojournal/bin/IPInfo.txt"; // the file to read the traffic signal controller IP and port 

char   ConfigFile[256] ;
int    CombinedPhase[8]={0};
char   rxMsgBuffer[MAX_MSG_BUFLEN];   // a buffer for received messages
SRM_t  * srm=0;
asn_enc_rval_t ec;        		 // Encoder return value 
asn_dec_rval_t rval;
int    iAppliedMethod=1;  		 // If the argument is -c 1 , the program will be used with traffic interface (priority and actuation method) . if -c 0 as argument, the program work with ISIG  ( COP )
int    iApplicationUsage=1;		 // If the PriorityRequestServer is used in field testings, this value is 1. If it is used in Simulation testing, it is 2. This variable is determined as argument.
char   cRequestType[32];   		 // Whether it is a request or resuest_clear or coord_request, this variable is use to fill up tempMsg 
char   tempMsg[1024];      		 // To write the requests info into this variable. this variable will be passed to UpdateList function to update the request lists
char   cIntersectionName[32]; 	 
int    PhaseStatus[8];    		 // Determine the phase status for generate the split phases
int    phaseColor[8];
int    iColor[2][8];						 // iColor[1][*]  has the current phase color, and iColor[0][*] has the previous signal status phase color.
double dTime=0.0;                // The reference time, in FIELD it will be gps time, in SIMULATION it will be VISSIM time
double dVISSIMtime=0.0;
double dDiffSystemAndGPS=0.0;    // The difference between GPS and Sytem time in FIELD. This is used when the gps.time is not a number, but we ought to have a time for coordinatio in FIELD
double dCurrentTimeInCycle=100.0;// For example, if cycle is 100 and offset is 30, this variable will be between [-30 70)
LinkedList<ReqEntry> Req_List;   // List of all received requests
    
long   lTimeOut=300000;    		 // Time out of waiting for a new socket 0.3 sec!
int    iObsoleteTimeOfRemainedReq=30;   // if a request is not updated for iObsoleteTimeOfRemainingReq second in request list, it should be deleted ??????
double dCountDownIntervalForETA=1.0;    // The time interval that the ETA of requests in the requests table is updated for the purpose of count down  

bool  bAtOffsetRefPoint=0;
bool  bAtBeginingOfSmallerCoordSplit=0;
bool  bAtTheEndOfCoordPhaseSplits=0;
bool  bAtOffsetRefPointFlag=0;
bool  bAtBeginingOfSmallerCoordSplitFlag=0;
bool  bAtTheEndOfCoordPhaseSplitsFlag=0; 
int   iCoordMsgCont=0;	
float fCoordPhase1ETA=0.0;
float fCoordPhase2ETA=0.0;
float fCoordPhase1GreenHoldDuration=0.0;
float fCoordPhase2GreenHoldDuration=0.0;
double dTimeOfRepetativeSolve=0.0;           
int   iNumOfRxBytes=0; 
bool  bsetCoordForFistTime=0;
int   flagForClearingInterfaceCmd=0; // a flag to clear all commands in th interface when the request passed

PriorityConfig priorityConfig;
IntLanePhase lanePhase;


double getSimulationTime(char *);  //from VISSIM 
void  obtainInLaneOutLane( int srmInLane, int srmOutLane, int &inApproach,int &outApproach,int &iInlane, int &Outlane);
void  calculateETA(int beginMin, int beginSec, int endMin, int endSec, int &iETA );
void  packEventList(char* tmp_event_data, int &size);  // To send clear commands to the controller when the last request passes the intersection
void  sendClearCommandsToInterface();
void  setupConnection();                               // To set up udp socket to get SRM (FIELD case), or SRM and VISSIM time (SIMULATION case)
void  getControllerIPaddress();
void  getSignalConfigFile();
void  readPhaseTimingStatus(int PhaseStatus[8]);  // We need to get signal status to determin the split phase in case one priority vehicle is EV
int   getSignalColor(int PhaseStatusNo);
int   outputlog(char *output);
void  printReqestFile2Log(char *resultsfile);
void  xTimeStamp( char * pc_TimeStamp_ );
int   msleep(unsigned long milisec);
void  getRSUid();
void  clearRequestFiles();
void  creatLogFile();
int   handleArguments(int argc, char * argv[]);
void  setupConfigurationAndConnection();
void  readSignalControllerAndGetActivePhases();
int   FindVehClassInList(LinkedList<ReqEntry> Req_List,int VehClass);
int   gpsInitialization();
int   doWeNeedToSolveForCoordRequests();
void  calculateETAofCoordRequests();
void  updateCoordRequestsInList();
void  updateETAofRequestsInList();
void  setCoordinationPriorityRequests();
//void  processRxMessage(char * Msg);       			   // Process the message. If it is SRM, decode it and populate SRM elements, update the request list and update the request files
void processRxMessage(LinkedList<ReqEntry> & Req_List);
double readGPStime();
double getSystemTime();
void identifyColor(int color[2][8],int greenGroup, int redGroup, int yellowGroup, int PhaseNext);
void whichPhaseIsGreen(int phase_Color[8], int greenGroup, int redGroup, int yellowGroup); // this function return the color of the first argument which is phaseNo.

	
int main ( int argc, char* argv[] )
{
	int iAddrLeng=0;
	double dLastETAUpdateTime=0.0;	
    handleArguments(argc, argv);
    priorityConfig.readPriorityConfig(priorityConfiguartionFile);   
	setupConfigurationAndConnection();
	readSignalControllerAndGetActivePhases();
	lanePhase.ReadLanePhaseMap(LanePhaseFile);
	
	while ( true )
    {
		cout << "\n..................Waiting for the request info..................\n";
		cout<<"Flag at the begining  "<<ReqListUpdateFlag<<endl;
	    if (iApplicationUsage == FIELD)
			dTime=readGPStime();   
		else if ( (iApplicationUsage == SIMULATION) && (priorityConfig.dCoordinationWeight <= 0) )
			dTime=getSystemTime();
		else if ( (iApplicationUsage == SIMULATION) && (priorityConfig.dCoordinationWeight > 0) )
			dTime=dVISSIMtime;
	    iNumOfRxBytes = recvfrom(iSockfd, rxMsgBuffer, sizeof(rxMsgBuffer), 0,(struct sockaddr *) &recvaddr, (socklen_t *)&iAddrLeng);
    	ReqListUpdateFlag=getCurrentFlagInReqFile(requestfilename_combined);
    
		if (iNumOfRxBytes>-1)                                       
   			processRxMessage(Req_List); 	
		if ( (dTime - dLastETAUpdateTime > dCountDownIntervalForETA ) && ( Req_List.ListSize() > 0 ) )
		{
			sprintf(temp_log,"Updated ETAs in the listat time : %.2f \n ",dTime); 
            outputlog(temp_log);
		 	dLastETAUpdateTime=dTime;
			updateETAofRequestsInList();     
		}
		
		if (priorityConfig.dCoordinationWeight > 0)
			setCoordinationPriorityRequests();	
      
        if ( ReqListUpdateFlag>0 && Req_List.ListSize()>0 ) 
        {
            sprintf(temp_log,"At time: %.2f. ******** Need to solve ******** \n ",dTime); 
            outputlog(temp_log);
          	updateETAofRequestsInList(); 
          //	ReqListUpdateFlag=0;  
        }
        cout<<"ReqListUpdateFlag "<<ReqListUpdateFlag<<endl;
        cout<<"flagForClearingInterfaceCmd "<<flagForClearingInterfaceCmd<<endl;
        
     	if ( priorityConfig.dCoordinationWeight <= 0 && ( (ReqListUpdateFlag > 0 && Req_List.ListSize() == 0 ) || flagForClearingInterfaceCmd == 1)  ) // Request list is empty and the last vehisle just passed the intersection 
        {
			cout<<"Req List gets Empty"<<endl;
			ReqListUpdateFlag=0;
			flagForClearingInterfaceCmd=0;
			updateETAofRequestsInList();   
      
	// MZP		UpdateCurrentList(Req_List);
	// MZP		PrintList2File(requestfilename,Req_List,1);  // Write the requests list into requests.txt, 
	//		printReqestFile2Log(requestfilename);
	// MZP		PrintList2File(requestfilename_combined,Req_List,0);//Write the requests list into  requests_combined.txt; This file will be different than requests.txt when we have EV
	//		printReqestFile2Log(requestfilename_combined);
			if (iAppliedMethod==PRIORITY) 
				sendClearCommandsToInterface();
		}
    }
	
    return 0;  
} 


//========================================================================================//

int outputlog(char *output)  // JD 12.2.11
{
	FILE *stream = fopen(logfilename, "r");
    if (stream==NULL)
    {
        perror ("Error opening file");
    }
    fseek( stream, 0L, SEEK_END );
    long endPos = ftell( stream );
    fclose( stream );
    fstream fs;
    if (endPos <50000000)
        fs.open(logfilename, ios::out | ios::app);
    else
        fs.open(logfilename, ios::out | ios::trunc);
    if (!fs || !fs.good())
    {
        cout << "could not open file!\n";
        return -1;
    }
    fs << output;
    if (fs.fail())
    {
        cout << "failed to append to file!\n";
        return -1;
    }
    fs.close();
    cout<< output <<endl;
    return 1;
}




void xTimeStamp( char * pc_TimeStamp_ )
{
    struct tm  * ps_Time;
    time_t       i_CurrentTime;
    char         ac_TmpStr[256];

    i_CurrentTime =  time(NULL);
    ps_Time = localtime( &i_CurrentTime );

    //year
    sprintf(ac_TmpStr, "%d", ps_Time->tm_year + 1900);
    strcpy(pc_TimeStamp_, ac_TmpStr);

    //month
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_mon + 1 );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //day
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_mday );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //hour
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_hour  );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //min
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_min );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //sec
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_sec );
    strcat(pc_TimeStamp_, ac_TmpStr);
}



double getSystemTime()
{
	struct timeval tv_tt;
	gettimeofday(&tv_tt, NULL);
	return tv_tt.tv_sec+tv_tt.tv_usec/1.0e6 ;  
}

void creatLogFile()
{
	 //------log file name with Time stamp---------------------------
    char pctimestamp[128]; // time stamp for logging 
    xTimeStamp(pctimestamp);
    strcat(logfilename,pctimestamp);
    strcat(logfilename,".log");
    fstream ftemp;
    ftemp.open(logfilename,fstream::out);
    if (!ftemp.good())
    {
        perror("Open logfilename failed!"); exit(1);
    }
    else
    {
        ftemp<<"Start logging PRS at time:\t"<<time(NULL)<<endl;
        ftemp.close();
    }
}
void clearRequestFiles()
{
	system("\\rm /nojournal/bin/requests.txt");
	FILE *fp_req=fopen(requestfilename,"w");
    fprintf(fp_req,"Num_req -1 0\n");
    fclose(fp_req);
    system("\\rm /nojournal/bin/requests_combined.txt");
    fp_req=fopen(requestfilename_combined,"w");
    fprintf(fp_req,"Num_req -1 0\n");
    fclose(fp_req);
}

int msleep(unsigned long milisec)
{
    struct timespec req={0};
    time_t sec=(int)(milisec/1000);
    milisec=milisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*1000000L;
    while(nanosleep(&req,&req)==-1)
        continue;
    return 1;
}


void sendClearCommandsToInterface()
{
	byte tmp_event_data[500];
	int size=0;
	packEventList(tmp_event_data, size);
	char* event_data;
	event_data= new char[size];			
	for(int i=0;i<size;i++)
		event_data[i]=tmp_event_data[i];	
	if (sendto(iSockfd,event_data,size+1 , 0,(struct sockaddr *)&sendaddr, addr_length))
	{
		sprintf(temp_log," The Event List sent to SignalControllerInterface to delete all previous commands, The size is %d  \n", size); 
		outputlog(temp_log);
	}
}
void getRSUid() //DJ add 06/24/2011
{
    fstream fs;
    fs.open(rsuid_filename);
    char temp[128];
    getline(fs,RSUID);
    if(RSUID.size()!=0)
    {
        sprintf(temp," RSU ID %s\n",RSUID.c_str());
        cout<<temp<<endl;
        outputlog(temp);
    }
    else
    {
        sprintf(temp,"Reading RSUID.txt file problem.\n");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }
    fs.close();
}

int handleArguments(int argc, char * argv[])
{
	int ret=0;
	while ((ret = getopt(argc, argv, "p:t:c:u:")) != -1)    // -p  the port that the program receive SRM, -t the timeout for listening to a socket, -t c codeusage whether it is for ISIG and priority or just priority, 
	{
		switch (ret) 
		{
		case 'p':
			iPORT = atoi (optarg);
			printf ("Port is : %d\n", iPORT);
			break;
		case 't':
			lTimeOut = atoi (optarg);
			printf ("Time out is : %ld \n", lTimeOut);
			break;
		case 'u':
			iAppliedMethod = atoi (optarg);
			if (iAppliedMethod==COP_AND_PRIORITY)
				printf ("PRS is being used for integrated priority with I-SIG \n");
			else 
				printf ("PRS is being used for priority and actuation \n");
			break;
		case 'c':
			iApplicationUsage = atoi (optarg);
			if (iApplicationUsage == FIELD)
				printf (" PRS is being used for field \n");
			else if(iApplicationUsage == SIMULATION)
				printf (" PRS is being used for simulation testing \n");
			break;
		default:
			return 0;
		}
	} 
}

//void processRxMessage(char * Msg)
void processRxMessage(LinkedList<ReqEntry> & req_List)
{
	float fETA;
	int iRequestedPhase; 
	int iPriorityLevel;
	int iInApproach=0;
	int ioutApproach=0;
	int iInLane=0;
	int iOutlane=0;	
	int iETA=0;
	double dMinGrn;
	int	iStartMinute,iStartSecond,iEndMinute,iEndSecond;
	int iStartHour,iEndHour;
	int iVehicleState;
	long lvehicleID;
	int iMsgCnt;
	double dSpeed;
	unsigned long lintersectionID;
	char bsmBlob[BSM_BLOB_SIZE];
	BasicVehicle vehIn;
	
	srm =  (SRM_t  *) calloc(1, sizeof * (srm) );
	srm->timeOfService=(struct DTime*)calloc(1,sizeof( DTime_t));
	srm->endOfService=(struct DTime*)calloc(1,sizeof(DTime_t));
	srm->transitStatus= (BIT_STRING_t*)calloc(1, sizeof(BIT_STRING_t));
	srm->vehicleVIN=( VehicleIdent_t*)calloc(1, sizeof(VehicleIdent_t));
		

	cout<<"Message Received "<<endl;
	rval = ber_decode(0, &asn_DEF_SRM,(void **)&srm, rxMsgBuffer, sizeof(rxMsgBuffer));	
	//xer_fprint(stdout, &asn_DEF_SRM, srm);
	if (rval.code==RC_OK)
	{
		sprintf(temp_log,"SRM Recieved: Decode Success\n");
		outputlog(temp_log);
		for (int i=0;i<38;i++)     // extracting the bsm blob from the srm
			bsmBlob[i]=srm->vehicleData.buf[i];
		vehIn.BSMToVehicle(bsmBlob);
	//	lintersectionID=(srm->request.id.buf[2]<<16)+(srm->request.id.buf[1]<<8)+srm->request.id.buf[0];
				
		lintersectionID=(srm->request.id.buf[1]<<8)+srm->request.id.buf[0];  // Potential bug! Intersection ID in SRM is an optional (2bytes to 4 bytes) size data element. Its ASN.1 notation in SRM is:  IntersectionID ::= OCTET STRING (SIZE(2..4)). While in PRG we populate it as 2 byte data element.  Here in PRS, we only read the two byte. If intersection ID is a big number > 65535 (interger type capacity), there will be error
	//cout<<"lintersectionID"<<lintersectionID<<endl;
	
		if (lanePhase.iIntersectionID==lintersectionID)  // if the intersection ID in SRM matches the MAP ID of the intersection, SRM should be processed
		{
			if (srm->request.isCancel->buf[0]==0)
				strcpy( cRequestType, "request");
			else
				strcpy( cRequestType, "request_clear");
			obtainInLaneOutLane( srm->request.inLane->buf[0] , srm->request.outLane->buf[0] , iInApproach, ioutApproach, iInLane, iOutlane); 
			iRequestedPhase=lanePhase.iInLaneOutLanePhase[iInApproach][iInLane][ioutApproach][iOutlane];  
			iPriorityLevel=srm->request.type.buf[0] ; // is this the currect element of SRM to populate with vehicle  type?!
// MZP 10/10/17 deleted vehiceVIN data element population in PRG ---> dMinGrn is embeded in iETA and obtained using iVehicleState  
//			dMinGrn=((srm->vehicleVIN->id->buf[1]<<8)+srm->vehicleVIN->id->buf[0])/10; 			 // there was no place in SMR to store MinGrn !!!!!
			iStartMinute=srm->timeOfService->minute;
			iStartSecond=srm->timeOfService->second;
			iEndMinute=srm->endOfService->minute;
			iEndSecond=srm->endOfService->second;
			iStartHour=srm->timeOfService->hour;
			iEndHour=srm->endOfService->hour;
			calculateETA(iStartMinute,iStartSecond,iEndMinute,iEndSecond,iETA );
			fETA=(float) iETA;
			lvehicleID=vehIn.TemporaryID;
			iVehicleState=srm->status->buf[0];
			iMsgCnt=srm->msgCnt;
			dSpeed=vehIn.motion.speed;
			//printsrmcsv (&srm);
			if (iVehicleState==3) // vehicle is in queue
				dMinGrn=(double) (fETA);
		//	// we need the following if only becauase in obu webserver, the 2 shows the vehicle is in queue and 3 shows the vehicle is leavign
		//	if (iVehicleState==3) // vehicle is in queue
		//		iVehicleState=2;
		//	else if (iVehicleState==2) // vehicle is in queue
		//		iVehicleState=3;	
			sprintf(tempMsg,"%s %s %ld %d %.2f %d %.2f %.2f %d %d %d %d %d %d %d %d %d %d %f", cRequestType, cIntersectionName, lvehicleID, 
			iPriorityLevel,fETA,iRequestedPhase,dMinGrn, dTime, 
			srm->request.inLane->buf[0] , srm->request.outLane->buf[0] , iStartHour, iStartMinute, iStartSecond, iEndHour,iEndMinute, iEndSecond,
			 iVehicleState, iMsgCnt,0.0);
			sprintf(temp_log,"........... The Received SRM  match the Intersection ID  ,  at time %.2f. \n",dTime); 
			outputlog(temp_log);
			//~ sprintf(temp_log," ID, Type, ETA , Phase, QCT, inLane, outLane, Shr, Smn, Ssec, Ehr, Emn, Esec, State, Speed, Cnt  \n");
			//~ outputlog(temp_log);
			sprintf(temp_log,"%s\t \n",tempMsg);
			outputlog(temp_log);	
			readPhaseTimingStatus(PhaseStatus);  // Get the current phase status for determining the split phases
	//	sprintf(temp_log,"Current Signal status:     %d  %d  %d  %d  %d  %d  %d  %d\t",PhaseStatus[0],PhaseStatus[1],PhaseStatus[2],PhaseStatus[3],PhaseStatus[4],PhaseStatus[5],PhaseStatus[6],PhaseStatus[7]);
	//		outputlog(temp_log);
	// MZP		ReqListFromFile(requestfilename,Req_List);
			UpdateList(req_List,tempMsg,PhaseStatus);   // Update the Req List data structure considering received message
	// MZP		PrintList2File(requestfilename,Req_List,1);  // Write the requests list into requests.txt, 
	// MZP		printReqestFile2Log(requestfilename);
	// MZP		PrintList2File(requestfilename_combined,Req_List,0);//Write the requests list into  requests_combined.txt; 
	// MZP		printReqestFile2Log(requestfilename_combined);
			cout<<"Flag After Processing Received SRM  "<<ReqListUpdateFlag<<endl;
		}
		
	}
	else  
	{
		if ( (iApplicationUsage == FIELD) )
		{
			sprintf(temp_log," SRM Decode Failed ! \n");
			outputlog(temp_log);
		}
		else if ( (iApplicationUsage == SIMULATION) && (priorityConfig.dCoordinationWeight > 0) )
		{
			dVISSIMtime=getSimulationTime(rxMsgBuffer);
			sprintf(temp_log,"The received message is VISSIM clock %.2f\n",dVISSIMtime);
			outputlog(temp_log);
		}else if ( (iApplicationUsage == SIMULATION) && (priorityConfig.dCoordinationWeight <= 0) )
		{
			sprintf(temp_log,"The received message is VISSIM clock but is not considered b/c coordination weight is zero \n");
			outputlog(temp_log);
		}
	}
	free(srm->vehicleVIN);
	free(srm->transitStatus);
	free(srm->endOfService);		
	free(srm->timeOfService);
	free(srm);
}

void updateETAofRequestsInList()
{                          
		updateETAofRequestsInList(Req_List);
		deleteThePassedVehicle(Req_List);
		// Write the requests list into requests.txt,

		PrintList2File(requestfilename,Req_List,1);   
		//printReqestFile2Log(requestfilename);
		//Write the requests list into  requests_combined.txt; 
		//This file will be different than requests.txt when we have EV
		PrintList2File(requestfilename_combined,Req_List,0); 
		printReqestFile2Log(requestfilename_combined);
}	
void getSignalConfigFile()
{
    fstream fs;
    fstream fs_phase; //*** Read in all phases in order to find the combined phase information.***//
    fs.open(ConfigInfo);
    string lineread;
    getline(fs,lineread);
    if(lineread.size()!=0)
    {
        sprintf(ConfigFile,"%s",lineread.c_str());
        cout<<ConfigFile<<endl; 
        outputlog(ConfigFile);
        int phase_num;
        fs_phase.open(ConfigFile);
        getline(fs_phase,lineread); //*** Read the first line to get the number of all phases.
        sscanf(lineread.c_str(),"%*s %d ",&phase_num);
        getline(fs_phase,lineread); //*** Read the second line of the combined phase into array CombinedPhase[8]
        //*** If the phase exsits, the value is not 0; if not exists, the default value is '0'.
        sscanf(lineread.c_str(),"%*s %d %d %d %d %d %d %d %d",
            &CombinedPhase[0],&CombinedPhase[1],&CombinedPhase[2],&CombinedPhase[3],
            &CombinedPhase[4],&CombinedPhase[5],&CombinedPhase[6],&CombinedPhase[7]);
        fs_phase.close();
    }
    else
    {
        sprintf(temp_log,"Reading configure file %s problem",ConfigInfo);
		outputlog(temp_log);
        exit(0);
    }
    fs.close();
}

int getSignalColor(int PhaseStatusNo)
{
    int ColorValue=RED;

    switch (PhaseStatusNo)
    {
    case 2:
    case 3:
    case 4:
    case 5:
        ColorValue=RED;
        break;
    case 6:
    case 11:
        ColorValue=YELLOW;
        break;
    case 7:
    case 8:
        ColorValue=GREEN;
        break;
    default:
        ColorValue=0;
    }
    return ColorValue;
}


void setCoordinationPriorityRequests()
{
	dCurrentTimeInCycle = (int (dTime)) % (int (priorityConfig.dCoordCycle)) - priorityConfig.dCoordOffset;   
	
	//Every Cycle, the priorityCoordination.txt file is being read to ensure if the coorinatin plan has changed.
	//timeToReadPriorityConfigAgain();                 
	
	cout<<"Time:                                    "<< dTime << endl;
	cout<<"Time in cycle (time%cycleLength-offset): "<< dCurrentTimeInCycle << endl;

	//This if condition is used to start coordinating the signal in a smarter way.
	//For example, if cycle is 100 and coordinated splits are 30 and 40, the coordiantion request is set for the first time if the dCurrentTimeInCycle value is greater than 40. 
	//This way, the request will be for the next cycle and there should be enough time for transition from actuated to coordinated actuated control
	//bsetCoordForFistTime will not be changed after being set in this condition
	if (dCurrentTimeInCycle > priorityConfig.dLargerCoordinationSplit)    
		bsetCoordForFistTime=1;
	if (bsetCoordForFistTime==1) 
	{
		 // If the output of this function if 6, then we solve the problem again. Also, as a result of this function,
		 // we obtain the value of bAtBeginingOfSmallerCoordSplitFlag, bAtTheEndOfCoordPhaseSplitsFlag,  and  bAtOffsetRefPoint 
		 // that will be used in the calculateETAofCoordRequests function 
		if (doWeNeedToSolveForCoordRequests()==1)
		{ 
			calculateETAofCoordRequests();  
			updateCoordRequestsInList();
			ReqListUpdateFlag=6;
		}
		else if ( ( dTime > dTimeOfRepetativeSolve + TIME_INTERVAL_OF_RESOLVING ) && (ReqListUpdateFlag == 0) && ( FindVehClassInList(Req_List,COORDINATION) == 1 ) )// when coordination weight is positive, we resolve the problem every 10 second as well as beggining and end of coordination phase split time .. The second term in theif condition is nessecary at the start of the program when there is no coordination request in the list (no need to resolve)
		{
			cout<<"Time to update the coordination request and resolve the problem. (This is done each 10 seconds)"<<endl;
			dTimeOfRepetativeSolve=dTime;
			ReqListUpdateFlag=7;
		}
	 }		 
}



void updateCoordRequestsInList()
{
	iCoordMsgCont=(iCoordMsgCont+10)%127;
	sprintf(temp_log,"\n******************  Coordination Request Is Set ****************** At simulation time %.2f. \n",dTime ); 
	outputlog(temp_log); 
// MZP 	ReqListFromFile(requestfilename,Req_List);
	
	
	if (priorityConfig.iCoordinatedPhase[0] > 0)
	{
		// Coordination priotiy type is set to be 6 and there are two fake id for the two coorinated phase 99998 and 99999. This is done only to make the format of data in the tempMsg similar to the time when we have priority request from vehicles
		sprintf(tempMsg,"%s %s %d %d %.2f %d %.2f %.2f %d %d %d %d %d %d %d %d %d %d %.2f", cRequestType, cIntersectionName, 99998, 6 ,fCoordPhase1ETA, 
		priorityConfig.iCoordinatedPhase[0] ,fCoordPhase1GreenHoldDuration, dTime,	0 , 0 , 0, 0, 0, 0,0, 0, 1, iCoordMsgCont,dCurrentTimeInCycle);
		sprintf(temp_log,"{ %s }\t ",tempMsg); 
		ReqListUpdateFlag=6;
		UpdateList(Req_List,tempMsg,PhaseStatus);      // Update the Req List data structure considering first coordination 
	}
	if (priorityConfig.iCoordinatedPhase[1]  > 0)
	{
		sprintf(tempMsg,"%s %s %d %d %.2f %d %.2f %.2f %d %d %d %d %d %d %d %d %d %d %.2f", cRequestType, cIntersectionName, 99999, 6 ,fCoordPhase2ETA,
		priorityConfig.iCoordinatedPhase[1] ,fCoordPhase2GreenHoldDuration, dTime,	0 , 0 , 0, 0, 0, 0,0, 0, 1, iCoordMsgCont,dCurrentTimeInCycle); // the id for the second ring coordination request is set to 99999
		sprintf(temp_log,"{ %s }\t ",tempMsg); 
		ReqListUpdateFlag=6;
		UpdateList(Req_List,tempMsg,PhaseStatus);        // Update the Req List data structure considering second coordination 
	}

/*	
	PrintList2File(requestfilename,Req_List,1);         // Write the requests list into requests.txt, 
	printReqestFile2Log(requestfilename);
	PrintList2File(requestfilename_combined,Req_List,0);//Write the requests list into  requests_combined.txt; 
	printReqestFile2Log(requestfilename_combined);
*/
}

// As a result of this funciton, we obtain the value of fCoordPhase1GreenHoldDuration,fCoordPhase1ETA, 
// fCoordPhase2GreenHoldDuration,fCoordPhase2ETA that will be used in the updateCoordRequestInReqList.
void calculateETAofCoordRequests()
{                   
	strcpy( cRequestType, "coord_request" );	
	if (priorityConfig.bSimilarCoordinationSplit == 1) 	// if the two coordinated phase split times are the same
	{
		if (bAtOffsetRefPointFlag == 1) 						// if the signal is at the beginning of coordination offset reference point  
		{
		// MZP	bAtOffsetRefPoint=0;
			fCoordPhase1GreenHoldDuration= (float)  priorityConfig.dLargerCoordinationSplit+1;
			fCoordPhase2GreenHoldDuration= (float)  priorityConfig.dLargerCoordinationSplit+1;
			fCoordPhase1ETA=0.0;
			fCoordPhase2ETA=0.0;					
		}else 											// if the signal is at the end of the coordinated phases split time 
		{
			fCoordPhase1GreenHoldDuration=0.0;
			fCoordPhase2GreenHoldDuration=0.0;
			fCoordPhase1ETA= (float)  priorityConfig.dCoordCycle - priorityConfig.dLargerCoordinationSplit;
			fCoordPhase2ETA= (float)  priorityConfig.dCoordCycle - priorityConfig.dLargerCoordinationSplit;					
		}
	}  
		// if the two coorinated phase split times are different
	else 
	{
		// if the signal is at the beginning of coordination offset reference point
		if (bAtOffsetRefPointFlag == 1) 						   
		{
		// MZP	bAtOffsetRefPoint=0;
			if (priorityConfig.dCoordinationPhaseSplit[priorityConfig.iCoordinatedPhase[0]-1] > priorityConfig.dCoordinationPhaseSplit[priorityConfig.iCoordinatedPhase[1]-1] )   // if coordinated phase 1 has a larger split time that coordinated phase 2
			{
				fCoordPhase1GreenHoldDuration= (float)  priorityConfig.dCoordinationPhaseSplit[priorityConfig.iCoordinatedPhase[0]-1] + 1;
				fCoordPhase2GreenHoldDuration= 0.0;
				fCoordPhase1ETA = 0.0;
				fCoordPhase2ETA = (float) priorityConfig.dSplitDifference;					
			}else
			{
				fCoordPhase1GreenHoldDuration=0.0;
				fCoordPhase2GreenHoldDuration= (float) priorityConfig.dCoordinationPhaseSplit[priorityConfig.iCoordinatedPhase[1]-1]+1;
				fCoordPhase1ETA = (float)  priorityConfig.dSplitDifference;
				fCoordPhase2ETA = 0.0;					
			}
		}  
		// if we are at the begining of the smallet coordination split time
		else if (bAtBeginingOfSmallerCoordSplitFlag == 1)  
		{
				fCoordPhase1GreenHoldDuration= (float)  priorityConfig.dLargerCoordinationSplit - priorityConfig.dSplitDifference;
				fCoordPhase2GreenHoldDuration= fCoordPhase1GreenHoldDuration;
				fCoordPhase1ETA=0.0;
				fCoordPhase2ETA=0.0;					
		}
			// if the signal is at the end of the coordinated phases split time
		else if (bAtTheEndOfCoordPhaseSplitsFlag == 1)		  
		{
			fCoordPhase1GreenHoldDuration=0.0; 
			fCoordPhase2GreenHoldDuration=0.0;
			 // if coordinated phase 1 has a larger split time that coordinated phase 2
			if (priorityConfig.dCoordinationPhaseSplit[priorityConfig.iCoordinatedPhase[0]-1] > priorityConfig.dCoordinationPhaseSplit[priorityConfig.iCoordinatedPhase[1]-1] )  
			{
				
				fCoordPhase1ETA= (float)  priorityConfig.dCoordCycle-priorityConfig.dLargerCoordinationSplit;
				fCoordPhase2ETA= (float)  priorityConfig.dCoordCycle-priorityConfig.dLargerCoordinationSplit+priorityConfig.dSplitDifference;					
			}else
			{
				fCoordPhase1ETA= (float) priorityConfig.dCoordCycle-priorityConfig.dLargerCoordinationSplit+priorityConfig.dSplitDifference;					
				fCoordPhase2ETA= (float) priorityConfig.dCoordCycle-priorityConfig.dLargerCoordinationSplit;
			}
		}
	}

}

void setupConfigurationAndConnection()
{
	creatLogFile();								
    clearRequestFiles(); 						// clear the content of requests.txt and requests_combined.txt
	if ( (iApplicationUsage==FIELD) && (priorityConfig.dCoordinationWeight>0))  				// initializing the gps to get gps time for setting coordination time in case the application is running in field.
		gpsInitialization();
    getControllerIPaddress();  					// Get the ip address of controller
    getRSUid();  								// Get the current RSU ID from "rsuid.txt" into string "RSUID"
    getSignalConfigFile(); 					    // Read the configinfo_XX.txt from ConfigInfo.txt
    setupConnection();                          // For the wireless connection
 	strcpy(cIntersectionName, RSUID.c_str());   // get the intersection name
}


int gpsInitialization()
{
	int is_async = 0;
    int fd;
    char tempLogg[256];
    double dSystemTime=0.0;
    gps_handle = savari_gps_open(&fd, is_async);
    if (gps_handle == 0) 
    {
		sprintf(tempLogg,"No gpsd running, CAN NOT GET TIME TO SET COORDINATION REQUEST !!!! \n"); 
        cout<<tempLogg<<endl;
        outputlog(tempLogg);
        return -1;
    }
    else  // try to read the gps time for the first time
    {
		for (int i=0;i<30;i++) // 30 is a random number, if after 30 times we cannot read the gps time, we continue 
		{
			savari_gps_read(&gps, gps_handle);
			if (isnan(gps.time)==0) 
			{
				dTime=gps.time;                       // seting the time according to gps. 
				dSystemTime=getSystemTime();
				dDiffSystemAndGPS=dTime-dSystemTime;  // capturing this difference in case later in readGPStime(), the gps time is not a number. In that case, this differece will be used.
				break;
			}
			else
			{
				sprintf(tempLogg,"gpsd is running but gps time is not a number. Trying to read gps time again for the %d time \n", i ); 
				cout<<tempLogg<<endl;
				outputlog(tempLogg);
			}
			i++;
			msleep(100);  //sleep for 100 milisecond
		}			
	}
    return 0;	
}


double readGPStime()
{
	double dSystemTime=0.0;
	double dTempTime=0.0;
	savari_gps_read(&gps, gps_handle);
	if (isnan(gps.time)==0) 
	{
		dTempTime=gps.time;
		dSystemTime=getSystemTime();
	}
	else
	{
		dSystemTime=getSystemTime();
		dTempTime=dSystemTime+dDiffSystemAndGPS;
	}
	return dTempTime;
}



int FindVehClassInList(LinkedList<ReqEntry> Req_List,int VehClass)
{
    Req_List.Reset();

    int Have=0;
	if(Req_List.ListEmpty()==0)
    {
		while(!Req_List.EndOfList())
		{
			if(Req_List.Data().VehClass==VehClass)
			{
				return (Have=1);
			}
			else
			{
				Req_List.Next();
			}
		}
	}
    return Have;
}

	//At the reference offset time, we should put the coordination request for current cycle. 
	//If we have two different coordinated split times , we should set request at the beginning of the smallet split time.
	//At the end of the coordination phase split we should set the next cycle coordination request						
		
	// If we are at the beginnig of offset reference point , bAtOffsetRefPoint becomes one and the other flags are zero
	// For example, if the cycle is 100 second, offset is 20 and coord phase 1 split time is 40 and  coord phase 2 split time is 30
	// Therefore we know that -20 <dCurrentTimeInCycle<80 
	// dCurrentTimeInCycle 			    0    10    40    (40 79) and (-20 0)
	// bAtOffsetRefPoint  			    1    0      0     	    0
	// bAtBeginingOfSmallerCoordSplit   0    1      1     		1
	// bAtTheEndOfCoordPhaseSplits      0    0      1     		1 
int doWeNeedToSolveForCoordRequests()
{
	int iYesSolveIt=0;
	bAtBeginingOfSmallerCoordSplitFlag=0;
	bAtTheEndOfCoordPhaseSplitsFlag=0;
	bAtOffsetRefPointFlag=0;
	if ( (bAtOffsetRefPoint == 0) && (dCurrentTimeInCycle >= 0) &&
	   ( ((priorityConfig.bSimilarCoordinationSplit == 1) && (dCurrentTimeInCycle < priorityConfig.dLargerCoordinationSplit)) ||
		 ((priorityConfig.bSimilarCoordinationSplit == 0) && (dCurrentTimeInCycle < priorityConfig.dSplitDifference        )) )) 
	{
		bAtOffsetRefPoint=1;
		bAtOffsetRefPointFlag=1;
		bAtBeginingOfSmallerCoordSplit=0;
		bAtTheEndOfCoordPhaseSplits=0;
		iYesSolveIt=1;
	}
	 // If we are at the end of both two coordinated phases  split time
	if ( (dCurrentTimeInCycle >= priorityConfig.dLargerCoordinationSplit) && (bAtTheEndOfCoordPhaseSplits == 0) ) 
	{
		bAtOffsetRefPoint=0;
		bAtTheEndOfCoordPhaseSplits=1;
		bAtTheEndOfCoordPhaseSplitsFlag=1;
		iYesSolveIt=1; 
	}
	// If we are at the beiginnirng of the smaller coordinated phase split  ( incase the two coordinated phase split time are different)
	// For example, if the cycle is 100 second, offset is 20 and coord phase 1 split time is 40 and  coord phase 2 split time is 30, 
	// then this condition holds when dCurrentTimeInCycle >= (40-30)=10
	if ( (priorityConfig.bSimilarCoordinationSplit == 0) && (dCurrentTimeInCycle >= priorityConfig.dSplitDifference) && (dCurrentTimeInCycle < priorityConfig.dLargerCoordinationSplit) && (bAtBeginingOfSmallerCoordSplit == 0) )
	{	
		bAtOffsetRefPoint=0;
		bAtBeginingOfSmallerCoordSplit =1;
		bAtBeginingOfSmallerCoordSplitFlag =1;
		iYesSolveIt=1;
	}
	
	// MZP
	/*cout<<"bAtBeginingOfSmallerCoordSplitFlag"<<bAtBeginingOfSmallerCoordSplitFlag<<endl;
	cout<<"bAtTheEndOfCoordPhaseSplitsFlag"<<bAtTheEndOfCoordPhaseSplitsFlag<<endl;
	cout<<"bAtOffsetRefPointFlag"<<bAtOffsetRefPointFlag<<endl;
*/
	
	
	return iYesSolveIt;
}

/*
  
void readPhaseTimingStatus(int PhaseStatus[8])
{
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len;
    netsnmp_variable_list *vars;
    int status;
    init_snmp("ASC");   //Initialize the SNMP library
    snmp_sess_init( &session );  //Initialize a "session" that defines who we're going to talk to
    // set up defaults 
    char ipwithport[64];
    strcpy(ipwithport,INTip);
    strcat(ipwithport,":");
    strcat(ipwithport,INTport); //for ASC get status
    session.peername = strdup(ipwithport);
    session.version = SNMP_VERSION_1; //for ASC intersection  // set the SNMP version number 
    // set the SNMPv1 community name used for authentication 
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);
    SOCK_STARTUP;
    ss = snmp_open(&session);                     // establish the session 
    if (!ss)
    {
        snmp_sess_perror("ASC", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    
    // Create the PDU for the data for our request.
    //  1) We're going to GET the system.sysDescr.0 node.
    
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;
    //---#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan
    char ctemp[50];
    for(int i=1;i<=8;i++)
    {
        sprintf(ctemp,"%s%d",PHASE_STA_TIME2_ASC,i);
        if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }
        snmp_add_null_var(pdu, anOID, anOID_len);
    }
    
    // Send the Request out.
    
    status = snmp_synch_response(ss, pdu, &response);
    
    // Process the response.
    
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
        
        // SUCCESS: Print the result variables
        
        int *out = new int[MAX_CONTROLLER_OUTPUT_VALUES];
        int i =0;
        //~ for(vars = response->variables; vars; vars = vars->next_variable)
        //~	 print_variable(vars->name, vars->name_length, vars);
        // manipuate the information ourselves 
        for(vars = response->variables; vars; vars = vars->next_variable)
        {
            if (vars->type == ASN_OCTET_STR)
            {
                char *sp = (char *)malloc(1 + vars->val_len);
                memcpy(sp, vars->val.string, vars->val_len);
                sp[vars->val_len] = '\0';
                //printf("value #%d is a string: %s\n", count++, sp);
                free(sp);
            }
            else
            {

                int *aa;
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                //printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
            }
        }
        // GET the results from controller 
        for(int i=0;i<MAX_NO_OF_PHASES;i++)
        {
            PhaseStatus[i]=out[i];
            //PhaseStatus[i]=getSignalColor(out[i]);// DO not Convert
        }
    }
    else
    {
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet\nReason: %s\n",
            snmp_errstring(response->errstat));
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
            session.peername);
        else
            snmp_sess_perror("snmpdemoapp", ss);

    }
  
    // Clean up:    *  1) free the response.   *  2) close the session.
    
    if (response)        snmp_free_pdu(response);
    snmp_close(ss);
    SOCK_CLEANUP;
}
*/
void readPhaseTimingStatus(int PhaseStatus[8])
{
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len;
    netsnmp_variable_list *vars;
    int status;
	init_snmp("ASC");   //Initialize the SNMP library
    snmp_sess_init( &session );  //Initialize a "session" that defines who we're going to talk to
    /* set up defaults */
    //char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
    //char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
    char ipwithport[64];
    strcpy(ipwithport,INTip);
    strcat(ipwithport,":");
    strcat(ipwithport,INTport); //for ASC get status, DO NOT USE port!!!
    session.peername = strdup(ipwithport);
    session.version = SNMP_VERSION_1; //for ASC intersection  /* set the SNMP version number */
    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);
    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */
    if (!ss)
    {
        snmp_sess_perror("ASC", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to GET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;

    //---#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

    char ctemp[50];

  
	sprintf(ctemp,"%s",PHASE_GROUP_STATUS_GREEN);
	if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	{
		snmp_perror(ctemp);
		SOCK_CLEANUP;
		exit(1);
	}
	snmp_add_null_var(pdu, anOID, anOID_len);
	
	
	sprintf(ctemp,"%s",PHASE_GROUP_STATUS_RED);
	if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	{
		snmp_perror(ctemp);
		SOCK_CLEANUP;
		exit(1);
	}
	snmp_add_null_var(pdu, anOID, anOID_len);

	sprintf(ctemp,"%s",PHASE_GROUP_STATUS_YELLOW);
	if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	{
		snmp_perror(ctemp);
		SOCK_CLEANUP;
		exit(1);
	}
	snmp_add_null_var(pdu, anOID, anOID_len);
	
	sprintf(ctemp,"%s",PHASE_GROUP_STATUS_NEXT);
	if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	{
		snmp_perror(ctemp);
		SOCK_CLEANUP;
		exit(1);
	}
	snmp_add_null_var(pdu, anOID, anOID_len);
	
    /*
    * Send the Request out.
    */
    status = snmp_synch_response(ss, pdu, &response);

    /*
    * Process the response.
    */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
        /*
        * SUCCESS: Print the result variables
        */
      
        int *out = new int[MAX_CONTROLLER_OUTPUT_VALUES];
        int i =0;
        //~ for(vars = response->variables; vars; vars = vars->next_variable)
            //~ print_variable(vars->name, vars->name_length, vars);

        /* manipuate the information ourselves */
        for(vars = response->variables; vars; vars = vars->next_variable)
        {
            if (vars->type == ASN_OCTET_STR)
            {
                char *sp = (char *)malloc(1 + vars->val_len);
                memcpy(sp, vars->val.string, vars->val_len);
                sp[vars->val_len] = '\0';
                //printf("value #%d is a string: %s\n", count++, sp);
                free(sp);
            }
            else
            {

                int *aa;
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                //printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
            }
        }
        //****** GET the results from controller *************//
        identifyColor(iColor,out[0],out[1],out[2],out[3]);
        whichPhaseIsGreen(phaseColor, out[0],out[1],out[2]);
        for(int i=0;i<MAX_NO_OF_PHASES;i++)
        {
		// MZP ??? Needs more thought to get out the right split time	phase_read.phaseColor[i]=phaseColor[i];
            PhaseStatus[i]=iColor[1][i];   
        }
cout<<"phasecolor"<<endl;
	for(int i=0;i<MAX_NO_OF_PHASES;i++)
        {
			cout<< phaseColor[i] << " ";   
        }
        cout<<endl;
        
cout<<"iColor"<<endl;
         for(int i=0;i<MAX_NO_OF_PHASES;i++)
        {
			cout<< iColor[1][i] << " ";   
        }
        cout<<"  "<<endl;

        cout<<"Signal Gourp green "<< out[0]<< "  red " << out[1] << " yellow "<< out[2]<<" next "<< out[3]<<endl;
    }
    else
    {
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet\nReason: %s\n",
            snmp_errstring(response->errstat));
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
            session.peername);
        else
            snmp_sess_perror("snmpdemoapp", ss);

    }

    /*
    * Clean up:    *  1) free the response.   *  2) close the session.
    */
    if (response)        snmp_free_pdu(response);

    snmp_close(ss);

    SOCK_CLEANUP;
}

void readSignalControllerAndGetActivePhases()
{
	// geting the current active phase from controller
    readPhaseTimingStatus(PhaseStatus); // First found out the not used(enabled) phases if there is any.
    for(int i=0;i<8;i++)
    {
        if(PhaseStatus[i]==3)
            v_PhaseNotEnabled.push_back(i+1);
    }
    if(v_PhaseNotEnabled.size()>0)
    {
        v_PhaseNotEnabled.push_back(100); // JD 4/1/12 100 is a random number, later the v_PhaseNotEnabled will be used to search the unused phases
    }
}

void identifyColor(int i_Color[2][8], int greenGroup, int redGroup, int yellowGroup, int nextPhase)
{
	
	// numbers 2, 4,5,6 and 7 were the output of ASC/3 proprietry NTCIP OID for different status of a phase.   ( PHASE_STA_TIME2_ASC in the MIB.h )
	// 4 is for red phase, 7 for green 6 for yellow, 2 for the next and 5 for a phase that just gets red after being yellow.
	// Since the application was originally written based on this proprietary OID, we tranfer the obtained signal status out of red, green, yellow next group NTCIP OIDs to it.
// ????	for(int iii=0; iii<8; iii++)
//	{
// ???		if (ConfigIS.Gmax[iii]==0 || (( iii == ConfigIS.MissPhase[0] && ConfigIS.MissPhase[0]>=0) || (iii == ConfigIS.MissPhase[1] && ConfigIS.MissPhase[1]>=0) ) )  // in case phase iii is missing, the max is zero and i_Color gets 3. Since the Econolite proprietory OID, returns the  missing phase as 3, we need this conversion.  We do not want to use vendors proprietary OID
//			i_Color[1][iii]=3;
//		else
//			i_Color[1][iii]=4;    	
//	}
	if (redGroup!=255) // all phases are not red
	{
		if (greenGroup!=0) // at least one phase is green
		{
			switch (greenGroup)
			{
				case 1:
					i_Color[1][0]=7; // phase 1 is green
					break;
				case 2:
					i_Color[1][1]=7; //phase 2 is green
					break;
				case 4:
					i_Color[1][2]=7;
					break;
				case 8:
					i_Color[1][3]=7;
					break;
				case 16:
					i_Color[1][4]=7;
					break;
				case 32:
					i_Color[1][5]=7;
					break;
				case 64:
					i_Color[1][6]=7;
					break;
				case 128:
					i_Color[1][7]=7;
					break;
					
				case 17:
					i_Color[1][0]=7; // phase 1 is green
					i_Color[1][4]=7; // phase 5 is green
					break;
				case 33:
					i_Color[1][0]=7; //phase 2 is green
					i_Color[1][5]=7; 
					break;
				case 18:
					i_Color[1][1]=7;
					i_Color[1][4]=7; 
					break;
				case 34:
					i_Color[1][1]=7;
					i_Color[1][5]=7;
					break;
				case 68:
					i_Color[1][2]=7;
					i_Color[1][6]=7;
					break;
				case 132:
					i_Color[1][2]=7;
					i_Color[1][7]=7;
					break;
				case 72:
					i_Color[1][3]=7;
					i_Color[1][6]=7;
					break;
				case 136:
					i_Color[1][7]=7;
					i_Color[1][3]=7;
					break;
			}
		}
		if (yellowGroup!=0) // at least one phase is yellow
		{
			switch (yellowGroup)
			{
				case 1:
					i_Color[1][0]=6;
					break;
				case 2:
					i_Color[1][1]=6; 
					break;
				case 4:
					i_Color[1][2]=6;
					break;
				case 8:
					i_Color[1][3]=6;
					break;
				case 16:
					i_Color[1][4]=6;
					break;
				case 32:
					i_Color[1][5]=6;
					break;
				case 64:
					i_Color[1][6]=6;
					break;
				case 128:
					i_Color[1][7]=6;
					break;
					
				case 17:
					i_Color[1][0]=6; 
					i_Color[1][4]=6; 
					break;
				case 33:
					i_Color[1][0]=6; 
					i_Color[1][5]=6; 
					break;
				case 18:
					i_Color[1][1]=6;
					i_Color[1][4]=6; 
					break;
				case 34:
					i_Color[1][1]=6;
					i_Color[1][5]=6;
					break;
				case 68:
					i_Color[1][2]=6;
					i_Color[1][6]=6;
					break;
				case 132:
					i_Color[1][2]=6;
					i_Color[1][7]=6;
					break;
				case 72:
					i_Color[1][3]=6;
					i_Color[1][6]=6;
					break;
				case 136:
					i_Color[1][7]=6;
					i_Color[1][3]=6;
					break;
			}
		}
		
		if (nextPhase!=0) // at least one phase is yellow and the next phase after it is known
		{
			switch (nextPhase)
			{
				case 1:
					i_Color[1][0]=2; 
					break;
				case 2:
					i_Color[1][1]=2;
					break;
				case 4:
					i_Color[1][2]=2;
					break;
				case 8:
					i_Color[1][3]=2;
					break;
				case 16:
					i_Color[1][4]=2;
					break;
				case 32:
					i_Color[1][5]=2;
					break;
				case 64:
					i_Color[1][6]=2;
					break;
				case 128:
					i_Color[1][7]=2;
					break;
					
				case 17:
					i_Color[1][0]=2; 
					i_Color[1][4]=2; 
					break;
				case 33:
					i_Color[1][0]=2; 
					i_Color[1][5]=2; 
					break;
				case 18:
					i_Color[1][1]=2;
					i_Color[1][4]=2; 
					break;
				case 34:
					i_Color[1][1]=2;
					i_Color[1][5]=2;
					break;
				case 68:
					i_Color[1][2]=2;
					i_Color[1][6]=2;
					break;
				case 132:
					i_Color[1][2]=2;
					i_Color[1][7]=2;
					break;
				case 72:
					i_Color[1][3]=2;
					i_Color[1][6]=2;
					break;
				case 136:
					i_Color[1][7]=2;
					i_Color[1][3]=2;
					break;
			}
		}
		
		// put the current status into the previous status
		for(int iii=0; iii<8; iii++)
			i_Color[0][iii]=i_Color[1][iii];  
	}
	else // redGroup==255   , we need this else condition to distinguish which one of the current red phases are just turned to red from yellow
	{	
		for(int iii=0; iii<8; iii++)
		{	
			if (i_Color[0][iii]==6)  // previously was yellow
				i_Color[1][iii]=5;  // 5 means the phase was yellow and now it is red
			else if (i_Color[0][iii]==2) 
				i_Color[1][iii]=2;
			else
				i_Color[1][iii]=i_Color[0][iii];
		}
	}	
		
}


void whichPhaseIsGreen(int phase_Color[8], int greenGroup, int redGroup, int yellowGroup) // this function return the color of the first argument which is phaseNo.
{
	for(int iii=0; iii<8; iii++)
			phase_Color[iii]=REAL_RED_STATUS;
	
	if (redGroup!=255) // if redGroup=255 therfore all phases are RED 
	{
		if (greenGroup!=0) // at least one phase is green
		{
			switch (greenGroup)
			{
				case 1:
					phase_Color[0]=REAL_GREEN_STATUS; // phase 1 is green
					break;
				case 2:
					phase_Color[1]=REAL_GREEN_STATUS; //phase 2 is greeen
					break;
				case 4:
					phase_Color[2]=REAL_GREEN_STATUS;
					break;
				case 8:
					phase_Color[3]=REAL_GREEN_STATUS;
					break;
				case 16:
					phase_Color[4]=REAL_GREEN_STATUS;
					break;
				case 32:
					phase_Color[5]=REAL_GREEN_STATUS;
					break;
				case 64:
					phase_Color[6]=REAL_GREEN_STATUS;
					break;
				case 128:
					phase_Color[7]=REAL_GREEN_STATUS;
					break;
					
				case 17:
					phase_Color[0]=REAL_GREEN_STATUS; // phase 1 is green
					phase_Color[4]=REAL_GREEN_STATUS; // phase 5 is green
					break;
				case 33:
					phase_Color[0]=REAL_GREEN_STATUS; //phase 2 is green
					phase_Color[5]=REAL_GREEN_STATUS; 
					break;
				case 18:
					phase_Color[1]=REAL_GREEN_STATUS;
					phase_Color[4]=REAL_GREEN_STATUS; 
					break;
				case 34:
					phase_Color[1]=REAL_GREEN_STATUS;
					phase_Color[5]=REAL_GREEN_STATUS;
					break;
				case 68:
					phase_Color[2]=REAL_GREEN_STATUS;
					phase_Color[6]=REAL_GREEN_STATUS;
					break;
				case 132:
					phase_Color[2]=REAL_GREEN_STATUS;
					phase_Color[7]=REAL_GREEN_STATUS;
					break;
				case 72:
					phase_Color[3]=REAL_GREEN_STATUS;
					phase_Color[6]=REAL_GREEN_STATUS;
					break;
				case 136:
					phase_Color[7]=REAL_GREEN_STATUS;
					phase_Color[3]=REAL_GREEN_STATUS;
					break;
			}
		}
		if (yellowGroup!=0) // at least one phase is green
		{
			switch (yellowGroup)
			{
				case 1:
					phase_Color[0]=REAL_YELLOW_STATUS; // phase 1 is green
					break;
				case 2:
					phase_Color[1]=REAL_YELLOW_STATUS; //phase 2 is green
					break;
				case 4:
					phase_Color[2]=REAL_YELLOW_STATUS;
					break;
				case 8:
					phase_Color[3]=REAL_YELLOW_STATUS;
					break;
				case 16:
					phase_Color[4]=REAL_YELLOW_STATUS;
					break;
				case 32:
					phase_Color[5]=REAL_YELLOW_STATUS;
					break;
				case 64:
					phase_Color[6]=REAL_YELLOW_STATUS;
					break;
				case 128:
					phase_Color[7]=REAL_YELLOW_STATUS;
					break;
					
				case 17:
					phase_Color[0]=REAL_YELLOW_STATUS; // phase 1 is green
					phase_Color[4]=REAL_YELLOW_STATUS; // phase 5 is green
					break;
				case 33:
					phase_Color[0]=REAL_YELLOW_STATUS; //phase 2 is green
					phase_Color[5]=REAL_YELLOW_STATUS; 
					break;
				case 18:
					phase_Color[1]=REAL_YELLOW_STATUS;
					phase_Color[4]=REAL_YELLOW_STATUS; 
					break;
				case 34:
					phase_Color[1]=REAL_YELLOW_STATUS;
					phase_Color[5]=REAL_YELLOW_STATUS;
					break;
				case 68:
					phase_Color[2]=REAL_YELLOW_STATUS;
					phase_Color[6]=REAL_YELLOW_STATUS;
					break;
				case 132:
					phase_Color[2]=REAL_YELLOW_STATUS;
					phase_Color[7]=REAL_YELLOW_STATUS;
					break;
				case 72:
					phase_Color[3]=REAL_YELLOW_STATUS;
					phase_Color[6]=REAL_YELLOW_STATUS;
					break;
				case 136:
					phase_Color[7]=REAL_YELLOW_STATUS;
					phase_Color[3]=REAL_YELLOW_STATUS;
					break;
			}
		}
	}
}

void getControllerIPaddress()
{
    fstream fs;	
    fs.open(IPInfo);
    string lineread; 	
    getline(fs,lineread);
    if(lineread.size()!=0)
    {
        sprintf(INTip,"%s",lineread.c_str());
        cout<<"Controller IP Address is "<< INTip<<endl;
        getline(fs,lineread);
        sprintf(INTport,"%s",lineread.c_str());
        cout<<"Controller PORT Address is "<< INTport<<endl;
    }
    else
    {
        cout<<"A problem in reading IPinfo.txt file problem"<<endl;
        exit(0);
    }
    fs.close();
}


void printReqestFile2Log(char *resultsfile)
{
    fstream fss;
    fss.open(resultsfile,fstream::in);
    if (!fss)
    {
        cout<<"***********Error opening the plan file in order to print to a log file!\n";
        sprintf(temp_log,"***********Error opening the plan file in order to print to a log file!\n");
        outputlog(temp_log);
        exit(1);
    }
    string lineread;
    sprintf(temp_log," Content of requeast files  :");
    outputlog(temp_log);
    while(!fss.eof())
    {
        getline(fss,lineread);
        strcpy(temp_log,lineread.c_str());
        strcat(temp_log," "); 
        outputlog(temp_log);
    }
    fss.close();
}



void obtainInLaneOutLane( int srmInLane, int srmOutLane, int & inApproach, int &outApproach, int &iInlane, int &Outlane)
{
	
	inApproach=(int) (srmInLane/10);
	iInlane=srmInLane-inApproach*10;
	outApproach=(int) (srmOutLane/10);
	Outlane=srmOutLane-outApproach*10;
}


void calculateETA(int beginMin, int beginSec, int endMin, int endSec, int &iETA )
{
	
	 if ((beginMin ==59) && (endMin == 0)) // 
	 	 iETA= (60-beginSec)+endSec;
	 if ((beginMin ==59) && (endMin == 1)) // 
	 	 iETA= (60-beginSec)+endSec+60;
 	 if ((beginMin ==58) && (endMin == 0)) // 
	 	 iETA= (60-beginSec)+endSec+60;
	 if (endMin - beginMin  ==0)
	 	 iETA= endSec-beginSec;
	 else if ( endMin - beginMin  ==1)
	 	 iETA=(60-beginSec)+endSec;
	 else if ( endMin - beginMin ==2)
	 	 iETA=(60-beginSec)+endSec+60;
}




//~ 
//~ void printsrmcsv (J2735SRM_t *srm) 
//~ {
//~ printf("Msg ID :%2x\n", srm->message_id);                                                             
//~ printf("Msg Count :%2x\n", srm->msgcount);                                                            
//~ printf("Intersection ID %x\n", srm->intersection_id);                                                 
//~ printf("Cancel Priority %x Cancel Preemption %x\n", srm->cancelreq_priority, srm->cancelreq_preemption);                                                                           
//~ printf("Signal  Preempt %x Priority %x \n", srm->signalreq_priority, srm->signalreq_preemption);                                              
//~ printf("Lane Num  IN %x OUT %x \n", srm->in_lanenum, srm->out_lanenum);                      
//~ printf("Veh Type %x  Veh Class %x \n", srm->vehicleclass_type, srm->vehicleclass_level);                                        
//~ printf("Code word %s\n", srm->code_word);                                                    
//~ printf("Start Time %02d:%02d:%02d -> End Time %02d:%02d:%02d\n", srm->starttime_hour,srm->starttime_min,srm->starttime_sec, srm->endtime_hour, srm->endtime_min, srm->endtime_sec);                                                                           
//~ printf("Transit status %x\n", srm->transitstatus);                                               
//~ printf("Vehicle Name %s\n", srm->vehicle_name);                                                  
//~ printf("Vin %s\n", srm->vin);                                                                    
//~ printf("Vehicle owner code %s\n", srm->vehicle_ownercode);                                       
//~ printf("Ident temp id %llx\n", srm->temp_ident_id);                                              
//~ printf("Vehicle type %d\n", srm->vehicle_type);                                                  
//~ printf("Class %x\n", srm->vehicle_class);                                                        
//~ printf("Group type %d\n", srm->vehicle_grouptype);                                               
//~ 
//~ printf("Temp ID :%llx\n", srm->temp_id);                                                         
//~ printf("SecMark :%x\n", srm->dsecond);                                                           
//~ printf("Latitude %8.2f\n", srm->latitude);                                                       
//~ printf("Longitude %8.2f\n", srm->longitude);                                                     
//~ printf("Elevation %8.2f\n", srm->elevation);                                                     
//~ printf("Pos 0 %8.2f\n", srm->positionalaccuracy[0]);                                             
//~ printf("Pos 1 %8.2f\n", srm->positionalaccuracy[1]);
//~ 
//~ printf("Pos 2 %8.2f\n", srm->positionalaccuracy[2]);        
//~ printf("TState %lf\n", srm->transmissionstate);                                                            
//~ printf("Speed %8.2f\n", srm->speed);                                                                       
//~ printf("Heading %8.2f\n", srm->heading);                                                                   
//~ printf("Angle %8.2lf\n", srm->angle);                                                                      
//~ printf("longAccel %8.2f\n", srm->longaccel);                                                               
//~ printf("LatAccel %8.2f\n", srm->lataccel);                                                                 
//~ printf("VertAccel %8.2f\n", srm->vertaccel);                                                               
//~ printf("YawRate %8.2f\n", srm->yawrate);                                                                   
//~ printf("Wheel Brake %x\n", srm->wheelbrake);                                                               
//~ printf("Wheel Brake AV %x\n", srm->wheelbrakeavailable);                                                   
//~ printf("SpareBit %x\n", srm->sparebit);                                                                    
//~ printf("Traction %x\n", srm->traction);                                                                    
//~ printf("ABS %x\n", srm->abs);                                                                              
//~ printf("Stab Contl %x\n", srm->stabilitycontrol);                                                          
//~ printf("BrakeBoost %x\n", srm->brakeboost);                                                                
//~ printf("Aux Brakes %x\n", srm->auxbrakes);                                                                 
//~ printf("Width %8.2f\n", srm->vehicle_width);                                                               
//~ printf("Length %8.2f\n", srm->vehicle_length);                                                             
//~ 
//~ 
//~ printf("*******srm PART1*********\n");                                                                     
//~ printf("Vehicle status %x\n", srm->vehicle_status);     
//~ 
//~ 
//~ }
//~ 

 
 
 
 

void setupConnection()
{
	//Struct for UDP socket timeout: 1s
    tv.tv_sec = 0;
    tv.tv_usec = lTimeOut;
    addr_length = sizeof ( recvaddr );
  
	// -------------------------Network Connection for receiving SRM from VISSIM--------//
	 if((iSockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
    {
        perror("iSockfd");
        exit(1);
    }
    
    int optval = 1;
    setsockopt(iSockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    
    //Setup time out
   if (setsockopt(iSockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)  
    {
		      perror("Error");
    }
    
    // setup socket so that it can be reuse for vissim clock as well
    
    //
    //~ if((setsockopt(iSockfd,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof broadcast)) == -1)
    //~ {
        //~ perror("setsockopt - SO_SOCKET ");
        //~ exit(1);
    //~ }
    //~ 
     // set up sending socket to interface to clean all commands when there is no request in the table
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_port = htons(iPORT);
    recvaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("10.254.56.255") ;;
    memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
    if(bind(iSockfd, (struct sockaddr*) &recvaddr, sizeof recvaddr) == -1)
    {
        perror("bind");
        exit(1);
    }
    //addr_len = sizeof  sendaddr ;
 	//~ sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = htons(iPRStoInterfacePort);//htons(PortOfInterface);   // PRS sends a mesage to traffic control interface to delete all the commands when ther is no request in the request table
	sendaddr.sin_addr.s_addr = inet_addr("127.0.0.1") ; //INADDR_ANY; // //INADDR_BROADCAST;
	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);
}




void packEventList(char* tmp_event_data, int &size)
{
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    // byte    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
    //header 2 bytes
	tmp_event_data[offset]=0xFF;
	offset+=1;
	tmp_event_data[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x03 for signal event data send to Signal Control Interface
	tmp_event_data[offset]=0x03;
	offset+=1;
	//No. events in R1
	int numberOfPhase=4;
	int tempTime=0;
	int tempCmd=3;
	tempUShort = (unsigned short)numberOfPhase;
	pByte = (byte* ) &tempUShort;
    tmp_event_data[offset+0] = (byte) *(pByte + 1); 
    tmp_event_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	//Events in R1
	for (int iii=0;iii<4;iii++)
	{
		//Time 
		tempLong = (long)(tempTime); 
		pByte = (byte* ) &tempLong;
		tmp_event_data[offset+0] = (byte) *(pByte + 3); 
		tmp_event_data[offset+1] = (byte) *(pByte + 2); 
		tmp_event_data[offset+2] = (byte) *(pByte + 1); 
		tmp_event_data[offset+3] = (byte) *(pByte + 0); 
		offset = offset + 4;
		//phase
		tempUShort = (unsigned short) iii+1 ;
		pByte = (byte* ) &tempUShort;
		tmp_event_data[offset+0] = (byte) *(pByte + 1); 
		tmp_event_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//action
		tempUShort = (unsigned short)tempCmd;
		pByte = (byte* ) &tempUShort;
		tmp_event_data[offset+0] = (byte) *(pByte + 1); 
		tmp_event_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
	}

	tempUShort = (unsigned short)numberOfPhase;
	pByte = (byte* ) &tempUShort;
    tmp_event_data[offset+0] = (byte) *(pByte + 1); 
    tmp_event_data[offset+1] = (byte) *(pByte + 0); 
	//Events in R
	offset = offset + 2;
	for (int iii=0;iii<4;iii++)
	{
		//Time 
		tempLong = (long)(tempTime); 
		pByte = (byte* ) &tempLong;
		tmp_event_data[offset+0] = (byte) *(pByte + 3); 
		tmp_event_data[offset+1] = (byte) *(pByte + 2); 
		tmp_event_data[offset+2] = (byte) *(pByte + 1); 
		tmp_event_data[offset+3] = (byte) *(pByte + 0); 
		offset = offset + 4;
		//phase
		tempUShort = (unsigned short) iii+5 ;
		pByte = (byte* ) &tempUShort;
		tmp_event_data[offset+0] = (byte) *(pByte + 1); 
		tmp_event_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//action
		tempUShort = (unsigned short)tempCmd;
		pByte = (byte* ) &tempUShort;
		tmp_event_data[offset+0] = (byte) *(pByte + 1); 
		tmp_event_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
	}
	size=offset;
}

double getSimulationTime(char * buffer)
{
	unsigned char byteA, byteB, byteC, byteD;
	byteA = buffer[0];
	byteB = buffer[1];
	byteC = buffer[2];
	byteD = buffer[3];
	long  DSecond = (long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD)); // in fact unsigned
	return DSecond/10.0;
}
//~ 
//~ int msleep_sim(int duration)
//~ {
	//~ float t1,t2;
	//~ 
	//~ recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
                        //~ (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);
	//~ recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
                        //~ (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);    
	//~ 
	//~ 
	//~ unsigned char byteA, byteB, byteC, byteD;
    //~ byteA = buf_clock[0];
    //~ byteB = buf_clock[1];
    //~ byteC = buf_clock[2];
    //~ byteD = buf_clock[3];
    //~ 
	//~ long  DSecond = (long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD)); // in fact unsigned
	//~ 
	//~ t1=DSecond/10.0;
	//~ t2=t1;
	//~ while(t2-t1<duration/1000)
	//~ {
		//~ recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
                        //~ (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);
		//~ recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
                        //~ (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);    
//~ 
		//~ byteA = buf_clock[0];
		//~ byteB = buf_clock[1];
		//~ byteC = buf_clock[2];
		//~ byteD = buf_clock[3];
    //~ 
		//~ DSecond = (long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD)); // in fact unsigned
	//~ 
		//~ t2=DSecond/10.0;
	//~ }
//~ return 0;	
	//~ 
//~ }
/*
void readCoordinationConfig( char * filename)
{
	char TempStr[256];
	double dCoordinationWeight=0.0;
    int iCoordinatedPhase[2];
    int iPhases[8];
    double iTransitWeight=0.0;
    double iTruckWeight=0.0;
    double dCoordOffset=0.0;
    double dCoordCycle=0.0;
    double dCoordPhaseSplit[8];
    string lineread;
    fstream FileRead2;
    FileRead2.open(filename,ios::in);
    if(!FileRead2)
    {
        cerr<<"Unable to open priorityConfiguration.txt file!"<<endl;
        exit(1);
    }
    getline(FileRead2,lineread);
      
    // priorityConfiguration.txt 
    // # this file is to set the priority configuration and coorination setup, an example will be :
    // coordination_weight 1
    // cycle 90 
    // offset 0
    // coordinated_phase1 2
    // coordinated_phase2 6
    // phase_split 
    // 1 20
    // 2 30
    // 3 10
    // 4 30
    // 5 20
    // 6 30
    // 7 10
    // 8 30
    // transit_weight 3
    // truck_weight 1
    
	getline(FileRead2,lineread);
	if (lineread.size()!=0)
	{
		sscanf(lineread.c_str(),"%*s %lf ",&dCoordinationWeight);
		getline(FileRead2,lineread);
		sscanf(lineread.c_str(),"%*s %lf ",&dCoordCycle );
		getline(FileRead2,lineread);
		sscanf(lineread.c_str(),"%*s %lf ",&dCoordOffset );
		getline(FileRead2,lineread);
		sscanf(lineread.c_str(),"%*s %ld ",&iCoordinatedPhase[0]);
		getline(FileRead2,lineread);
		sscanf(lineread.c_str(),"%*s %ld ",&iCoordinatedPhase[1]);
		getline(FileRead2,lineread);
		for (int cnt=0;cnt<8;cnt++)
		{
			getline(FileRead2,lineread);
			sscanf(lineread.c_str(),"%d %lf", &iPhases[cnt] , &dCoordPhaseSplit[cnt] );				
		}	
		getline(FileRead2,lineread);
		sscanf(lineread.c_str(),"%*s %lf ",&iTransitWeight);
		getline(FileRead2,lineread);
		sscanf(lineread.c_str(),"%*s %lf ", &iTruckWeight);                
	}
    FileRead2.close();
  
	if (dCoordinationWeight>0)
        priorityConfig.dCoordinationWeight=1;
    else
        priorityConfig.dCoordinationWeight=0;
    cout<< " Coordination Weight" << dCoordinationWeight<< endl;
    if ( priorityConfig.dCoordinationWeight==1)
    {
		cout<< " Coordination Cycle  :" << dCoordCycle<< endl;
		cout<< " Coordination Offset :" << dCoordOffset<< endl;
		if (iCoordinatedPhase[0]>0)
		{
			cout<< " Coordinated Phase 1 :" << iCoordinatedPhase[0]<< endl;
			cout<< " Coordinated Phase 1 Split :" << dCoordPhaseSplit[iCoordinatedPhase[0]-1]<< endl; 
			iCoordinatePhase1=iCoordinatedPhase[0];
		    dCoordPhase1Split=dCoordPhaseSplit[iCoordinatedPhase[0]-1];
		}
		if (iCoordinatedPhase[1]>0)
		{
			cout<< " Coordinated Phase 2 :" << iCoordinatedPhase[1]<< endl;
			cout<< " Coordinated Phase 2 Split :" << dCoordPhaseSplit[iCoordinatedPhase[1]-1]<< endl; 
			iCoordinatePhase2=iCoordinatedPhase[1];
			dCoordPhase2Split=dCoordPhaseSplit[iCoordinatedPhase[1]-1];
		}
	}  
    
    dCoordinationCycle =dCoordCycle;
    dOffset=dCoordOffset;	
}
*/
