/*NOTICE:  Copyright 2017 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   

/* long_term_planning.cpp  
*  Created by Byungho Beak
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.
*
*/
   

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>   
#include <iostream>
#include <sstream>
#include <istream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <glpk.h>
#include <cmath>

#include "math.h"
#include "GetInfo.h"
#include "Mib.h"
#include "Config.h"   // <vector>
#include "Signal.h"  
#include "Array.h"
#include "Schedule.h"
#include "LinkedList.h"
#include "PriorityRequest.h"
#include "ReqEntry.h"

//!!! B.H : for trajectory awareness
#include "ConnectedVehicle.h"


using namespace std;

//---- The following two for function RunScheduleG()
//#define NEW 1   //??
//#define REGULAR 0  //??
//#define OPT_MSG_SIZE 256
#ifndef byte
    #define byte  char  // needed if byte not defined
#endif

#ifndef DEG2ASNunits
    #define DEG2ASNunits  (1000000.0)  // used for ASN 1/10 MICRO deg to unit converts
#endif

#define EV  1
#define TRANSIT 2
#define TRUCK 3
#define PED 4
#define COORDINATION 6
#define MaxGrnTime 50         //ONLY change the phase {2,4,6,8} for EV requested phases

//!!! B.H : long term planning => Later the interval will be calculated from the platoon trajectories 
int coordination_interval = 30;

//define log file name with time stamp.
char logfilename[256]     			= "/nojournal/bin/log/long_term_planning_templog_";
char logfilename2[256]     			= "/nojournal/bin/log/Long_term_planning";
char signal_plan_file_long[256]      	= "/nojournal/bin/log/signal_Plan_long_";
//---Store the file name of the Config file.
//--- For Example: ConfigInfo_Hastings.txt, which has the minGrn and maxGrn etc.
char ConfigInfo[256]	  			= "/nojournal/bin/ConfigInfo.txt";
char IPInfo[64]			  			= "/nojournal/bin/IPInfo.txt";
char rsuid_filename[64]   			= "/nojournal/bin/rsuid.txt";

char requestfilename_long[128] 			= "/nojournal/bin/requests_long.txt"; //requests

char signal_filename[128]		 	= "/nojournal/bin/signal_status.txt";

//!!! B.H : long term planning
char resultsfile_long[128]    			= "/nojournal/bin/Results_long.txt"; // Result from the GLPK solver.

//!!! B.H : peer priority
char peerrequests[128]  = "/nojournal/bin/Peer_requests.txt"; 
char lanepeermapping[64]  = "/nojournal/bin/Lane_Peer_Mapping.txt"; 

// -------- BEGIN OF GLOABAL PARAMETERS-----//
// wireless connection variables
int sockfd;
int broadcast=1;
struct sockaddr_in sendaddr;
struct sockaddr_in recvaddr;
int RxArrivalTablePort=6666; // The port that arrival table data comes from
int ArrTabMsgID=66;
int TxOPTPort=5555;          // the port that the optimal phase timing plane sends to SignalControl (COP)

int PeerPort= 80000; //!!! B.H : the port for sending CP to the Solver

int TxEventListPort=44444;
int OPTMsgID=55;
char cOPTbuffer[256];

//!!! B.H :  
char Cp_buffer[256];

char temp_log[256];

double dGlobalGmax[8]={0.0}; // this vector get the values of ConfigIS.Gmax an will keep those values unless we need to change the max green time to ConfigIS.Gmax*(1+MaxGrnExtPortion)

RSU_Config ConfigIS;
RSU_Config ConfigIS_EV;
LinkedList<ReqEntry> Req_List_Combined;  // Req list for requests in requests_combined.txt
LinkedList<ReqEntry> ReqList;   // Req list for requests in requests.txt: For updating the ReqListUpdateFlag only

//!!! B.H : peer requests
LinkedList<ReqEntry> Local_Peer_ReqList; 
LinkedList<ReqEntry> Local_Peer_ReqList_temp;
LinkedList<ReqEntry> Upstream_Peer_ReqList;


//!!! B.H : for trajectory list
LinkedList <ConnectedVehicle> trackedveh;

Phase Phases;  //----Important one----//
string RSUID;
char ConfigFile[256];   //= "/nojournal/bin/ConfigInfo_MountainSpeedway.txt";

int ReqListUpdateFlag=0;

//!!! B.H : For local peer request
int Peer_ReqListUpdateFlag=0;
int UpstreamPeer_ReqNo=0;
int LocalPeer_ReqNo=0;

int peer_reqPhase[2];

double lat_busStop_east[2];
double long_busStop_east[2];
double lat_busStop_west[2];
double long_busStop_west[2];

int pre_approach_long[2]; 
int curr_approach_long[2]; 

double ETA_east;
double ETA_west;

char eastbound_IP[64];
char westbound_IP[64];

char tmp_log[256];
char tmp_log2[256]; //!!! B.H 

int CurPhaseStatus[8]={0};    // when do phaseReading: the same results as phase_read withoug conversion
PhaseStatus phase_read={0};   // value would be {1,3,4}
char INTip[64];
char INTport[16];
int CombinedPhase[8]={0};
int Phase_Status[NumPhases]={0};
int codeUsage=1;
int congested=0;
int solving_check=0;

int ii=0;

int LaneNo[8]; 

//!!! B.H : socket connection
int UDP_PORT=20000;

char buf_traj[500000];  

char peer1_data[256];

int outputlog(char *output); // for logging 
int outputlog2(char *output); // for logging the matlab

void PrintList2File(char *Filename,LinkedList<ReqEntry> &ReqList,int BroadCast=0); //If BroadCast=1, will add RSUID as the first line

void PrintList2File_down(char *Filename);

void UpdatePeerList2File(LinkedList<ReqEntry> &Local_List, LinkedList<ReqEntry> &Local_List_temp); 

void UpstreamPeer_ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List1);

void LocalPeer_ReqListFromFile(LinkedList<ReqEntry>& Req_List2);

//!!! B.H : For the peer request received from upstream => write the req in request_long.txt
void PrintList2File_up(char *Filename,LinkedList<ReqEntry> &Up_ReqList);

void UpdateList_peer(LinkedList <ConnectedVehicle> &trackedveh, LinkedList<ReqEntry> &Upstream_Peer_ReqList); //LinkedList <ConnectedVehicle> trackedveh

//!!! B.H : long term planning
void PrintPlan2Log(char *resultsfile_long);
void PrintFile2Log(char *resultsfile_long);

//!!! B.H : Request the trajectory awareness data and unpack data from trajectory awareness
void UnpacktrajData3(char* ablob);

void UnpackpeerReq(char* ablob);

void PackpeerReq(char* tmp_peer_data, int &size, int vehid, int Vehclass, double eta, int peerPhase);

//void packCPforsolver(char* CP_buffer,double cp[2][3][16], int &size_CP); //void packOPT(char * buf,double cp[2][3][5],int msgCnt); 
void packCPforsolver(char* CP_buffer,LinkedList<ReqEntry> Req_List, int &size_CP);

//!!! B.H : To read lane peer mapping
void readlanepeermapping();



int main ( int argc, char* argv[] )
{
	struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
	
     int ch;
     while ((ch = getopt(argc, argv, "c:s:")) != -1) 
     {
		switch (ch) 
		{
		    case 'c':
				codeUsage=atoi (optarg);
				if (codeUsage==1)
					printf ("Code usage is for Prioirty Alg + Actuation \n");
				else
					printf ("Code usage is for Prioirty Alg + COP \n");
				break;
			case 's':
				congested=atoi (optarg);
				if (congested==1)
					printf (" We have super congested situation \n");
				else
					printf (" Normal Traffic \n");
				break;
            default:
			     return 0;
		 }
    }
	int iOPTmsgCnt=0;
	int iArrTabMsgCnt;
    unsigned int type = 0;
    int SEND_OFF_CMD=3;
    int RequestFileNo=0;
    int IsListEmpty=1;  // ---IsListEmpty==1 means empty, will do nothing; =0, is not empty, need to carry on the plan.--//
    int InitPhase[2];
    double InitTime[2],GrnElapse[2];
    int coordinationIndicator=0;
 
    double TimeStamp[2][2];   // For Recording the end time of each cycle including the InitTime. // MZ changed to consider 2 cycles! 10/8/14

    //------log file name with Time stamp---------------------------
    char timestamp[128];
    string Logfile;
    string Logfile2; //!!! B.H : for matlab gui
    xTimeStamp(timestamp);
    
    //!!! B.H : No timestamp for MATLAB
    strcat(logfilename,timestamp);    Logfile=string(logfilename);
    strcat(logfilename,".log");
    //!!! B.H : for matlab gui
    //strcat(logfilename2,timestamp);    Logfile2=string(logfilename2);
    strcat(logfilename2,".log");
    
    strcat(signal_plan_file_long,timestamp);     strcat(signal_plan_file_long,".log");
    
    std::fstream fs_log, fs_signal_plan,fs_signal_status,fs_request_send;
    //!!! B.H : Overwrite the log for MATLAB
    //std::fstream fs_log2;
    
    fs_log.open(logfilename, fstream::out | fstream::trunc);
    
    fs_signal_plan.open(signal_plan_file_long,fstream::out | fstream::trunc); 
    //------end log file name-----------------------------------
	
	
	//!!! B.H : Read Lane_Peer_Mapping.txt for the peer request
    readlanepeermapping();
   
	sprintf(tmp_log," Test read lane mapping ETA_east : %5.2lf pre_approach_long[0] : %d curr_approach_long[0] : %d eastbound_IP : %s westbound_IP : %s \n", ETA_east,pre_approach_long[0],curr_approach_long[0],eastbound_IP,westbound_IP);
	outputlog(tmp_log);	
     
	//system("\\rm /nojournal/bin/requests_long.txt");
	//FILE *fp_req=fopen(requestfilename_long,"w");
    //fprintf(fp_req,"Upstream_Peer_Num -1 0\n");
    //fclose(fp_req);

    
	//!!! B.H : To track the approach of local peer after stop bar
	//------------init: Begin of Network connection for Trajectory Awareness ------------------------------------
    int sockfd_trajectory;

	struct sockaddr_in sendaddr_trajectory;
	struct sockaddr_in recvaddr_trajectory;
	int numbytes, addr_len_trajectory;
	int broadcast=1;

	if((sockfd_trajectory = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd");
		exit(1);
	}
    
    if (setsockopt(sockfd_trajectory, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Error");
	}
    
	//if((setsockopt(sockfd_trajectory,SOL_SOCKET,SO_BROADCAST,
	//	&broadcast,sizeof broadcast)) == -1)
	//{
	//	perror("setsockopt - SO_SOCKET ");
	//	exit(1);
	//}
     
	sendaddr_trajectory.sin_family = AF_INET;
	sendaddr_trajectory.sin_port = htons(99999);  //*** IMPORTANT: the trajectory pushing code should also have this port. ***//
	sendaddr_trajectory.sin_addr.s_addr = INADDR_ANY;//inet_addr(OBU_ADDR);//INADDR_ANY;

	memset(sendaddr_trajectory.sin_zero,'\0',sizeof sendaddr_trajectory.sin_zero);

	if(bind(sockfd_trajectory, (struct sockaddr*) &sendaddr_trajectory, sizeof sendaddr_trajectory) == -1)
	{
		perror("bind");        exit(1);
	}

	recvaddr_trajectory.sin_family = AF_INET;
	recvaddr_trajectory.sin_port = htons(UDP_PORT);
	recvaddr_trajectory.sin_addr.s_addr = inet_addr("127.0.0.1") ; //local host;
	memset(recvaddr_trajectory.sin_zero,'\0',sizeof recvaddr_trajectory.sin_zero);

	int addr_length_trajectory = sizeof ( recvaddr_trajectory );
	int recv_data_len_trajectory;
	//-----------------------End of Network Connection------------------//
	
	
	//!!! B.H : To listen to the peer request coming from upstream intersections (east and west) and send the request to the peer request to downtream intersections (east and west)  
	//------------init: Begin of Network connection for Upstream peer request-----------------------------------

    int sockfd_upstream;

	struct sockaddr_in sendaddr_upstream;
	struct sockaddr_in recvaddr_upstream;
    int addr_len_upstream;
	//int broadcast=1;

	if((sockfd_upstream = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd");
		exit(1);
	}

    //Setup time out
	if (setsockopt(sockfd_upstream, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Error");
	}
     
	sendaddr_upstream.sin_family = AF_INET;
	sendaddr_upstream.sin_port = htons(77777);  //*** IMPORTANT: the trajectory pushing code should also have this port. coordinator : 55555 
	sendaddr_upstream.sin_addr.s_addr = INADDR_ANY;//inet_addr(OBU_ADDR);//INADDR_ANY;

	memset(sendaddr_upstream.sin_zero,'\0',sizeof sendaddr_upstream.sin_zero);

	if(bind(sockfd_upstream, (struct sockaddr*) &sendaddr_upstream, sizeof sendaddr_upstream) == -1)
	{
		perror("bind");        exit(1);
	}
	
	int addr_length_upstream = sizeof ( recvaddr_upstream );
	int recv_data_len_upstream;
	
	//-----------------------End of Network Connection------------------//
	
	
	
	//!!! B.H : To send the critical point (CP) to Solver 
	//------------init: Begin of Network connection for Solver ------------------------------------
	
	int sockfd_solver;

	//struct sockaddr_in sendaddr_solver;
	struct sockaddr_in recvaddr_solver;
	//int numbytes, 
	int addr_len_solver;
	//int broadcast=1;

	if((sockfd_solver = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd");
		exit(1);
	}
    
    if (setsockopt(sockfd_solver, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Error");
	}
     
	//sendaddr_solver.sin_family = AF_INET;
	//sendaddr_solver.sin_port = htons(99999);  //*** IMPORTANT: the trajectory pushing code should also have this port. ***//
	//sendaddr_solver.sin_addr.s_addr = INADDR_ANY;//inet_addr(OBU_ADDR);//INADDR_ANY;

	//memset(sendaddr_solver.sin_zero,'\0',sizeof sendaddr_solver.sin_zero);

	//if(bind(sockfd_solver, (struct sockaddr*) &sendaddr_solver, sizeof sendaddr_solver) == -1)
	//{
	//	perror("bind");        exit(1);
	//}

	recvaddr_solver.sin_family = AF_INET;
	recvaddr_solver.sin_port = htons(PeerPort);   //!!! Solver uses "80000" (RxCriticalPointPort) to receive CP_long from LPT 
	recvaddr_solver.sin_addr.s_addr = inet_addr("127.0.0.1") ; //local host;
	memset(recvaddr_solver.sin_zero,'\0',sizeof recvaddr_solver.sin_zero);

	int addr_length_solver = sizeof ( recvaddr_solver );
	int recv_data_len_solver;
	//-----------------------End of Network Connection------------------//

	
	
	// ------ Readng Configuration---
	get_ip_address();           // READ the ASC controller IP address into INTip from "IPInfo.txt"
    get_rsu_id();               // Get rsu id for string RSUID from "rsuid.txt"
    //-------(1) Read the name of "configinfo_XX.txt" into "ConfigFile" from ConfigInfo.txt--------//
    //-------(2) Read in the CombinedPhase[8] for finding the split phases--
    get_configfile();
    //-----------------Read in the ConfigIS Every time in case of changing plan-----------------------//
    int curPlan=CurTimingPlanRead();
    
    //!!! B.H : For simple test, it was hard coded 
    curPlan =1;
    
    sprintf(tmp_log,"Current timing plan is:\t%d\n",curPlan);  
    outputlog(tmp_log);

    IntersectionConfigRead(curPlan,ConfigFile);  // Generate: configinfo_XX.txt
   
    PrintPlan2Log(ConfigFile);
        
    PrintRSUConfig(); //!!! B.H : located in config.cpp => ConfigIS.Ring1No : 4, ConfigIS.Phase_Seq_R1 : 0,1,2,3, ConfigIS.Ring2No : 4, ConfigIS.Phase_Seq_R2 : 0,1,2,3, 
     
    
          
    int addr_length = sizeof ( recvaddr );
    //--------------End of Read in the ConfigIS Every time in case of changing plan-----------------------//  //---- If we have EV priority, then we need to generate the special "NewModelData_EV.mod" through "ConfigIS" and "requested phases"
    //--- When in Red or Yellow, init1 & 2 are non-zero, Grn1&Grn2 (GrnElapse) are zero
    //--- When in Green, init1 & 2 are zero, Grn1&Grn2 are non-zero
    InitTime[0]=0; 
    InitTime[1]=0;
    GrnElapse[0]=0;
    GrnElapse[1]=0;
    
    for (int i=0; i<8; i++)
    {
		sprintf(tmp_log,"Test configIS.Ring1No : %d configIS.Ring2No : %d ConfigIS.Red[i] : %5.2lf ConfigIS.Yellow[i] : %5.2lf ConfigIS.Gmin[i] : %5.2lf \n", ConfigIS.Ring1No, ConfigIS.Ring2No, ConfigIS.Red[i],ConfigIS.Yellow[i],ConfigIS.Gmin[i]);
		outputlog(tmp_log); 
	}
    
    
    double updateTime = GetSeconds();
    
    int DL_flag = 0;  //!!! B.H : if DL_flag ==0; dDLs is based on queue clearing time. If DL_flag ==1; dDLs is based on iCoordinationPhaseSplit[i]
    //!!! B.H : for one time solving => while ( true  && solving_check ==0)
    while ( true ) 
    {
		
		//!!! B.H: Open socket to peer request messages to listen to upstream intersections (east ans west) 
		recv_data_len_upstream = recvfrom(sockfd_upstream, peer1_data, sizeof(peer1_data), 0,
                        (struct sockaddr *)&sendaddr_upstream, (socklen_t *)&addr_length_upstream);	
        
        if(recv_data_len_upstream<0)
		{
			sprintf(tmp_log,"No Peer Request received from upstream!! \n");
			outputlog(tmp_log); //cout<<tmp_log;
			
			memset(peer1_data, 0, sizeof(peer1_data)); //clear the buf if nothing received   
			// constraint_requested=-1; //means nothing is received
			//continue;
		}
        else
        {
			sprintf(tmp_log,"Received peer request from upstream!! \n");
			outputlog(tmp_log); 
			
			cout<<" Received peer request from upstream!! "<<endl;
			
			sprintf(tmp_log,"UpstreamPeer_ReqNo : %d\n",UpstreamPeer_ReqNo);
			outputlog(tmp_log); //cout<<tmp_log;
			
			Upstream_Peer_ReqList.Reset();
			
			UnpackpeerReq(peer1_data);
			
			//!!! B.H : Update the peer request list on "requests_long.txt"
			PrintList2File_up(requestfilename_long,Upstream_Peer_ReqList); 
			
			
			//!!! Test : Remove later !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			Upstream_Peer_ReqList.Reset();
			while (!Upstream_Peer_ReqList.EndOfList())
			{
				sprintf(tmp_log,"size of the peer list : %d \n", Upstream_Peer_ReqList.ListSize()); 
				outputlog(tmp_log);
				sprintf(tmp_log,"%d %d %lf %d %f %f %d %d %d %d %d %d %d %d %d %d \n",Upstream_Peer_ReqList.Data().VehID,Upstream_Peer_ReqList.Data().VehClass,Upstream_Peer_ReqList.Data().ETA,Upstream_Peer_ReqList.Data().Phase,Upstream_Peer_ReqList.Data().MinGreen,
					Upstream_Peer_ReqList.Data().fSetRequestTime, Upstream_Peer_ReqList.Data().iInLane, Upstream_Peer_ReqList.Data().iOutLane, Upstream_Peer_ReqList.Data().iStrHour,Upstream_Peer_ReqList.Data().iStrMinute,Upstream_Peer_ReqList.Data().iStrSecond,
					Upstream_Peer_ReqList.Data().iEndHour,Upstream_Peer_ReqList.Data().iEndMinute,Upstream_Peer_ReqList.Data().iEndSecond,Upstream_Peer_ReqList.Data().iVehState, Upstream_Peer_ReqList.Data().iMsgCnt);
				outputlog(tmp_log);
					
				Upstream_Peer_ReqList.Next();
			}
		}
		
		
		double t_1,t_2; //---time stamps used to determine wether we connect to the RSE or not.
		t_1=GetSeconds();
		//---- Read the signal information------//
		PhaseTimingStatusRead();
		
		//cout<< " test 1 phase_read "<<  phase_read <<endl;
		
		t_2=GetSeconds();
		
		if( (t_2-t_1)<2.0 ) // We do connect to RSE
		{
			
			//Phases.UpdatePhase(phase_read);
			
			////------------Begin of recording the phase_status into signal_status.txt----------------//
			/*
			Phases.RecordPhase(signal_plan_file_long);
			for(int ii=0;ii<2;ii++)
			{
				InitPhase[ii]=Phases.InitPhase[ii]+1;   //{1-8}
				InitTime[ii] =Phases.InitTime[ii];      // ALSO is the (Yellow+Red) Left for the counting down time ***Important***
				GrnElapse[ii]=Phases.GrnElapse[ii];     // If in Green
			}
			*/
			//sprintf(tmp_log," ***** initial phase of ring 1 is %d  Ring 2 is %d \n", InitPhase[0], InitPhase[1]);
			//outputlog(tmp_log);	
    
			//sprintf(tmp_log," ***** GrnElapse[0] is %5.2lf GrnElapse[1] is %5.2lf\n", GrnElapse[0],GrnElapse[1]);
			//outputlog(tmp_log);	
    
			
			//IF we can get signal information, then we connect to the ASC controller
			FILE * fp=fopen("/nojournal/bin/connection.txt","w"); // For determination of connecting to a ASC controller or not. Nothing to do with OBU
			fprintf(fp,"%ld",time(NULL));
			fclose(fp);
			msleep(20);   
			
					
			Local_Peer_ReqList_temp.ClearList();
			
			//!!! B.H : Check if we received "request_clear" from PRS through "Peer_requests.txt" AND Remove the local req in "Peer_requests.txt"
			//LocalPeer_ReqListFromFile(peerrequests,Local_Peer_ReqList);
			LocalPeer_ReqListFromFile(Local_Peer_ReqList_temp);
			//!!! B.H : AND Remove the local req in "Peer_requests.txt"
			PrintList2File_down(peerrequests);
			
			//!!! B.H :  Add the local request to "Local_Peer_reqList"
			if (LocalPeer_ReqNo > 0)
			{
				UpdatePeerList2File(Local_Peer_ReqList, Local_Peer_ReqList_temp);
			}
			
			
			//sprintf(tmp_log,"LocalPeer_ReqNo : %d\n", LocalPeer_ReqNo );
			//outputlog(tmp_log); //cout<<tmp_log;
			
			if (!Local_Peer_ReqList.ListEmpty())
			{
				//!!! request trajectory data Only if the interval is over 1 second !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if ((t_1 - updateTime) > 0.5)
				{	
					updateTime = GetSeconds();
					
					sprintf(tmp_log,"Received trajectory data when req veh just passed the stop bar : %5.2lf \n", updateTime);
					outputlog(tmp_log); //cout<<tmp_log;
									
					char buf[64]="Request Trajectory from Long_Term_Planning";
					recvaddr_trajectory.sin_port = htons(UDP_PORT);
					
					sendto(sockfd_trajectory,buf,sizeof(buf) , 0,(struct sockaddr *)&recvaddr_trajectory, addr_length_trajectory);

					recv_data_len_trajectory = recvfrom(sockfd_trajectory, buf_traj, sizeof(buf_traj), 0,
									(struct sockaddr *)&sendaddr_trajectory, (socklen_t *)&addr_length_trajectory);
					
					if(recv_data_len_trajectory<0)
					{
						//printf("Receive Request failed\n");
						sprintf(tmp_log,"Receive Request failed\n");
						//cout<<tmp_log; 
						outputlog(tmp_log);
						// continue;
					}
					trackedveh.ClearList(); 
					UnpacktrajData3(buf_traj);
					//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!		
				}
					
				//!!! B.H : Should be removed after testing
				//trackedveh.Reset();
				//while(!trackedveh.EndOfList())
				//{
				//	sprintf(tmp_log,"test 1 Int : %d ID : %d Req : %d previous approach : %d approach : %d Veh_type : %d busStop_S : %d \n",trackedveh.Data().intersection, trackedveh.Data().TempID,trackedveh.Data().req_phase, trackedveh.Data().previous_approach, trackedveh.Data().approach ,trackedveh.Data().VehType, trackedveh.Data().busStop_S);
				//	outputlog(tmp_log); //cout<<tmp_log;
				//		
				//	trackedveh.Next();	
				//}
								
				Local_Peer_ReqList.Reset();
                while(!Local_Peer_ReqList.EndOfList())
                {
					if (trackedveh.ListSize()>0)
					{
						 
						trackedveh.Reset();
						while(!trackedveh.EndOfList())
						{
							
							if (trackedveh.Data().TempID == Local_Peer_ReqList.Data().VehID)
							{							
								sprintf(tmp_log,"ID : %d  previous approach : %d approach : %d \n", Local_Peer_ReqList.Data().VehID,trackedveh.Data().previous_approach, trackedveh.Data().approach);
								outputlog(tmp_log); //cout<<tmp_log;
								
								if (trackedveh.Data().approach != curr_approach_long[0] && trackedveh.Data().approach != curr_approach_long[1] ) 
								{
									sprintf(tmp_log,"Wait until the req pass the square\n");
									outputlog(tmp_log); //cout<<tmp_log;
									
									break;
								}
								else
								{
									//!!!! Send the local peer req to the downstream on eastbound  
									if (trackedveh.Data().approach == curr_approach_long[0]) 
									{
										if (lat_busStop_east[0] > 0)  //!!! In case that there is far-side bus stop
										{
											if (trackedveh.Data().busStop_S == 1)    //(is_busStop[0] == 0)
											{
												sprintf(tmp_log,"Now send the local peer request to the downstream on eastbound!!\n");
												outputlog(tmp_log); 
												//cout<<"Now send the local peer request to the downstream on eastbound!! "<<endl;
												
												recvaddr_upstream.sin_family = AF_INET;
												recvaddr_upstream.sin_port = htons(77777);  
												recvaddr_upstream.sin_addr.s_addr = inet_addr(eastbound_IP) ; //
												memset(recvaddr_upstream.sin_zero,'\0',sizeof recvaddr_upstream.sin_zero);
												
												int temp_vehID = Local_Peer_ReqList.Data().VehID;
												double temp_system = time(NULL);
												double temp_ETA = ETA_east + temp_system;
												int vehclass = Local_Peer_ReqList.Data().VehClass;
												
												int peer_Phase = peer_reqPhase[0]; //eastbound request phase for downstream							
												
												//sprintf(tmp_log,"temp_system : %0.2f ETA_east : %0.2f temp_ETA : %0.2f\n",temp_system, ETA_east, temp_ETA);
												//outputlog(tmp_log); //cout<<tmp_log;
												
												
												byte tmp_peer_data[500];
												int size=0;
					
												PackpeerReq(tmp_peer_data, size,temp_vehID, vehclass, temp_ETA, peer_Phase);  //Pack_Event_List(tmp_event_data, size);
										
												char* peer_data;
												peer_data= new char[size];			
												for(int i=0;i<size;i++)
													peer_data[i]=tmp_peer_data[i];	
						
												sendto(sockfd_upstream, peer_data, size+1, 0,(struct sockaddr *)&recvaddr_upstream, addr_length_upstream);
												
												delete[] peer_data;		
												
												//!!! Get rid of the req sent from "Local_Peer_ReqList"
												Local_Peer_ReqList.Reset(Local_Peer_ReqList.CurrentPosition());
												Local_Peer_ReqList.DeleteAt();
												//PrintList2File_down(peerrequests,Local_Peer_ReqList);
												
												break;
												//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!										
											}	
											else
											{
												sprintf(tmp_log,"Send the local peer request to the downstream on eastbound after bus stop!!\n");
												outputlog(tmp_log); //cout<<tmp_log;
												
												break;
											}
											
										}
										else   //!!! B.H : No bus stop on eastbound
										{
											if (trackedveh.Data().previous_approach != trackedveh.Data().approach-1)
											{
												sprintf(tmp_log,"Now send the local peer request to the downstream on eastbound!!\n");
												outputlog(tmp_log); 
												//cout<<"Now send the local peer request to the downstream on eastbound!! "<<endl;
												
												recvaddr_upstream.sin_family = AF_INET;
												recvaddr_upstream.sin_port = htons(77777);  
												recvaddr_upstream.sin_addr.s_addr = inet_addr(eastbound_IP) ; //
												memset(recvaddr_upstream.sin_zero,'\0',sizeof recvaddr_upstream.sin_zero);
												
												int temp_vehID = Local_Peer_ReqList.Data().VehID;
												double temp_system = time(NULL);
												double temp_ETA = ETA_east + temp_system;
												int vehclass = Local_Peer_ReqList.Data().VehClass;
												
												int peer_Phase = peer_reqPhase[0]; //eastbound request phase for downstream							
												
												//sprintf(tmp_log,"temp_system : %0.2f ETA_east : %0.2f temp_ETA : %0.2f\n",temp_system, ETA_east, temp_ETA);
												//outputlog(tmp_log); //cout<<tmp_log;
												
												
												byte tmp_peer_data[500];
												int size=0;
					
												PackpeerReq(tmp_peer_data, size,temp_vehID, vehclass, temp_ETA, peer_Phase);  //Pack_Event_List(tmp_event_data, size);
										
												char* peer_data;
												peer_data= new char[size];			
												for(int i=0;i<size;i++)
													peer_data[i]=tmp_peer_data[i];	
						
												sendto(sockfd_upstream, peer_data, size+1, 0,(struct sockaddr *)&recvaddr_upstream, addr_length_upstream);
												
												delete[] peer_data;		
												
												//!!! Get rid of the req sent from "Local_Peer_ReqList"
												Local_Peer_ReqList.Reset(Local_Peer_ReqList.CurrentPosition());
												Local_Peer_ReqList.DeleteAt();
												//PrintList2File_down(peerrequests,Local_Peer_ReqList);
												
												break;
												//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!										
											}	
											else
											{
												sprintf(tmp_log," Need to check the approach for eastbound, previous appraoch : %d current approach : %d \n", trackedveh.Data().previous_approach, trackedveh.Data().approach);
												outputlog(tmp_log); //cout<<tmp_log;
												
												break;
											}
										}
									}
									//!!!! Send the local peer req to the downstream on westbound 
									if (trackedveh.Data().approach == curr_approach_long[1]) 
									{
										//!!! In case that there is far-side bus stop
										if (lat_busStop_west[0] > 0)
										{
											if (trackedveh.Data().busStop_S == 1) //(is_busStop[1] == 0)
											{
												sprintf(tmp_log,"Now send the local peer request to the downstream on westbound!!\n");
												outputlog(tmp_log); //cout<<tmp_log;
												//cout<<"Now send the local peer request to the downstream on westbound!! "<<endl;
												
												recvaddr_upstream.sin_family = AF_INET;
												recvaddr_upstream.sin_port = htons(77777);  
												recvaddr_upstream.sin_addr.s_addr = inet_addr(westbound_IP) ; //
												memset(recvaddr_upstream.sin_zero,'\0',sizeof recvaddr_upstream.sin_zero);
												
												int temp_vehID = Local_Peer_ReqList.Data().VehID;	
												double temp_system = time(NULL);
												double temp_ETA = ETA_west + temp_system;
												int vehclass = Local_Peer_ReqList.Data().VehClass;
												
												int peer_Phase = peer_reqPhase[1]; //westbound request phase for downstream							
												
												byte tmp_peer_data[500];
												int size=0;
												
												
												PackpeerReq(tmp_peer_data, size,temp_vehID, vehclass, temp_ETA, peer_Phase);  //Pack_Event_List(tmp_event_data, size);
											
												
												char* peer_data;
												peer_data= new char[size];			
												for(int i=0;i<size;i++)
													peer_data[i]=tmp_peer_data[i];	
												
												
												sendto(sockfd_upstream, peer_data, size+1, 0,(struct sockaddr *)&recvaddr_upstream, addr_length_upstream);
												
																						
												delete[] peer_data;		
										
												//!!! Get rid of the req sent from "Local_Peer_ReqList"	
												Local_Peer_ReqList.Reset(Local_Peer_ReqList.CurrentPosition());
												Local_Peer_ReqList.DeleteAt();
												//PrintList2File_down(peerrequests,Local_Peer_ReqList);
												
												break;
												//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
											}	
											else
											{
												sprintf(tmp_log,"Send the local peer request to the downstream on westbound after bus stop!!\n");
												outputlog(tmp_log); //cout<<tmp_log;
												
												break;
											}
										}
										else
										{
											if (trackedveh.Data().previous_approach != trackedveh.Data().approach-1)
											{
												sprintf(tmp_log,"Now send the local peer request to the downstream on westbound!!\n");
												outputlog(tmp_log); //cout<<tmp_log;
												//cout<<"Now send the local peer request to the downstream on westbound!! "<<endl;
												
												recvaddr_upstream.sin_family = AF_INET;
												recvaddr_upstream.sin_port = htons(77777);  
												recvaddr_upstream.sin_addr.s_addr = inet_addr(westbound_IP) ; //
												memset(recvaddr_upstream.sin_zero,'\0',sizeof recvaddr_upstream.sin_zero);
												
												int temp_vehID = Local_Peer_ReqList.Data().VehID;	
												double temp_system = time(NULL);
												double temp_ETA = ETA_west + temp_system;
												int vehclass = Local_Peer_ReqList.Data().VehClass;
												
												int peer_Phase = peer_reqPhase[1]; //westbound request phase for downstream							
												
												byte tmp_peer_data[500];
												int size=0;
												
												
												PackpeerReq(tmp_peer_data, size,temp_vehID, vehclass, temp_ETA, peer_Phase);  //Pack_Event_List(tmp_event_data, size);
											
												
												char* peer_data;
												peer_data= new char[size];			
												for(int i=0;i<size;i++)
													peer_data[i]=tmp_peer_data[i];	
												
												
												sendto(sockfd_upstream, peer_data, size+1, 0,(struct sockaddr *)&recvaddr_upstream, addr_length_upstream);
												
																						
												delete[] peer_data;		
										
												//!!! Get rid of the req sent from "Local_Peer_ReqList"	
												Local_Peer_ReqList.Reset(Local_Peer_ReqList.CurrentPosition());
												Local_Peer_ReqList.DeleteAt();
												//PrintList2File_down(peerrequests,Local_Peer_ReqList);
												
												break;
												//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
											}	
											else
											{
												sprintf(tmp_log," Need to check the approach for westbound, previous appraoch : %d current approach : %d \n", trackedveh.Data().previous_approach, trackedveh.Data().approach);
												outputlog(tmp_log); //cout<<tmp_log;
												
												break;
											}
										}
										
									}
								}
							}
							trackedveh.Next();
							//!!! B.H : If it could not find the req veh trajectory in the BSM, remove the req list from the local peer req list
							if(trackedveh.EndOfList()==1)
							{
								sprintf(tmp_log,"End of List, but could not find the ID of req veh ( %d ) matching to the trajectory!. The transit might dwell in the bus stop \n", Local_Peer_ReqList.Data().VehID);
								outputlog(tmp_log); //cout<<tmp_log;
								
								if (GetSeconds() -	Local_Peer_ReqList.Data().fSetRequestTime > 120)
								{
									sprintf(tmp_log," Error 1 !! Get rid of this local req from Local_Peer_ReqList since the local req has been in the list over two minutes : %d\n", Local_Peer_ReqList.Data().VehID);
									outputlog(tmp_log); //cout<<tmp_log;
								
									Local_Peer_ReqList.Reset(Local_Peer_ReqList.CurrentPosition());
									Local_Peer_ReqList.DeleteAt();
									//PrintList2File_down(peerrequests,Local_Peer_ReqList);
								}
							}
								
						}
					}
					else //!!! nothing in the trackedveh
					{
						//sprintf(tmp_log,"Nothing is in the trajectory list from BSM receiver when there is a peer local list!!\n");
						//outputlog(tmp_log);
						
						if (GetSeconds() -	Local_Peer_ReqList.Data().fSetRequestTime > 120)
						{
							sprintf(tmp_log," Error 2 !! Get rid of this local req from Local_Peer_ReqList since the local req has been in the list over two minutes : %d\n", Local_Peer_ReqList.Data().VehID);
							outputlog(tmp_log); //cout<<tmp_log;
						
							Local_Peer_ReqList.Reset(Local_Peer_ReqList.CurrentPosition());
							Local_Peer_ReqList.DeleteAt();
							//PrintList2File_down(peerrequests,Local_Peer_ReqList);
						}
						
					}
					Local_Peer_ReqList.Next();	
				}
			}
			
			//!!! B.H : First checking point if requests from upstream enter into DSRC range (which mean the req was received by trajectory awareness component)
			//!!!       => In case that the upstream req enter DSRC range, remove the req from "Upstream_Peer_ReqList"
			if (Upstream_Peer_ReqList.ListSize()>0) 
			{
				//!!! B.H : Update the short term request to compare with peer requests
				//!!!       => Using trajectoryawareness data .......................
				
				
				//!!! request trajectory data Only if the interval is over 1 second !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//if ((t_1 - updateTime) > 0.5)
				//{	
				//	updateTime = GetSeconds();
				
					sprintf(tmp_log,"Received trajectory data when there exist peer req from upstream : %5.2lf \n", GetSeconds());
					outputlog(tmp_log); //cout<<tmp_log;
					
					char buf[64]="Request Trajectory from Long_Term_Planning";
					recvaddr_trajectory.sin_port = htons(UDP_PORT);
					
					sendto(sockfd_trajectory,buf,sizeof(buf) , 0,(struct sockaddr *)&recvaddr_trajectory, addr_length_trajectory);

					recv_data_len_trajectory = recvfrom(sockfd_trajectory, buf_traj, sizeof(buf_traj), 0,
									(struct sockaddr *)&sendaddr_trajectory, (socklen_t *)&addr_length_trajectory);
					
					if(recv_data_len_trajectory<0)
					{
						//printf("Receive Request failed\n");
						sprintf(tmp_log,"Receive Request failed\n");
						//cout<<tmp_log; 
						outputlog(tmp_log);
						// continue;
					}
					trackedveh.ClearList(); 
					UnpacktrajData3(buf_traj);
				//}
					
				
				//!!! B.H : Update the "Upstream_Peer_ReqList" while checking the short term list (If it is in the short term list, remove it from "Upstream_Peer_ReqList"
				//void UpdateList_peer(LinkedList <ConnectedVehicle> &trackedveh, LinkedList<ReqEntry> &Upstream_Peer_ReqList);
				UpdateList_peer(trackedveh,Upstream_Peer_ReqList);
				
				//!!! B.H : write the peer req in "requests_long.txt" so that PRS checks if there are peer requests coming before the PRS release the NTCIP command (send new event list with all zeros to the traffic interface)
				//PrintList2File_up(request_combined_filename_long,Upstream_Peer_ReqList); 
				PrintList2File_up(requestfilename_long,Upstream_Peer_ReqList); 
				
				
			}
			
			//!!! B.H : Keep sending "adCPs_long" to Solver only when there exist peer requests in the "Upstream_Peer_ReqList"
			if (Upstream_Peer_ReqList.ListSize()>0) 
			{
				
									
				
				int size_CP=0;
				packCPforsolver(Cp_buffer,Upstream_Peer_ReqList,size_CP);
				
				recvaddr_solver.sin_family = AF_INET;
				recvaddr_solver.sin_port = htons(PeerPort);  //PeerPort = 80000;
				//recvaddr_upstream.sin_addr.s_addr = inet_addr(eastbound_IP) ; //
				recvaddr_solver.sin_addr.s_addr = inet_addr("127.0.0.1") ; //INADDR_BROADCAST;
				memset(recvaddr_solver.sin_zero,'\0',sizeof recvaddr_solver.sin_zero);
				
				if ( (sendto(sockfd_solver, Cp_buffer, size_CP+1, 0,(struct sockaddr *)&recvaddr_solver, addr_length_solver) > 0) ) //sendto(sockfd, cOPTbuffer,OPT_MSG_SIZE, 0,(struct sockaddr *)&recvaddr, addr_length)>0)                     
				{
					sprintf(tmp_log," The peer requests were sent to SOLVER At time: %.2f.......... \n", GetSeconds());
					outputlog(tmp_log);
				}
				
			}
			else //!!! B.H : for matlab gui => if there's nothing in peer req list, "ReqNo" should be equal to -100 
			{
				FILE * fp5=fopen(logfilename2,"w"); // For determination of connecting to a ASC controller or not. Nothing to do with OBU
				fprintf(fp5,"ReqNo %d \n",-100);
				fclose(fp5);
				//int error_num3 = fclose(fp5); 
				//sprintf(tmp_log,"error_num3: %d \n",error_num3);
				//outputlog(tmp_log);
				//msleep(20);
			}
		
			
		}
		else  // // We do not connect to RSE
		{
			sprintf(tmp_log,"******Cannot connect to RSE, At time: [%.2f].\n",GetSeconds());
			//cout<<tmp_log;      
			outputlog(tmp_log);
			msleep(400);
			continue;
		}  // end of  if( (t_2-t_1)<2.0 ) // We do connect to RSE
		
		//!!! B.H : Get rid of this later !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
		//msleep(3000-(1000*(GetSeconds()-TestTime)));
		
		
		
		
    }  // end of While(true)

    fs_log.close();
    //fs_log2.close();
    fs_signal_plan.close();
    return 0;
}



					
int outputlog(char *output)
{
    FILE * stream = fopen( logfilename, "r" );
    fseek( stream, 0L, SEEK_END );
    long endPos = ftell( stream );
    fclose( stream );

    std::fstream fs;
    if (endPos <100000000)
        fs.open(logfilename, std::ios::out | std::ios::app);
    else
        fs.open(logfilename, std::ios::out | std::ios::trunc);
    if (!fs || !fs.good())
    {
        std::cout << "could not open file!\n";
        return -1;
    }
    fs << output;// << std::endl;

    if (fs.fail())
    {
        std::cout << "failed to append to file!\n";
        return -1;
    }
    fs.close();
    return 1;
}

int outputlog2(char *output)
{
    FILE * stream = fopen( logfilename2, "r" );
    fseek( stream, 0L, SEEK_END );
    long endPos = ftell( stream );
    fclose( stream );

    std::fstream fs2;
    if (endPos <10000000)
        fs2.open(logfilename2, std::ios::out | std::ios::app);
    else
        fs2.open(logfilename2, std::ios::out | std::ios::trunc);
    if (!fs2 || !fs2.good())
    {
        std::cout << "could not open file!\n";
        return -1;
    }
    fs2 << output;// << std::endl;

    if (fs2.fail())
    {
        std::cout << "failed to append to file!\n";
        return -1;
    }
    fs2.close();
    return 1;
}


//!!! B.H : Send the CP to SOlver
void packCPforsolver(char* CP_buffer,LinkedList<ReqEntry> Req_List, int &size_CP)   //void packCPforsolver(char* CP_buffer,double cp[2][3][16], int msgCnt); 
{
	
	char tmp_string[256];
	
	
	int k=0;
	
	int temp_Size= Req_List.ListSize();

	int* OBU_ID;
	OBU_ID = new int[temp_Size];
	int* Veh_Class;
	Veh_Class = new int[temp_Size];
	int* Req_Phase;
	Req_Phase = new int[temp_Size];
	double* ETA;
	ETA = new double[temp_Size];
	double* temp_ETA;
	temp_ETA = new double[temp_Size];
	
	
	//int OBU_ID=0; //!!! OBU_ID[128];
    //int Veh_Class=0;
    //int Req_Phase=0;
    //double ETA=0.0; 
    //double temp_ETA;
	
    
    int Req_SplitPhase=0; //int abs_time=0;
    float fsetrequesttime=0.0;
    float MinGrn=0.0; int iInLane=0; int iOutLane=0;
	int iStrHour=0; int iStrMinute=0; int iStrSecond=0; int iEndHour=0;
	int iEndMinute=0; int iEndSecond=0;	int iVehState=0; int iMsgCnt=0;	 
		
	Req_List.Reset();
	for(int jj=0;jj<temp_Size;jj++)
	{
		sprintf(tmp_string,"%d %d %lf %d \n",Req_List.Data().VehID,Req_List.Data().VehClass,Req_List.Data().ETA,Req_List.Data().Phase);
				
		sscanf(tmp_string,"%d %d %lf %d ",&OBU_ID[jj],&Veh_Class[jj], &ETA[jj], &Req_Phase[jj]);
					
        //!!! B.H : Change the ETA of peer req from upstream back to the relevant time (Not absolute system time)
		temp_ETA[jj] = ETA[jj] - time(NULL);					
		
		if (temp_ETA[jj] < 0)
		{
			sprintf(tmp_log,"ETA error : %5.2lf \n", temp_ETA[jj]); 
			outputlog(tmp_log);
		}
		
		Req_List.Next();
	}
	
	
		
	
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    byte    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
    float   tempFloat;
    //header 2 bytes
	CP_buffer[offset]=0xFF;
	offset+=1;
	CP_buffer[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x03 for signal event data send to Signal Control Interface
	CP_buffer[offset]=0x03;
	offset+=1;
	
	//No. peer requests 
	//int No_peer= 1; //Local_Peer_ReqList.ListSize();
	tempUShort = (unsigned short)temp_Size;
	pByte = (byte* ) &tempUShort;
    CP_buffer[offset+0] = (byte) *(pByte + 1); 
    CP_buffer[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
    
    
    
    for (int k=0; k<temp_Size; k++)
    {
		//.Data().VehID, .Data().VehClass, .Data().ETA, .Data().Phase
		
		//VehID 
		tempUShort = (unsigned short)OBU_ID[k];
		pByte = (byte* ) &tempUShort;
		CP_buffer[offset+0] = (byte) *(pByte + 1); 
		CP_buffer[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		
		//VehClass
		tempUShort = (unsigned short)Veh_Class[k];
		pByte = (byte* ) &tempUShort;
		CP_buffer[offset+0] = (byte) *(pByte + 1); 
		CP_buffer[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		
		//ETA
		tempLong = (long)(temp_ETA[k] * DEG2ASNunits); 
		pByte = (byte* ) &tempLong;
		CP_buffer[offset+0] = (byte) *(pByte + 3); 
		CP_buffer[offset+1] = (byte) *(pByte + 2); 
		CP_buffer[offset+2] = (byte) *(pByte + 1); 
		CP_buffer[offset+3] = (byte) *(pByte + 0); 
		offset = offset + 4;
		
		//Phase
		tempUShort = (unsigned short)Req_Phase[k];
		pByte = (byte* ) &tempUShort;
		CP_buffer[offset+0] = (byte) *(pByte + 1); 
		CP_buffer[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
	
	    //sprintf(tmp_log,"pack test for Solver k : %d OBU_ID :%d Vehclass : %d eta : %0.2f peerPhase : %d \n",k, OBU_ID[k],Veh_Class[k],temp_ETA[k],Req_Phase[k]);
		//outputlog(tmp_log); //cout<<tmp_log;
		
	}

	size_CP = offset;
	
}


void PrintList2File_down(char *Filename)
{
	ofstream fs;
    //iostream fs;
    fs.open(Filename);
    
    if(!fs.good())
    {
		sprintf(tmp_log,"Error to open Peer_requests.txt !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
		outputlog(tmp_log);	
	}
	    
	//!!! B.H : after adding the req to the list, remove it from "Peer_requests.txt"
	fs<<"Local_Num_req  "<<0<<" "<<55<<endl;
	
    fs.close();
    
    //int error_num = fs.fail();
   //sprintf(tmp_log,"1-1 error_num : %d\n",error_num);
	//outputlog(tmp_log);

}

//!!! B.H : To write the peer request received from upstream
void PrintList2File_up(char *Filename,LinkedList<ReqEntry> &Up_ReqList)	//PrintList2File_up(requestfilename_long,Upstream_Peer_ReqList); 
{
	ofstream fs;
    //iostream fs;
    fs.open(Filename);
    if(fs.good())
    {
		
        fs<<"Upstream_Peer_Num_req  "<<Up_ReqList.ListSize()<<" "<<100<<endl;  //!!! *** IMPORTANT: the peer Flag changed to "-100": means keep solving ***//        
        
        if (!Up_ReqList.ListEmpty())
        {
			Up_ReqList.Reset();
            
            while(!Up_ReqList.EndOfList())
            {
				if (Up_ReqList.Data().VehClass ==6)
				{
					fs<<Up_ReqList.Data().VehID<<"  "<<Up_ReqList.Data().VehClass<<"  "<<Up_ReqList.Data().ETA<<"  "<<Up_ReqList.Data().Phase                        
						<<" "<<Up_ReqList.Data().MinGreen<<" "<<Up_ReqList.Data().fSetRequestTime<<" "<<Up_ReqList.Data().Split_Phase<<" "<<Up_ReqList.Data().iInLane<<" "<<Up_ReqList.Data().iOutLane
						<<" "<<Up_ReqList.Data().iStrHour<<" "<<Up_ReqList.Data().iStrMinute<<" "<<Up_ReqList.Data().iStrSecond<<" "<<Up_ReqList.Data().iEndHour
						<<" "<<Up_ReqList.Data().iEndMinute<<" "<<Up_ReqList.Data().iEndSecond<<" "<<Up_ReqList.Data().iVehState<<" "<<Up_ReqList.Data().iMsgCnt<< " " << endl;
				}
                else
                {
					fs<<Up_ReqList.Data().VehID<<"  "<<Up_ReqList.Data().VehClass<<"  "<<Up_ReqList.Data().ETA<<"  "<<Up_ReqList.Data().Phase                        
					<<" "<<Up_ReqList.Data().MinGreen<<" "<<Up_ReqList.Data().fSetRequestTime<<" "<<Up_ReqList.Data().Split_Phase<<" "<<Up_ReqList.Data().iInLane<<" "<<Up_ReqList.Data().iOutLane
					<<" "<<Up_ReqList.Data().iStrHour<<" "<<Up_ReqList.Data().iStrMinute<<" "<<Up_ReqList.Data().iStrSecond<<" "<<Up_ReqList.Data().iEndHour
					<<" "<<Up_ReqList.Data().iEndMinute<<" "<<Up_ReqList.Data().iEndSecond<<" "<<Up_ReqList.Data().iVehState<<" "<<Up_ReqList.Data().iMsgCnt<< " " << endl;
				}
                Up_ReqList.Next();
            }
        }
	}
    else
	{
		sprintf(tmp_log,"Error in PrintList2File_up !!!!!!!!!!!!!!!!!!!!!!!!!11!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  \n");
		outputlog(tmp_log); 
	}
    
    
    fs.close();

	//int error_num = fs.fail();
		//sprintf(tmp_log,"10-1 error_num : %d\n",error_num);
		//outputlog(tmp_log);

}



/*
void ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List) //!!! B.H : filename ="requests.txt"   <= ReqListFromFile(requestfilename,ReqList);
{
    fstream fss1;

    fss1.open(filename,fstream::in); //fss.open(filename,fstream::in);
    
    if(fss1.good())
    {
		ReqEntry req_temp;
		int ReqNo_test=-1;
		char RSU_ID[128];
		int OBU_ID; //!!! OBU_ID[128];
		int Veh_Class,Req_Phase,Req_SplitPhase;
		int abs_time;
		double ETA;
		float MinGrn;
		int iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt;	 
		
		string lineread;
		Req_List.ClearList();

		getline(fss1,lineread);
		sscanf(lineread.c_str(),"%*s %d %d",&ReqNo_test,&ReqListUpdateFlag);

		//cout<<"The total Requests is:"<<ReqNo<<endl;

		sprintf(tmp_log,"request number is: %d, Flag is{%d}\n",ReqNo_test,ReqListUpdateFlag);
		outputlog(tmp_log);


		while(!fss1.eof() && ReqNo_test>0 )
		{
			getline(fss1,lineread);
			if(lineread.size()!=0)
			{
				sscanf(lineread.c_str(),"%s %d %d %lf %d %f %d %d %d %d %d %d %d %d %d %d %d %d ",RSU_ID,&OBU_ID,&Veh_Class,
					&ETA,&Req_Phase,&MinGrn,&abs_time,&Req_SplitPhase,&iInLane,&iOutLane,&iStrHour,&iStrMinute,&iStrSecond,&iEndHour,&iEndMinute,&iEndSecond,&iVehState,&iMsgCnt);

				ReqEntry req_temp(OBU_ID,Veh_Class,ETA,Req_Phase,MinGrn,abs_time,Req_SplitPhase,iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt);

				Req_List.InsertAfter(req_temp);

				cout<<lineread<<endl;

				//outputlog(lineread.c_str());
			}
		}
	}
	else
	{
		sprintf(tmp_log,"Error to open ReqListFromFile  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
		outputlog(tmp_log);
	}
	
    fss1.close();
    
    int error_num = fss1.fail();
		sprintf(tmp_log,"9-2 error_num : %d\n",error_num);
		outputlog(tmp_log);
}
*/



/*
void ReqListFromFile_EV(char *filename,LinkedList<ReqEntry>& Req_List)  //!!! B.H : ReqListFromFile_EV(request_combined_filename,Req_List_Combined);
{
    fstream fss2;

    fss2.open(filename,fstream::in); //fss2.open(filename,fstream::in);
	
	if(fss2.good())
    {
		ReqEntry req_temp;
		int ReqNo_test=-1;
		char RSU_ID[128];
		int OBU_ID; //!!! OBU_ID[128];
		int Veh_Class,Req_Phase,Req_SplitPhase;
		int abs_time;
		double ETA;
		float MinGrn;
		int iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt;	 
		
		string lineread;
		Req_List.ClearList();

		getline(fss2,lineread);
		sscanf(lineread.c_str(),"%*s %d %d",&ReqNo_test,&ReqListUpdateFlag);

		sprintf(tmp_log,"request Combined number is: %d, Flag is{%d}\n",ReqNo_test,ReqListUpdateFlag);
		outputlog(tmp_log);

		while(!fss2.eof())
		{
			getline(fss2,lineread);
			if(lineread.size()!=0)
			{
				 sscanf(lineread.c_str(),"%s %d %d %lf %d %f %d %d %d %d %d %d %d %d %d %d %d %d ",RSU_ID,&OBU_ID,&Veh_Class,
					&ETA,&Req_Phase,&MinGrn,&abs_time,&iInLane,&iOutLane,&iStrHour,&iStrMinute,&iStrSecond,&iEndHour,&iEndMinute,&iEndSecond,&iVehState,&iMsgCnt);
				ReqEntry req_temp(OBU_ID,Veh_Class,ETA,Req_Phase,MinGrn,abs_time,0,iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt);
				Req_List.InsertAfter(req_temp);
				cout<<lineread<<endl;
				//outputlog(lineread.c_str());
			}
		}
	}
	else
	{
		sprintf(tmp_log,"Error to open ReqListFromFile_EV  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
		outputlog(tmp_log);
	}
		
    fss2.close();
    
    int error_num = fss2.fail();
		sprintf(tmp_log,"10-2 error_num : %d\n",error_num);
		outputlog(tmp_log);
    
    
}
*/



//!!! B.H : For peer request from upstream => Peer_ReqListFromFile(requestfilename_long,Upstream_Peer_ReqList); 
void UpstreamPeer_ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List1) 
{
	
	fstream fss3;

    fss3.open(filename,ios::in); //fss3.open(filename,fstream::in);
    
    if (fss3.good())
    { 
		ReqEntry req_temp;
		//int Peer_ReqNo;
		char RSU_ID[128];
		 int OBU_ID; //!!! OBU_ID[128];
		int Veh_Class,Req_Phase,Req_SplitPhase;
		//int abs_time;
		float fsetrequesttime;
		double ETA;
		float MinGrn;
		int iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt;	 
		
		string lineread;
		Req_List1.ClearList();

		getline(fss3,lineread);
		sscanf(lineread.c_str(),"%*s %d %d",&UpstreamPeer_ReqNo,&Peer_ReqListUpdateFlag);
		

		while(!fss3.eof() && UpstreamPeer_ReqNo>0)
		{
			getline(fss3,lineread);
			if(lineread.size()!=0)
			{
				sscanf(lineread.c_str(),"%d %d %lf %d %f %f %d %d %d %d %d %d %d %d %d %d %d ",&OBU_ID,&Veh_Class,
					&ETA,&Req_Phase,&MinGrn,&fsetrequesttime,&Req_SplitPhase,&iInLane,&iOutLane,&iStrHour,&iStrMinute,&iStrSecond,&iEndHour,&iEndMinute,&iEndSecond,&iVehState,&iMsgCnt);

				ReqEntry req_temp(OBU_ID,Veh_Class,ETA,Req_Phase,MinGrn,fsetrequesttime,Req_SplitPhase,iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt);

				Req_List1.InsertAfter(req_temp);

				sprintf(tmp_log,"Test 4 requests_long.txt ETA : %lf\n",ETA);
				outputlog(tmp_log); //cout<<tmp_log;
		
			}
		}
	}
	else
	{
		sprintf(tmp_log,"Error to open UpstreamPeer_ReqListFromFile  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
		outputlog(tmp_log);
	}
	
    fss3.close();
    
    //int error_num = fss3.fail();
		//sprintf(tmp_log,"11 error_num : %d\n",error_num);
		//outputlog(tmp_log);
}

//!!! B.H : For local Peer request ("request_clear" from PRS)  => Local_Peer_ReqList; peerrequests
void LocalPeer_ReqListFromFile(LinkedList<ReqEntry>& Req_List2)  //LocalPeer_ReqListFromFile(Local_Peer_ReqList_temp);
{
	
    fstream fs;

    fs.open(peerrequests, fstream::in ); // fs.open(filename,fstream::in); "/nojournal/bin/Peer_requests.txt"
    
    if (fs.fail() != 0|| fs.bad() != 0|| fs.rdstate() != 0)
	{
		sprintf(tmp_log,"Error in Peer_requests.txt !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
		outputlog(tmp_log);
	}

    ReqEntry req_temp;
    //int Peer_ReqNo;
    char RSU_ID[128];
     int OBU_ID; //!!! OBU_ID[128];
    int Veh_Class,Req_Phase,Req_SplitPhase;
    //int abs_time;
    float fsetrequesttime; 
    double ETA;
    float MinGrn;
	int iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt;	 
	
    string lineread;
    Req_List2.ClearList();

    getline(fs,lineread);
    sscanf(lineread.c_str(),"%*s %d %d",&LocalPeer_ReqNo,&Peer_ReqListUpdateFlag);
	
	if (LocalPeer_ReqNo >0)
	{
		while(!fs.eof())
		{
			getline(fs,lineread);
			if(lineread.size()!=0)
			{
				sscanf(lineread.c_str(),"%d %d %lf %d %f %f %d %d %d %d %d %d %d %d %d %d %d ",&OBU_ID,&Veh_Class,
					&ETA,&Req_Phase,&MinGrn,&fsetrequesttime,&Req_SplitPhase,&iInLane,&iOutLane,&iStrHour,&iStrMinute,&iStrSecond,&iEndHour,&iEndMinute,&iEndSecond,&iVehState,&iMsgCnt);

				ReqEntry req_temp(OBU_ID,Veh_Class,ETA,Req_Phase,MinGrn,fsetrequesttime,Req_SplitPhase,iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt);

				Req_List2.InsertAfter(req_temp);
				
				cout<<lineread<<endl;

				sprintf(tmp_log,"request_clear was written by PRS on Peer_Requests.txt OBU_ID : %d\n",OBU_ID);
				outputlog(tmp_log); //cout<<tmp_log;
		
			}
		}
	}
    
    fs.close();
    
    //int error_num = fs.fail();
    //sprintf(tmp_log,"1 error_num : %d\n",error_num);
	//outputlog(tmp_log);
	
	
    
}


//void PrintFile2Log(char *resultsfile) //PrintFile2Log(prioritydatafile_long);
void PrintFile2Log(char *resultsfile_long) //!!! B.H : PrintFile2Log(request_combined_filename); PrintFile2Log(requestfilename); PrintFile2Log(prioritydatafile); // Log the NewModel.dat file for glpk solver
{
    fstream fss;
    fss.open(resultsfile_long,fstream::in); //!!! B.H : fss.open(resultsfile,fstream::in);

    if (!fss)
    {
        sprintf(tmp_log,"***********Error opening the plan file in order to print to a log file!\n");   
        outputlog(tmp_log);
        exit(1);
    }
    string lineread;
    
    while(!fss.eof())
    {
        getline(fss,lineread);
        strcpy(tmp_log,lineread.c_str());	
        strcat(tmp_log,"\n"); outputlog(tmp_log);		
    }   

    fss.close();

}

//void PrintPlan2Log(char *resultsfile)
void PrintPlan2Log(char *resultsfile_long)
{
    fstream fss;
    fss.open(resultsfile_long,fstream::in);   //!!! B.H : fss.open(resultsfile,fstream::in);

    if (!fss)
    {
        sprintf(tmp_log,"***********Error opening the plan file in order to print to a log file!\n");   
        outputlog(tmp_log);
        exit(1);
    }

    string lineread;
    
    while(!fss.eof())
    {
        getline(fss,lineread);
        strcpy(tmp_log,lineread.c_str());	
        strcat(tmp_log,"\n"); 
        //cout<<tmp_log<<endl;
        outputlog(tmp_log);		
    }   

    fss.close();

}


//!!! B.H : To read Lane_Peer_Mapping.txt
void readlanepeermapping() 
{
    fstream fs;

    fs.open(lanepeermapping, ios::in);

	if (!fs)
    {
        sprintf(tmp_log," ***********Error opening lane peer mapping file!\n");
		outputlog(tmp_log);	
        exit(1); 
    }
            
    char temp[128];
	
	string lineread;

    getline(fs,lineread);  //First line is comment
	getline(fs,lineread);  //Second line is comment
	
	getline(fs,lineread);  
    sscanf(lineread.c_str(),"%d %d %d %lf %lf %lf %lf %lf %s",&pre_approach_long[0],&curr_approach_long[0], &peer_reqPhase[0], &lat_busStop_east[0], &long_busStop_east[0], &lat_busStop_east[1], &long_busStop_east[1], &ETA_east, &eastbound_IP);
    getline(fs,lineread);
    sscanf(lineread.c_str(),"%d %d %d %lf %lf %lf %lf %lf %s",&pre_approach_long[1],&curr_approach_long[1], &peer_reqPhase[1], &lat_busStop_west[0], &long_busStop_west[0], &lat_busStop_west[1], &long_busStop_west[1], &ETA_west, &westbound_IP);    
    
    //getline(fs,lineread);
    //sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",&phase_serving[0],&phase_serving[1], &phase_serving[2], &phase_serving[3], &phase_serving[4], &phase_serving[5], &phase_serving[6], &phase_serving[7]);
    
    
    fs.close();
    
    //int error_num = fs.fail();
    //sprintf(tmp_log,"4 error_num : %d\n",error_num);
	//outputlog(tmp_log);
    
}


//!!! B.H : unpacking the data for trajectory awareness
void UnpacktrajData3(char* ablob)
{
   // void UnpackPGData(char* ablob, int& No_port, double (&probability_Green_intersection) [120][2][6], int (&cyclic_Arrivalflow_intersection) [120][2][6])
   
	//sprintf(tmp_log,"Test Unpack %d \n",Inter);
	//outputlog(tmp_log); cout<<tmp_log;
    
    int port_num; 
    port_num = atoi(INTport);
    
    //sprintf(tmp_log,"Test port_num %d \n", port_num);
	//outputlog(tmp_log); cout<<tmp_log;
		
    int total_num;
	int offset=0;
	//int i,j;
	int k;
	unsigned short   tempUShort; // temp values to hold data in final format
	long    tempLong;
	//float tempFloat;
	unsigned char   byteA;  // force to unsigned this time,
	unsigned char   byteB;  // we do not want a bunch of sign extension 
    unsigned char   byteC;  // math mucking up our combine logic
	unsigned char   byteD;
	
	//Header
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	int temp = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;
	//message id
	temp = (char)ablob[offset];
	offset = offset + 1; // move past to next item
	
	//Do vehicle number
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	total_num = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;
	
	//Do each vehicle
	for(k=0;k<total_num;k++)
	{		
		
		ConnectedVehicle TempVeh;
		
		TempVeh.intersection = port_num;
		//TempVeh.simulation_time = simulation_currenttime;
		//TempVeh.int_num = No_port
		
		//Do Veh ID
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.TempID = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		
		//do speed
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.Speed = (tempLong /  DEG2ASNunits); // convert and store as float
		offset = offset + 4;
		
		//do distance to stop bar
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.stopBarDistance = (tempLong /  DEG2ASNunits); // convert and store as float
		offset = offset + 4;
		
		//do acceleration
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.acceleration = (tempLong /  DEG2ASNunits); // convert and store as float
		offset = offset + 4;
		
		//do req_phase
		TempVeh.req_phase = (char)ablob[offset];
	    offset = offset + 1; // move past to next item
		
		//Do Veh Previous Approach
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.previous_approach = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		
		//Do Veh Approach
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.approach = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		
		//parameter S for bus stop
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.busStop_S = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;		
		
		
		trackedveh.InsertRear(TempVeh);   //add the new vehicle to the tracked list	
	
	}
			
}

void PackpeerReq(char* tmp_peer_data, int &size, int vehid, int Vehclass, double eta, int peerPhase) //PackpeerReq(tmp_peer_data, size,temp_vehID, vehclass, temp_ETA, peer_Phase); 
{
	int k;
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    byte    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
    float   tempFloat;
    //header 2 bytes
	tmp_peer_data[offset]=0xFF;
	offset+=1;
	tmp_peer_data[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x03 for signal event data send to Signal Control Interface
	tmp_peer_data[offset]=0x03;
	offset+=1;
	
	//No. peer requests 
	//!!! B.H : Send the peer req one by one
	int No_peer= 1; //Local_Peer_ReqList.ListSize();
	tempUShort = (unsigned short)No_peer;
	pByte = (byte* ) &tempUShort;
    tmp_peer_data[offset+0] = (byte) *(pByte + 1); 
    tmp_peer_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
    
    for (k=0; k<No_peer; k++)
    {
		//.Data().VehID, .Data().VehClass, .Data().ETA, .Data().Phase
		
		//VehID 
		tempUShort = (unsigned short)vehid;
		pByte = (byte* ) &tempUShort;
		tmp_peer_data[offset+0] = (byte) *(pByte + 1); 
		tmp_peer_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		
		//VehClass
		tempUShort = (unsigned short)Vehclass;
		pByte = (byte* ) &tempUShort;
		tmp_peer_data[offset+0] = (byte) *(pByte + 1); 
		tmp_peer_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		
		//ETA
		tempLong = (long)eta; 
		pByte = (byte* ) &tempLong;
		tmp_peer_data[offset+0] = (byte) *(pByte + 3); 
		tmp_peer_data[offset+1] = (byte) *(pByte + 2); 
		tmp_peer_data[offset+2] = (byte) *(pByte + 1); 
		tmp_peer_data[offset+3] = (byte) *(pByte + 0); 
		offset = offset + 4;
		
		//Phase
		tempUShort = (unsigned short)peerPhase;
		pByte = (byte* ) &tempUShort;
		tmp_peer_data[offset+0] = (byte) *(pByte + 1); 
		tmp_peer_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
	}
	
	sprintf(tmp_log,"pack test  vehid :%d Vehclass : %d eta : %0.2f peerPhase : %d \n",vehid,Vehclass,eta,peerPhase);
	outputlog(tmp_log); //cout<<tmp_log;
										
	
	size=offset;
}


void UnpackpeerReq(char* ablob)
{
    int total_num;
	int offset=0;
	//int i,j;
	int k;
	byte    tempByte; 
	byte    tempByte1[64];
	unsigned short   tempUShort; // temp values to hold data in final format
	long    tempLong;
	float   tempFloat;
	//float tempFloat;
	unsigned char   byteA;  // force to unsigned this time,
	unsigned char   byteB;  // we do not want a bunch of sign extension 
    unsigned char   byteC;  // math mucking up our combine logic
	unsigned char   byteD;
	
	//Header
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	int temp = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;
	//message id
	temp = (char)ablob[offset];
	offset = offset + 1; // move past to next item
	
	//Do vehicle number
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	total_num = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;
	
	//Upstream_Peer_ReqList.Data().VehID,Upstream_Peer_ReqList.Data().VehClass,Upstream_Peer_ReqList.Data().ETA,Upstream_Peer_ReqList.Data().Phase
	
	//Do each vehicle
	for(k=0;k<total_num;k++)
	{		
		ReqEntry TempVeh;
		
		//Do Veh ID
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.VehID = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		
		//do VehClass
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.VehClass = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		
		//do ETA
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.ETA = (tempLong); // convert and store as float
		offset = offset + 4;		
		
		//Do phase
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.Phase = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		
		Upstream_Peer_ReqList.InsertRear(TempVeh);   //add the new vehicle to the tracked list	
	
	}
			
}

//!!! B.H : 
//UpdateList_peer(trackedveh,Upstream_Peer_ReqList);
void UpdateList_peer(LinkedList <ConnectedVehicle> &trackedveh, LinkedList<ReqEntry> &Upstream_Peer_ReqList)
{
	
	//!!! B.H : Checking whether the peer req from upstream is received by short term PRS (if it is in the "ReqList")
	if ((!Upstream_Peer_ReqList.ListEmpty()) && (!trackedveh.ListEmpty()))
    {
		
		//!!! Test : Remove later !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//trackedveh.Reset();
		//while(!trackedveh.EndOfList())
		//{
		//	sprintf(tmp_log,"test2 Int : %d ID : %d Req : %d previous approach : %d approach : %d Veh_type : %d busStop_S : %d \n",trackedveh.Data().intersection, trackedveh.Data().TempID,trackedveh.Data().req_phase, trackedveh.Data().previous_approach, trackedveh.Data().approach ,trackedveh.Data().VehType, trackedveh.Data().busStop_S);
		//	outputlog(tmp_log); //cout<<tmp_log;
		//			
		//	trackedveh.Next();	
		//}
		
		Upstream_Peer_ReqList.Reset();
		while (!Upstream_Peer_ReqList.EndOfList())
		{
			trackedveh.Reset();
			while (!trackedveh.EndOfList())
			{
			
				if (trackedveh.Data().TempID == Upstream_Peer_ReqList.Data().VehID)  //!!! if (strcmp(ReqList.Data().VehID, Upstream_Peer_ReqList.Data().VehID) ==0)
				{
					if (trackedveh.Data().req_phase > 0)
					{
						Upstream_Peer_ReqList.Reset(Upstream_Peer_ReqList.CurrentPosition());
						Upstream_Peer_ReqList.DeleteAt();
						sprintf(tmp_log,"Stop long term planning process because it it in DSRC range (Get rid of the peer request from the peer req list) ID : %d Stopbardistance : %5.2lf\n", trackedveh.Data().TempID, trackedveh.Data().stopBarDistance); 
						outputlog(tmp_log);
						break; 
					}
					else if (trackedveh.Data().req_phase == 0)
					{
						sprintf(tmp_log,"Keep performaing long term planning process because it has not touched the last nmap point ID : %d rep_phase : %d\n", Upstream_Peer_ReqList.Data().VehID, trackedveh.Data().req_phase); 
						outputlog(tmp_log);
					}
					else //request_phase = -1
					{
						Upstream_Peer_ReqList.Reset(Upstream_Peer_ReqList.CurrentPosition());
						Upstream_Peer_ReqList.DeleteAt();
						sprintf(tmp_log,"Error - Stop long term planning process because it has passed the stop-bar ID : %d Stopbardistance : %5.2lf\n", trackedveh.Data().TempID, trackedveh.Data().stopBarDistance); 
						outputlog(tmp_log);
						break; 
					}
				}
				trackedveh.Next();	
				if (trackedveh.EndOfList() ==1 )
				{
					sprintf(tmp_log,"Keep performaing long term planning process because it is still out of the DSRC range  ID : %d\n", Upstream_Peer_ReqList.Data().VehID); 
					outputlog(tmp_log); 
				}
			}
			Upstream_Peer_ReqList.Next();
		}
	}
	
	
}


//UpdatePeerList2File(peerrequests,Local_Peer_ReqList, Local_Peer_ReqList_temp);
void UpdatePeerList2File(LinkedList<ReqEntry> &Local_List, LinkedList<ReqEntry> &Local_List_temp) 
{   
    /*
    //!!! B.H : add the local request from Peer_requests.txt to "Local_Peer_reqList"
    if (!Local_List_temp.ListEmpty())
	{
		
		Local_List_temp.Reset();
		
		ReqEntry req_temp;
		
		req_temp = Local_List_temp.Data();
		
		Local_List.InsertRear(req_temp);
		
	}
	*/
	
	if (!Local_List_temp.ListEmpty())
	{
		Local_List_temp.Reset();
		while(!Local_List_temp.EndOfList())
		{
			if (Local_List.ListEmpty()==1)
			{
				//Local_List_temp.Reset();
		
				ReqEntry req_temp;
				
				req_temp = Local_List_temp.Data();
				
				Local_List.InsertRear(req_temp);
				
				sprintf(tmp_log,"Add this req to the Local_list (No previous list exist) ID : %d\n", Local_List_temp.Data().VehID);
				outputlog(tmp_log); //cout<<tmp_log;
				
			}
			else
			{
				Local_List.Reset();			
				while(!Local_List.EndOfList())
				{
					if (Local_List_temp.Data().VehID == Local_List.Data().VehID)
					{
						sprintf(tmp_log,"Error !!! Received request_clear message twice !!!! Do not add the req to the local_list : %d\n", Local_List_temp.Data().VehID);
						outputlog(tmp_log); //cout<<tmp_log;	
						break;	
					}
					else
					{
						Local_List.Next();
						
						if (Local_List.EndOfList()==1)
						{
							//!!! B.H : add the local request from Peer_requests.txt to "Local_Peer_reqList"
							//Local_List_temp.Reset();
							
							ReqEntry req_temp;
								
							req_temp = Local_List_temp.Data();
								
							Local_List.InsertRear(req_temp);
								
							sprintf(tmp_log,"Add this req to the Local_list ID : %d\n", Local_List_temp.Data().VehID);
							outputlog(tmp_log); //cout<<tmp_log;
							
							break;
							
						}
					}	
					
				}
				
			}
			Local_List_temp.Next();
		}
	}
	else
	{
		sprintf(tmp_log,"Error !!! Received local peer request, but the list is empty\n");
		outputlog(tmp_log); //cout<<tmp_log;	
	}

	
}
