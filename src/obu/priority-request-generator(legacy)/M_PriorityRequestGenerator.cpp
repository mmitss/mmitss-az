/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  MRP_PriorityRequestServer.cpp  
 *  Created by Mehdi Zamanipour on 9/27/14.
 *  University of Arizona
 *  ATLAS Research Center
 *  College of Engineering
 *
 *  This code was develop under the supervision of Professor Larry Head
 *  in the ATLAS Research Center.
 *
 *  Revision History:
 *  1. SRM ASN library added to support j2735 srm standard.
 *  2. 
 *  3. 
 *  3. 
 *  
 */


/* This application can   1) Receive and process the SRM that is being sent from VISSIM DriveModel.dll   (Require for Laboratory Tests)
 * 						  2) Acquire GPS position from OBE GPS unit and struct the SRM  (Require for Road Tests) 										
   This applicaton should be run on Savari ASD
Prerequisits:
1. Create a log file in the /nojournal/bin folder on the device for logging
2. Modify the ConfigInfo.txt file in the folder of the VISSIM model and write the IP address of the OBE in it
3. required lib in the make file              LIBS =-L$(TOOLCHAIN)/lib -L$(TOOLCHAIN)/usr/lib -lstdc++ -luClibc++ -leloop -lnetsnmp -lj2735 -lsae -lm -lgps -lgpsapi -lsavarimath
4. Arguments that can be modified by users are :
   - The broadcasting address can be changed by the user. If Communication is through WME the address should be 127.0.0.1 otherwise the address is 192.168.1.255
   - If iLogIndicator is 2, more details information is logged
   - If outof the map detection time is 5, means after 5 * (0.5) =2.5 second if the vehicle GPS location shows the vehicle is out of the map, the request would be deleted
   - Key characters for arguments are:
    -c if c=1 the code works in Lab test, if c=2 code works in road. (by default c=2)
    -b change broadcastaddress
    -l change logging option
    -r DSRC Range 
    -o Out of The MAP detection time 
    -p the port of SRM should be sent to , default is 4444
    -s the percentage for speed change limit. SRM will be sent if the speed changes beyond this level
     -u the request serving Rectangle Range by default it is 6 seconds 
5. The preprosessor drivitives added to prg to run the exact code on savari device and in simulation networks on docker. The header file in make directory should be changed to add CFLAGS+= -Dsimulation or CFLAGS+= -Dfieldtesting
	
*/




#ifdef fieldtesting
#include "savariGPS.h"
#endif


#ifdef simulation
#include "dummysavariGPS.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <istream>
#include "math.h"
#include "time.h"
#include <time.h>
#include <vector>
#include <sys/time.h>
#include <iomanip>
#include <stddef.h>
#include <dirent.h>
using namespace std;
// Added header files
#include "NMAP.h"
#include "geoCoord.h"
#include "BasicVehicle.h"
#include "PriorityVehicle.h"
#include "LinkedList.h"
#include "MAPEntry.h"
#include "ReqEntry.h"
#include <SRM.h>


#ifndef SRM_MSG_SIZE
#define SRM_MSG_SIZE  (197)
#endif

#ifndef PI
#define PI 3.14159265
#endif

#ifndef BSM_MSG_SIZE
#define BSM_MSG_SIZE  (45)
#endif


#define FIELD 1
#define SIMULATION 2

#define ACTIVE 1
#define NOT_ACTIVE -1
#define LEAVE 0

#define APPROACHING 1
#define LEAVING 2
#define INQUEUE 3
#define NOTINMAP 4
#define PASSEDINTERSECTION 5

#define GEOFENCE_LIMIT 8  //8 meters

//define log file name and intermediate files
char predir[64]          = "/nojournal/bin/";
char logfilename[256]    = "/nojournal/bin/log/MRP_PRG_";
char ConfigInfo[256]	 = "/nojournal/bin/ConfigInfo.txt";
char vehicleidFile[256]  = "/nojournal/bin/vehicleid.txt";
char MAP_File_Name[128]  = "/nojournal/bin/Daysi_Gav_Reduced.nmap"; 
char MAPsListFile[256]   = "/nojournal/bin/Intersection_maps.txt";
char ActiveMAPFile[256]  = "/nojournal/bin/ActiveMAP.txt";
char RxAcknowledgedRequestsFile[128]   = "/nojournal/bin/psm.txt";
char cBusStopsFile[128]  = "/nojournal/bin/busStopsRange.txt";
char vehicleinfoFile[128]="/nojournal/bin/vehicleinfo.txt"; // added by shayan to show vehicle info in the v2v aaplication
BasicVehicle vehIn;       					 	// This variable is used in fieldtesting usage of PRG to create the bsmblob part of srm
PriorityVehicle TempVeh;   				  	 	// This variable is used to store the priority vehicle trajectory
LinkedList <PriorityVehicle> priorityReqList;   // List of current priority vehicle (in case of fieldtesting, there is only one priority vehicle. In case of simulation, we may have multiple priority vehicels)
LinkedList <ReqEntry> sentPriorityReqList;      // List of the priority vehicles that already have sent out their request (need it to chase the acknowledgement logic)
LinkedList <MAP> MAPsParsedList;  				// to store the parsed list from MapEntryList 
LinkedList <ReqEntry> acknowledgedPriorityReq;  // Req list for requests that are being acknowleged by RSU.
int    icodeUsage=1;  						    // if codeusage=2 the code works in Lab test, if codeusage=1 code works in road. (by default c=1)
double iSpeedChngPrc=0.2; 						// Critical Speed change percentage. SRM will be sent again if the speed changes beyond this percentage
double dSpeedChangeLmtForSRM=0.0; 				// Critical Speed change value. SRM will be sent again if the speed changes beyond this level. This variabe has different values depending on simulation and fieldtesting
int    iRu_RlRange=2; 							// The range between upper estimated arrival time and lower estimated arrival time. ( upper and lower bound of serving rectangle )
int    iLogIndicator=0;  						// indicates the level of detail logging we need
char   cBroadcastAddress[64]="127.0.0.1";   	// DSRC address we send the srm to
int    iSRMPort=4444;  							// The port we send the srm to
int    iOutOfMAPTimer=5;  						// If outof the map detection time is 5, means after 5 * (0.5) =2.5 second if the vehicle GPS location shows the vehicle is out of the map, hhe request would be deleted
int    iDSRCRange=300; 							// DSRC Rannge by default is 300 meters
int    iMsgCnt=0;
// Vairables required for getting GPS 
double m_vehlat,dHeading,m_vehlong,dElevation;
float  m_vehspeed;
double dgpsTime=0.0;
double dgpsTime_pre=0.0;			 // we need to know the previous value for every gps related variable in case the gps unit does not return a valid number
double dHeading_pre=0.0;
double m_vehlat_pre=0.0;
double m_vehlong_pre=0.0;
float  m_vehspeed_pre=0.0;
double dElevation_pre=0.0;
geoCoord geoPoint;				
double ecef_x,ecef_y,ecef_z;			
//Important!!!
//Note the rectangular coordinate system: local_x is N(+) S(-) side, and local_y is E(+) W(-) side 
//This is relative distance to the ref point!!!!
//local z can be considered as 0 because no elevation change.
double local_x,local_y,local_z;
// wireless connection variables
int    sockfd;
struct sockaddr_in sendaddr;
struct sockaddr_in recvaddr;
// srm related variables
SRM_t * srm=0;
asn_enc_rval_t ec; 					// Encoder return value 
asn_dec_rval_t rval; 				// Decoder return valuevoid setupConnection();
//transit vehicles related varibales
bool bIsItaTransit=0;  			 	// if the vehicle type is a transit, we have to have the location of  bus stops. 
bool bBusIsInRange=0;				// If the transit has paased the bus stop and is in the range of an intersection 
int  iBusStopsApprRange[100][8]; 	// consider 100 intersection at most for a transit route. Each intersetcion has 8 approaches at most. 
int  iBusStopsMAPID[100]; 			// the ID of the intersections in the bus route
int  iNoOfIntersectionInBusRoute=0;  // total number of intersection in the transit route!
int  iNumberOfAcknowledgedRequests=0;// Number of Requests in the psm.txt
int  iVehicleId;    					// It is read from vehicleID.txt file in nojournal/bin/ when we use the code for fieldtesting. In simulation this comes from vissim
int  iVehiclePriorityLevel;  		// This is the vehicle class:  EV is 1, TRANSIT is 2, Truck is 3. The information is included in the vehicleID.txt and is used when the code is for fieldtesting. In simulation this comes from vissim.
char BSMblob[38];					// To store the bsm blob
char buf[SRM_MSG_SIZE];				// To store the srm buffer for sending out 
char SRMbuf[SRM_MSG_SIZE];			// To store the srm buffer for decoding
char cVehicleName[128];				// It is read from vehicleID.txt file in nojournal/bin/ when we use the code for fieldtesting.
char predir_temp[128];				// To contain /nojournal/bin/Intersection_MAP
char temp_log[1024];			    // To contain logging text whenever we call outputlog function
// srm savari library related variable and functions which we do not used in the ASN version of PRG
//~ char buf[SRM_MSG_SIZE];
//~ uint8_t srmbuf[SRM_MSG_SIZE]; 
//~ int iType;
//~ J2735SRM_t srm;
//~ uint32_t oob;
//~ void printsrmcsv (J2735SRM_t *srm);
//~ void fillSRM(SRM_t *srm, BasicVehicle *veh, int intersectioId, int iMsgCnt, bool bIsCanceled, int iInLane, int ioutlane, int iETA, double dMingrn, int vehState, int prioritylevel, int Ru_Rl, char* cvehiclename);   // this function is used when MRP_PRG applied for road test
//~ void fillSRM(J2735SRM_t *srm, int intersectioId, int iMsgCnt,bool bIsCanceled,int iInLane,int ioutlane, int iETA,double dMingrn,int vehState , int Rl_Ru, char *vehiclename);  // this function is used when MRP_PRG applied for lab test

int  outputlog(char *output);
void xTimeStamp( char * pc_TimeStamp_ );
void setup_logging();
void setupConnection();
void get_veh_id(char filename[]);
void getBusStopsRange(char filename[]); // Getting the Bus stops range to intersections for the transit vehicles (if there is any).  This function read the distance of the bus stops (far-side bus stops) to the intersections from file. 
int  msleep(unsigned long milisec);
double getSeconds();  // in fieldtesting it is gps time, in simulation it is system time
int  gps_init();
void populate_gps_values();
void formBSMOfSRM(BasicVehicle *veh, int tempID, int msgcnt, double lat, double lon, double heading, double speed, double elev);   // this function is used when MRP_PRG applied for road test
void calculateETA(int iStartMinute,int iStartSecond,int iEndMinute,int iEndSecond,int &iETA );
void obtainInLaneOutLane( int srmInLane, int srmOutLane, int & inApproach, int &outApproach, int &iInlane, int &Outlane);
void obtainEndofService( int ihour,int imin,int isec, int iETA,int &iHour,int &iMin,int &iSec, int iMinGrn,int Ru_Rl);
void fillSRM(SRM_t *srm,  LinkedList<PriorityVehicle> & priorityReqList, int i, char * bsmblob);  
int  reqListFromRxAcknowledgedRequestsFile(char *filename,LinkedList<ReqEntry>& Req_List); //read the request list from request_combine txt file and retunr the number of request and also put the read request into the request data structure
int  thisVehIDPositionInList( long VehID, LinkedList<ReqEntry> Req_List); // Will return the position of the request with VehID in the Req_List
bool getDataFromVissim(int receiverAddressLength);
int  whatIsVISSIMType(int vissimVehType); 
void createMapFile(long mapID);  // to create Intersection_Map_IDXXXX.nmap file in /nojournal/bin
void readMAPsFromFile(char *filename,LinkedList<MAPEntry>& MapEntryList);
void handelReceivedMaps(LinkedList<MAP> & MAPParsedList,LinkedList<MAPEntry>  MapEntryList);
void findPriorityVehicleInMap(double Speed, double Heading,int nFrame,double N_Offset1,double E_Offset1,double N_Offset2,double E_Offset2, MAP NewMap,double &Dis_curr, double &ETA, int &iVehcleStatus,int &approach, int &lane, double &dMinGrn);
void storeVehicleTrajectory(LinkedList <PriorityVehicle> & trackedVeh);
void populateMAPNodes(MAP *NewMap, geoCoord *geocoord); // add MAP node to the NEW MAP structure and calculate node headings 
void distanceFromLine(double cx, double cy, double ax, double ay ,double bx, double by, double &distanceSegment, double &distanceLine); //Shayan Added for Geo Fencing 10.3.14  
double  geoFencing(MAP NewMap, double localx, double localy);
void findOffsets(LinkedList <PriorityVehicle> trackedVeh, geoCoord geoPoint, double & dN_Offset1, double & dE_Offset1, double & dN_Offset2, double & dE_Offset2);
void initializeRefPoint(MAP * newmap, geoCoord &geocoord);  
int  findMAPPositionInList(LinkedList<MAP> MapEntryList,long id);
bool isTheBusInRange(MAP map, int approach,double distance);
int  obtainOutLane(MAP map,int temp_approach, int temp_lane);
void updateActiveMapFile(LinkedList<PriorityVehicle> & priorityReq);
void initiateActiveMapFile();	
long whatIsTheLeavingIntersection(LinkedList <PriorityVehicle> &priorityReq);
long whatIsTheClosestApproachingIntersection(LinkedList <PriorityVehicle> &priorityReq);
void whatIsVehicleStatusWithRespectToMaps(LinkedList<MAP> MAPParsedList,LinkedList<MAPEntry> MapEntryList,LinkedList <PriorityVehicle> &trackedVeh);
void checkToSeeWeShouldSendOutRequest(LinkedList<PriorityVehicle> &sentPriorityReq, LinkedList<PriorityVehicle> acknowledgedPriorityReq);
bool shouldSendOutRequest(LinkedList<PriorityVehicle> &priorityReq,LinkedList<ReqEntry> &sentPriorityReq,LinkedList<ReqEntry> &acknowledgedPriorityReq, int iMapEntry);
void sendRequest(LinkedList<PriorityVehicle>  &priorityReqList, int i);
void writeVehInfoToFile(); // this function is added for the V2V usecase scnarios!!! Shayan has used this part to show the information of this vehicle to other vehicles that are nearby!!
int handleArguments(int argc, char * argv[]);

int main ( int argc, char* argv[] )
{	
	
	int iAddr_length = sizeof ( recvaddr );
	srm = (SRM_t *) calloc(1, sizeof * srm);
	srm->timeOfService=( DTime*)calloc(1,sizeof( DTime_t));
	srm->endOfService=( DTime*)calloc(1,sizeof(DTime_t));
	srm->transitStatus= (BIT_STRING_t*)calloc(1, sizeof(BIT_STRING_t));
	srm->vehicleVIN=( VehicleIdent_t*)calloc(1, sizeof(VehicleIdent_t));
	handleArguments(argc,argv);
	setup_logging();
	getBusStopsRange(cBusStopsFile);
	if (icodeUsage == SIMULATION) // If the lab version of the PRG is used. (incomplete SRM somes out of dll every 1 seconds)
	{	
		dSpeedChangeLmtForSRM=4.8;
		printf("About to get data from Vissim...............\n"); 
	}
	else if (icodeUsage == FIELD) // If the field version of the PRG is used.
	{
		dSpeedChangeLmtForSRM=8.1;
		get_veh_id(vehicleidFile);
		gps_init() ;
		printf("About to query gps data..................\n");
		if (iVehiclePriorityLevel==2 )	
			bIsItaTransit=true;
	}
	setupConnection();	
	while (true)
	{
		cout<<"Piority list size"<<priorityReqList.ListSize()<<endl;;
		// Start by getting gps position (in road version) or receiving incomplete SRM from Vissim (lab version)
		if (icodeUsage == FIELD) 
		{
			msleep(500); // 500 miliseconds sleep 
			read_gps();
			populate_gps_values();
			formBSMOfSRM(&vehIn,iVehicleId, iMsgCnt,  m_vehlat, m_vehlong, dHeading, m_vehspeed, dElevation);
			vehIn.VehicleToBSM(BSMblob);
			writeVehInfoToFile(); 
		}else if (icodeUsage == SIMULATION)
			if (getDataFromVissim(iAddr_length)==0)
				continue;
		LinkedList<MAPEntry>  MapEntryList;
		readMAPsFromFile(MAPsListFile,MapEntryList);
		handelReceivedMaps(MAPsParsedList,MapEntryList);
		cout<<"HERE"<<endl;
		storeVehicleTrajectory(priorityReqList);  
		cout<<"here out1"<<endl;
		whatIsVehicleStatusWithRespectToMaps(MAPsParsedList,MapEntryList,priorityReqList);
cout<<"here out2"<<endl;
		
	//	cout<<"Approaching  "<<priorityReqList.Data().lApproachingIntersectionID<<endl;
	//	cout<<"Leaving      "<<priorityReqList.Data().lLeavingIntersectionID<<endl;
		iNumberOfAcknowledgedRequests = reqListFromRxAcknowledgedRequestsFile(RxAcknowledgedRequestsFile,acknowledgedPriorityReq);  // Acknowledged requestes are located in psm.txt
		acknowledgedPriorityReq.Reset();
		priorityReqList.Reset();
		sentPriorityReqList.Reset();
		while (!priorityReqList.EndOfList())
		{ 
			int temp=priorityReqList.Data().iNoOfIntersectionsInRange;
			for (int i=0; i< temp; i++)
			{	
				if (shouldSendOutRequest(priorityReqList,sentPriorityReqList,acknowledgedPriorityReq,i) == 1)
				{
					fillSRM(srm, priorityReqList, i, BSMblob);
					sendRequest(priorityReqList,i);            // send srm to intersection ID: priorityReqList.Data().lRxIntersectionID[i]
					updateActiveMapFile(priorityReqList);      // update the ActiveMap.txt file to show which map is active and being sent the SRM	
					iMsgCnt=priorityReqList.Data().iMsgCnt;    // keep track of iMsgCnt for acknowledgement purpose
				}
			}
			priorityReqList.Next();	
		}
	}
	return 0;
} 

int handleArguments(int argc, char * argv[])
{
	int ret=0;
	// ---------- Managing the input arguments-------------
	while ((ret = getopt(argc, argv, "c:b:l:r:o:u:p:s:")) != -1)    // -b change broadcastaddress -l change logging option -r DSRC Range -o Out of The MAP detection time -p the port of SRM should be sent  default is 4444 -c code usage 
	{
		switch (ret) 
		{
	    case 'c':
			icodeUsage = atoi (optarg);
			if (icodeUsage==FIELD)
				printf("Application is testing in the FIELD \n");
			if (icodeUsage==SIMULATION)
				printf("Application is testing in the SIMULATION \n");
			break;
		case 'b':
			strcpy (cBroadcastAddress, optarg);
			printf ("BroadCast Address : %s\n", cBroadcastAddress);
			break;
		case 'l':
			iLogIndicator = atoi (optarg);
			printf ("Logging Option : %d\n", iLogIndicator);
			break;
		case 'r' :
			iDSRCRange = atoi(optarg);
			printf ("DSRC Range : %d\n", iDSRCRange);
			break;
		case 'u' :
			iRu_RlRange = atoi (optarg);
			printf ("Serving Rectangle Range : %d\n", iRu_RlRange);
			break;
		case 'o' :
			iOutOfMAPTimer = atoi (optarg);
			printf ("Out of MAP Timer : %d\n", iOutOfMAPTimer);
			break;
		case 'p' :
			iSRMPort = atoi (optarg);
			printf ("Port is : %d\n", iSRMPort);
			break;
		case 's' :
			iSpeedChngPrc = atoi (optarg);
			printf ("Speed Chaneg Limit Percentage for Resending the Request is : %f\n", iSpeedChngPrc);
			break;
		default:
			return 0;
		}
	}  
}
void writeVehInfoToFile()
{
	fstream fs_vehinfo;
	fs_vehinfo.open(vehicleinfoFile, ios::out);
	fs_vehinfo<<"1 ";   //There is Vehicle Info
	sprintf(temp_log,"%.8f %.8f %.3f %.3f %.2lf\n",m_vehlat,m_vehlong,dHeading,m_vehspeed,dgpsTime);
	fs_vehinfo<<temp_log;
	fs_vehinfo.close();
}
void sendRequest(LinkedList<PriorityVehicle>  &priorityReqList, int i) 
{
	der_encode_to_buffer(&asn_DEF_SRM, srm,SRMbuf,SRM_MSG_SIZE);
	for (int it=0;it<SRM_MSG_SIZE;it++) // put buffer from type (char *) to srmbuf which is a (uint8_t)
		buf[it]=SRMbuf[it];
	if ((sendto(sockfd, buf,SRM_MSG_SIZE, 0,(struct sockaddr *)&recvaddr, sizeof ( recvaddr ))>0) )
	{	
		sprintf(temp_log,"  REQUEST from Vehicle ID %ld successfully sent to IntersectionID %ld  at GPS time %.2lf\n",priorityReqList.Data().lVehID, priorityReqList.Data().lRxIntersectionID[i] , getSeconds()); 
		outputlog(temp_log); 
		cout<< temp_log<< endl;
	} 
}

bool getDataFromVissim(int addr_length)
{
	bool bTemp=0;
	memset(SRMbuf,0,SRM_MSG_SIZE);
	memset(srm,0, sizeof * srm);
	if (recvfrom(sockfd, SRMbuf, sizeof(SRMbuf), 0 , (struct sockaddr *)&sendaddr, (socklen_t *)&addr_length) <0 )
	{ 	
		printf("Receive Request failed\n");	
		bTemp=0;
	}
	else
		bTemp=1;
	rval = ber_decode(0, &asn_DEF_SRM,(void **)&srm, SRMbuf, sizeof(SRMbuf));	
	//xer_fprint(stdout, &asn_DEF_SRM, srm);
	if (rval.code==RC_OK)
	{	
		sprintf(temp_log,"\n SRM Recieved From VISSIM: Decode Success");
		outputlog(temp_log);
		cout<<temp_log<<endl; 
	}
	else
	{	
		sprintf(temp_log,"\n SRM Received From VISSIM: Decode Failure");
		outputlog(temp_log); 
		cout<<temp_log<<endl; 
	}
	for ( int i=0;i<38;i++) 
		BSMblob[i]=srm->vehicleData.buf[i];
	vehIn.BSMToVehicle(BSMblob);
	// To identify the vehicle type and also to check whether the vehicle in vissim is a transit vehicle or not
	bIsItaTransit=whatIsVISSIMType(srm->request.type.buf[0]);
	return bTemp;
}
bool shouldSendOutRequest(LinkedList<PriorityVehicle>  &  priorityReq, LinkedList<ReqEntry>  &  sentPriorityReq,  LinkedList<ReqEntry> &acknowledgedPriorityReq, int iMapEntry)
{
	//  sentPriorityReq is the last priority with similar VehID that was sent out! acknowledgedPriorityReq is the acknowledged priority request by rsu and it is obtained from psm.txt !
	//  We send out srm when: 1) Vehicle Status changes 2) Last sent srm is not being acknowleged yet
	long lVehID=priorityReq.Data().lVehID;
	ReqEntry req_temp;
	bool bTemp=0;
	bool bShouldSendOut=0;
	int iMsgCnt=0;
	int isentMsgDivisor=0;
	int iacknMsgDivisor=0;
	sentPriorityReq.Reset();  
	acknowledgedPriorityReq.Reset();
	//cout<<"VEHICLE ID"<<lVehID<<endl;
	if ( ( priorityReqList.Data().iVehStatus[iMapEntry] == APPROACHING ) || ( priorityReqList.Data().iVehStatus[iMapEntry] == INQUEUE ) )
	{
		if ( ( ( iNumberOfAcknowledgedRequests > 0 ) && ( thisVehIDPositionInList(lVehID,acknowledgedPriorityReq) < 0 ) )  
		       || ( iNumberOfAcknowledgedRequests<=0 ) )   // This Request is not in the Request list( psm.txt). Therefore we should send Priority Request fot the first time for this ID
		{ 
			
			if (thisVehIDPositionInList(lVehID,sentPriorityReqList)>=0)  //already sent to rsu but has not received the acknowledgement
			{
				sprintf(temp_log,"Vehicle ENTERED into the intersection map, sent out the first srm again b/c acknowledgment has not being received !!\n"); 
				outputlog(temp_log);
				cout<<temp_log;
				sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
				iMsgCnt=sentPriorityReqList.Data().iSendingMsgCnt+1;
				iMsgCnt=iMsgCnt%127;
				
			}else
			{
				sprintf(temp_log,"Vehicle ENTERED into the intersection map, send out the first srm\n"); 
				outputlog(temp_log);
				cout<<temp_log;
				iMsgCnt=0;
				req_temp.setRequestValues(priorityReq.Data().lVehID, 0.0, priorityReq.Data().dSpeed, iMsgCnt, priorityReq.Data().iInLane,priorityReq.Data().dDistanceToStopBar[iMapEntry]); 
				sentPriorityReq.InsertRear(req_temp);
			}
			priorityReq.Data().iMsgCnt=iMsgCnt;
			bShouldSendOut=1;
		}
		// This Request is already in the Request List (in the PSM), We will resend SRM when speed changes or when acknowledgment has not been received or when vehicle goes to lef turn bay!!
		else if ( ( thisVehIDPositionInList(lVehID,acknowledgedPriorityReq) >= 0 ) && ( thisVehIDPositionInList(lVehID,sentPriorityReq) >= 0 ) ) 
		{
			acknowledgedPriorityReq.Reset(thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)); 
			sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
			isentMsgDivisor= (int) sentPriorityReq.Data().iSendingMsgCnt /10; 
			iacknMsgDivisor= (int) acknowledgedPriorityReq.Data().iMsgCnt /10;
			if (isentMsgDivisor==iacknMsgDivisor) // The previous SRM already received acknowledgment
				iMsgCnt=(isentMsgDivisor+1)*10;
			else
				iMsgCnt=(sentPriorityReqList.Data().iSendingMsgCnt+1);
			iMsgCnt=iMsgCnt%127;
			if (iLogIndicator==3)
			{	
				cout<<"In shouldSendOutRequest function: "<<endl;		
				cout<<"Initial speed when entered the range "<< sentPriorityReqList.Data().dInitialSpeed<<endl;
				cout<<"Current speed  "<<priorityReq.Data().dSpeed<<endl;
				cout<<"Last sent speed  "<<sentPriorityReqList.Data().dSpeed<<endl;
				cout<<"Last sent Msg Count  "<< sentPriorityReqList.Data().iSendingMsgCnt<<endl;
				cout<<"Acknow Msg Count " <<acknowledgedPriorityReq.Data().iMsgCnt<<endl;
				cout<<"Sent iInLane "<<sentPriorityReqList.Data().iInLane<<endl;
				cout<<"Acknow iInLane " << acknowledgedPriorityReq.Data().iInLane<<endl;
				cout<<"Vehicle ID "<<priorityReq.Data().lVehID<<endl;
		    }	 
			
			bTemp=0;
			// send a new SRM if speed changes happen when driver slows down upto 1/2 of inistialspeed(vehicle speed when vehicle entered to the map  --> decelating flag =2
			if ( (priorityReq.Data().dSpeed>sentPriorityReq.Data().dInitialSpeed/2) && (priorityReq.Data().dSpeed >=dSpeedChangeLmtForSRM) && 
				 ( abs((sentPriorityReq.Data().dSpeed-priorityReq.Data().dSpeed)/sentPriorityReq.Data().dSpeed) > iSpeedChngPrc )   ) 
			{
				sprintf(temp_log,"Vehicle speed Changed, send out updated SRM\n"); 
				outputlog(temp_log);
				cout<<temp_log;
				bTemp=1;						
			}
			// send a new SRM when the vehicle join the queue
			else if ( (priorityReq.Data().dSpeed < dSpeedChangeLmtForSRM) && (sentPriorityReq.Data().dSpeed > dSpeedChangeLmtForSRM) )  
			{
				sprintf(temp_log,"Vehicle joins queue, send out updated SRM\n"); 
				outputlog(temp_log);
				cout<<temp_log;
				priorityReqList.Data().iVehStatus[iMapEntry] = INQUEUE; // the threshold for being in the queue should be tighter than the output of findPriorityVehicleInMap.  Therefore we changed the vehicls status here.  
				bTemp=1;							
			}
			// send a new SRM when vehicle start to accelerate from being in queue  or when vehicle moves 10 meter while its speed is less than dSpeedChangeLmtForSRM!
			else if ( ( (sentPriorityReq.Data().dSpeed<dSpeedChangeLmtForSRM) && (sentPriorityReq.Data().dDistanceToStpBar-priorityReq.Data().dDistanceToStopBar[iMapEntry] > 10) ) 
					|| ( (sentPriorityReq.Data().dSpeed<dSpeedChangeLmtForSRM) && ( priorityReq.Data().dSpeed > 4.8) ) )  
			{
				sprintf(temp_log,"Vehicle moves in the queue, send out updated SRM\n"); 
				outputlog(temp_log);
				cout<<temp_log;
				priorityReqList.Data().iVehStatus[iMapEntry] = INQUEUE; // the threshold for being in the queue should be tighter than the output of findPriorityVehicleInMap.  Therefore we changed the vehicls status here.  
				bTemp=1;	
			}
			else if (sentPriorityReq.Data().iInLane!=priorityReq.Data().iInLane) // vehicle changed ita lane, so it may enter to left turn bay, send new SRM
			{
				sprintf(temp_log,"Vehicle moves in to a new lane, send out updated SRM\n"); 
				outputlog(temp_log);
				cout<<temp_log;
				bTemp=1;	
			}
			else if  (isentMsgDivisor!=iacknMsgDivisor )      // we have not receivced back any acknowledgment 
			{
				sprintf(temp_log,"Vehicle is approaching but Acknowledgment has not received, send out updated SRM\n"); 
				outputlog(temp_log);
				cout<<temp_log;
				bTemp=1;	
			}	
			if (bTemp==1)
			{
				priorityReq.Data().iMsgCnt=iMsgCnt;
				bShouldSendOut=1;	
				sentPriorityReq.Data().iSendingTimeofReq=(int) getSeconds();
				sentPriorityReq.Data().dSpeed=priorityReq.Data().dSpeed;
				sentPriorityReq.Data().iSendingMsgCnt=iMsgCnt;
				sentPriorityReq.Data().iInLane=priorityReq.Data().iInLane;
				sentPriorityReq.Data().dDistanceToStpBar=priorityReq.Data().dDistanceToStopBar[iMapEntry];
			}
		}
	}
/*	else if  ( priorityReqList.Data().iVehStatus[iMapEntry] == LEAVING )
	{
		if ( (thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)>=0) && 
			(thisVehIDPositionInList(lVehID,sentPriorityReq)>=0) ) //acknowledgedPriorityReq is read from psm.txt file. The content of this file comes from rsu.
		{
			acknowledgedPriorityReq.Reset(thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)); 
			sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
			isentMsgDivisor= (int) sentPriorityReq.Data().iSendingMsgCnt /10;
			iacknMsgDivisor= (int) acknowledgedPriorityReq.Data().iMsgCnt /10;
			if (isentMsgDivisor==iacknMsgDivisor) // The previous SRM already received acknowledgment
				iMsgCnt=(isentMsgDivisor+1)*10;
			else
				iMsgCnt=isentMsgDivisor+1;
			sprintf(temp_log,"Vehicle is leaving, send out CANCEL SRM\n"); 
			outputlog(temp_log);
			cout<<temp_log;
			bShouldSendOut=1;
			priorityReq.Data().iMsgCnt=iMsgCnt%127;
			priorityReqList.Data().bCanceled=1;
			sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
			sentPriorityReq.DeleteAt();
			acknowledgedPriorityReq.Reset(thisVehIDPositionInList(lVehID,acknowledgedPriorityReq));
			acknowledgedPriorityReq.DeleteAt();
		}
		else if ((thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)<0) && 
			(thisVehIDPositionInList(lVehID,sentPriorityReq)>=0))
		{
			sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
			sentPriorityReq.DeleteAt();
		}
		else if((thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)>=0) && 
			(thisVehIDPositionInList(lVehID,sentPriorityReq)<0))
		{
			if (((int) priorityReqList.Data().iMsgCnt /10 ) == ((int) acknowledgedPriorityReq.Data().iMsgCnt /10) )// The previous SRM already received acknowledgment
				bShouldSendOut=0;
			else // cancel the request with the similar Divisor was not received in psm.txt 
			{
				bShouldSendOut=1;
				iMsgCnt=(priorityReqList.Data().iMsgCnt+1)*10;
				priorityReqList.Data().iMsgCnt=iMsgCnt%127;
				priorityReqList.Data().bCanceled=1;	
			}
			acknowledgedPriorityReq.Reset(thisVehIDPositionInList(lVehID,acknowledgedPriorityReq));
			acknowledgedPriorityReq.DeleteAt();
		}
	}
	else if ( (priorityReqList.Data().iVehStatus[iMapEntry] == NOTINMAP)  && ( ( thisVehIDPositionInList(lVehID,acknowledgedPriorityReq) >= 0 ) // The cancel srm was not received by rsu (prs component)
			|| ( thisVehIDPositionInList(lVehID,sentPriorityReq) >= 0 ) ) )
	{
		sprintf(temp_log,"Vehicle is out of map but the request is :  "); 
		outputlog(temp_log);
		cout<<temp_log;
		bShouldSendOut=0;		
		if (thisVehIDPositionInList(lVehID,sentPriorityReq)>=0)
		{
			sprintf(temp_log,"in the sentPriorityReq list, therefore delete it from the list)\n"); 
			outputlog(temp_log);
			cout<<temp_log;
			bShouldSendOut=0;
			sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
			sentPriorityReq.DeleteAt();
		}
		if (thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)>=0) 
		{
			sprintf(temp_log,"still appearing in psm.txt \n");
			outputlog(temp_log);
			cout<<temp_log;
			acknowledgedPriorityReq.Reset(thisVehIDPositionInList(lVehID,acknowledgedPriorityReq));
			if (acknowledgedPriorityReq.Data().iVehState!=NOTINMAP) 
			{
				// MZP should it be here?!!		if (((int) priorityReqList.Data().iMsgCnt /10 ) == ((int) acknowledgedPriorityReq.Data().iMsgCnt /10) )// The previous SRM already received acknowledgment
				//			bShouldSendOut=0;
				//		else // cancel request with the similar devisor was not received in psm.txt 				
				bShouldSendOut=1;
				iacknMsgDivisor=(int) acknowledgedPriorityReq.Data().iMsgCnt /10;
				iMsgCnt=(iacknMsgDivisor+1)*10;
				priorityReqList.Data().iMsgCnt=iMsgCnt%127;
				priorityReqList.Data().iVehStatus[iMapEntry]=NOTINMAP;
				priorityReqList.Data().bCanceled=1;	
				priorityReqList.Data().lLeavingIntersectionID=priorityReqList.Data().lRxIntersectionID[iMapEntry];
			}
		}
	}
	*/	 		
	else if  ( priorityReqList.Data().iVehStatus[iMapEntry] == LEAVING )
	{
		if ( (thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)>=0) && 
			(thisVehIDPositionInList(lVehID,sentPriorityReq)>=0) ) //acknowledgedPriorityReq is read from psm.txt file. The content of this file comes from rsu.
		{
			acknowledgedPriorityReq.Reset(thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)); 
			sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
			isentMsgDivisor= (int) sentPriorityReq.Data().iSendingMsgCnt /10;
			iacknMsgDivisor= (int) acknowledgedPriorityReq.Data().iMsgCnt /10;
			if (isentMsgDivisor==iacknMsgDivisor) // The previous SRM already received acknowledgment
				iMsgCnt=(isentMsgDivisor+1)*10;
			else
				iMsgCnt=isentMsgDivisor+1;
			sprintf(temp_log,"Vehicle is leaving, send out CANCEL SRM\n"); 
			outputlog(temp_log);
			cout<<temp_log;
			bShouldSendOut=1;
			priorityReq.Data().iMsgCnt=iMsgCnt%127;
			priorityReqList.Data().bCanceled=1;
	//		sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
	//		sentPriorityReq.DeleteAt();
	//		acknowledgedPriorityReq.Reset(thisVehIDPositionInList(lVehID,acknowledgedPriorityReq));
	//		acknowledgedPriorityReq.DeleteAt();
		}
		else if ((thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)<0) && 
			(thisVehIDPositionInList(lVehID,sentPriorityReq)>=0))
		{
			sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
			sentPriorityReq.DeleteAt();
			priorityReqList.DeleteAt();
		}
		else if((thisVehIDPositionInList(lVehID,acknowledgedPriorityReq)>=0) && 
			(thisVehIDPositionInList(lVehID,sentPriorityReq)<0))
		{
			if ( (acknowledgedPriorityReq.Data().iVehState != INQUEUE) || (acknowledgedPriorityReq.Data().iVehState != APPROACHING) ) // The previous SRM already received acknowledgment
				bShouldSendOut=0;
			else // cancel the request with the similar Divisor was not received in psm.txt 
			{
				bShouldSendOut=1;
				iMsgCnt=(priorityReqList.Data().iMsgCnt+1)*10;
				priorityReqList.Data().iMsgCnt=iMsgCnt%127;
				priorityReqList.Data().bCanceled=1;	
				req_temp.setRequestValues(priorityReq.Data().lVehID, 0.0, 0.0, priorityReqList.Data().iMsgCnt, 0, 0.0); 
				sentPriorityReq.InsertRear(req_temp);
			}
		}
	}
	else if ( (priorityReqList.Data().iVehStatus[iMapEntry] == NOTINMAP)  && ( ( thisVehIDPositionInList(lVehID,acknowledgedPriorityReq) >= 0 ) // The cancel srm was not received by rsu (prs component)
			|| ( thisVehIDPositionInList(lVehID,sentPriorityReq) >= 0 ) ) )
	{
		sprintf(temp_log,"Vehicle is out of map but the request is"); 
		outputlog(temp_log);
		cout<<temp_log;
		bShouldSendOut=0;		
		if ( (thisVehIDPositionInList(lVehID,sentPriorityReq)>=0) && (thisVehIDPositionInList(lVehID,acknowledgedPriorityReq) <0 ) )
		{
			sprintf(temp_log," in the sentPriorityReq list, therefore delete it from the sentPriorityReq and priorityReqList list)\n"); 
			outputlog(temp_log);
			cout<<temp_log;
			bShouldSendOut=0;
			sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
			sentPriorityReq.DeleteAt();
			priorityReqList.DeleteAt();
		}
		if ( (thisVehIDPositionInList(lVehID,sentPriorityReq)<0) && (thisVehIDPositionInList(lVehID,acknowledgedPriorityReq) >=0 ) )
		{
			sprintf(temp_log," still appearing in psm.txt \n");
			outputlog(temp_log);
			cout<<temp_log;
			acknowledgedPriorityReq.Reset(thisVehIDPositionInList(lVehID,acknowledgedPriorityReq));
		//	if (acknowledgedPriorityReq.Data().iVehState!=NOTINMAP) 
			//{
				// MZP should it be here?!!		if (((int) priorityReqList.Data().iMsgCnt /10 ) == ((int) acknowledgedPriorityReq.Data().iMsgCnt /10) )// The previous SRM already received acknowledgment
				//			bShouldSendOut=0;
				//		else // cancel request with the similar devisor was not received in psm.txt 				
				bShouldSendOut=1;
				iacknMsgDivisor=(int) acknowledgedPriorityReq.Data().iMsgCnt /10;
				iMsgCnt=(iacknMsgDivisor+1)*10;
				priorityReqList.Data().iMsgCnt=iMsgCnt%127;
				priorityReqList.Data().iVehStatus[iMapEntry]=NOTINMAP;
				priorityReqList.Data().bCanceled=1;	
				priorityReqList.Data().lLeavingIntersectionID=priorityReqList.Data().lRxIntersectionID[iMapEntry];
				req_temp.setRequestValues(priorityReq.Data().lVehID, 0.0, 0.0, priorityReqList.Data().iMsgCnt, 0, 0.0); 
				sentPriorityReq.InsertRear(req_temp);
			//}
		}
		if ( (thisVehIDPositionInList(lVehID,sentPriorityReq)>=0) && (thisVehIDPositionInList(lVehID,acknowledgedPriorityReq) >=0 ) )
		{
			sprintf(temp_log," still appearing in psm.txt and in sentPriorityReq list  \n");
			outputlog(temp_log);
			cout<<temp_log;
			if ( (acknowledgedPriorityReq.Data().iVehState != INQUEUE) || (acknowledgedPriorityReq.Data().iVehState != APPROACHING) ) // The previous SRM already received acknowledgment
			{	
				sentPriorityReq.Reset(thisVehIDPositionInList(lVehID,sentPriorityReq));
				sentPriorityReq.DeleteAt();
				priorityReqList.DeleteAt();
				bShouldSendOut=0;
			}	
			else // cancel the request with the similar Divisor was not received in psm.txt 
			{
				bShouldSendOut=1;
				iacknMsgDivisor=(int) acknowledgedPriorityReq.Data().iMsgCnt /10;
				iMsgCnt=(iacknMsgDivisor+1)*10;
				priorityReqList.Data().iMsgCnt=iMsgCnt%127;
				priorityReqList.Data().iVehStatus[iMapEntry]=NOTINMAP;
				priorityReqList.Data().bCanceled=1;	
				priorityReqList.Data().lLeavingIntersectionID=priorityReqList.Data().lRxIntersectionID[iMapEntry];
			}
		}
	}	 
	return bShouldSendOut;
}
			
				

void initiateActiveMapFile()
{
	fstream fsActiveMAP;
	fsActiveMAP.open(ActiveMAPFile, ios::out | ios:: trunc);
	fsActiveMAP.close();	
}

void updateActiveMapFile(LinkedList<PriorityVehicle>  &priorityReq)   // update the ActiveMap.txt file to show which map is active and being sent the SRM	
{
	fstream fsActiveMAP;
	fsActiveMAP.open(ActiveMAPFile, ios::out );
	if (priorityReq.Data().lLeavingIntersectionID>0)
	{
		fsActiveMAP << LEAVE <<" " << priorityReq.Data().lLeavingIntersectionID << " "<<priorityReq.Data().iMsgCnt<<endl;
		fsActiveMAP.close();	
	}
	else if (priorityReq.Data().lApproachingIntersectionID>0)
	{
		fsActiveMAP << ACTIVE <<" " << priorityReq.Data().lApproachingIntersectionID << " "<<priorityReq.Data().iMsgCnt<<endl;
		fsActiveMAP.close();	
	}
	else 
	{
		fsActiveMAP << NOT_ACTIVE <<" " << priorityReq.Data().lApproachingIntersectionID << " "<<priorityReq.Data().iMsgCnt<<endl;
		fsActiveMAP.close();	
	}
}
void handelReceivedMaps(LinkedList<MAP> & MAPParsedList,LinkedList<MAPEntry>  MapEntryList)
{
	MapEntryList.Reset();
	while (!MapEntryList.EndOfList())
	{
		cout<<"Map size "<<MapEntryList.ListSize()<<endl;
		MAP NewMap;
		geoCoord geoPoint;
		long lMAPID=0;
		lMAPID=MapEntryList.Data().ID;
		int iMAPPositionInList=0;
		cout<<"lMAPID"<<endl;
		cout<<lMAPID<<endl;
		iMAPPositionInList=findMAPPositionInList(MAPParsedList,lMAPID);
		//sprintf(temp_log,"Stored MAP ID %ld\n", lMAPID ); 
		//outputlog(temp_log);
		//cout<<temp_log;
		if(iMAPPositionInList>=0) // we already have the map in our map list!
		{ 
			MAPsParsedList.Reset(iMAPPositionInList);
			NewMap=MAPParsedList.Data();
			initializeRefPoint(&NewMap,geoPoint);
			populateMAPNodes(&NewMap,&geoPoint);
			
		}
		else  // we do not have the map in our map list, we should read it for the first time!
		{
			createMapFile(lMAPID);
			NewMap.ParseIntersection(predir_temp); //  File up the file: /nojournal/bin/Intersection_MAP_XXX.nmap
			initializeRefPoint(&NewMap,geoPoint);
			populateMAPNodes(&NewMap,&geoPoint);
			MAPParsedList.InsertRear(NewMap);		
			sprintf(temp_log,"MAP id  :  %d  Added succesfully\n", MAPParsedList.Data().intersection.ID ); 
			outputlog(temp_log); 
			cout<<temp_log;
		}
		MapEntryList.Next();
	}
}

void whatIsVehicleStatusWithRespectToMaps(LinkedList<MAP> MAPParsedList, LinkedList<MAPEntry>  MapEntryList, LinkedList <PriorityVehicle> & priorityReq)
{
	int iVehStatus=0;
	int iApproach=0;
	int iLane=0;
	double dQueueClearTime=0.0;
	double dDistanceToStopBar=0.0;
	double dETAtoStopBar=0.0;
	double dN_Offset1=0.0;
	double dE_Offset1=0.0;
	double dN_Offset2=0.0;
	double dE_Offset2=0.0;
	double distanceToLine=0.0;
	long lMAPID=0;
	int  iMAPPositionInList=0;
	priorityReq.Reset();
	while (!priorityReq.EndOfList())
	{
		double dTmpSpeed=priorityReq.Data().dSpeed;
		double dTmpHeading=priorityReq.Data().heading;
		int iTmpFrame=priorityReq.Data().nFrame;
		priorityReq.Data().iNoOfIntersectionsInRange=MapEntryList.ListSize();  
		int iMapNumber=0;
		MapEntryList.Reset();
		while (!MapEntryList.EndOfList())
		{
			MAP NewMap;
			geoCoord geoPoint;
			lMAPID=MapEntryList.Data().ID;
			iMAPPositionInList=findMAPPositionInList(MAPsParsedList,lMAPID);
			MAPsParsedList.Reset(iMAPPositionInList);
			NewMap=MAPsParsedList.Data();
			initializeRefPoint(&NewMap,geoPoint);
			priorityReq.Data().lRxIntersectionID[iMapNumber]=lMAPID;
			findOffsets(priorityReq, geoPoint, dN_Offset1, dE_Offset1, dN_Offset2, dE_Offset2);
			distanceToLine=geoFencing(NewMap,dN_Offset2,dE_Offset2);
			// ---- If the vehicle is too far way from the intersection, 
			// or the vehicle is out of geoo fencing (8 meters outside),directly consider as not on the map		
			if(fabs(dN_Offset1)>iDSRCRange || fabs(dE_Offset1)>iDSRCRange || distanceToLine > GEOFENCE_LIMIT)  
			{
				sprintf(temp_log,"Vehicle ID %ld is not on the map of intersection ID %ld\n ", priorityReq.Data().lVehID, lMAPID);
				outputlog(temp_log); 
				cout<<temp_log;
				priorityReq.Data().bIntersectionInRange[iMapNumber]=0;
				priorityReq.Data().iVehStatus[iMapNumber]=NOTINMAP;
				priorityReq.Data().lLeavingIntersectionID=0;
				priorityReq.Data().lApproachingIntersectionID=0;
			}	
			else
			{
				findPriorityVehicleInMap(dTmpSpeed, dTmpHeading, iTmpFrame, dN_Offset1, dE_Offset1, dN_Offset2, dE_Offset2, NewMap,
							 dDistanceToStopBar,dETAtoStopBar,iVehStatus,iApproach,iLane,dQueueClearTime);
				if (iLogIndicator==3)
				{			
					cout<< "findPriorityVehicleInMap function output:"<<endl;	
					cout<< "dDistanceToStopBar"<<dDistanceToStopBar<<endl;
					cout<< "dETAtoStopBar"<<dETAtoStopBar<<endl;
					cout<< "iVehStatus"<<iVehStatus<<endl;
					cout<< "iApproach"<<iApproach<<endl;
					cout<< "iLane"<<iLane<<endl;
				}	    
				priorityReq.Data().bIntersectionInRange[iMapNumber]=1;
				priorityReq.Data().dDistanceToStopBar[iMapNumber]=dDistanceToStopBar;
				priorityReq.Data().dETAtoStopBar[iMapNumber]=dETAtoStopBar;
				priorityReq.Data().iIntersectionApproach[iMapNumber]=iApproach;
				priorityReq.Data().iIntersectionLane[iMapNumber]=iLane;
				
					
				if ( (iVehStatus==APPROACHING)  || (iVehStatus==INQUEUE) )
				{
					priorityReq.Data().iOutLane=obtainOutLane(NewMap,iApproach,iLane);
					// ---- If the vehicle is a transit, we should consider the bus stop location
					// to the next intersection as the range ( this range comes from busStopsRange.txt file)
					if (bIsItaTransit==1)
					{
						bBusIsInRange=isTheBusInRange(NewMap,iApproach,dDistanceToStopBar);
						if  (bBusIsInRange==0)
						{
							sprintf(temp_log,"This transit vehicle is not in the range! Check the busStopRange.txt file. The received intersection map ID does not exist in busSopRange.txt file  !!!! \n ");
							outputlog(temp_log); 
							cout<<temp_log;
							iVehStatus=NOTINMAP;
							priorityReq.Data().bIntersectionInRange[iMapNumber]=0;
						}
					}
				}			
				priorityReq.Data().iVehStatus[iMapNumber]=iVehStatus;		
				priorityReq.Data().iInLane=iApproach*10+iLane;
				priorityReq.Data().dQueueClearTime[iMapNumber]=dQueueClearTime;
			}
			iMapNumber++;
			MapEntryList.Next();
		}  // while MapEntryList 	
		priorityReq.Data().lApproachingIntersectionID=whatIsTheClosestApproachingIntersection(priorityReq);
		priorityReq.Data().lLeavingIntersectionID=whatIsTheLeavingIntersection(priorityReq);
		if (priorityReq.Data().lLeavingIntersectionID>0)
		{
			priorityReq.Data().iOutLane=0;
			priorityReq.Data().bCanceled=1;
		}
		else
			priorityReq.Data().bCanceled=0;
		priorityReq.Next();
	}
}


int obtainOutLane(MAP map,int temp_approach, int temp_lane)
{
	int iOutLane=0;
	int iOutLaneApproach=0;
	char tmp1[16];
	sprintf(tmp1,"%d.%d",temp_approach,temp_lane);
	string tmp_string(tmp1);
	for( int i=0; i<map.intersection.Approaches[temp_approach-1].Lane_No;i++)
	{
		if (tmp_string.compare(map.intersection.Approaches[temp_approach-1].Lanes[i].Lane_Name)==0)
		{
			iOutLaneApproach=map.intersection.Approaches[temp_approach-1].Lanes[i].Connections[0].ConnectedLaneName.Approach;
			iOutLane=iOutLaneApproach*10+ map.intersection.Approaches[temp_approach-1].Lanes[i].Connections[0].ConnectedLaneName.Lane	;
		}
	}
	if (iOutLane>1000)  //// check it!!! MZP why is it neccesary ???
		iOutLane=0;
	return iOutLane;
}



bool isTheBusInRange(MAP map, int approach,double distance)
{
	bool bBusInRange=0;
	if ( bIsItaTransit==1)
	{
		for (int k=0 ; k<iNoOfIntersectionInBusRoute; k++)
		{
			if ( (iBusStopsMAPID[k]==map.intersection.ID)&&(distance<=iBusStopsApprRange[k][approach]))
				bBusInRange=1;
		}
	}
	return bBusInRange;
}
long whatIsTheClosestApproachingIntersection(LinkedList <PriorityVehicle> & priorityReq)
{
	long lTempIntID=0;
	double dClosestIntersectionDist=1000.0;
	for (int i=0;i<priorityReq.Data().iNoOfIntersectionsInRange;i++)
	{
		if (priorityReq.Data().bIntersectionInRange[i]==1)
		{
			if ( ((priorityReq.Data().iVehStatus[i]==APPROACHING)  || ( priorityReq.Data().iVehStatus[i]==INQUEUE ))
					&&  ( ((bIsItaTransit==1) && (bBusIsInRange==1)) || (bIsItaTransit==0) ) ) 
				if (priorityReq.Data().dDistanceToStopBar[i]<=dClosestIntersectionDist)
				{
					dClosestIntersectionDist=priorityReq.Data().dDistanceToStopBar[i];
					lTempIntID=priorityReq.Data().lRxIntersectionID[i];
				}
		}	
	}	
	return lTempIntID;
}

long whatIsTheLeavingIntersection(LinkedList <PriorityVehicle> &priorityReq)
{
	long lTempIntID=0;
	for (int i=0;i<priorityReq.Data().iNoOfIntersectionsInRange;i++)
		if (priorityReq.Data().bIntersectionInRange[i]==1)
			if (priorityReq.Data().iVehStatus[i]==LEAVING)
				lTempIntID=priorityReq.Data().lRxIntersectionID[i];
	return lTempIntID;
}


void findOffsets(LinkedList <PriorityVehicle> priorityReq, geoCoord geopoint, double & dN_Offset1, double & dE_Offset1, double & dN_Offset2, double & dE_Offset2)
{
	double dLocalX=0.0;
	double dLocalY=0.0;
	double dLocalZ=0.0;
	double decef_X=0.0;
	double decef_Y=0.0;
	double decef_Z=0.0;
	int  inFrame=priorityReq.Data().nFrame;
	
	geopoint.lla2ecef(priorityReq.Data().traj[inFrame-1].longitude,priorityReq.Data().traj[inFrame-1].latitude,0.0,&decef_X,&decef_Y,&decef_Z);
	geopoint.ecef2local(decef_X,decef_Y,decef_Z,&dLocalX,&dLocalY,&dLocalZ);					
	dN_Offset1=dLocalX;
	dE_Offset1=dLocalY;
	geopoint.lla2ecef(priorityReq.Data().traj[inFrame-2].longitude,priorityReq.Data().traj[inFrame-2].latitude,0.0,&decef_X,&decef_Y,&decef_Z);
	geopoint.ecef2local(decef_X,decef_Y,decef_Z,&dLocalX,&dLocalY,&dLocalZ);					
	dN_Offset2=dLocalX;
	dE_Offset2=dLocalY;
}
int whatIsVISSIMType(int type)
{
	int iIsItATransitVeh=0;
	if (type==4)// which the category of Tram in VISSIM     
		iVehiclePriorityLevel=1;   // in priority applications, priority level of EV is 1
	else if (type==3)  // which the category of bus in VISSIM
		iVehiclePriorityLevel=2;   // in priority applications, priority level of bus is 2
	else if (type==2)  // which the category of truck in VISSIM
		iVehiclePriorityLevel=3;    // in priority applications, priority level of truck is 3
	else if (type==1)  // which the category of car in VISSIM
		iVehiclePriorityLevel=4;    // in priority applications, priority level of regular car is 4
	else
		iVehiclePriorityLevel=0;
	if (iVehiclePriorityLevel==2) // a transit
		iIsItATransitVeh=1;  
	return iIsItATransitVeh;
}
double  geoFencing( MAP NewMap, double local_x, double local_y)
{
	//Find the Nearest Lane info from the current vehicle location
	int N_Node, N_App, N_Lane; //Nearest Node, approach and lane
	int N_pos; //node position in the vector
	double Tempdis = 10000000.0;
	double distanceSegment, distanceToGeoLine;
		//find the nearest lane node
	for(unsigned int s = 0; s < NewMap.MAP_Nodes.size(); s++)
	{
		double Dis = sqrt(pow((local_x - NewMap.MAP_Nodes[s].N_Offset),2) + pow((local_y - NewMap.MAP_Nodes[s].E_Offset),2));
		if (Dis<Tempdis)
		{
			Tempdis = Dis;
			N_App = NewMap.MAP_Nodes[s].index.Approach;
			N_Lane = NewMap.MAP_Nodes[s].index.Lane;
			N_Node = NewMap.MAP_Nodes[s].index.Node;
			N_pos = s;
		}
	}
	if(Tempdis<1000000.0)  // Find a valid node
	{
		if(N_Node == 1) //Case when the nearest node is the first node of the lane: The line bw MAP_Nodes[s] and MAP_Nodes[s+1]
		{
			distanceFromLine(local_x, local_y, NewMap.MAP_Nodes[N_pos].N_Offset, NewMap.MAP_Nodes[N_pos].E_Offset , NewMap.MAP_Nodes[N_pos+1].N_Offset, NewMap.MAP_Nodes[N_pos+1].E_Offset, distanceSegment, distanceToGeoLine);
			//cout<<"Two points are: "<<MAP_Nodes[N_pos].index.Approach<<"."<<MAP_Nodes[N_pos].index.Lane<<"."<<MAP_Nodes[N_pos].index.Node<<" and "<<MAP_Nodes[N_pos+1].index.Approach<<"."<<MAP_Nodes[N_pos+1].index.Lane<<"."<<MAP_Nodes[N_pos+1].index.Node<<endl; 
		}
		else
		{
			distanceFromLine(local_x, local_y, NewMap.MAP_Nodes[N_pos].N_Offset, NewMap.MAP_Nodes[N_pos].E_Offset , NewMap.MAP_Nodes[N_pos-1].N_Offset, NewMap.MAP_Nodes[N_pos-1].E_Offset, distanceSegment, distanceToGeoLine);
			//cout<<"Two points are: "<<MAP_Nodes[N_pos].index.Approach<<"."<<MAP_Nodes[N_pos].index.Lane<<"."<<MAP_Nodes[N_pos].index.Node<<" and "<<MAP_Nodes[N_pos-1].index.Approach<<"."<<MAP_Nodes[N_pos-1].index.Lane<<"."<<MAP_Nodes[N_pos-1].index.Node<<endl; 
		}
	}
	return distanceToGeoLine;
}


//To find the Shortest Distance between the vehicle location and two nearest lane nodes ----> GeoFencing
//Shayan Added 10.3.14  
void distanceFromLine(double cx, double cy, double ax, double ay ,double bx, double by, double &distanceSegment, double &distanceLine)
{
	// find the distance from the point (cx,cy) to the line
	// determined by the points (ax,ay) and (bx,by)
	//
	// distanceSegment = distance from the point to the line segment
	// distanceLine = distance from the point to the line (assuming
	//					infinite extent in both directions

	double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
	double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
	double r = r_numerator / r_denomenator;
    double s =  ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / r_denomenator;
	distanceLine = fabs(s)*sqrt(r_denomenator);
}
void createMapFile(long mapID)
{
	strcpy(predir_temp,"");
	strcpy(predir_temp,predir);
	char cMAPfile[128]      = "Intersection_MAP_";
	strcat(predir_temp,cMAPfile);
	char charid[16];
	sprintf(charid,"%ld",mapID);
	strcat(predir_temp,charid);
	strcat(predir_temp,".nmap");
}
void populateMAPNodes(MAP * NewMap, geoCoord *geoPoint)
{
	NewMap->MAP_Nodes.clear();
	for (unsigned int i=0;i<NewMap->intersection.Approaches.size();i++)
		for(unsigned int j=0;j<NewMap->intersection.Approaches[i].Lanes.size();j++)
			for(unsigned k=0;k<NewMap->intersection.Approaches[i].Lanes[j].Nodes.size();k++)
			{	
				LaneNodes temp_node;
				temp_node.index.Approach=NewMap->intersection.Approaches[i].Lanes[j].Nodes[k].index.Approach;
				temp_node.index.Lane=NewMap->intersection.Approaches[i].Lanes[j].Nodes[k].index.Lane;
				temp_node.index.Node=NewMap->intersection.Approaches[i].Lanes[j].Nodes[k].index.Node;
				temp_node.Latitude=NewMap->intersection.Approaches[i].Lanes[j].Nodes[k].Latitude;
				temp_node.Longitude=NewMap->intersection.Approaches[i].Lanes[j].Nodes[k].Longitude;
				geoPoint->lla2ecef(temp_node.Longitude,temp_node.Latitude,0.0 ,&ecef_x,&ecef_y,&ecef_z);
				geoPoint->ecef2local(ecef_x,ecef_y,ecef_z,&local_x,&local_y,&local_z);
				temp_node.N_Offset=local_x;
				temp_node.E_Offset=local_y;						
				NewMap->MAP_Nodes.push_back(temp_node);
			}
	int tmp_pos=0;
	for (unsigned int i=0;i<NewMap->intersection.Approaches.size();i++)
		for(unsigned int j=0;j<NewMap->intersection.Approaches[i].Lanes.size();j++)
			for(unsigned k=0;k<NewMap->intersection.Approaches[i].Lanes[j].Nodes.size();k++)
			{
				
				if(NewMap->MAP_Nodes[tmp_pos].index.Approach%2==1)  //odd approaches, approching lanes
				{
					if(k<NewMap->intersection.Approaches[i].Lanes[j].Nodes.size()-1)
					{
						NewMap->MAP_Nodes[tmp_pos].Heading=atan2(NewMap->MAP_Nodes[tmp_pos].N_Offset-NewMap->MAP_Nodes[tmp_pos+1].N_Offset,NewMap->MAP_Nodes[tmp_pos].E_Offset-NewMap->MAP_Nodes[tmp_pos+1].E_Offset)*180.0/PI;
						NewMap->MAP_Nodes[tmp_pos].Heading=90.0-NewMap->MAP_Nodes[tmp_pos].Heading;
					}
					else
						NewMap->MAP_Nodes[tmp_pos].Heading=NewMap->MAP_Nodes[tmp_pos-1].Heading;
				}
				if(NewMap->MAP_Nodes[tmp_pos].index.Approach%2==0)  //even approaches, leaving lanes
				{
					if(k<NewMap->intersection.Approaches[i].Lanes[j].Nodes.size()-1)
					{
						NewMap->MAP_Nodes[tmp_pos].Heading=atan2(NewMap->MAP_Nodes[tmp_pos+1].N_Offset-NewMap->MAP_Nodes[tmp_pos].N_Offset,NewMap->MAP_Nodes[tmp_pos+1].E_Offset-NewMap->MAP_Nodes[tmp_pos].E_Offset)*180.0/PI;
						NewMap->MAP_Nodes[tmp_pos].Heading=90.0-NewMap->MAP_Nodes[tmp_pos].Heading;
					}
					else
						NewMap->MAP_Nodes[tmp_pos].Heading=NewMap->MAP_Nodes[tmp_pos-1].Heading;	
				}									
				if(NewMap->MAP_Nodes[tmp_pos].Heading<0)
				     NewMap->MAP_Nodes[tmp_pos].Heading+=360;
				tmp_pos++;								
			}
}

// This function is used when lab version of the code is applied
void fillSRM(SRM_t * srm_t,LinkedList <PriorityVehicle> &priorityRequest, int i,  char * bsmBlob)
{
	int iMsgCnt,iInLane,iOutLane,iETA,vehState,vehtype,ret;
	long lintID=0;
	bool bIsCanceled;
	double dMinGr; 
	if (priorityRequest.Data().iVehStatus[i] == LEAVING)
		lintID=priorityRequest.Data().lLeavingIntersectionID;
	else if ((priorityRequest.Data().iVehStatus[i] == APPROACHING) || (priorityRequest.Data().iVehStatus[i] == INQUEUE) )
		lintID=priorityRequest.Data().lApproachingIntersectionID;
	else if ((priorityRequest.Data().iVehStatus[i] == NOTINMAP) && (priorityRequest.Data().bCanceled==1)) // in case approacing vehicle suddenlly gets out of the map
		lintID=priorityRequest.Data().lLeavingIntersectionID;
		
	vehState=priorityRequest.Data().iVehStatus[i];
	iMsgCnt=priorityRequest.Data().iMsgCnt;
	bIsCanceled=priorityRequest.Data().bCanceled;
	iInLane=priorityRequest.Data().iInLane;
	iOutLane=priorityRequest.Data().iOutLane;
	iETA=(int) priorityRequest.Data().dETAtoStopBar[i];
	dMinGr=priorityRequest.Data().dQueueClearTime[i];
	if (dMinGr>0) // Previously, we were sending the qclearancetime in a separate data element in SRM. There is no such space for qclerancetime in SRM. Now, we embed it in iETA using this logic: if the vehState is INQUEUE (meaning dMinGr>0), then the iETA gets the dMinGr value. 
		iETA=(int) dMinGr;
		
	vehtype=priorityRequest.Data().iVehicleType;	
	time_t theTime = time(NULL);
	struct tm *aTime = localtime(&theTime);
	srm_t->msgID=14;
	srm_t->msgCnt=iMsgCnt;
	srm_t->request.inLane=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char *)(&iInLane),1);
	srm_t->request.outLane=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char *)(&iOutLane),1);
	srm_t->request.isCancel=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char *)(&bIsCanceled),1);
	ret = OCTET_STRING_fromBuf(&srm_t->vehicleData,bsmBlob,38);
	uint16_t tIntersectionId = (uint16_t)lintID;
	ret = OCTET_STRING_fromBuf(&(srm->request.id), (char *)(&tIntersectionId), 2);
// MZP  	ret=OCTET_STRING_fromBuf(&srm_t->request.id,(char *)(&lintID),-1);
	int reAction=209;
	srm_t->request.requestedAction =OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)(&reAction ),-1);
	// In VISSIM different class!! of vehicles are diffrentiated by their category!. So in DrivelModel.dll , DRIVER_DATA_VEH_CATEGORY gets different values for different class of vehicles!!  car = 1, truck = 2, bus = 3, tram = 4, pedestrian = 5, bike = 6 .
	// When we pack the SRM in dll, we put these numbers in  vehicleclass_level  field of SRM. 
	// Here in PRG, we put the srm->vehicleclass_level into srm->vehicle_type
	// Since we do not have any EV in dll, we consider trams as EV!! 
	
	uint8_t tVehType=(uint8_t) vehtype;
	ret=OCTET_STRING_fromBuf(&srm_t->request.type,(char *)(&tVehType),1);
	char* icodeword="MMITSS password";
	srm_t->request.codeWord=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,icodeword,-1);
	// timeOfService
	srm_t->timeOfService->hour=aTime->tm_hour;  //
	srm_t->timeOfService->minute=aTime->tm_min;
	srm_t->timeOfService->second=aTime->tm_sec;
	int iHour=0;
	int iMin=0;
	int iSec=0;
	int itempMinGrn=(int) dMinGr;
	obtainEndofService( aTime->tm_hour,aTime->tm_min,aTime->tm_sec,iETA,iHour, iMin,iSec,itempMinGrn,iRu_RlRange);
	if (bIsCanceled==0)
	{
		srm_t->endOfService->hour=iHour;
		srm_t->endOfService->minute=iMin;
		srm_t->endOfService->second=iSec;
	}else if (bIsCanceled==1)
	{ 	
		srm_t->timeOfService->hour=0;  //
		srm_t->timeOfService->minute=0;
		srm_t->timeOfService->second=0;
		srm_t->endOfService->hour=0;
		srm_t->endOfService->minute=0;
		srm_t->endOfService->second=0;
	}
	uint8_t tVehStatus=(uint8_t) vehState;
	srm_t->status= OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*) (&tVehStatus),1); 
	long lTemp=12345;
	srm_t->vehicleVIN->id= OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&lTemp,3);
	// transit Status
	srm_t->transitStatus->buf=(uint8_t *)calloc(1,1);  
	assert(srm->transitStatus->buf);
	srm_t->transitStatus->size = 1;
	srm_t->transitStatus->buf[0] |=1<<0;//<<  (7 - TransitStatus_anADAuse);  just filled it up !
	srm_t->transitStatus->bits_unused = 2;	
	// Vehicle Ident
	char* ivin="91769fjut";
 	srm_t->vehicleVIN->name= OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,cVehicleName,-1);
	srm_t->vehicleVIN->vin= OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,ivin,-1);
}


void setup_logging()
{
	//------Init log file name with Time stamp------------------
	char ctimestamp[128];
	xTimeStamp(ctimestamp);  // This function create a log file that has time stamp at the end 
	strcat(logfilename,ctimestamp);
	strcat(logfilename,".log");
	cout<<"logfilename is: "<<logfilename<<endl;
	
    fstream ftemp;
    ftemp.open(logfilename,fstream::out);
    if (!ftemp.good())
    {
        perror("Open logfilename failed!"); exit(1);
    }
    else
    {
        ftemp<<"Start Recording PRG at time:\t"<<time(NULL)<<endl;
        ftemp.close();
    }
}
int outputlog(char *output)
{
	FILE * stream = fopen( logfilename, "r" );
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
	return 1;
}




void get_veh_id(char filename[]) 
{
	string lineread;
	char temp[256]; 
	
	ifstream load;
	load.open(filename);
	if (load.good())
	{
		getline(load,lineread);
		sscanf(lineread.c_str(),"%s",temp);
		iVehicleId=atoi(temp);
		sscanf(lineread.c_str(),"%*s %s",temp);
		iVehiclePriorityLevel=atoi(temp);
		sscanf(lineread.c_str(),"%*s %*s %s",temp);
		strcpy(cVehicleName,temp);	
		cout<<"Read vehicleid.txt successfully."<<endl;
		cout<<"Vehicle ID: " << iVehicleId<<endl;
		if (iVehiclePriorityLevel==1)
		{ 
			sprintf(temp_log,"Priority level: Emergency Vehicle");	
			outputlog(temp_log); 
			cout<< temp_log <<endl;
		}
		if (iVehiclePriorityLevel==2)
		{ 
			sprintf(temp_log,"Priority level: TRANSIT");	
			outputlog(temp_log); 
			cout<< temp_log <<endl;
		}
		if (iVehiclePriorityLevel==3)
		{ 
			sprintf(temp_log,"Priority level: TRUCK");	
			outputlog(temp_log); 
			cout<< temp_log <<endl;
		}
		cout<<"Vehicle Name: " << cVehicleName<<endl;
	}
	else
	{
		perror("cannot open vehicleid.txt file.");
		sprintf(temp_log,"cannot open vehicleid.txt file.");
		outputlog(temp_log);
		cout<<temp_log<<endl;
	}
	load.close();
}

void setupConnection()
{
	int broadcast=1;
	if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd");
		exit(1);
	}
	if((setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,
					&broadcast,sizeof broadcast)) == -1)
	{
		perror("setsockopt - SO_SOCKET ");
		exit(1);
	}
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = htons(30000);  //*** IMPORTANT: the vissim should also have this port. ***//
	sendaddr.sin_addr.s_addr =  INADDR_ANY; //inet_addr ("127.0.0.1"); //inet_addr(OBU_ADDR);// inet_addr ("150.135.152.35"); //

	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);

	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");        
		exit(1);
	}
	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(iSRMPort);
	recvaddr.sin_addr.s_addr = inet_addr(cBroadcastAddress) ; //INADDR_BROADCAST;
	memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
}


void initializeRefPoint(MAP * newmap, geoCoord &geoPoint)
{
	double ref_lat=newmap->intersection.Ref_Lat;
	double ref_long=newmap->intersection.Ref_Long;
	double ref_ele=newmap->intersection.Ref_Ele/10;
	geoPoint.init(ref_long, ref_lat, ref_ele);
}



double getSeconds()
{
	double temp=0.0;
	if (icodeUsage== SIMULATION)
	{
		struct timeval tv_tt;
		gettimeofday(&tv_tt, NULL);
		temp=(tv_tt.tv_sec+tv_tt.tv_usec/1.0e6);   
	}
	else if (icodeUsage== FIELD)
	{ 
		temp=dgpsTime;
	}
	return temp;
}


void storeVehicleTrajectory( LinkedList <PriorityVehicle> & priorityReq) //Edited by YF 08/20/2014    by MZP 10/23/17
{
	int found=0;
	double t1=getSeconds();
	priorityReq.Reset();
	cout<<"HERE@"<<endl;	
	//Checking Vechicle ID to see if we have it in the vehicle list or not. (in case we use the code for simulation, there might be several priority vehicles at intersection and the prg sould handle them all
	while(!priorityReq.EndOfList())  //match vehicle according to vehicle ID
	{	
		cout<<"HERE1"<<endl;	
	
		//if ID is a match, store trajectory information
		//If ID will change, then need an algorithm to do the match!!!!
		if(priorityReq.Data().lVehID==vehIn.TemporaryID)  //every 0.5s processes a trajectory 
		{
			//	pro_flag=1;   //need to map the point						
			priorityReq.Data().receive_timer=t1;  //reset the timer
			priorityReq.Data().traj[priorityReq.Data().nFrame].latitude=vehIn.pos.latitude;
			priorityReq.Data().traj[priorityReq.Data().nFrame].longitude=vehIn.pos.longitude;
			priorityReq.Data().traj[priorityReq.Data().nFrame].elevation=vehIn.pos.elevation; 
			//change GPS points to local points
			if (iLogIndicator==3)
			{
				sprintf(temp_log, " In the StoreTrajectory longitute %lf latitude %lf evelvation %lf    \n ",  vehIn.pos.longitude, vehIn.pos.latitude,vehIn.pos.elevation); 
				outputlog(temp_log);
				cout<<temp_log;
			}
			priorityReq.Data().dSpeed=vehIn.motion.speed;
	//		if (icodeUsage == FIELD) // road version of PRG
	//			priorityReq.Data().dSpeed==(vehIn.motion.speed)/3.6;  // convert from kmph to mps
	//		else if (icodeUsage == SIMULATION) // lab version of PRG , speed comes from VISSIM
	//			priorityReq.Data().dSpeed==vehIn.motion.speed;
			priorityReq.Data().heading=vehIn.motion.heading;
			priorityReq.Data().Dsecond=vehIn.DSecond;
			if (iLogIndicator==3)
			{
				sprintf(temp_log, " In the storeVehicleTrajectory function: longitute %lf latitude %lf evelvation %lf    \n ",  vehIn.pos.longitude,vehIn.pos.latitude,vehIn.pos.elevation); 
				outputlog(temp_log); 
				cout<<temp_log;
			}
			priorityReq.Data().active_flag=5;  //reset active_flag every time RSE receives BSM from the vehicle
			priorityReq.Data().time[priorityReq.Data().nFrame]=t1;
			priorityReq.Data().nFrame++; 
			//if reached the end of the trajectory, start over
			if(priorityReq.Data().nFrame==4999)
				priorityReq.Data().nFrame=0;
			found=1;
			break;
		}
		priorityReq.Next();
	}
	if(found==0)  //this is a new vehicle
	{
	
		PriorityVehicle TempVeh;
		TempVeh.lVehID=vehIn.TemporaryID;
		TempVeh.traj[0].latitude=vehIn.pos.latitude;
		TempVeh.traj[0].longitude=vehIn.pos.longitude;
		TempVeh.traj[0].elevation=vehIn.pos.elevation;
		TempVeh.dSpeed=vehIn.motion.speed;
		TempVeh.nFrame=1;
		TempVeh.Phase_Request_Counter=0;
		TempVeh.active_flag=5;  //initilize the active_flag
		TempVeh.receive_timer=getSeconds();
		TempVeh.time[0]=TempVeh.receive_timer;
		TempVeh.iVehicleType=iVehiclePriorityLevel;
		sprintf(temp_log,"Add Vehicle No. %ld \n",TempVeh.lVehID);
		outputlog(temp_log); cout<<temp_log;
		priorityReq.InsertRear(TempVeh);   //add the new vehicle to the tracked list
	}
	
 
}
//Find Vehicle position in the map
//find the nearest Lane nodes from the vehicle location
void findPriorityVehicleInMap(double Speed, double Heading,int nFrame,double N_Offset1,double E_Offset1,double N_Offset2,double E_Offset2, MAP NewMap, double &Dis_curr, double &est_TT, int &iVehicleStatus,int &approach, int &lane, double &dminGrn)
{
	int t_App,t_Lane,t_Node;
	int t_pos=0; //node position in the vector
	double lane_heading;
	//temp vehicle point
	//calculate the vehicle in the MAP
	//find lane, requesting phase and distance 
	double tempdis=1000000000.0;
	//find the nearest lane node
    // ----- loging 
	if (iLogIndicator==3)
	{
		sprintf(temp_log," In the FinVehInMap Funciton:  local x is   %lf and  local y  is %lf\n", N_Offset1,E_Offset1);
		outputlog(temp_log); cout<<temp_log;
	}
	// ---- end of logging
	dminGrn=0.0; // initialize the queue clearance time
	double veh_heading=Heading;
	if (veh_heading<0)
		veh_heading+=360;
	int match=0;  //whether the vehicle's heading match the lanes heading  
	for(unsigned int jj=0;jj<NewMap.MAP_Nodes.size();jj++)
	{		
		double dis=sqrt((N_Offset1-NewMap.MAP_Nodes[jj].N_Offset)*(N_Offset1-NewMap.MAP_Nodes[jj].N_Offset)+(E_Offset1-NewMap.MAP_Nodes[jj].E_Offset)*(E_Offset1-NewMap.MAP_Nodes[jj].E_Offset));
		if (dis<tempdis)
		{
			tempdis=dis;
			t_App=NewMap.MAP_Nodes[jj].index.Approach;
			t_Lane=NewMap.MAP_Nodes[jj].index.Lane;
			t_Node=NewMap.MAP_Nodes[jj].index.Node;
			t_pos=jj;
		}
	}
	approach=t_App;
	lane=t_Lane;
	if(nFrame>=2) //start from second frame and we can find a node has same heading
	{
		// determine it is approaching the intersection or leaving the intersection or in queue
		// The threshold for determing in queue: 89.4 cm = 2mph
		//calculate the distance from the reference point here is 0,0;
		//int veh_state; 		// 1: appraoching; 2: leaving; 3: queue
		double N_Pos;  //current vehicle position
		double E_Pos;
		double E_Pos2; //previous vehicle position
		double N_Pos2;
		//find the first node (nearest of intersection) of the lane
		double inter_pos_N=NewMap.MAP_Nodes[t_pos-t_Node+1].N_Offset;
		double inter_pos_E=NewMap.MAP_Nodes[t_pos-t_Node+1].E_Offset;
		N_Pos=N_Offset1;//current position
		E_Pos=E_Offset1;
		N_Pos2=N_Offset2;//previous frame position
		E_Pos2=E_Offset2;
		//double veh_heading=atan2(N_Pos-N_Pos2,E_Pos-E_Pos2)*180/PI;
		lane_heading=NewMap.MAP_Nodes[t_pos].Heading;
		//cout<<"lane_heading"<<lane_heading<<endl;
		if(lane_heading<0)
			lane_heading+=360;
		if( abs(veh_heading - lane_heading)> 120 && abs(veh_heading - lane_heading) <240)  // if GPS drifted and we locate the vefhicle in the egress approach instead of ingreaa approach, we should recover the correct approach and lane!!
		{
			if(approach%2 == 1) //When the approach is 1, 3, 5,or 7
				approach = approach +1;
			else //When the approach number is 2, 4, 6,or 8
				approach = approach -1;
			match=1;
			double dTempDist=0.0;
			double dTempDist2=10000000.0;
			for(unsigned int jj=0;jj<NewMap.MAP_Nodes.size();jj++)
			{
				if (NewMap.MAP_Nodes[jj].index.Approach==approach)
				{
					dTempDist=sqrt((N_Offset1-NewMap.MAP_Nodes[jj].N_Offset)*(N_Offset1-NewMap.MAP_Nodes[jj].N_Offset)+(E_Offset1-NewMap.MAP_Nodes[jj].E_Offset)*(E_Offset1-NewMap.MAP_Nodes[jj].E_Offset));	
					if (dTempDist<dTempDist2)
					{
						dTempDist2=dTempDist;
						lane=NewMap.MAP_Nodes[jj].index.Lane;
					}
				}
			}
		}
		if (abs(veh_heading-lane_heading)<20)   //threshold for the difference of the heading
			match=1;
		if (veh_heading>340 && lane_heading<20)
		{
			veh_heading=veh_heading-360;
			if (abs(veh_heading-lane_heading)<20)
				match=1;
		}
		
		if (lane_heading>340 && veh_heading<20)
		{
			lane_heading=lane_heading-360;
			if (abs(lane_heading-veh_heading)<20)
				match=1;
		}
		double Dis_pre= sqrt((N_Pos2-inter_pos_N)*(N_Pos2-inter_pos_N)+(E_Pos2-inter_pos_E)*(E_Pos2-inter_pos_E));
		Dis_curr=sqrt((N_Pos-inter_pos_N)*(N_Pos-inter_pos_N)+(E_Pos-inter_pos_E)*(E_Pos-inter_pos_E));
		
		if(match==1 && approach%2==1)   //odd approach: igress approaches
		{
			if (fabs(Dis_pre-Dis_curr)<0.894/10)  //unit m/0.1s =2mph //Veh in queue is also approaching the intersection, leaving no queue
			{
				iVehicleStatus=INQUEUE;  
			}
			if (fabs(Dis_pre-Dis_curr)>=0.894/10)
			{
				if (Dis_curr<Dis_pre)
					iVehicleStatus=APPROACHING;  
				else
				{
					iVehicleStatus=LEAVING; 
					//request_phase=-1;  //if leaving, no requested phase
				}
			}
		}
		
		if(match==1 && approach%2==0)   //even approach: engress approaches
		{
			iVehicleStatus=LEAVING;
		}
		
		if (iVehicleStatus==LEAVING && Dis_curr > 30) // if the vehicle is leaving and passed the intersection for 30 meter 
			iVehicleStatus=PASSEDINTERSECTION; 
		if (match==0) 
		{
			iVehicleStatus=NOTINMAP;   
			//request_phase=-1;  
		}


		if (iVehicleStatus==APPROACHING || iVehicleStatus==INQUEUE) //only vehicle approaching intersection need to do something
		{
			if (icodeUsage==FIELD)
			{
				if(Speed<dSpeedChangeLmtForSRM) // when vehicle speed drops below dSpeedChangeLmtForSRM   kmph on the road, we consider it as stoped vehicle!
				{
					dminGrn=1.8*(Dis_curr/7)+2;  // dminGrn assumed average length of vehicle is 7 meter and 1.8 is headway between two consecutive cars. 2 is the  vehicle reaction time. 
					est_TT=dminGrn;    //if the vehicle is in queue, assume ETA is 0
				}
				else
				{
					est_TT=Dis_curr/(Speed/3.6); //if we use the lab version of the PRG (means if codeusage=1), speed is meter per second. But if road version is being used (codeusage=2),speed  should be converter to meter per second from km per hour
					dminGrn=0.0;
				}
			} else if (icodeUsage==SIMULATION)
			{
				if(Speed<dSpeedChangeLmtForSRM) // when vehicle speed drops below dSpeedChangeLmtForSRM meter per second in the simulation , we consider it as stoped vehicle!
				{
					dminGrn=1.8*(Dis_curr/7)+2;  // dminGrn assumed average length of vehicle is 7 meter and 1.8 is headway between two consecutive cars. 2 is  vehicle reaction time. 
					est_TT=dminGrn;    //if the vehicle is in queue, assume ETA is 0
				}
				else
				{
					est_TT=Dis_curr/Speed; //if we use the lab version of the PRG (means if codeusage=1), speed is meter per second. But if road version is being used (codeusage=2),speed  should be converter to meter per second from km per hour
					dminGrn=0.0;
				}				
			}				
			if(est_TT>9999)
				est_TT=9999;
		}
		else //Veh in State 2 or 4
		{
			est_TT=99999;
			Dis_curr=99999;
		}
	}
}

void obtainEndofService( int ihour,int imin,int isec, int iETA,int &iHour,int &iMin,int &iSec, int iMinGrn,int Ru_Rl)
{
	int iRu_Rl=(int) (Ru_Rl/2);
	if (iMinGrn==0)
	{
		if ((iETA>=0) && (iETA<59))
		{
			if (imin ==59)
			{
				if ((isec + iETA +iRu_Rl)>=60)
				{
					if (ihour==23)
					{
						iHour   = 0;
						iMin = 0;
						iSec = isec + iETA+iRu_Rl - 60;
					}else
					{
						iHour   = ihour + 1;
						iMin = 0;
						iSec = isec + iETA+iRu_Rl - 60;
					}
				}else
				{
					iHour    = ihour;
					iMin  = imin;
					iSec  = isec + iRu_Rl+iETA;
				}
			}else
			{
				if ((isec + iRu_Rl + iETA)>=60)
				{
					iHour    = ihour;
					iMin  = imin+1;
					iSec  = isec + iRu_Rl +iETA- 60;
				}
				else
				{
					iHour    = ihour;
					iMin  = imin;
					iSec = isec + iRu_Rl+iETA;
				}
			}
		}
		if ((iETA>=60) && (iETA<119))
		{
			if (imin ==59) 
			{
				if ((isec + iETA +iRu_Rl)<120)
				{
					if (ihour==23)
					{
						iHour   = 0;
						iMin = 0;
						iSec = isec + iETA+iRu_Rl - 60;
					}else
					{
						iHour   = ihour + 1;
						iMin = 0;
						iSec = isec + iETA+iRu_Rl - 60;
					}
				}else
				{
					if (ihour==23)
					{
						iHour   = 0;
						iMin = 1;
						iSec = isec + iETA+iRu_Rl - 120;
					}else
					{
						iHour   = ihour + 1;
						iMin = 1;
						iSec = isec + iETA+iRu_Rl - 120;
					}
				}
			}else if (imin ==58)
			{
				if ((isec + iETA +iRu_Rl)<120)
				{
					iHour   = ihour ;
					iMin = 59;
					iSec = isec + iETA+iRu_Rl - 60;

				}else
				{
					if (ihour==23)
					{
						iHour   = 0;
						iMin = 0;
						iSec = isec + iETA+iRu_Rl - 120;
					}else
					{
						iHour   = ihour + 1;
						iMin = 0;
						iSec = isec + iETA+iRu_Rl - 120;
					}
				}
			}else if (imin <58)
			{
				if ((isec + iETA +iRu_Rl)<120)
				{
					iHour=ihour;
					iMin = imin+1;
					iSec = isec + iETA+iRu_Rl - 60;

				}else
				{
					iHour=ihour;
					iMin = imin+2;
					iSec = isec + iETA+iRu_Rl - 120;
				}
			}
		}
	}
	if (iMinGrn>0)
	{
		
		if ((iMinGrn>=0) && (iMinGrn<59))
		{
			if (imin ==59)
			{
				if ((isec + iMinGrn)>=60)
				{
					if (ihour==23)
					{
						iHour   = 0;
						iMin = 0;
						iSec = isec + iMinGrn - 60;
					}else
					{
						iHour   = ihour + 1;
						iMin = 0;
						iSec = isec + iMinGrn - 60;
					}
				}else
				{
					iHour    = ihour;
					iMin  = imin;
					iSec  = isec +iMinGrn;
				}
			}else
			{
				if ((isec + iMinGrn)>=60)
				{
					iHour    = ihour;
					iMin  = imin+1;
					iSec  = isec +iMinGrn- 60;
				}
				else
				{
					iHour    = ihour;
					iMin  = imin;
					iSec = isec +iMinGrn;
				}
			}
		}
		if ((iMinGrn>=60) && (iMinGrn<119))
		{
			if (imin ==59) 
			{
				if ((isec + iMinGrn )<120)
				{
					if (ihour==23)
					{
						iHour   = 0;
						iMin = 0;
						iSec = isec + iMinGrn- 60;
					}else
					{
						iHour   = ihour + 1;
						iMin = 0;
						iSec = isec + iMinGrn- 60;
					}
				}else
				{
					if (ihour==23)
					{
						iHour   = 0;
						iMin = 1;
						iSec = isec + iMinGrn - 120;
					}else
					{
						iHour   = ihour + 1;
						iMin = 1;
						iSec = isec + iMinGrn - 120;
					}
				}
			}else if (imin ==58)
			{
				if ((isec + iMinGrn )<120)
				{
					iHour   = ihour ;
					iMin = 59;
					iSec = isec + iMinGrn - 60;

				}else
				{
					if (ihour==23)
					{
						iHour   = 0;
						iMin = 0;
						iSec = isec + iMinGrn - 120;
					}else
					{
						iHour   = ihour + 1;
						iMin = 0;
						iSec = isec + iMinGrn - 120;
					}
				}
			}else if (imin <58)
			{
				if ((isec + iMinGrn)<120)
				{
					iHour=ihour;
					iMin = imin+1;
					iSec = isec + iMinGrn - 60;

				}else
				{
					iHour=ihour;
					iMin = imin+2;
					iSec = isec + iMinGrn - 120;

				}

			}
		}
	}
}	


void readMAPsFromFile(char *filename,LinkedList<MAPEntry>& mapList)
{
	fstream fsmap;
	fsmap.open(filename);
	string lineread;
	long mapID;
	int attime;
	mapList.ClearList();
	if(fsmap.good())
	{
		while(!fsmap.eof())
		{
			getline(fsmap,lineread);

			if(lineread.size()!=0)
			{

				if(sscanf(lineread.c_str(),"%ld %d",&mapID,&attime)!=2)
				{
					perror("Not all fields are assigned.");
					outputlog("Not all fields are assigned.");
					exit(1);
				}
				else
				{
					MAPEntry currentMap(mapID,attime);
					mapList.InsertAfter(currentMap);
				}
			}
		}
	}
	else
	{
		perror("cannot open RNDF.txt file.");
		strcpy(temp_log,"cannot open RNDF.txt file.");
		outputlog(temp_log);
	}

	if(mapList.ListSize()<=0)
	{
		strcpy(temp_log,"Empty Intersection_map file ");
		outputlog(temp_log); cout<<"Empty Intersection_map file "<< endl;
	}
	fsmap.close();
}



void getBusStopsRange(char filename[])
{
	// the file should be in this format  
	// Route_Name xxxx 
	// Number_Of_Intersections xxx
	// Intersection MAP ID, number of considered approach
	// Approach number, bus stop distance to stop bar of this approach 
	string lineread;
	char temp[256]; 
	char cRouteName[128];
	int iApprNumber=0;
	int iNoAppr=0;
	int iMAPID=0;
	int iTempDistance=9999;
	memset(iBusStopsApprRange,0,sizeof(iBusStopsApprRange));
	memset(iBusStopsMAPID,0, sizeof(iBusStopsMAPID));
	ifstream load;
	load.open(filename);
	if(load.good())
	{
	 	getline(load,lineread);
		sscanf(lineread.c_str(),"%*s %s ",temp);
		strcpy(cRouteName,temp);
		getline(load,lineread);
		sscanf(lineread.c_str(),"%*s %s",temp);
		iNoOfIntersectionInBusRoute=atoi(temp);
		for(int i=0;i<iNoOfIntersectionInBusRoute;i++)
		{
			getline(load,lineread);
			sscanf(lineread.c_str(),"%s",temp);
			iMAPID=atoi(temp);
			iBusStopsMAPID[i]=iMAPID;
			sscanf(lineread.c_str(),"%*s %s",temp);
			iNoAppr=atoi(temp);
			for (int j=0;j<iNoAppr; j++)
			{
				getline(load,lineread);
				sscanf(lineread.c_str(),"%s",temp);
				iApprNumber=atoi(temp);
				sscanf(lineread.c_str(),"%*s %s",temp);
				iTempDistance=atoi(temp);
				iBusStopsApprRange[i][iApprNumber]=iTempDistance;				
			}
		}
		sprintf(temp_log," Read Transit Route %s  bus stops ranges successfully ", cRouteName); 
		outputlog(temp_log);
		cout<<temp_log<<endl;
		sprintf(temp_log," Number of Intersections in the bus route %d  ", iNoOfIntersectionInBusRoute); 
		outputlog(temp_log);
		cout<<temp_log<<endl;
		for (int i=0;i<iNoOfIntersectionInBusRoute;i++)
		{
			for (int j=0;j<8;j++)
			{
				if (iBusStopsApprRange[i][j]!=0)
				{
					sprintf(temp_log," MAP ID  %d in approach %d with range %d ", iBusStopsMAPID[i] , j , iBusStopsApprRange[i][j] ); 
					outputlog(temp_log);
				}
			}
		}
	}else
	{
		perror("cannot open busStopsRange.txt file or there is no Transit Route information.");
		sprintf(temp_log,"cannot open busStopsRange.txt file or there is no Transit Route information.");
		outputlog(temp_log);
		cout<<temp_log<<endl;
	}
	load.close();
}


int findMAPPositionInList(LinkedList<MAP> mapList,long id)
{
	mapList.Reset();
	int temp=-1;
	if(mapList.ListEmpty())
		return temp;
	else
	{
		while(!mapList.EndOfList())
		{

			if(mapList.Data().intersection.ID==id)
			{
				return mapList.CurrentPosition();
			}
			mapList.Next();
		}
	}
	return temp;
}

//This funciton is used when road verison of the code is applied
void formBSMOfSRM(BasicVehicle *veh, int tempID, int msgcnt, double lat, double lon, double heading, double speed,double elev)
{
	veh->TemporaryID = tempID ;
	veh->MsgCount = msgcnt ;
	veh->motion.speed = speed ;
	veh->motion.heading = heading ;
	veh->pos.latitude = lat ;
	veh->pos.longitude = lon ;
	veh->pos.elevation=elev;
	veh->DSecond=0;
}


void     xTimeStamp( char * pc_TimeStamp_ )
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





int reqListFromRxAcknowledgedRequestsFile(char *filename,LinkedList<ReqEntry>& Req_List)
{
	fstream fss;
	fss.open(filename,fstream::in);
	ReqEntry req_temp;
	int ReqNo=0;
	long lOBU_ID;
	int Veh_Class=0;
	int Req_Phase=0;
	int abs_time=0;
	int ETA=0;
	float fETA=0.0;
	float MinGrn=0.0;
	int inlane=0;
	int outlane=0;  
	int strhour=0; 
	int strmin=0; 
	int strsec=0; 
	int endhour=0; 
	int endmin=0; 
	int endsec=0;
	int vehstate=0;
	int imsgcnt=100;
	string lineread;
	Req_List.ClearList();
	getline(fss,lineread);
	sscanf(lineread.c_str(),"%*s %d",&ReqNo); 
	while(!fss.eof())
	{
		getline(fss,lineread);
		if(lineread.size()!=0)    
		{
			sscanf(lineread.c_str(),"%ld %d %f %d %f %d %d %d %d %d %d %d %d %d %d %d ",&lOBU_ID,&Veh_Class,
				&fETA,&Req_Phase,&MinGrn,&abs_time,&inlane, &outlane, &strhour, &strmin, &strsec, &endhour, &endmin, &endsec, &vehstate, &imsgcnt);
			ETA=(int)fETA;
			req_temp.ReqEntryFromFile(lOBU_ID, Veh_Class, ETA, Req_Phase, MinGrn, abs_time,0, inlane, outlane, strhour, strmin, strsec, endhour, endmin, endsec, vehstate,  imsgcnt);
			Req_List.InsertAfter(req_temp);
			
	//		cout<<"IN PSM.TXT VEHICLE ID"<<lOBU_ID<<endl;	
			
	//		cout<<"IN PSM.TXT vehstate"<<vehstate<<endl;		
		//	printf(" Veh State %s \n",vehstate);
		}
	}
	fss.close();
    return ReqNo;
}

int thisVehIDPositionInList( long VehID, LinkedList<ReqEntry> Req_List)
{
	Req_List.Reset();
	int iTemp=-1;
	if(Req_List.ListEmpty()==0) // when list is not empty
    {
	    while (!Req_List.EndOfList())
		{
			if (Req_List.Data().lVehID==VehID)
			{
				iTemp=Req_List.CurrentPosition();
			}
			Req_List.Next();
		}
	}
	//else
	//{
	//	cout<<"List is empty in thisVehIDPositionInList function"<<endl;
	//} 
	return iTemp;	
}




//~ void fillSRM(SRM_t *srm, BasicVehicle *veh, int intID, int iMsgCnt, bool bIsCanceled, int iInLane, int iOutLane, int iETA, double dMinGr, int vehState, int iprioritylevel, int Ru_Rl, char* cVehicleName)
//~ {
	//~ time_t theTime = time(NULL);
	//~ struct tm *aTime = localtime(&theTime);
	//~ srm->message_id=14;       
	//~ srm->msgcount=iMsgCnt;                                                      
	//~ srm->intersection_id =intID;   
	//~ srm->cancelreq_priority=bIsCanceled;
    //~ srm->cancelreq_preemption = 0;       // Extra fields that should be initialized 
	//~ srm->signalreq_priority = J2735_NAV; // Extra fields that should be initialized 
	//~ srm->signalreq_preemption = 2;       // Extra fields that should be initialized   
	//~ srm->in_lanenum=iInLane;
	//~ srm->out_lanenum=iOutLane;
//~ 
	//~ ret=OCTET_STRING_fromBuf(&srm->request.type,(char *)(&iprioritylevel),1);
	//~ 
	//~ srm->vehicleclass_level = 1;   // Extra fields that should be initialized    
	//~ strcpy(srm->code_word, "MMITSS code word");
	//~ 
	//~ srm->starttime_hour = aTime->tm_hour; 
	//~ srm->starttime_min  = aTime->tm_min; 
	//~ srm->starttime_sec  = aTime->tm_sec; 
	//~ int iHour=0;
	//~ int iMin=0;
	//~ int iSec=0;
	//~ int itempMinGrn=(int) dMinGr;
	//~ obtainEndofService( aTime->tm_hour,aTime->tm_min,aTime->tm_sec,iETA,iHour, iMin,iSec,itempMinGrn,Ru_Rl);
	//~ if (bIsCanceled==0)
	//~ {
		//~ srm->endtime_hour =  iHour;
		//~ srm->endtime_min  =  iMin;
		//~ srm->endtime_sec  =  iSec;
	//~ }else if (bIsCanceled==1)
	//~ { 	
		//~ srm->starttime_hour =0; 
		//~ srm->starttime_min = 0; 
		//~ srm->starttime_sec = 0; 
		//~ srm->endtime_hour  = 0;
		//~ srm->endtime_min   = 0;
		//~ srm->endtime_sec   = 0;
	//~ }
//~ 
//~ 
    //~ srm->transitstatus = 1;
    //~ strcpy(srm->vehicle_name,cVehicleName);
    //~ strcpy(srm->vin, "MMITSS2014");
	//~ strcpy(srm->vehicle_ownercode,"MMITSS");
	//~ srm->temp_ident_id =veh->TemporaryID;
	//~ srm->vehicleclass_type = iprioritylevel;  
	//~ srm->vehicle_class = 9985;					// Extra fields that should be initialized  
    //~ srm->vehicle_grouptype = 1;					// Extra fields that should be initialized  
//~ 
   //~ // Filling hte BSM blob part of theSRM
    //~ srm->bsm_msgcount = 0;						// Extra fields that should be initialized  
    //~ srm->temp_id=veh->TemporaryID ;
    //~ srm->dsecond	  = 0;						// Extra fields that should be initialized  
	//~ srm->latitude=veh->pos.latitude;
	//~ srm->longitude=veh->pos.longitude;
	//~ srm->elevation=veh->pos.elevation;
	//~ srm->positionalaccuracy[0]		= 0.0;
	//~ srm->positionalaccuracy[1]		= 0.0;
	//~ srm->positionalaccuracy[2]		= 0.0;
	//~ srm->speed=veh->motion.speed ;
	//~ srm->heading=veh->motion.heading;
	//~ srm->angle						= 0; 		// Extra fields that should be initialized  
	//~ srm->longaccel					= 0.0;
	//~ srm->lataccel					= 0.0;
	//~ srm->vertaccel					= 0.0;
	//~ srm->yawrate = dMinGr;						
	//~ srm->wheelbrake					= 1;	// Extra fields that should be initialized  
	//~ srm->wheelbrakeavailable		= 1;	// Extra fields that should be initialized  
	//~ srm->sparebit					= 1;	// Extra fields that should be initialized  
	//~ srm->traction					= 1;	// Extra fields that should be initialized  
	//~ srm->abs						= 0;	// Extra fields that should be initialized  
	//~ srm->stabilitycontrol			= 1;	// Extra fields that should be initialized  
	//~ srm->brakeboost					= 1;	// Extra fields that should be initialized  
	//~ srm->auxbrakes					= 1;	// Extra fields that should be initialized  
//~ 
	//~ srm->vehicle_width=2.0;
	//~ srm->vehicle_length=2.0;                                                            
	//~ srm->vehicle_status=vehState;
//~ 
//~ }



//~ // This function is used when lab version of the code is applied
//~ void fillSRM(J2735SRM_t *srm,int intID,int iMsgCnt,bool bIsCanceled,int iInLane,int iOutLane,int iETA, double dMinGr, int vehState,int Ru_Rl,char *cVehicleName)
//~ {
	//~ time_t theTime = time(NULL);
	//~ struct tm *aTime = localtime(&theTime);
	//~ srm->message_id=14;                                                             
	//~ srm->msgcount=iMsgCnt;
	//~ srm->in_lanenum=iInLane;
	//~ srm->out_lanenum=iOutLane;
	//~ srm->cancelreq_priority=bIsCanceled;
	//~ strcpy(srm->vehicle_name,cVehicleName);
//~ 
	//~ // In VISSIM different class!! of vehicles are diffrentiated by their category!. So in DrivelModel.dll , DRIVER_DATA_VEH_CATEGORY gets different values for different class of vehicles!!  car = 1, truck = 2, bus = 3, tram = 4, pedestrian = 5, bike = 6 .
	//~ // When we pack the SRM in dll, we put these numbers in  vehicleclass_level  field of SRM. 
	//~ // Here in PRG, we put the srm->vehicleclass_level into srm->vehicle_type
	//~ // Since we do not have any EV in dll, we consider trams as EV!! 
//~ /*	if (srm->vehicleclass_level==4)// which the category of Tram in VISSIM     
	    //~ srm->vehicle_type=1;  // in priority applications, priority level of EV is 1
    //~ if (srm->vehicleclass_level==3)  // which the category of bus in VISSIM
		//~ srm->vehicle_type=2;   // in priority applications, priority level of bus is 2
	//~ if (srm->vehicleclass_level==2)  // which the category of truck in VISSIM
		//~ srm->vehicle_type=3;    // in priority applications, priority level of truck is 3
	//~ if (srm->vehicleclass_level==1)  // which the category of car in VISSIM
		//~ srm->vehicle_type=4;    // in priority applications, priority level of car is 4
//~ */	
//~ 
	//~ if (srm->vehicleclass_level==4)// which the category of Tram in VISSIM     
	   //~ srm->vehicleclass_type=1;  // in priority applications, priority level of EV is 1
    //~ if (srm->vehicleclass_level==3)  // which the category of bus in VISSIM
		//~ srm->vehicleclass_type=2;   // in priority applications, priority level of bus is 2
	//~ if (srm->vehicleclass_level==2)  // which the category of truck in VISSIM
		//~ srm->vehicleclass_type=3;    // in priority applications, priority level of truck is 3
	//~ if (srm->vehicleclass_level==1)  // which the category of car in VISSIM
		//~ srm->vehicleclass_type=4;    // in priority applications, priority level of car is 4
//~ 
//~ 
	//~ srm->intersection_id =intID;   
	//~ strcpy(srm->code_word, "MMITSS code word");
	//~ srm->starttime_hour = aTime->tm_hour; 
	//~ srm->starttime_min  = aTime->tm_min; 
	//~ srm->starttime_sec  = aTime->tm_sec; 
	//~ int iHour=0;
	//~ int iMin=0;
	//~ int iSec=0;
	//~ int itempMinGrn=(int) dMinGr;
//~ 
	//~ obtainEndofService( aTime->tm_hour,aTime->tm_min,aTime->tm_sec,iETA,iHour, iMin,iSec,itempMinGrn,Ru_Rl);
//~ 
//~ 
	//~ if (bIsCanceled==0)
	//~ {
		//~ srm->endtime_hour =  iHour;
		//~ srm->endtime_min  =  iMin;
		//~ srm->endtime_sec  =  iSec;
	//~ }else if (bIsCanceled==1)
	//~ { 	
		//~ srm->starttime_hour =0; 
		//~ srm->starttime_min = 0; 
		//~ srm->starttime_sec = 0; 
		//~ srm->endtime_hour  = 0;
		//~ srm->endtime_min   = 0;
		//~ srm->endtime_sec   = 0;
	//~ }
	//~ strcpy(srm->vin, "MMITSS2014");
	//~ strcpy(srm->vehicle_ownercode,"MMITSS");
	//~ srm->transitstatus = 1;
	//~ srm->temp_ident_id = srm->temp_id;//  in the field IT should be iVehicleId ---- BE CAREFULL ----
	//~ srm->yawrate = dMinGr;
	//~ srm->vehicle_status=vehState;
	//~ printsrmcsv(srm);
	//~ 
	//~ 
//~}


/* testing sample of daisy gavilan intersection
int I=0	
if (I== 7)
	{m_vehlat=33.840768;
	m_vehlong=-112.135355;
	}
if (I== 6)
	{m_vehlat= 33.841400;
	m_vehlong=-112.135234;
	}
	* if (I== 5)
	{m_vehlat=33.842151;
	m_vehlong =-112.135238;
	}if (I== 4)
	{m_vehlat=33.842669;
	m_vehlong=-112.135268;
	 }
	if (I== 3)
	{m_vehlat=33.843168;
	m_vehlong=-112.135321;
	}if (I== 2)
	{m_vehlat= 33.843472;
	m_vehlong=-112.135353;
	}if (I== 1)
	{m_vehlat=33.843830;
	m_vehlong =-112.135409;
	}if (I== 0)
	{m_vehlat=33.844668;
	m_vehlong=-112.135718;
	 }
	dHeading=162;
	I++;
m_vehspeed=25;
dElevation=12;
*/
