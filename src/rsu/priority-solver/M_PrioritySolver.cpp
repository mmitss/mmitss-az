/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   

/*  mprSolver.cpp  
*  Created by Mehdi Zamanipour
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.
*
*  Revision History:
*  1. This code can be used only for pririty eligible vehicles OR can be integrated into COP OR can be used in extended formulation to consider regular vehicle as well as priority vehicles in the mathematical formulation

* 
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
#include <glpk.h>

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
//#include "ConnectedVehicle.h"
#include "EVLS.h"

using namespace std;

#define OPT_MSG_SIZE 256
#ifndef byte
    #define byte  char                 // needed if byte not defined
#endif

#ifndef DEG2ASNunits
    #define DEG2ASNunits  (1000000.0)  // used for ASN 1/10 MICRO deg to unit converts
#endif

#define ROBUSTTIME_LOW 2  	 		   // Lower bound in second: reduced from the ETA for ROBUST control 
#define ROBUSTTIME_UP  4               // Upper bound in second: add to the ETA for ROBUST control

#define ROBUSTTIME_LOW_EV 4            // Lower bound in second: reduced from the ETA for ROBUST control
#define ROBUSTTIME_UP_EV  3            // Upper bound in second: add to the ETA for ROBUST control

#define EV  1
#define TRANSIT 2
#define TRUCK 3
#define PED 4
#define COORDINATION 6



#define MaxGrnTime 50                 //ONLY change the phase {2,4,6,8} for EV requested phases

#define COP_AND_PRIORITY 0  
#define PRIORITY 1
#define ADAPTIVE_PRIORITY 2


//define log file name with time stamp.
char logfilename[256]     			= "/nojournal/bin/log/MMITSS_MRP_Priority_Solver";
char signal_plan_file[256]      	= "/nojournal/bin/log/signal_Plan_";
//---Store the file name of the Config file.
//--- For Example: ConfigInfo_Hastings.txt, which has the minGrn and maxGrn etc.
char ConfigInfo[256]	  			= "/nojournal/bin/ConfigInfo.txt";
char IPInfo[64]			  			= "/nojournal/bin/IPInfo.txt";
char rsuid_filename[64]   			= "/nojournal/bin/rsuid.txt";
char PriorityConfigFile[64]  		= "/nojournal/bin/priorityConfiguration.txt";   // for importing the priority related inputs such as weights 
char request_combined_filename[128] = "/nojournal/bin/requests_combined.txt"; //requests_combined
char requestfilename[128] 			= "/nojournal/bin/requests.txt"; //requests
char signal_filename[128]		 	= "/nojournal/bin/signal_status.txt";
char prioritydatafile[128]			= "/nojournal/bin/NewModelData.dat";
char resultsfile[128]    			= "/nojournal/bin/Results.txt"; // Result from the GLPK solver.
char Lane_No_File_Name[64]  		= "/nojournal/bin/Lane_No_MPRSolver.txt"; 
char Lane_Phase_File_Name[64]  		= "/nojournal/bin/Lane_Phase_Mapping.txt"; 

// -------- BEGIN OF GLOABAL PARAMETERS-----//
// wireless connection variables
int sockfd;
int broadcast=1;
struct sockaddr_in sendaddr;
struct sockaddr_in recvaddr;
void setupConnection(); 				// Setup socket connection to send the optimize schedule to traffic controller interface or intelligent traffic control component. Also to settup connection to the trajectory aware component inorder to get arrival table
int RxArrivalTablePort=33333; 			// The port that arrival table data comes from
int TrajRequestPort=20000; 				// The port to send request to trajectory aware componet to acquire  arrival table in case the code is being used for adaptive priority algorithm
int ArrTabMsgID=66;
int TxOPTPort=5555;         			// The port that the optimal phase timing plane sends to SignalControl (COP)
int TxEventListPort=44444;
int OPTMsgID=55;
char cOPTbuffer[256];

char temp_log[512];
double MaxGrnExtPortion=.15; 		    // Extended the MaxGrn of requested phases to this portion. 1ant: keep the number with no 0 before .  , we need this format when we build the glpk mod file. GenerateMod function
RSU_Config ConfigIS;
RSU_Config ConfigIS_EV;
LinkedList<ReqEntry> Req_List_Combined; // Req list for requests in requests_combined.txt   . requests.combined.txt does not have the requests split phase in case we have EV in the list
LinkedList<ReqEntry> ReqList;           // Req list for requests in requests.txt: For updating the ReqListUpdateFlag only.  requests.txt has the requests split phase in case we have EV in the list
double  adCriticalPoints[2][3][15];     // This array keeps the crtitical point of the optimal schedule . The first dimention is Ring ( 2 rings [2] ) the senod dimention is the left or right side of the Critical zone ( [2] ) the third dimension is the total number of phases we look ahead in the schedule. which is maximum 15 phases.
int omitPhase[8];   				    // The phases that should be omitted when we have EV among the priority vehicles.
LinkedList <Schedule> Eventlist_R1;
LinkedList <Schedule> Eventlist_R2;
Phase Phases;  							// To get current signal tatus from signal controler to creat optimization model
string RSUID;
char ConfigFile[256];  				    //= "/nojournal/bin/ConfigInfo_MountainSpeedway.txt";
int AddPhantom=0; 						// used in case of EV
int PhaseVCMissing=0; 					// used in case of EV
int PhaseInitMissing=0;					// used in case of EV	
int ReqListUpdateFlag=0;
int HaveEVInList=0;   					// if there is EV in Req, =1; No EV, =0.
int HaveCoordInList=0;  			    // if there is Coordination in Req, =1; No Coordination, =0.
int HaveTransitInList=0;			    // if there is Transair in Req, =1; No Transair, =0.
int HaveTruckInList=0;   			    // if there is Truck in Req, =1; No Truck, =0.
int HavePedCall=0;   				    // if there is ped call in Req, =1; else =0.
char tmp_log[256];
int CurPhaseStatus[8]={0};  		    // when do phaseReading: the same results as phase_read without conversion
PhaseStatus phase_read={0}; 		    // value would be {1,3,4}
double red_elapse_time[8];   			// Red elapse time of each phase to be used in EVLS
char INTip[128];
char INTport[16];
int CombinedPhase[8]={0};
int codeUsage=1;
fstream fs_log,fs_signal_plan;
int iOPTmsgCnt=0;  
int InitPhase[2]={0}; 
double InitTime[2]={0}; 				// When in Red or Yellow, init1 & 2 are non-zero, Grn1&Grn2 (GrnElapse) are zero
double GrnElapse[2]={0}; 				//	When in Green, init1 & 2 are zero, Grn1&Grn2 are non-zero




//Parameters for trajectory control
char buf_traj[500000];  				//The Trajectory message from MRP_EquippedVehicleTrajecotryAware
LinkedList <ConnectedVehicle> trackedveh;
LinkedList <ConnectedVehicle> vehlist_each_phase;
LinkedList <ConnectedVehicle> vehlist_phase; 
LinkedList <ConnectedVehicle> vehlist_lane;
int NoVehInQ[8][6];						 // There is a list for all vehicles in each lane of each phase. It is assumed there are at most 8 phases and at most 6 lanes per phase.
double QSize[8][6]; 					 // length of the queue in each lane (in meter)
int phaseColor[8];
int iColor[2][8];						 // iColor[1][*]  has the current phase color, and iColor[0][*] has the previous signal status phase color 
bool bLaneSignalState[48];				 // is 1 for the lanes that their signal is green, otherwise 0 
double dQDischargingSpd=3.0;
double dQueuingSpd=2.0;
int currentTotalVeh=0;   
//Parameters for EVLS
float penetration=1;     						 //penetration rate
float dsrc_range=200;
int LaneNo[8]; 									// this array will record the number of lanes per each phase. 
int LaneNo2[8]; 								// this array will record the number of lanes per each phase. But if two phases belong to one approach, the LaneNo2 value for the second phase of the approach include the Laneno2 valuse of the first phase as well. As example, if phase 4 has 3 lanes and phase 7 (same approach) has 2 lanes, then LaneNo2[4-1]=3 and LaneNo2[7-1]= 3+2. This array is used in calculateQ function. 
int TotalLanes=0;
int iLog=1;

	
int outputlog(char *output); 					// for logging 
void PrintFile2Log(char *resultsfile);
void printCriticalPoints();
int GLPKSolutionValidation(char *filename); 	//determine whether glpksolver get a feasible solution
void GenerateMod(char *Filename,RSU_Config ConfigIS,char *OutFilename,int haveEV);  // generate .mod file for glpk optimizer , //------- If there is no EV, using ConfigIS; if there is EV, using ConfigIS_EV. --------//
void GLPKSolver();
void deductSolvingTimeFromCPs(double aCriticalPoints[2][3][15],double tt2,double tt1);
void populateCP(double  CP[2][3][15], double *Ring1LeftCP,double *Ring1RightCP,int *phaseSeqRing1 ,int t1, double *Ring2LeftCP,double *Ring2RightCP,int *phaseSeqRing2 ,int t2);
int  numberOfPlannedPhase(double adCritPont[3][15]);
void readOptPlanFromFile(char *filename,double adCriticalPoints[2][3][15]);
void readOptPlanFromFileForEV(char *filename,double adCriticalPoints[2][3][15], int omitPhase[8]);
void packOPT(char * buf,double cp[2][3][15],int msgCnt);
void Construct_eventlist(double cp[2][3][15],LinkedList<ReqEntry> Req_List);
void Construct_eventlist_EV(double cp[2][3][15],int omitPhas[8],LinkedList<ReqEntry> Req_List);
void Pack_Event_List(byte* tmp_event_data,int &size);
int  RequestPhaseInList(LinkedList<ReqEntry> ReqList,int phase);  // return the position of the phase in the ReqList
int  FindRingNo(int phase);
void PrintList2File(char *Filename,LinkedList<ReqEntry> &ReqList); 
void PrintList2File_EV(char *Filename,LinkedList<ReqEntry> &ReqList);
void ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List);
void ReqListFromFile_EV(char *filename,LinkedList<ReqEntry>& Req_List);
void LinkList2DatFile(LinkedList<ReqEntry> Req_List,char *filename,double InitTime[2],int InitPhase[2],double GrnElapse[2]);
void LinkList2DatFileForEV(LinkedList<ReqEntry> Req_List,char *filename,double InitTime[2],int InitPhase[2],double GrnElapse[2],RSU_Config configIS, int ChangeMaxGrn=0);
void LinkList2DatFileForAdaptivePriority(LinkedList<ReqEntry> Req_List,char *filename,double InitTime[2],int InitPhase[2],double GrnElapse[2],double transitWeigth, double truckWeigth, double coordinationweigt,double dVehDistToStpBar[130],double dVehSpd[130],int iVehPhase[130],double Ratio[130],double indic[130],int laneNumber[130],double qsize[48],double Tq[130]);
void removeZeroWeightReq(LinkedList<ReqEntry> Req_List_, LinkedList<ReqEntry> &Req_List_New);
void FindReqWithSamePriority(LinkedList<ReqEntry> Req_List_, int priority,LinkedList<ReqEntry> &Req_List_New);
int  FindListHighestPriority(LinkedList<ReqEntry> Req_List);
int  FindVehClassInList(LinkedList<ReqEntry> Req_List,int VehClass);
int  FindSplitPhase(int phase,int phaseStatus[8]);
void whichPhaseIsGreen(int phaseColor[8], int greenGroup, int redGroup, int yellowGroup);
void identifyColor(int color[2][8],int greenGroup, int redGroup, int yellowGroup, int PhaseNext);
void setupConfigurationAndConnection();
int handleArguments(int argc, char* argv[]);
void testConnectionToController();
void captureRequiredSignalStatus();
void readRequestListFromFiles();
void logRequestList();
void handleEVCase();
void deleteMissingPhaseFromList();
void sendEventListToSignalCntlInterface();
void sendEventListToCOP();
void creatLogFiles();
void addGrnExtToRequestedPhaseMaxGrnTime(LinkedList<ReqEntry> ReqList,double dGmax[8],int iGmax[2][4]);
void setMaxGrnTimeForCoordinatedPhases(double dGmax[8],int  iGmax[2][4]);
void doubleCheckMissingPhasesInTintersections(double dGmax[8],int iGmax[2][4]);
void intitializdGmaxAndiGmax( double dGmax[8],int iGmax[2][4]);
						
// Related to AdaptivePriority
void get_lane_no();   //get number of lanes each phase from Lane_No.txt file
void get_lane_phase_mapping(int phase[4][2], int PhaseOfEachLane[48]);
void UnpackTrajData1(char* ablob); 
void calculateQ(int Phaseofapp[4][2],int PhaseOfEachLane[48], double qsize[48]);
void extractOptModInputFromTraj(double dVehDistToStpBar[130],double dVehSpd[130],int iVehPhase[130],double Ratio[130],double indic[130],int laneNumber[130],double dQsizeOfEachLane[48],double Tq[130], int phase[4][2], int PhaseOfEachLane[48],int InitPhase[2],double GrnElapse[2]);
void calculateRedElapseTime(double red_start_time[] ,int previous_signal_color[]);
void initializeRedStartVar(double red_start_time[], int previous_signal_color[]);
void handleAdaptivePriorityCase();


int main ( int argc, char* argv[] )
{    
	handleArguments(argc, argv);
	setupConfigurationAndConnection();
    double dStartTimeOfGLPKcall=0.0;
	double dEndTimeOfGLPKcall=0.0;
	double dStartTime=0.0;
	double dEndTime=0.0;   														//time stamps used to determine whether we connect to the RSE or not.
    testConnectionToController();
    // initializeRedStartVar(red_start_time,previous_signal_color);  			// in case of Adaptive Priority
    while ( true )
    {
		dStartTime=GetSeconds();
		PhaseTimingStatusRead();
		dEndTime=GetSeconds();
		if( dEndTime-dStartTime < 2.0 ) // We do connect to RSE
		{
			captureRequiredSignalStatus(); //DD: How to know which phase status is required
			// calculateRedElapseTime(red_start_time,previous_signal_color);  	// in case of Adaptive Priority
			Req_List_Combined.ClearList();   
			ReqList.ClearList();
			readRequestListFromFiles();
			logRequestList();
			if( ReqListUpdateFlag>0  ) 
			{	
				sprintf(tmp_log,"\n...............Solve the problem..............\t FLAG is : {%d} At time: %10.2lf", ReqListUpdateFlag, GetSeconds());
				outputlog(tmp_log);
				if(HaveEVInList==1)												 //If there is EV, we need to treat the problem differently. Split phase of ther requested phase should be considered. (e.g. if the EV requested phase is 8, the split phase is 3. Also, we should omit the non requested phases
				{
					handleEVCase();
					LinkList2DatFileForEV(Req_List_Combined,prioritydatafile,InitTime,InitPhase,GrnElapse,ConfigIS_EV,HaveEVInList);	// construct .dat file for the glpk
					PrintFile2Log(prioritydatafile);  							 // Log the .dat file for glpk solver
					deleteMissingPhaseFromList();
				}
				if ((HaveEVInList==0)&&(Req_List_Combined.ListSize()>0)) // At least one priority vehicle except EV is in the list!
				{
					LinkList2DatFile(Req_List_Combined,prioritydatafile,InitTime,InitPhase,GrnElapse) ;  // construct .dat file for the glpk
					//LinkList2DatFileForAdaptivePriority(Req_List_Combined,prioritydatafile,InitTime,InitPhase,GrnElapse, ConfigIS.dTransitWeight, ConfigIS.dTruckWeight,ConfigIS.dCoordinationWeight,dVehDistToStpBar,dVehSpd,iVehPhase,Ratio,indic,laneNumber,dQsizeOfEachLane, Tq ); // construct .dat file for the glpk	
					PrintFile2Log(prioritydatafile); // Log the .dat file for glpk solver
				}    
				// Rewright the request list into the file and SET the ReqListUpdateFlag in requests.txt to:"0"   ***IMPORTANT***
				ReqListUpdateFlag=0;
				PrintList2File(requestfilename,ReqList);
				PrintList2File_EV(request_combined_filename,Req_List_Combined);
				// Solve the problem, write to "Results.txt"			
				dStartTimeOfGLPKcall=GetSeconds();
				GLPKSolver();  
				dEndTimeOfGLPKcall=GetSeconds();
				sprintf(tmp_log,"Time for solving the problem is about: {%.3f}.\n",dEndTimeOfGLPKcall-dStartTimeOfGLPKcall); 
				outputlog(tmp_log);
				int success=GLPKSolutionValidation(resultsfile);
				if (success==1)  
				{					
					sprintf(tmp_log,"...............New optimal signal schedule is being set..............:\t At time: %.2f\n",GetSeconds());
					outputlog(tmp_log); 
					PrintFile2Log(resultsfile);
					if(HaveEVInList==1)
						readOptPlanFromFileForEV(resultsfile,adCriticalPoints, omitPhase);
					else
						readOptPlanFromFile(resultsfile,adCriticalPoints);
				
					deductSolvingTimeFromCPs(adCriticalPoints,dEndTimeOfGLPKcall,dStartTimeOfGLPKcall);	
					// If the Solver works with COP, we pack the Critical Points and send it to COP, othewise we transfer the CP to event list and send the event list to controller.
					if (codeUsage==PRIORITY) 				// Send Optimal Signal Schedule (OPT)to Interface 
					{
						Eventlist_R1.ClearList();
						Eventlist_R2.ClearList();
						if(HaveEVInList==1)  				 // MZP 5/5/17  check the EV case , should we have same opt model? should we have _EV.mod file created every time ? how should we read the plan? are left and right points the same?
							Construct_eventlist_EV(adCriticalPoints,omitPhase,Req_List_Combined);
						else
							Construct_eventlist(adCriticalPoints,Req_List_Combined);
						sendEventListToSignalCntlInterface();				
					}
					else if (codeUsage==COP_AND_PRIORITY) 	// Send to COP
						sendEventListToCOP();				
					//printCriticalPoints();
				}
				else
				{
					sprintf(temp_log," No feasible solution found !!!!!!!! At time: %.2f.......... \n", GetSeconds());
					outputlog("No feasible solution!\n");
				}
			}
			/* if (codeUsage==ADAPTIVE_PRIORITY) 		// Integrated Priority Alg and Adaptive Control				
				handleAdaptivePriorityCase();
			*/ 
			else 				  // if there is the priority flag is zero and if the codeusage is Priority Control and Actuation
			{
				sprintf(tmp_log,"No Need to solve, At time: %.2f \n",GetSeconds()); 
				outputlog("\n");
				msleep(100);   		
			}
			msleep(20);   
		}
		else  // // We do not connect to RSE
		{
			sprintf(tmp_log," Cannot connect to RSU!!!!!  At time: %.2f \n",GetSeconds());
			outputlog(tmp_log);
			msleep(400);
			continue;
		}  // 
    }  // end of While(true)
    fs_log.close();
    fs_signal_plan.close();
    return 0;
}

int handleArguments(int argc, char* argv[])
{
	 int ch;
     int iExtention=0;
     while ((ch = getopt(argc, argv, "c:e:l:")) != -1) 
     {
		switch (ch) 
		{
		    case 'c':
				codeUsage=atoi (optarg);
				if (codeUsage==ADAPTIVE_PRIORITY)
					printf ("Code usage is for integrated Prioirty alg and Adaptive Control \n");
				else if (codeUsage==PRIORITY)
					printf ("Code usage is for Prioirty Alg + Actuation \n");
				else if (codeUsage==COP_AND_PRIORITY)
					printf ("Code usage is for Prioirty Alg + COP \n");
				break;
			case 'e':
				iExtention=atoi (optarg);
				MaxGrnExtPortion=iExtention/100;
				break;
				
			case 'l': // if iLog is 0 then nothing is logged.
				iLog=atoi (optarg);
				break;
            default:
			     return 0;
		 }
    }
}
void setupConfigurationAndConnection()
{
	creatLogFiles();
	
	//Get number of lanes each phase
	// get_lane_no(); // in case of Adaptive Priority
	//Get phase of each lane
	//get_lane_phase_mapping(PhaseOfEachApproach,PhaseOfEachLane);  // in case of Adaptive Priority 
	
	// ------ Readng Configuration---
	get_ip_address();           // READ the ASC controller IP address into INTip from "IPInfo.txt"
    get_rsu_id();               // Get rsu id for string RSUID from "rsuid.txt"
    get_configfile();  // 1) Read the name of "configinfo_XX.txt" into "ConfigFile" from ConfigInfo.txt-//  // 2) Read in the CombinedPhase[8] for finding the split phases--
		
    //-----------------Read in the ConfigIS Every time in case of changing plan-----------------------//
    int curPlan=CurTimingPlanRead();
    sprintf(tmp_log,"Current timing plan is:\t%d\n",curPlan);  
    outputlog(tmp_log);

    IntersectionConfigRead(curPlan,ConfigFile);  // Generate: configinfo_XX.txt
    PrintFile2Log(ConfigFile);
    ReadInConfig(ConfigFile,PriorityConfigFile); // Get Configuration from priorityConfiguration.txt and controler
    PrintRSUConfig();
    GenerateMod(ConfigFile,ConfigIS,"/nojournal/bin/NewModel.mod",0);
    //--------------End of Read in the ConfigIS Every time in case of changing plan-----------------------//  //---- If we have EV priority, then we need to generate the special "NewModelData_EV.mod" through "ConfigIS" and "requested phases"
	
    setupConnection(); // to get arrival time from trajectory aware component (just in case sodeusage is AdaptivePriority) and send the Event list to Signal Controller OR to send the Event list to COP OR to only send the Event list to Signal Controller    
}

void creatLogFiles()
{
	//------log file name with Time stamp---------------------------
    char timestamp[128];
    string Logfile;
    xTimeStamp(timestamp);
    strcat(logfilename,timestamp); 
    Logfile=string(logfilename);
    strcat(logfilename,".log");
    strcat(signal_plan_file,timestamp);    
    strcat(signal_plan_file,".log");
    fs_log.open(logfilename, fstream::out | fstream::trunc);
    fs_signal_plan.open(signal_plan_file,fstream::out | fstream::trunc);
    //------end log file name-----------------------------------
}	

void testConnectionToController()
{
	//warm up to see if we are connected to controller
	for(int i=3;i>0;i--) 
	{
		PhaseTimingStatusRead();
		Phases.UpdatePhase(phase_read);
		Phases.Display();
		//Phases.RecordPhase(signal_plan_file);
		msleep(200);
	}
}   
  
void initializeRedStartVar(double red_start_time[] ,int previous_signal_color[])
{
	//initialize red start time
	for(int i=0;i<8;i++)
	{
		previous_signal_color[i]=phase_read.phaseColor[i];
		if(phase_read.phaseColor[i]==1)
			red_start_time[i]=GetSeconds();
		else
			red_start_time[i]=0;
	}
}
//			//Calculate red elapse time for each phase, if the signal turns to green, keep the previous red duration until turns to red again (TO BE USED IN ELVS)
void calculateRedElapseTime(double red_start_time[] ,int previous_signal_color[])// in case of Adaptive Priority
{	
	for(int i=0;i<8;i++)
	{
		if (previous_signal_color[i]!=1 && phase_read.phaseColor[i]==1)  //signal changed from other to red
		{
			red_start_time[i]=GetSeconds();
		}		
		if (phase_read.phaseColor[i]==1)
		{
			red_elapse_time[i]=GetSeconds()-red_start_time[i];
		}
		previous_signal_color[i]=phase_read.phaseColor[i];
	}
}			

void captureRequiredSignalStatus()
{
	Phases.UpdatePhase(phase_read);
	////------------Begin of recording the phase_status into signal_status.txt----------------//
	Phases.RecordPhase(signal_plan_file);
	for(int ii=0;ii<2;ii++)
	{
		InitPhase[ii]=Phases.InitPhase[ii]+1;   //{1-8}
		InitTime[ii] =Phases.InitTime[ii];      // ALSO is the (Yellow+Red) Left for the counting down time ***Important***
		GrnElapse[ii]=Phases.GrnElapse[ii];     // If in Green
	}
	
	//IF we can get signal information, then we connect to the ASC controller
	FILE * fp=fopen("/nojournal/bin/connection.txt","w"); // For determination of connecting to a ASC controller or not. 
	fprintf(fp,"%ld",time(NULL));
	fclose(fp);		
}

void logRequestList()
{
//	if (Req_List_Combined.ListSize()>0)
	{
		if(ReqListUpdateFlag>0)
		{
			sprintf(tmp_log,"Requests Modified : Flag is { %d } at %.3f",ReqListUpdateFlag,GetSeconds());    //cout<<tmp_log;
			outputlog(tmp_log);
		//	PrintFile2Log(request_combined_filename);
		//	PrintFile2Log(requestfilename);
		}
		else
		{
   	//		sprintf(tmp_log,"Read in the request list at %.3f",GetSeconds());    //cout<<tmp_log;
	//		outputlog(tmp_log);
			PrintFile2Log(request_combined_filename);
		//	PrintFile2Log(requestfilename);
		}
	}
//	else  // if (Req_List_Combined.ListSize()>0)
//	{
//		sprintf(tmp_log,"No Request yet, at %lf",GetSeconds());    
//		outputlog(tmp_log);
// }  // end of "if (Req_List_Combined.ListSize()>0)"
}		
	
	
void readRequestListFromFiles()
{		
	//*** IMPORTANT***: THESE two request files are created in PRS, so PRS 
	//*** Read from the uncombined phase file: requests.txt get ReqListUpdateFlag.  requests.txt has the split phase information in case we have EV in the list.
	ReqListFromFile(requestfilename,ReqList);
	// Read from the combined phase file: requests_combined.txt
	ReqListFromFile_EV(request_combined_filename,Req_List_Combined);
	
	HaveEVInList=FindVehClassInList(Req_List_Combined,EV);
	HaveCoordInList=FindVehClassInList(Req_List_Combined,COORDINATION);
	HaveTruckInList=FindVehClassInList(Req_List_Combined,TRUCK);
	HaveTransitInList=FindVehClassInList(Req_List_Combined,TRANSIT);	
	HavePedCall=FindVehClassInList(Req_List_Combined, PED);
}



void handleEVCase()
{
	sprintf(tmp_log,"...... We have EV. Need Dynamic Mod file......\t FLAG { %d } At time: %s",ReqListUpdateFlag,GetDate());
	outputlog(tmp_log); 
	//----------------------------------------Begin to generate the dynamic mod file----------------------------------------//
	int size_init=2;   //--- We always have two initial phases: if has{1,2,6,8}, timing 8, initial will be {4,8}
	int size_request=Req_List_Combined.ListSize();
	//---------------- EV phase vector------------------
	vector<int> EV_Phase_vc;
	Req_List_Combined.Reset();
	for(int i=0;i<size_request;i++)
	{
		if(Req_List_Combined.Data().VehClass==EV)
			EV_Phase_vc.push_back(Req_List_Combined.Data().Phase);
		Req_List_Combined.Next();
	}
	int EV_Phase_size=EV_Phase_vc.size();
	int SamePhase=AllSameElementOfVector(EV_Phase_vc); // If only have one element, return 0
	int ReqPhase;
	PhaseVCMissing=0;
	PhaseInitMissing=0;
	for(int i=0;i<EV_Phase_size;i++)
	{
		ReqPhase=EV_Phase_vc[i];
		PhaseVCMissing=BarrierPhaseIsMissing(ReqPhase,EV_Phase_vc);  // phase missing in the vector
		PhaseInitMissing=BarrierPhaseIsMissing(ReqPhase,InitPhase,2);
		if(PhaseInitMissing!=0 && PhaseVCMissing!=0)  // Then they should be the same
		{
			EV_Phase_vc.push_back(PhaseInitMissing);
		}
		int Pos=RequestPhaseInList(Req_List_Combined,ReqPhase);
		if(PhaseInitMissing==0 && PhaseVCMissing!=0)
		{
			if(Pos>=0)	Req_List_Combined.Reset(Pos);
			ReqEntry PhantomEntry=Req_List_Combined.Data();
			PhantomEntry.Phase=PhaseVCMissing;
			if(AddPhantom>0) // MZP ain't it already  done in PRS?!!! if so, delete AddPhntom and this if condition
			{
				Req_List_Combined.InsertAfter(PhantomEntry);
				outputlog("\n Add a Phantom Req Entry!\n");
			}
		}
	}
	EV_Phase_size=EV_Phase_vc.size(); //  EV_Phase_size could be changed.
	int TotalSize=size_init+EV_Phase_size;
	int *Phase_Infom=new int[TotalSize];
	for(int i=0;i<EV_Phase_size;i++)
	{
		Phase_Infom[i]=EV_Phase_vc[i];
	}
	for(int i=0;i<size_init;i++)
	{
		Phase_Infom[EV_Phase_size+i]=InitPhase[i];
	}
	selectionSort(Phase_Infom, TotalSize);   // Sort all the involved phases for removing duplicated phases
	int NoRepeatSize=removeDuplicates(Phase_Infom, TotalSize);
	RSUConfig2ConfigFile("/nojournal/bin/ConfigInfo_EV.txt",Phase_Infom,NoRepeatSize,ConfigIS);
	PrintFile2Log("/nojournal/bin/ConfigInfo_EV.txt");// Log the EV configInfo.
	delete [] Phase_Infom;
	GenerateMod("/nojournal/bin/ConfigInfo_EV.txt",ConfigIS,"/nojournal/bin/NewModel_EV.mod",HaveEVInList); // We need all {2,4,6,8}
	ConfigIS_EV=ReadInConfig("/nojournal/bin/ConfigInfo_EV.txt",1);
	PrintRSUConfig2File(ConfigIS_EV,tmp_log);
	PrintRSUConfig(ConfigIS_EV);
}	
			
void deleteMissingPhaseFromList()
{
	if(PhaseVCMissing!=0 && PhaseInitMissing==0)
		{
			int Pos=RequestPhaseInList(Req_List_Combined,PhaseVCMissing);
			if(Pos>=0)
			{
				Req_List_Combined.Reset(Pos);
				Req_List_Combined.DeleteAt();
			}
		}
}		

void sendEventListToSignalCntlInterface()
{	
	int addr_length = sizeof ( recvaddr );
    byte tmp_event_data[8192];
	int size=0;
	Pack_Event_List(tmp_event_data, size);
	char* event_data;
	event_data= new char[size];			
	for(int i=0;i<size;i++)
		event_data[i]=tmp_event_data[i];	
	
	recvaddr.sin_port = htons(TxEventListPort);
	if (sendto(sockfd,event_data,size+1 , 0,(struct sockaddr *)&recvaddr, addr_length))
	{
		sprintf(temp_log," The new Event List sent to SignalControllerInterface, The size is %d and time is : %.2f.......... \n", size, GetSeconds()); 
		outputlog(temp_log);
	}
}
			

void sendEventListToCOP()
{	
	int addr_length = sizeof ( recvaddr );			
	iOPTmsgCnt++;
	iOPTmsgCnt=iOPTmsgCnt%127;
	packOPT(cOPTbuffer,adCriticalPoints,iOPTmsgCnt);
	recvaddr.sin_port = htons(TxOPTPort);
	if ((sendto(sockfd, cOPTbuffer,OPT_MSG_SIZE, 0,(struct sockaddr *)&recvaddr, addr_length)>0) )
	{
		sprintf(temp_log," The new OPT set sent to SignalControl (COP) At time: %.2f.......... \n", GetSeconds());
		outputlog(temp_log);
	}
}
			
int outputlog(char *output)
{
	if (iLog==1) // if logging is active other wise nothing is logged
	{
		FILE * stream = fopen( logfilename, "r" );
		fseek( stream, 0L, SEEK_END );
		long endPos = ftell( stream );
		fclose( stream );

		std::fstream fs;
		if (endPos <10000000)
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
		cout<<output<<endl;
	}
    return 1;
}



void setupConnection()
{
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
	sendaddr.sin_port = htons(RxArrivalTablePort);  //*** IMPORTANT: MPRsolver uses this port to receive the arrival table form Trajectory aware compponent //
	sendaddr.sin_addr.s_addr =  INADDR_ANY; //inet_addr ("127.0.0.1"); //inet_addr(OBU_ADDR);// inet_addr ("150.135.152.35"); //

	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);

	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");        exit(1);
	}


	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(TrajRequestPort);   // MPRsolver uses this port to send the optimal solution to ... COP or Controller
	recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1") ; //INADDR_BROADCAST;
	memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
}



int GLPKSolutionValidation(char *filename)
{
	fstream r_f;
	r_f.open(filename,fstream::in);
	int i,j;
	int IsValid=0;
	int flag=0;
	if (!r_f)
	{
		cout<<"***********Error opening the Result.txt file in validation function!\n";
		exit(1);
	}

  string lineread;
  double V[3][8];
  getline(r_f,lineread);
  getline(r_f,lineread);
  for (i=0;i<3;i++)
  {
	getline(r_f,lineread);
	sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",&V[i][0],&V[i][1],&V[i][2],&V[i][3],&V[i][4],&V[i][5],&V[i][6],&V[i][7]);
//	sprintf(tmp_log,"%lf %lf %lf %lf %lf %lf %lf %lf ",V[i][0],V[i][1],V[i][2],V[i][3],V[i][4],V[i][5],V[i][6],V[i][7]);
//	outputlog(tmp_log);
  }
 // outputlog("\n");
  for(i=0;i<3;i++)
  {
	for(j=0;j<8;j++)
	{
		if (V[i][j]>0.0)
		{
			IsValid=1;  //have valid solution
			flag=1;
			break;
		}
	}
	if (flag==1)
	break;
  }
  return IsValid;
}

 
void GenerateMod(char *Filename,RSU_Config ConfigIS,char *OutFilename,int haveEVinList)
{
	//-------reading the signal configuration file ---------
    fstream FileRead;
    FileRead.open(Filename,ios::in);

    if(!FileRead)
    {
        cerr<<"Unable to open file: "<<Filename<<endl;
        exit(1);
    }

    string lineread;

    int PhaseNo;
    char TempStr[16];
    vector<int> P11,P12,P21,P22;
    int PhaseSeq[8], Gmin[8], Gmax[8];
    float Yellow[8], Red[8];  // If

    for(int i=1;i<8;i=i+2) // Add {2,4,6,8} into the PhaseSeq: NECESSARY Phases
    {
        PhaseSeq[i]=i+1;
    }
    //------- Read in the parameters--
    while(!FileRead.eof())
    {
        getline(FileRead,lineread);

        if (lineread.size()!=0)
        {
            sscanf(lineread.c_str(),"%s",TempStr);

            if (strcmp(TempStr,"Phase_Num")==0)
            {
                sscanf(lineread.c_str(),"%s %d",TempStr,&PhaseNo);
                cout<<"Total Phase Number is:"<<PhaseNo<<endl;
            }

            else if(strcmp(TempStr,"Phase_Seq")==0)
            {
                //sscanf(lineread.c_str(),"%*s %d %d %d %d %d %d %d %d",
                //    &PhaseSeq[0],&PhaseSeq[1],&PhaseSeq[2],&PhaseSeq[3],
                //    &PhaseSeq[4],&PhaseSeq[5],&PhaseSeq[6],&PhaseSeq[7]);
                sscanf(lineread.c_str(),"%*s %d %*d %d %*d %d %*d %d %*d",
                    &PhaseSeq[0],&PhaseSeq[2],&PhaseSeq[4],&PhaseSeq[6]);

            }
            else if(strcmp(TempStr,"Yellow")==0)
            {
                sscanf(lineread.c_str(),"%*s %f %f %f %f %f %f %f %f",
                    &Yellow[0],&Yellow[1],&Yellow[2],&Yellow[3],
                    &Yellow[4],&Yellow[5],&Yellow[6],&Yellow[7]);
            }
            else if(strcmp(TempStr,"Red")==0)
            {
                sscanf(lineread.c_str(),"%*s %f %f %f %f %f %f %f %f",
                    &Red[0],&Red[1],&Red[2],&Red[3],
                    &Red[4],&Red[5],&Red[6],&Red[7]);
            }
            else if(strcmp(TempStr,"Gmin")==0)
            {
                sscanf(lineread.c_str(),"%*s %d %d %d %d %d %d %d %d",
                    &Gmin[0],&Gmin[1],&Gmin[2],&Gmin[3],
                    &Gmin[4],&Gmin[5],&Gmin[6],&Gmin[7]);
            }
            else if(strcmp(TempStr,"Gmax")==0)
            {
                sscanf(lineread.c_str(),"%*s %d %d %d %d %d %d %d %d",
                    &Gmax[0],&Gmax[1],&Gmax[2],&Gmax[3],
                    &Gmax[4],&Gmax[5],&Gmax[6],&Gmax[7]);
            }
        }
    }

    FileRead.close();
    

    //-------------Handle the parameters for non-complete phases case----//
    for(int i=0;i<8;i++)
    {
        if (PhaseSeq[i]>0)
        {
            switch (PhaseSeq[i])
            {
            case 1:
            case 2:
                P11.push_back(PhaseSeq[i]);
                break;
            case 3:
            case 4:
                P12.push_back(PhaseSeq[i]);
                break;
            case 5:
            case 6:
                P21.push_back(PhaseSeq[i]);
                break;
            case 7:
            case 8:
                P22.push_back(PhaseSeq[i]);
                break;
            }
        }
    }


	//-------READING the priority Configuratio file ---------------
    double dCoordinationWeight;
    int iCoordinatedPhase[2];
    double dTransitWeight;
    double dTruckWeight;
    double dCoordinationOffset;
    double dCoordinationCycle;
    double dCoordinationSplit[2];
    dCoordinationWeight=ConfigIS.dCoordinationWeight;
    
    
    iCoordinatedPhase[0]=ConfigIS.iCoordinatedPhase[0];
    iCoordinatedPhase[1]=ConfigIS.iCoordinatedPhase[1];
    dTransitWeight=ConfigIS.dTransitWeight;
    dTruckWeight=ConfigIS.dTruckWeight;
    dCoordinationCycle=ConfigIS.dCoordCycle;
    dCoordinationOffset=ConfigIS.dCoordOffset;
    if ((iCoordinatedPhase[0]>0) && (iCoordinatedPhase[0]<9))
        dCoordinationSplit[0]=ConfigIS.dCoordinationPhaseSplit[iCoordinatedPhase[0]-1];
    else
        dCoordinationSplit[0]=0.0; 
    if ((iCoordinatedPhase[1]>0) && (iCoordinatedPhase[1]<9))
		dCoordinationSplit[1]=ConfigIS.dCoordinationPhaseSplit[iCoordinatedPhase[1]-1];
    else
        dCoordinationSplit[1]=0.0; 
    
	if (dCoordinationWeight<0)
			dCoordinationWeight=0.0;

	if (haveEVinList==1)
		dCoordinationWeight=0.0;

    // ---------------- Writing the .mod  file------------------
    
	fstream FileMod;
    FileMod.open(OutFilename,ios::out);

    if(!FileMod)
    {
        cerr<<"Cannot open file: NewModel.mod to write!\n";
        exit(1);
    }

    int PhaseSeqArray[8];
    int kk=0;
	
	
	// =================Defining the sets ======================
    if(P11.size()==1)
    {
        FileMod<<"set P11:={"<<P11[0]<<"}; \n";
        PhaseSeqArray[kk]=P11[0];
                kk++;
    }
    else if(P11.size()==2)
    {
        FileMod<<"set P11:={"<<P11[0]<<","<<P11[1]<<"};  \n";
        PhaseSeqArray[kk]=P11[0];
        kk++;
        PhaseSeqArray[kk]=P11[1];
        kk++;

     }

     if(P12.size()==1)
    {
        FileMod<<"set P12:={"<<P12[0]<<"}; \n";
        PhaseSeqArray[kk]=P12[0];
        kk++;
    }
    else if(P12.size()==2)
    {
        FileMod<<"set P12:={"<<P12[0]<<","<<P12[1]<<"};\n";
        PhaseSeqArray[kk]=P12[0];
        kk++;
        PhaseSeqArray[kk]=P12[1];
        kk++;
     }

     if(P21.size()==1)
    {
        FileMod<<"set P21:={"<<P21[0]<<"};\n";
        PhaseSeqArray[kk]=P21[0];
        kk++;
    }
    else if(P21.size()==2)
    {
        FileMod<<"set P21:={"<<P21[0]<<","<<P21[1]<<"};\n";
        PhaseSeqArray[kk]=P21[0];
        kk++;
        PhaseSeqArray[kk]=P21[1];
        kk++;
     }

     if(P22.size()==1)
    {
        FileMod<<"set P22:={"<<P22[0]<<"};\n";
        PhaseSeqArray[kk]=P22[0];
        kk++;
    }
    else if(P22.size()==2)
    {
        FileMod<<"set P22:={"<<P22[0]<<","<<P22[1]<<"};\n";
        PhaseSeqArray[kk]=P22[0];
        kk++;
        PhaseSeqArray[kk]=P22[1];
        kk++;
     }

    FileMod<<"set P:={";
    for(int i=0;i<kk;i++)
    {
        if(i!=kk-1)
            FileMod<<" "<<PhaseSeqArray[i]<<",";
        else
            FileMod<<" "<<PhaseSeqArray[i];
    }
    FileMod<<"};\n"; 
    FileMod<<"set K  := {1..3};\n"; // Only two cycles ahead are considered in the model. But we should count the third cycle in the cycle set. Because, assume we are in the midle of cycle one. Therefore, we have cycle 1, 2 and half of cycle 3.
    FileMod<<"set J  := {1..10};\n";
	FileMod<<"set P2 := {1..8};\n";
    FileMod<<"set T  := {1..10};\n";	// at most 10 different types of vehicle may be considered , EV are 1, Transit are 2, Trucks are 3
        
    // at most 4 rcoordination requests may be considered, 2 phase * 2 cycles ahead. For example, if phases 2 and 6 are coordinated,
    // then CP={2,6} and CP1={2} and CP2={6}. The values of Cl1 and Cu1 show the lower and upper bound of the arrival time of coordination request for phase p in CP1
    // The values of Cl2 and Cu2 show the lower and upper bound of the arrival time of coordination request for phase p in CP2
    FileMod<<"set E:={1,2};\n";  
	
    FileMod<<"set C :={1,2};\n";  
	FileMod<<"set CP:={";
	if ( (dCoordinationSplit[0]>0) && (dCoordinationSplit[1]>0) )
		FileMod<<iCoordinatedPhase[0]<<","<<iCoordinatedPhase[1]<<"};\n";
	else if  ( (dCoordinationSplit[0]>0) && (dCoordinationSplit[1]<=0) )
		FileMod<<iCoordinatedPhase[0]<<"};\n";
	else if  ( (dCoordinationSplit[0]<=0) && (dCoordinationSplit[1]>0) )
		FileMod<<iCoordinatedPhase[1]<<"};\n";
	else
		FileMod<<" };\n";
	
	FileMod<<"set CP1:={";
	if (dCoordinationSplit[0]>0) 
		FileMod<<iCoordinatedPhase[0]<<"};\n";
	else
		FileMod<<" };\n";

	FileMod<<"set CP2:={";
	if (dCoordinationSplit[1]>0) 
		FileMod<<iCoordinatedPhase[1]<<"};\n";
	else
		FileMod<<" };\n";
	
	if (codeUsage==ADAPTIVE_PRIORITY) // If arrival table is considered ( in Integrated Priority and Adaptive Control) at most 130 vehicles and at most 20 lanes
	{
			FileMod<<"set I  :={1..130};\n";
			FileMod<<"set L  :={1..20};\n";
	}
    FileMod<<"  \n";
   //========================Parameters=========================
  
    FileMod<<"param y    {p in P}, >=0,default 0;\n";
    FileMod<<"param red  {p in P}, >=0,default 0;\n";
    FileMod<<"param gmin {p in P}, >=0,default 0;\n";
    FileMod<<"param gmax {p in P}, >=0,default 0;\n";
    FileMod<<"param init1,default 0;\n";
    FileMod<<"param init2,default 0;\n";
    FileMod<<"param Grn1, default 0;\n";
    FileMod<<"param Grn2, default 0;\n";
    FileMod<<"param SP1,  integer,default 0;\n";
    FileMod<<"param SP2,  integer,default 0;\n";
    FileMod<<"param M:=9999,integer;\n";
    FileMod<<"param alpha:=100,integer;\n";
    FileMod<<"param Rl{p in P, j in J}, >=0,  default 0;\n";
    FileMod<<"param Ru{p in P, j in J}, >=0,  default 0;\n";
    FileMod<<"param Cl1{p in CP, c in C}, >=0,  default 0;\n";
    FileMod<<"param Cu1{p in CP, c in C}, >=0,  default 0;\n";
    FileMod<<"param Cl2{p in CP, c in C}, >=0,  default 0;\n";
    FileMod<<"param Cu2{p in CP, c in C}, >=0,  default 0;\n";
   
	if (dCoordinationWeight > 0)
		FileMod<<"param coordinationOn,:= 1;\n";
	else
		FileMod<<"param coordinationOn,:= 0;\n";
	FileMod<<"param cycle, :="<<dCoordinationCycle  <<";\n"; //    # if we have coordination, the cycle length  
	FileMod<<"param active_coord1{p in CP, c in C}, integer, :=(if (Cl1[p,c]>0 and coordinationOn=1) then	1 else	0);\n";
	FileMod<<"param active_coord2{p in CP, c in C}, integer, :=(if (Cl2[p,c]>0 and coordinationOn=1) then	1 else	0);\n";
	FileMod<<"param PrioType { t in T}, >=0, default 0;  \n";
	FileMod<<"param PrioWeigth { t in T}, >=0, default 0;  \n";
	FileMod<<"param priorityType{j in J}, >=0, default 0;  \n";
	FileMod<<"param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  \n";
	FileMod<<"param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);\n";
    FileMod<<"param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);\n";
    FileMod<<"param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);\n";
    FileMod<<"param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);\n";
    FileMod<<"param ReqNo:=sum{p in P,j in J} active_pj[p,j];\n";
    FileMod<<"param CoordNo:= sum{p in CP1,c in C} active_coord1[p,c]+sum{p in CP2,c in C} active_coord2[p,c];\n"; 
    
    // the following parameters added in order to consider case when the max green time in one barrier group expired but not in the other barrier group
    FileMod<<"param sumOfGMax11, := sum{p in P11} (gmax[p]*coef[p,1]);\n";
	FileMod<<"param sumOfGMax12, := sum{p in P12} (gmax[p]*coef[p,1]);\n";
	FileMod<<"param sumOfGMax21, := sum{p in P21} (gmax[p]*coef[p,1]);\n";
	FileMod<<"param sumOfGMax22, := sum{p in P22} (gmax[p]*coef[p,1]);\n";
	FileMod<<"param barrier1GmaxSlack, := sumOfGMax11 - sumOfGMax21 ;\n";
	FileMod<<"param barrier2GmaxSlack, := sumOfGMax12 - sumOfGMax22 ;\n";
	FileMod<<"param gmaxSlack{p in P}, := (if coef[p,1]=0 then 0 else (if (p in P11) then gmax[p]*max(0,-barrier1GmaxSlack)/sumOfGMax11  else ( if (p in P21) then gmax[p]*max(0,+barrier1GmaxSlack)/sumOfGMax21  else ( if (p in P12) then gmax[p]*max(0,-barrier2GmaxSlack)/sumOfGMax12  else ( if (p in P22) then gmax[p]*max(0,barrier2GmaxSlack)/sumOfGMax22  else 0) ) ) )    ); \n";
	FileMod<<"param gmaxPerRng{p in P,k in K}, := (if (k=1) then gmax[p]+gmaxSlack[p] else	gmax[p]);\n";

    
    
    if (codeUsage==ADAPTIVE_PRIORITY) // If arrival table is considered ( in Integrated Priority and Adaptive Control)
	{
		FileMod<<"param s ,>=0,default 0;\n"; 
		FileMod<<"param qSp ,>=0,default 0;\n"; 
		FileMod<<"param Ar{i in I}, >=0,  default 0;\n";
		FileMod<<"param Tq{i in I}, >=0,  default 0;\n";
		FileMod<<"param active_arr{i in I}, integer, :=(if Ar[i]>0 then 1 else 0);\n";
		FileMod<<"param SumOfActiveArr, := (if (sum{i in I} Ar[i])>0 then (sum{i in I} Ar[i]) else 1);\n";
		FileMod<<"param Ve{i in I}, >=0,  default 0;\n";
		FileMod<<"param Ln{i in I}, >=0,  default 0;\n";
		FileMod<<"param Ph{i in I}, >=0,  default 0;\n";
		FileMod<<"param L0{l in L}, >=0,  default 0;\n";  
		FileMod<<"param LaPhSt{l in L}, integer;\n"; 
		FileMod<<"param Ratio{i in I}, >=0,  default 0;\n";
		FileMod<<"param indic{i in I},:= (if (Ratio[i]-L0[Ln[i]]/(s-qSp))> 0 then 1 else 0);\n";
	}
    FileMod<<"  \n";
	// ==================== VARIABLES =======================
    FileMod<<"var t{p in P,k in K,e in E}, >=0;\n";
    FileMod<<"var g{p in P,k in K,e in E}, >=0;\n";
    FileMod<<"var v{p in P,k in K,e in E}, >=0;\n";
    FileMod<<"var d{p in P,j in J}, >=0;\n";
    FileMod<<"var theta{p in P,j in J}, binary;\n";
	FileMod<<"var ttheta{p in P,j in J}, >=0;\n";
	FileMod<<"var PriorityDelay;\n";
	FileMod<<"var Flex;\n";
	FileMod<<"var zeta1{p in CP1,c in C}, binary;\n";
	FileMod<<"var zetatl{p in CP1,c in C}, >=0;\n";
	FileMod<<"var zeta2{p in CP1,c in C}, binary;\n";
	FileMod<<"var zetatu{p in CP1,c in C}, >=0;\n";
	FileMod<<"var gamma1{p in CP2,c in C}, binary;\n";
	FileMod<<"var gammatl{p in CP2,c in C}, >=0;\n";
	FileMod<<"var gamma2{p in CP2,c in C}, binary;\n";
	FileMod<<"var gammatu{p in CP2,c in C}, >=0;\n";
	FileMod<<"var coordDelay1{p in CP1,c in C}, >=0;\n";
	FileMod<<"var coordDelay2{p in CP2,c in C}, >=0;\n";
	
	if (codeUsage==ADAPTIVE_PRIORITY) // If arrival table is considered ( in Integrated Priority and Adaptive Control)
	{
		FileMod<<"var miu{i in I}, binary; \n";
		FileMod<<"var rd{i in I}, >=0; \n";
		FileMod<<"var q{i in I}, >=0; \n";
		FileMod<<"var qq{i in I}; \n";
		FileMod<<"var qIndic{i in I}, binary; \n";
		FileMod<<"var qmiu{i in I}, >=0; \n";
		FileMod<<"var tmiu{ i in I}, >=0; \n";
		FileMod<<"var gmiu{ i in I}, >=0; \n";
		FileMod<<"var del{i in I}, binary; \n";
		FileMod<<"var TotRegVehDel, >=0; \n";
		FileMod<<"var delT1{i in I}, >=0; \n";
		FileMod<<"var delT2{i in I}, >=0; \n";
		FileMod<<"var ze{i in I}, binary; \n";
		FileMod<<"var delZe{i in I}, binary; \n";
		FileMod<<"var delZeT1{i in I}, >=0; \n";
		FileMod<<"var delZeT2{i in I}, >=0; \n";
	}
	FileMod<<"  \n";
	
	// ===================Constraints==============================
	
	FileMod<<"s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  \n";
    FileMod<<"s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  \n";
    FileMod<<"s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  \n";
    // # constraints in the same cycle in same P?? 
    FileMod<<"s.t. Prec_11_11_c1{e in E,p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod<<"s.t. Prec_12_12_c1{e in E,p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod<<"s.t. Prec_21_21_c1{e in E,p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod<<"s.t. Prec_22_22_c1{e in E,p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    // # constraints in the same cycle in connecting   
    FileMod<<"s.t. Prec_11_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
    FileMod<<"s.t. Prec_11_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
    FileMod<<"s.t. Prec_21_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
    FileMod<<"s.t. Prec_21_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
    // #================ END of cycle 1======================#  
    
    // # constraints in the same cycle in same P??  
    FileMod<<"s.t. Prec_11_11_c23{e in E,p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";  
    FileMod<<"s.t. Prec_12_12_c23{e in E,p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";  
    FileMod<<"s.t. Prec_21_21_c23{e in E,p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";  
    FileMod<<"s.t. Prec_22_22_c23{e in E,p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";  
    
    // # constraints in the same cycle in connecting   
    FileMod<<"s.t. Prec_11_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
    FileMod<<"s.t. Prec_11_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
    FileMod<<"s.t. Prec_21_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";
    FileMod<<"s.t. Prec_21_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";
    
    // # constraints in connecting in different cycles  
    FileMod<<"s.t. Prec_12_11_c23{e in E,p in P11, k in K: (card(P11)+p+1)=4 and k>1 }:    t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";  
    FileMod<<"s.t. Prec_22_11_c23{e in E,p in P11, k in K: (card(P11)+p+1+4)=8 and k>1 }:  t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";  
    FileMod<<"s.t. Prec_12_21_c23{e in E,p in P21, k in K: (card(P21)+p+1-4)=4 and k>1 }:  t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";  
    FileMod<<"s.t. Prec_22_21_c23{e in E,p in P21, k in K: (card(P21)+p+1)=8 and k>1 }:    t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";  
    
  
	
	FileMod<<"s.t. PhaseLen{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];\n"; 
	FileMod<<"s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";   
	FileMod<<"s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";  
	   
	FileMod<<"s.t. PrioDelay1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; \n"; 
	FileMod<<"s.t. PrioDelay2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n"; 
	FileMod<<"s.t. PrioDelay3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];\n"; 
	FileMod<<"s.t. PrioDelay4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);\n"; 
	FileMod<<"s.t. PrioDelay5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];\n"; 
	FileMod<<"s.t. PrioDelay6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];\n"; 
	FileMod<<"s.t. PrioDelay7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];\n"; 
	FileMod<<"s.t. PrioDelay8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; \n";
	FileMod<<"s.t. PrioDelay9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";
    if (dCoordinationWeight>0)
	{
		FileMod<<"s.t. c0{p in CP1,c in C: active_coord1[p,c]>0}: coordDelay1[p,c] = ( zetatl[p,c] - Cl1[p,c]*zeta1[p,c] ) + (-zetatu[p,c] + Cu1[p,c]*zeta2[p,c]);\n";
		FileMod<<"s.t. cc0{p in CP2,c in C: active_coord2[p,c]>0}: coordDelay2[p,c] = ( gammatl[p,c] - Cl2[p,c]*gamma1[p,c] ) + (-gammatu[p,c] + Cu2[p,c]*gamma2[p,c]);\n";

		FileMod<<"s.t. c1{p in CP1: active_coord1[p,1]>0}: M*zeta1[p,1] >= (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) - Cl1[p,1]; \n";
		FileMod<<"s.t. c2{p in CP1: active_coord1[p,1]>0}: Cl1[p,1] - (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) <= M*(1-zeta1[p,1]);\n";
		FileMod<<"s.t. c3{p in CP1: active_coord1[p,1]>0}: (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) - M*(1-zeta1[p,1]) <= zetatl[p,1]; \n";
		FileMod<<"s.t. c4{p in CP1: active_coord1[p,1]>0}: zetatl[p,1] <= (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) + M*(1-zeta1[p,1]) ; \n";
		FileMod<<"s.t. c5{p in CP1: active_coord1[p,1]>0}: zetatl[p,1] <= M*zeta1[p,1]; \n";


		FileMod<<"s.t. c6{p in CP1: active_coord1[p,2]>0}: M*zeta1[p,2] >= (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) - Cl1[p,2]; \n";
		FileMod<<"s.t. c7{p in CP1: active_coord1[p,2]>0}: Cl1[p,2] - (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) <= M*(1-zeta1[p,2]);\n";
		FileMod<<"s.t. c8{p in CP1: active_coord1[p,2]>0}: (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) - M*(1-zeta1[p,2]) <= zetatl[p,2]; \n";
		FileMod<<"s.t. c9{p in CP1: active_coord1[p,2]>0}: zetatl[p,2] <= (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) + M*(1-zeta1[p,2]) ; \n";
		FileMod<<"s.t. c10{p in CP1: active_coord1[p,2]>0}: zetatl[p,2] <= M*zeta1[p,2]; \n";


		FileMod<<"s.t. c11{p in CP1: active_coord1[p,1]>0}: M*zeta2[p,1] >= Cu1[p,1] - coef[p,1]*(g[p,1,1]+t[p,1,1]) - (1-coef[p,1])*(g[p,2,1]+t[p,2,1]); \n";
		FileMod<<"s.t. c12{p in CP1: active_coord1[p,1]>0}: coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) - Cu1[p,1] <= M*(1-zeta2[p,1]); \n";
		FileMod<<"s.t. c13{p in CP1: active_coord1[p,1]>0}: coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) - M*(1-zeta2[p,1]) <= zetatu[p,1]; \n";
		FileMod<<"s.t. c14{p in CP1: active_coord1[p,1]>0}: zetatu[p,1] <= coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) + M*(1-zeta2[p,1]) ; \n";
		FileMod<<"s.t. c15{p in CP1: active_coord1[p,1]>0}: zetatu[p,1] <= M*zeta2[p,1]; \n";



		FileMod<<"s.t. c16{p in CP1: active_coord1[p,2]>0}: M*zeta2[p,2] >= Cu1[p,2]- coef[p,1]*(g[p,2,1]+t[p,2,1]) - (1-coef[p,1])*(g[p,3,1]+t[p,3,1]); \n"; 
		FileMod<<"s.t. c17{p in CP1: active_coord1[p,2]>0}: (coef[p,1])*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1])-Cu1[p,2] <= M*(1-zeta2[p,2]); \n";
		FileMod<<"s.t. c18{p in CP1: active_coord1[p,2]>0}: coef[p,1]*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1]) - M*(1-zeta2[p,2])  <= zetatu[p,2]; \n";
		FileMod<<"s.t. c19{p in CP1: active_coord1[p,2]>0}: zetatu[p,2] <=  coef[p,1]*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1])+ M*(1-zeta2[p,2]) ;\n"; 
		FileMod<<"s.t. c20{p in CP1: active_coord1[p,2]>0}: zetatu[p,2] <= M*zeta2[p,2];\n"; 


		FileMod<<"s.t. cc1{p in CP2: active_coord2[p,1]>0}: M*gamma1[p,1] >= (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) - Cl2[p,1];\n"; 
		FileMod<<"s.t. cc2{p in CP2: active_coord2[p,1]>0}: Cl2[p,1] - (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) <= M*(1-gamma1[p,1]);\n";
		FileMod<<"s.t. cc3{p in CP2: active_coord2[p,1]>0}: (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) - M*(1-gamma1[p,1]) <= gammatl[p,1]; \n";
		FileMod<<"s.t. cc4{p in CP2: active_coord2[p,1]>0}: gammatl[p,1] <= (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) + M*(1-gamma1[p,1]);\n"; 
		FileMod<<"s.t. cc5{p in CP2: active_coord2[p,1]>0}: gammatl[p,1] <= M*gamma1[p,1]; \n";


		FileMod<<"s.t. cc6{p in CP2: active_coord2[p,2]>0}: M*gamma1[p,2] >= (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) - Cl2[p,2];\n"; 
		FileMod<<"s.t. cc7{p in CP2: active_coord2[p,2]>0}: Cl2[p,2] - (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) <= M*(1-gamma1[p,2]);\n";
		FileMod<<"s.t. cc8{p in CP2: active_coord2[p,2]>0}: (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) - M*(1-gamma1[p,2]) <= gammatl[p,2]; \n";
		FileMod<<"s.t. cc9{p in CP2: active_coord2[p,2]>0}: gammatl[p,2] <= (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) + M*(1-gamma1[p,2]);\n"; 
		FileMod<<"s.t. cc10{p in CP2: active_coord2[p,2]>0}: gammatl[p,2] <= M*gamma1[p,2]; \n";


		FileMod<<"s.t. cc11{p in CP2: active_coord2[p,1]>0}: M*gamma2[p,1] >= Cu2[p,1]-coef[p,1]*(g[p,1,1]+t[p,1,1]) - (1-coef[p,1])*(g[p,2,1]+t[p,2,1]); \n";
		FileMod<<"s.t. cc12{p in CP2: active_coord2[p,1]>0}: coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) - Cu2[p,1] <= M*(1-gamma2[p,1]); \n";
		FileMod<<"s.t. cc13{p in CP2: active_coord2[p,1]>0}: coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) - M*(1-gamma2[p,1]) <= gammatu[p,1]; \n";
		FileMod<<"s.t. cc14{p in CP2: active_coord2[p,1]>0}: gammatu[p,1] <= coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) + M*(1-gamma2[p,1]) ;\n"; 
		FileMod<<"s.t. cc15{p in CP2: active_coord2[p,1]>0}: gammatu[p,1] <= M*gamma2[p,1]; \n";



		FileMod<<"s.t. cc16{p in CP2: active_coord2[p,2]>0}: M*gamma2[p,2] >= Cu2[p,2]-coef[p,1]*(g[p,2,1]+t[p,2,1]) - (1-coef[p,1])*(g[p,3,1]+t[p,3,1]); \n"; 
		FileMod<<"s.t. cc17{p in CP2: active_coord2[p,2]>0}: (coef[p,1])*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1])-Cu2[p,2] <= M*(1-gamma2[p,2]);\n"; 
		FileMod<<"s.t. cc18{p in CP2: active_coord2[p,2]>0}: coef[p,1]*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1]) - M*(1-gamma2[p,2])  <= gammatu[p,2]; \n";
		FileMod<<"s.t. cc19{p in CP2: active_coord2[p,2]>0}: gammatu[p,2] <=  coef[p,1]*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1])+ M*(1-gamma2[p,2]) ;\n"; 
		FileMod<<"s.t. cc20{p in CP2: active_coord2[p,2]>0}: gammatu[p,2] <= M*gamma2[p,2]; \n";
	}
	
		
 	if (codeUsage==ADAPTIVE_PRIORITY)
	{
		FileMod<<"s.t. RVehD1{i in I: (active_arr[i]>0)}: rd[i] >= t[Ph[i],1]+q[i]/s-(Ar[i]-q[i])/Ve[i]+Tq[i];  \n ";
		FileMod<<"s.t. RVehD2{i in I: (active_arr[i]>0)} : M*miu[i] >= q[i]/s + Ar[i]/Ve[i] -g[Ph[i],1];  \n ";
		FileMod<<"s.t. RVehD3{i in I: (active_arr[i]>0 )} : rd[i] >= tmiu[i] + qmiu[i]/s - gmiu[i] - Ar[i]*miu[i]/Ve[i] + qmiu[i]/Ve[i] + Tq[i]; \n ";
		FileMod<<"s.t. RVehD5{i in I,ii in I: (active_arr[i]>0 and active_arr[ii]>0  and Ln[i]=Ln[ii] and Ar[i]<Ar[ii])} : miu[i] <= miu[ii] ; \n ";
		FileMod<<"s.t. RVehD6{i in I: (active_arr[i]>0)}: tmiu[i]<=M*miu[i];   \n ";
		FileMod<<"s.t. RVehD7{p in P,i in I: (active_arr[i]>0 and p=Ph[i])}: t[p,2]-M*(1-miu[i])<=tmiu[i];   \n ";
		FileMod<<"s.t. RVehD8{p in P,i in I: (active_arr[i]>0 and p=Ph[i])}: t[p,2]+M*(1-miu[i])>=tmiu[i]; \n ";
		FileMod<<"s.t. RVehD9{i in I: active_arr[i]>0 }: gmiu[i]<=M*miu[i];   \n ";
		FileMod<<"s.t. RVehD10{p in P,i in I: (active_arr[i]>0 and p=Ph[i])}: g[p,1]-M*(1-miu[i])<=gmiu[i];   \n ";
		FileMod<<"s.t. RVehD11{p in P,i in I: (active_arr[i]>0 and p=Ph[i])}: g[p,1]+M*(1-miu[i])>=gmiu[i]; \n ";
		FileMod<<"s.t. RVehD12{i in I: (active_arr[i]>0)}: qmiu[i]<=M*miu[i];   \n ";
		FileMod<<"s.t. RVehD13{i in I: (active_arr[i]>0)}: q[i]-M*(1-miu[i])<=qmiu[i];   \n ";
		FileMod<<"s.t. RVehD14{i in I: (active_arr[i]>0)}: q[i]+M*(1-miu[i])>=qmiu[i]; \n ";
		FileMod<<"s.t. RVehD15{i in I,k in K: (active_arr[i]>0)}: qq[i]=((LaPhSt[Ln[i]])*(L0[Ln[i]]+Ratio[i]*qSp-g[Ph[i],1]*s)*indic[i])+((1-LaPhSt[Ln[i]])*(L0[Ln[i]]*del[i]+Ratio[i]*qSp*del[i]-s*Ratio[i]*del[i]+s*((1-coef[Ph[i],1])*delT2[i]+coef[Ph[i],1]*delT1[i])-s*Ratio[i]*delZe[i]-s*((1-coef[Ph[i],1])*delZeT2[i]+coef[Ph[i],1]*delZeT1[i]))); \n ";
		FileMod<<"s.t. RVehD16{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*(t[Ph[i],1]-M*(1-del[i]))<=coef[Ph[i],1]*delT1[i];  \n ";
		FileMod<<"s.t. RVehD17{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*(t[Ph[i],1]+M*(1-del[i]))>=coef[Ph[i],1]*delT1[i]; \n ";
		FileMod<<"s.t. RVehD18{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*delT1[i]<=M*del[i]; \n ";
		FileMod<<"s.t. RVehD19{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*t[Ph[i],2]-M*(1-del[i])<=(1-coef[Ph[i],1])*delT2[i];  \n ";
		FileMod<<"s.t. RVehD20{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*t[Ph[i],2]+M*(1-del[i])>=(1-coef[Ph[i],1])*delT2[i];  \n ";
		FileMod<<"s.t. RVehD21{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*delT2[i]<=M*del[i];  \n ";
		FileMod<<"s.t. RVehD22{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*(t[Ph[i],1]-M*(1-delZe[i]))<=delZeT1[i];  \n ";
		FileMod<<"s.t. RVehD23{i in I: (active_arr[i]>0)}: t[Ph[i],1]+M*(1-delZe[i])>=coef[Ph[i],1]*delZeT1[i];  \n ";
		FileMod<<"s.t. RVehD24{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*delZeT1[i]<=M*delZe[i];  \n ";
		FileMod<<"s.t. RVehD25{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*(t[Ph[i],2]-M*(1-delZe[i]))<=delZeT2[i];  \n ";
		FileMod<<"s.t. RVehD26{i in I: (active_arr[i]>0)}: t[Ph[i],2]+M*(1-delZe[i])>=(1-coef[Ph[i],1])*delZeT2[i];  \n ";
		FileMod<<"s.t. RVehD27{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*delZeT2[i]<=M*delZe[i];  \n ";
		FileMod<<"s.t. RVehD28{i in I: (active_arr[i]>0)}: delZe[i]<=del[i];  \n ";
		FileMod<<"s.t. RVehD29{i in I: (active_arr[i]>0)}: delZe[i]<=ze[i];  \n ";
		FileMod<<"s.t. RVehD30{i in I: (active_arr[i]>0)}: delZe[i]>=del[i]+ze[i]-1;  \n ";
		FileMod<<"s.t. RVehD31{i in I: (active_arr[i]>0)}: ((1-coef[Ph[i],1])*t[Ph[i],2] + coef[Ph[i],1]*t[Ph[i],1] - Ratio[i])<=M*ze[i];  \n ";
		FileMod<<"s.t. RVehD32{i in I: (active_arr[i]>0)}: ((1-coef[Ph[i],1])*t[Ph[i],2] + coef[Ph[i],1]*t[Ph[i],1] - Ratio[i])>=M*(ze[i]-1);  \n ";
		FileMod<<"s.t. RVehD33{i in I: (active_arr[i]>0)}: s*(((1-coef[Ph[i],1])*t[Ph[i],2] + coef[Ph[i],1]*t[Ph[i],1]) + L0[Ln[i]])/(s-qSp)- Ratio[i]<=M*del[i];  \n ";
		FileMod<<"s.t. RVehD34{i in I: (active_arr[i]>0)}: s*(((1-coef[Ph[i],1])*t[Ph[i],2] + coef[Ph[i],1]*t[Ph[i],1]) + L0[Ln[i]])/(s-qSp)- Ratio[i]>=M*(del[i]-1);  \n ";
		FileMod<<"s.t. RVehD35{i in I: (active_arr[i]>0)}: qq[i]<=M*qIndic[i];  \n ";
		FileMod<<"s.t. RVehD36{i in I: (active_arr[i]>0)}: qq[i]>=M*(qIndic[i]-1);  \n ";
		FileMod<<"s.t. RVehD37{i in I: (active_arr[i]>0)}: qq[i]-M*(1-qIndic[i])<=q[i];  \n ";
		FileMod<<"s.t. RVehD38{i in I: (active_arr[i]>0)}: qq[i]+M*(1-qIndic[i])>=q[i];  \n ";
		FileMod<<"s.t. RVehD39{i in I: (active_arr[i]>0)}: q[i]<=M*qIndic[i];  \n ";
		FileMod<<"s.t. TotRegVehDelay: TotRegVehDel=(sum{i in I} active_arr[i]*rd[i])/SumOfActiveArr;  \n";
	}
 
    
	FileMod<<"s.t. Flexib: Flex= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];\n ";
	if (haveEVinList!=1)
		FileMod<<"s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) )  + PrioWeigth[6]*(sum{p in CP1,c in C: active_coord1[p,c]>0} coordDelay1[p,c] + sum{p in CP2,c in C: active_coord2[p,c]>0} coordDelay2[p,c])  - 0.01*Flex; \n ";   // The coeficient to Flex should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points  
   	if (haveEVinList==1) // incase there is EV in the list, there is not need to provide felexibility for signal actuation and also no need for considering coorination
		FileMod<<"s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) ) ; \n ";  
	
   	if(codeUsage!=ADAPTIVE_PRIORITY) // in case we do not consider egular vehicles (no arrival table)
		FileMod<<"minimize delay: PriorityDelay  ;\n";
	else
		FileMod<<"  minimize delay:  TotRegVehDel+  PriorityDelay;     \n";
	//=============================Writing the Optimal Output into the Results.txt file ==================================
    FileMod<<"  \n";
    FileMod<<"solve;  \n";
    FileMod<<"  \n";
    FileMod<<"printf \" \" > \"/nojournal/bin/Results.txt\";  \n";
    FileMod<<"printf \"%3d  %3d \\n \",SP1, SP2 >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod<<"printf \"%5.2f  %5.2f %5.2f  %5.2f \\n \",init1, init2,Grn1,Grn2 >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod<<"for {k in K}   \n";
    FileMod<<" { \n";
    FileMod<<"     for {p in P2} \n";
    FileMod<<"        { \n";
    FileMod<<"           printf \"%5.2f  \", if(p in P)  then v[p,k,1] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod<<"        } \n";
    FileMod<<"        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod<<" } \n";
    FileMod<<"  \n";
    FileMod<<"for {k in K}   \n";
    FileMod<<" { \n";
    FileMod<<"     for {p in P2} \n";
    FileMod<<"        { \n";
    FileMod<<"           printf \"%5.2f  \", if(p in P)  then v[p,k,2] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod<<"        } \n";
    FileMod<<"        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod<<" } \n";
    FileMod<<"for {k in K}   \n";
    FileMod<<" { \n";
    FileMod<<"     for {p in P2} \n";
    FileMod<<"        { \n";
    FileMod<<"           printf \"%5.2f  \", if(p in P)  then g[p,k,1] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod<<"        } \n";
    FileMod<<"        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod<<" } \n";
    FileMod<<"  \n";
    FileMod<<"for {k in K}   \n";
    FileMod<<" { \n";
    FileMod<<"     for {p in P2} \n";
    FileMod<<"        { \n";
    FileMod<<"           printf \"%5.2f  \", if(p in P)  then g[p,k,2] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod<<"        } \n";
    FileMod<<"        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod<<" } \n";
    FileMod<<"  \n";
    FileMod<<"printf \"%3d \\n \", ReqNo >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod<<"  \n";
    FileMod<<"for {p in P,j in J : Rl[p,j]>0}  \n";
    FileMod<<" {  \n";
    FileMod<<"   printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1)), Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>\"/nojournal/bin/Results.txt\";\n"; // the  term " coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1))" is used to know the request is served in which cycle. For example, aasume there is a request for phase 4. If the request is served in firsr cycle, the term will be 4, the second cycle, the term will be 14 and the third cycle, the term will be 24 
    FileMod<<" } \n";
    FileMod<<"printf \"%3d \\n \", CoordNo >>\"/nojournal/bin/Results.txt\";  \n";
    if ( dCoordinationWeight >0)
    {
		if ( dCoordinationSplit[0] > 0.0 )
		{
			FileMod<<"for {c in C,p in CP: active_coord1[p,c]>0} \n"; 
			FileMod<<"        { \n";
			FileMod<<"		printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", p, Cl1[p,c],Cu1[p,c], coordDelay1[p,c] , 6 >>\"/nojournal/bin/Results.txt\";  \n";
			FileMod<<"        }\n";
		}
		if ( dCoordinationSplit[1] > 0.0 )
		{
			FileMod<<"for {c in C,p in CP: active_coord2[p,c]>0} " << "\n"; 
			FileMod<<"        { \n";
			FileMod<<"		printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", p, Cl2[p,c],Cu2[p,c], coordDelay2[p,c] , 6 >>\"/nojournal/bin/Results.txt\";  \n";
			FileMod<<"         }\n";
		}
	}
	
    FileMod<<"printf \"%5.2f \\n \", PriorityDelay + 0.01*Flex>>\"/nojournal/bin/Results.txt\"; \n";
    if(codeUsage==ADAPTIVE_PRIORITY) // in case we do not consider egular vehicles (no arrival table)
		FileMod<<"printf \"%5.2f \\n \", TotRegVehDel >>\"/nojournal/bin/Results.txt\";  	\n ";
	FileMod<<"printf \"%5.2f \\n \", sum{p in CP1,c in C: active_coord1[p,c]>0} coordDelay1[p,c] + sum{p in CP2,c in C: active_coord2[p,c]>0} coordDelay2[p,c]  >>\"/nojournal/bin/Results.txt\"; \n";
    FileMod<<"printf \"%5.2f \\n \", Flex >>\"/nojournal/bin/Results.txt\"; \n";
    FileMod<<"printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    //------------- End of Print the Main body of mode----------------
    FileMod<<"end;\n";
    FileMod.close();
}



void UnpackTrajData1(char* ablob)
{
	int No_Veh;

	int offset;
	offset=0;
	long    tempLong;
	unsigned char   byteA;  // force to unsigned this time,
	unsigned char   byteB;  // we do not want a bunch of sign extension 
	unsigned char   byteC;  // math mucking up our combine logic
	unsigned char   byteD;
	
	//Header
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	int temp = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;
	
	//cout<<temp<<endl;
	
	//id
	temp = (char)ablob[offset];
	offset = offset + 1; // move past to next item
	//cout<<temp<<endl;
	
	
	//Do vehicle number
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	No_Veh = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;
	
	//cout<<No_Veh<<endl;
	
	//Do each vehicle
	for(int i=0;i<No_Veh;i++)
	{
		ConnectedVehicle TempVeh;
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
		
		//do approach
		TempVeh.approach = (char)ablob[offset];
	    offset = offset + 1; // move past to next item
		
		//do lane
		TempVeh.lane = (char)ablob[offset];
	    offset = offset + 1; // move past to next item
		
		//do req_phase
		TempVeh.req_phase = (char)ablob[offset];
	    offset = offset + 1; // move past to next item
		
		//do stop time
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.time_stop = (tempLong); // convert and store as float
		offset = offset + 4;
		
		//cout<<"E_Offset["<<j<<"] is:"<<TempVeh.E_Offset[j]<<endl;
		//cout<<"Done with one vehicle"<<endl;
		
		//Here reflects the penetration rate!!!!!!!!!!!!!!!!!!!!!!!!!!! Add only half of the vehicles
	//	if (penetration>0.99)  //%100 penetration rate
	//	{
		trackedveh.InsertRear(TempVeh); 
	//	}
	//	if (penetration>0.74 && penetration<0.76) //75% penetration rate
	//	{
	//		if (TempVeh.TempID%4==3 || TempVeh.TempID%4==1 ||TempVeh.TempID%4==2)
	//			trackedveh.InsertRear(TempVeh); 
	//	}
	//	if (penetration>0.49 && penetration<0.51)	//50% penetration rate
	//	{
	//		if (TempVeh.TempID%2==1)
	//			trackedveh.InsertRear(TempVeh); 
	//	}
	//	if (penetration>0.24 && penetration<0.26)	//25% penetration rate
	//	{
	//		if (TempVeh.TempID%4==1)
	//			trackedveh.InsertRear(TempVeh); 
	//	} 
	}
	
}


int numberOfPlannedPhase( double cp[3][15])
{
	{
		int NoPhaseRng1=0;
		for (int i=0; i<15; i++)
		{
			if (( cp[0][i]>0 && cp[0][i+1]==0 )|| (cp[1][i]>0 && cp[1][i+1]==0)) 
			{
				NoPhaseRng1=i+1;	
			}
		}
		return  NoPhaseRng1;
	}
}


void printCriticalPoints()
{
	cout<<" RING 1 CRITICAL POINTS "<<endl;
	for (int ii=0;ii<15;ii++)
	{
		cout<<"CP left              "<< ii << " "<< adCriticalPoints[0][0][ii]<<endl;
		cout<<"CP right             "<< ii << " "<< adCriticalPoints[0][1][ii]<<endl;
		cout<<"phase                "<< adCriticalPoints[0][2][ii] +1 <<endl;
	}
	cout<<endl;
	cout<<" RING 2 CRITICAL POINTS "<<endl;
	for (int ii=0;ii<15;ii++)
	{
		cout<<"CP left              "<< ii << " "<< adCriticalPoints[1][0][ii]<<endl;
		cout<<"CP right             "<< ii << " "<< adCriticalPoints[1][1][ii]<<endl;
		cout<<"phase                "<< adCriticalPoints[1][2][ii] + 5<<endl;
	}
	//sprintf(temp_log,".......... The new OPT set sent  At time: %.2f.......... \n", GetSeconds()); outputlog(temp_log); cout<< temp_log<< endl;
}


void packOPT(char* buffer,double cp[2][3][15], int msgCnt)
{
	int temp;
	int iNoPlannedPhaseInRing1=0;
    int iNoPlannedPhaseInRing2=0;
	iNoPlannedPhaseInRing1=numberOfPlannedPhase(cp[0]);
	iNoPlannedPhaseInRing2=numberOfPlannedPhase(cp[1]);
	cout<<"iNoPlannedPhaseInRing1 "<<iNoPlannedPhaseInRing1<<endl;
	cout<<"iNoPlannedPhaseInRing2 "<<iNoPlannedPhaseInRing2<<endl;
	int offset=0;
	char*   pByte;      // pointer used (by cast)to get at each byte 
	unsigned short   tempUShort;
	long templong;
	
	tempUShort = (unsigned short)OPTMsgID;
	pByte = (char*) & tempUShort;
	buffer[offset+0]= (char) * (pByte+1);
	buffer[offset+1]= (char) * (pByte+0);
	offset=offset+2;
	
	tempUShort = (unsigned short)msgCnt;
	pByte = (char*) & tempUShort;
	buffer[offset+0]= (char) * (pByte+1);
	buffer[offset+1]= (char) * (pByte+0);
	offset=offset+2;
	
	buffer[offset+0]=0X01;
	offset++;
	
	tempUShort = (unsigned short)iNoPlannedPhaseInRing1;
	pByte = (char*) & tempUShort;
	buffer[offset+0]= (char) * (pByte+1);
	buffer[offset+1]= (char) * (pByte+0);
	offset=offset+2;
	
	for (int ii=0;ii<iNoPlannedPhaseInRing1;ii++)
	{
		templong = (long)cp[0][0][ii]*1000000;
		pByte = (char*) & templong;
		buffer[offset+0]= (char) * (pByte+3);
		buffer[offset+1]= (char) * (pByte+2);
		buffer[offset+2]= (char) * (pByte+1);
		buffer[offset+3]= (char) * (pByte+0);
		offset=offset+4;
		
		templong = (long)cp[0][1][ii]*1000000;
		pByte = (char*) & templong;
		buffer[offset+0]= (char) * (pByte+3);
		buffer[offset+1]= (char) * (pByte+2);
		buffer[offset+2]= (char) * (pByte+1);
		buffer[offset+3]= (char) * (pByte+0);
		offset=offset+4;
		
		temp= (int) (cp[0][2][ii]+1);  // by adding 1 to cp[0][2][ii]  ,  the sent phase number is 1 2 3 or 4 
		tempUShort = (unsigned short)temp;
		pByte = (char*) & tempUShort;
		buffer[offset+0]= (char) * (pByte+1);
		buffer[offset+1]= (char) * (pByte+0);
		offset=offset+2;
	}
	buffer[offset+0]=0X02;
	offset++;
	
	tempUShort = (unsigned short)iNoPlannedPhaseInRing2;
	pByte = (char*) & tempUShort;
	buffer[offset+0]= (char) * (pByte+1);
	buffer[offset+1]= (char) * (pByte+0);
	offset=offset+2;	
	for (int i=0;i<iNoPlannedPhaseInRing2;i++)
	{
		templong = (long)cp[1][0][i]*1000000;
		pByte = (char*) & templong;
		buffer[offset+0]= (char) * (pByte+3);
		buffer[offset+1]= (char) * (pByte+2);
		buffer[offset+2]= (char) * (pByte+1);
		buffer[offset+3]= (char) * (pByte+0);
		offset=offset+4;
		
		templong = (long)cp[1][1][i]*1000000;
		pByte = (char*) & templong;
		buffer[offset+0]= (char) * (pByte+3);
		buffer[offset+1]= (char) * (pByte+2);
		buffer[offset+2]= (char) * (pByte+1);
		buffer[offset+3]= (char) * (pByte+0);
		offset=offset+4;
		
		temp= (int) (cp[1][2][i]+5);  // by adding 5 to cp[0][2][ii]  ,  the sent phase number is 5 6 7 or 8 
		tempUShort = (unsigned short)temp;
		pByte = (char*) & tempUShort;
		buffer[offset+0]= (char) * (pByte+1);
		buffer[offset+1]= (char) * (pByte+0);
		offset=offset+2;
	}
}

void Construct_eventlist_EV(double cp [2][3][15], int omitphase[8], LinkedList<ReqEntry> Req_List)
{
	int tempOmitPhases[8];
	int iNoOfOmit=0;
	Schedule Temp_event;
	
	int iNoPlannedPhaseInRing1=0;
    int iNoPlannedPhaseInRing2=0;
	iNoPlannedPhaseInRing1=numberOfPlannedPhase(cp[0]);
	iNoPlannedPhaseInRing2=numberOfPlannedPhase(cp[1]);
	
	// Consider the requested phases of EV to put CALL for them every 0.9 seconds
	int iReqPhases[8]={0};
	double dEndOfRequest[8]={0.0};
	double dEndofSchedule=0.0;// this variable holds the time that the HOLD commands should be put. The value equals to the ETA of the last EV.
	Req_List.Reset();
	if(Req_List.ListEmpty()==0)
	{
		while(!Req_List.EndOfList())
		{
			if (Req_List.Data().VehClass==1)
			{
				iReqPhases[Req_List.Data().Phase-1]=1;
				dEndOfRequest[Req_List.Data().Phase-1] = max ( dEndOfRequest[Req_List.Data().Phase-1]  , (double) Req_List.Data().ETA );
			}
			Req_List.Next();
		}
	}
	for (int i=0;i<8;i++)
	{
		dEndofSchedule=max(dEndOfRequest[i],dEndofSchedule);
	}

	// considering the omited phases
	for (int i=0;i<8;i++)
	{
		if (omitphase[i]>0)
		{
			tempOmitPhases[iNoOfOmit]=omitphase[i];
			iNoOfOmit++;	
		}
	}
	//set OMIT Events
	for (int i=0;i<iNoOfOmit;i++)
	{
		Temp_event.time=dEndofSchedule;
		Temp_event.action=PHASE_OMIT;
		Temp_event.phase=tempOmitPhases[i]; 
		if (tempOmitPhases[i]<5)
			Eventlist_R1.InsertRear(Temp_event);	
		else
			Eventlist_R2.InsertRear(Temp_event);	
	}
	
	for (int i=0; i<iNoPlannedPhaseInRing1; i++)
	{
		// hold
		Temp_event.time=cp[0][0][i];
		Temp_event.action=PHASE_HOLD;
		Temp_event.phase=((int)(cp[0][2][i]))+1; // converting phase number from 0-3 to  1-4
		Eventlist_R1.InsertRear(Temp_event);	
		
		// MZ commentee on 5/4/17
		/*
		// call , the call is neccessary before force off. The controller should know where to go ( which phase will come up after force off )
		if ( i<iNoPlannedPhaseInRing1-1)
		{
			
			Temp_event.time=cp[0][1][i];
			if ( ( ((int)(cp[0][2][i+1]))+1 ) ==5 )
				Temp_event.phase=1;
			else
				Temp_event.phase=((int)(cp[0][2][i+1]))+1;// converting phase number from 0-3 to  1-4
			Temp_event.action=PHASE_VEH_CALL;
			Eventlist_R1.InsertRear(Temp_event);	
			
		}
		*/
		// force off
		Temp_event.time=cp[0][1][i];
		Temp_event.action=PHASE_FORCEOFF;
		Temp_event.phase=((int)(cp[0][2][i]))+1;// converting phase number from 0-3 to  1-4
		Eventlist_R1.InsertRear(Temp_event);	
	}
	
	for (int i=0; i<iNoPlannedPhaseInRing2; i++)
	{
		// hold
		Temp_event.time=cp[1][0][i];
		Temp_event.action=PHASE_HOLD;
		Temp_event.phase=((int) (cp[1][2][i]))+5;// converting phase number from 0-3 to  5-8
		Eventlist_R2.InsertRear(Temp_event);	
			
		// MZ commentee on 5/4/17
		/*
		// call
		if ( i<iNoPlannedPhaseInRing2-1)
		{
			Temp_event.time=cp[1][1][i];
			Temp_event.action=PHASE_VEH_CALL;
			if ( ((int)(cp[1][2][i+1]))+5 == 9 )
				Temp_event.phase=5;
			else
				Temp_event.phase=((int)(cp[1][2][i+1]))+5;// converting phase number from 0-3 to  5-8
			Eventlist_R1.InsertRear(Temp_event);	
			
		}	
		*/ 
		// force off
		Temp_event.time=cp[1][1][i];
		Temp_event.action=PHASE_FORCEOFF;
		Temp_event.phase=((int) (cp[1][2][i]))+5;   // converting phase number from 0-3 to  5-8
		Eventlist_R2.InsertRear(Temp_event);	
	}
	

	Eventlist_R1.Reset();
	Eventlist_R2.Reset();
	cout<<"RING1 Event List!"<<endl;
	while (! Eventlist_R1.EndOfList())
	{
		cout<<"time "<<Eventlist_R1.Data().time<<" phase "<<Eventlist_R1.Data().phase;
		if (Eventlist_R1.Data().action == PHASE_FORCEOFF)
			cout<<" Force-Off at "<<endl;
		else if (Eventlist_R1.Data().action == PHASE_OMIT)
			cout<<" Omit until"<<endl;
		else if (Eventlist_R1.Data().action == PHASE_VEH_CALL)
			cout<<" Call at"<<endl;
		else if (Eventlist_R1.Data().action == PHASE_HOLD)
			cout<<" Hold until"<<endl;	
		Eventlist_R1.Next();
	}
	cout<<"RING2 Event List!"<<endl;
	while (! Eventlist_R2.EndOfList())
	{
		cout<<"time "<<Eventlist_R2.Data().time<<" phase "<<Eventlist_R2.Data().phase;
		if (Eventlist_R2.Data().action == PHASE_FORCEOFF)
			cout<<" Force-Off at "<<endl;
		else if (Eventlist_R2.Data().action == PHASE_OMIT)
			cout<<" Omit until"<<endl;
		else if (Eventlist_R2.Data().action == PHASE_VEH_CALL)
			cout<<" Call at"<<endl;
		else if (Eventlist_R2.Data().action == PHASE_HOLD)
			cout<<" Hold until"<<endl;	
		Eventlist_R2.Next();
	}
	
	for (int ii=0;ii<8;ii++)
		if (omitPhase[ii]>0)
			cout<<"  omitPhase" << omitPhase[ii]<< endl;
}




void Construct_eventlist(double cp [2][3][15],LinkedList<ReqEntry> Req_List)
{
	int iNoPlannedPhaseInRing1=0;
    int iNoPlannedPhaseInRing2=0;
	iNoPlannedPhaseInRing1=numberOfPlannedPhase(cp[0]);
	iNoPlannedPhaseInRing2=numberOfPlannedPhase(cp[1]);
	//cout<<"No of Planned Phase In Ring1 "<<iNoPlannedPhaseInRing1<<endl;
	//cout<<"No of Planned Phase In Ring2 "<<iNoPlannedPhaseInRing2<<endl;
	
	// Consider the requested phases to put CALL for them
	int iReqPhases[8]={0};
	double dEndOfRequest[8]={0.0};
	
	Req_List.Reset();
	if(Req_List.ListEmpty()==0)
	{
		while(!Req_List.EndOfList())
		{
			if (( 0 < Req_List.Data().Phase ) && ( 9 > Req_List.Data().Phase ) )
			{
				iReqPhases[Req_List.Data().Phase-1]=1;
				dEndOfRequest[Req_List.Data().Phase-1] = max ( dEndOfRequest[Req_List.Data().Phase-1]  , (double) Req_List.Data().ETA );
			}
			Req_List.Next();
		}
	}

	
	
	Schedule Temp_event;
	
	for (int i=0; i<iNoPlannedPhaseInRing1; i++)
	{
		// hold
		Temp_event.time=cp[0][0][i];
		Temp_event.action=PHASE_HOLD;
		Temp_event.phase=(int) (cp[0][2][i]+1); // converting phase number from 0-3 to  1-4
		Eventlist_R1.InsertRear(Temp_event);	
			
		// call, this call is neccesary when we run on Coordination. In coordination mode, 
		// usually the signal should be on recall for coordinated phase, but in case this is not true,
		// coordinated phase should alway have a call. 
		if (ConfigIS.dCoordinationWeight > 0 )
		{	
			double ii=0.0;			
			// Call in every 0.9 seconds to hedge against the worst case scenario when the NTCIP backup time is 1 sec. 
			if (cp[0][2][i]+1 == ConfigIS.iCoordinatedPhase[0])
			{
				while (ii<cp[0][1][i])
				{
					Temp_event.time=ii;
					Temp_event.action=PHASE_VEH_CALL;
					Temp_event.phase=ConfigIS.iCoordinatedPhase[0]; 
					Eventlist_R1.InsertRear(Temp_event);
					ii=ii+0.9;
					//  put Calls for at most next 120 seconds to avoid overwhelming the controller
					if (ii>120)
						ii=cp[0][1][i];
				}
			}
			
		}
		// force off
		Temp_event.time=cp[0][1][i];
		Temp_event.action=PHASE_FORCEOFF;
		Temp_event.phase=(int) (cp[0][2][i]+1);// converting phase number from 0-3 to  1-4
		Eventlist_R1.InsertRear(Temp_event);	
	}
	
	for (int i=0; i<iNoPlannedPhaseInRing2; i++)
	{
		// hold
		Temp_event.time=cp[1][0][i];
		Temp_event.action=PHASE_HOLD;
		Temp_event.phase=(int) (cp[1][2][i]+5);// converting phase number from 0-3 to  5-8
		Eventlist_R2.InsertRear(Temp_event);
		
		// call, this call is neccesary when we run on Coordination. In coordination mode, 
		// usually the signal should be on recall for coordinated phase, but in case this is not true,
		// coordinated phase should alway have a call. 
		if (ConfigIS.dCoordinationWeight >0 )
		{
			// Call in every 0.9 seconds to hedge against the worst case scenario when the NTCIP backup time is 1 sec. 
			double iii=0.0;
			if (cp[1][2][i]+5 ==ConfigIS.iCoordinatedPhase[1])
			{
				while (iii<cp[1][1][i])
				{
					Temp_event.time=iii;
					Temp_event.action=PHASE_VEH_CALL;
					Temp_event.phase=ConfigIS.iCoordinatedPhase[1]; 
					Eventlist_R2.InsertRear(Temp_event);
					iii=iii+0.9;
					//  put Calls for at most next 120 seconds to avoid overwhelming the controller
					if (iii>120)
						iii=cp[1][1][i];
				}
			}
			
		}
		//force off	
		Temp_event.time=cp[1][1][i];
		Temp_event.action=PHASE_FORCEOFF;
		Temp_event.phase=(int) (cp[1][2][i]+5);   // converting phase number from 0-3 to  5-8
		Eventlist_R2.InsertRear(Temp_event);	
	}
	
	// CALL all of the requested phase every 0.9 seconds. Exclude the coodrinated phase because it is already considered 
	// Call  every 0.9 seconds to hedge against the worst case scenario when the NTCIP backup time is 1 sec. 
	for(int i=0;i<4;i++)
	{
		if ( (iReqPhases[i]==1) ) 
		{
			if ( (ConfigIS.dCoordinationWeight>0) && ((i+1)!=ConfigIS.iCoordinatedPhase[0]) || (ConfigIS.dCoordinationWeight<=0) )
			{
				double ii=0.0;
				while (ii<dEndOfRequest[i])
				{
					Temp_event.time=ii;
					Temp_event.action=PHASE_VEH_CALL;
					Temp_event.phase=i+1; 
					Eventlist_R1.InsertRear(Temp_event);
					ii=ii+0.9;
					//  put Calls for at most next 120 seconds to avoid overwhelming the controller
					if (ii>120)
						ii=dEndOfRequest[i];
				}
			}
		}
	}
	for(int i=4;i<8;i++)
	{
		if ( (iReqPhases[i]==1) ) 
		{
			if ( (ConfigIS.dCoordinationWeight>0) && ((i+1)!=ConfigIS.iCoordinatedPhase[1]) || (ConfigIS.dCoordinationWeight<=0) )
			{
				double ii=0.0;
				while (ii<dEndOfRequest[i])
				{
					Temp_event.time=ii;
					Temp_event.action=PHASE_VEH_CALL;
					Temp_event.phase=i+1; 
					Eventlist_R2.InsertRear(Temp_event);
					ii=ii+0.9;
					//  put Calls at most next 120 seconds to avoid overwhelming the controller
					if (ii>120)
						ii=dEndOfRequest[i];
				}
			}
		}
	}
	
	
	Eventlist_R1.Reset();
	Eventlist_R2.Reset();
	cout<<"RING1 Event List!"<<endl;
	while (! Eventlist_R1.EndOfList())
	{
		cout<<"time "<<Eventlist_R1.Data().time<<" phase "<<Eventlist_R1.Data().phase;
		if (Eventlist_R1.Data().action == PHASE_FORCEOFF)
			cout<<" Force-Off at "<<endl;
		else if (Eventlist_R1.Data().action == PHASE_OMIT)
			cout<<" Omit until"<<endl;
		else if (Eventlist_R1.Data().action == PHASE_VEH_CALL)
			cout<<" Call at"<<endl;
		else if (Eventlist_R1.Data().action == PHASE_HOLD)
			cout<<" Hold until"<<endl;	
		Eventlist_R1.Next();
	}
	cout<<"RING2 Event List!"<<endl;
	while (! Eventlist_R2.EndOfList())
	{
		cout<<"time "<<Eventlist_R2.Data().time<<" phase "<<Eventlist_R2.Data().phase;
		if (Eventlist_R2.Data().action == PHASE_FORCEOFF)
			cout<<" Force-Off at "<<endl;
		else if (Eventlist_R2.Data().action == PHASE_OMIT)
			cout<<" Omit unti"<<endl;
		else if (Eventlist_R2.Data().action == PHASE_VEH_CALL)
			cout<<" Call at"<<endl;
		else if (Eventlist_R2.Data().action == PHASE_HOLD)
			cout<<" Hold until"<<endl;	
		Eventlist_R2.Next();
	}
}

void Pack_Event_List(char* tmp_event_data, int &size) // This function is written by YF
{
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
//    byte    tempByte;   // values to hold data once converted to final format
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
	int No_Event_R1= Eventlist_R1.ListSize();
	tempUShort = (unsigned short)No_Event_R1;
	pByte = (byte* ) &tempUShort;
    tmp_event_data[offset+0] = (byte) *(pByte + 1); 
    tmp_event_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	//Events in R1
	Eventlist_R1.Reset();
	while(!Eventlist_R1.EndOfList())
	{
		//Time 
		tempLong = (long)(Eventlist_R1.Data().time * DEG2ASNunits); 
		pByte = (byte* ) &tempLong;
		tmp_event_data[offset+0] = (byte) *(pByte + 3); 
		tmp_event_data[offset+1] = (byte) *(pByte + 2); 
		tmp_event_data[offset+2] = (byte) *(pByte + 1); 
		tmp_event_data[offset+3] = (byte) *(pByte + 0); 
		offset = offset + 4;
		//phase
		tempUShort = (unsigned short)Eventlist_R1.Data().phase;
		pByte = (byte* ) &tempUShort;
		tmp_event_data[offset+0] = (byte) *(pByte + 1); 
		tmp_event_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//action
		tempUShort = (unsigned short)Eventlist_R1.Data().action;
		pByte = (byte* ) &tempUShort;
		tmp_event_data[offset+0] = (byte) *(pByte + 1); 
		tmp_event_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		Eventlist_R1.Next();
	}
	//No. events in R2
	int No_Event_R2= Eventlist_R2.ListSize();
	tempUShort = (unsigned short)No_Event_R2;
	pByte = (byte* ) &tempUShort;
    tmp_event_data[offset+0] = (byte) *(pByte + 1); 
    tmp_event_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	//Events in R1
	Eventlist_R2.Reset();
	while(!Eventlist_R2.EndOfList())
	{
		//Time 
		tempLong = (long)(Eventlist_R2.Data().time * DEG2ASNunits); 
		pByte = (byte* ) &tempLong;
		tmp_event_data[offset+0] = (byte) *(pByte + 3); 
		tmp_event_data[offset+1] = (byte) *(pByte + 2); 
		tmp_event_data[offset+2] = (byte) *(pByte + 1); 
		tmp_event_data[offset+3] = (byte) *(pByte + 0); 
		offset = offset + 4;
		//phase
		tempUShort = (unsigned short)Eventlist_R2.Data().phase;
		pByte = (byte* ) &tempUShort;
		tmp_event_data[offset+0] = (byte) *(pByte + 1); 
		tmp_event_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//action
		tempUShort = (unsigned short)Eventlist_R2.Data().action;
		pByte = (byte* ) &tempUShort;
		tmp_event_data[offset+0] = (byte) *(pByte + 1); 
		tmp_event_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		Eventlist_R2.Next();	
	}
	size=offset;
	cout<< "Size of the signal EVENT LIST to Interface is : " << size<<endl;
}


int FindSplitPhase(int phase,int phaseStatus[8])
{
	// JD revised
    //*** From global array CombinedPhase[] to get the combined phase :get_configfile() generates CombinedPhase[]
    //*** If the phase exsits, the value is not 0; if not exists, the default value is '0'.; "-1"  means will not change later
    //*** The argument phase should be among {1..8}
    //int CombinedPhase[8]={1,2,3,4,5,6,7,8};
    //*** NOW also consider the case that if phase 6 is current or next, will only call 2, and 6 will be in the solver as well

    int combined_phase=0;

    switch (phase)
    {
    case 1:
        combined_phase= CombinedPhase[6-1];
        break;
    case 2: // if current phase or next phase is 6: we should not call phase 5, because cannot reverse from 6 to 5;
        {
            if(phaseStatus[6-1]==2 || phaseStatus[6-1]==7) // do not consider not enable case: phaseStatus[6-1]==3
                combined_phase=-1;
            else
                combined_phase= CombinedPhase[5-1];
            break;
        }
    case 3:
        combined_phase= CombinedPhase[8-1];
        break;
    case 4:
        {
            if(phaseStatus[8-1]==2 || phaseStatus[8-1]==7)
                combined_phase=-1;
            else
                combined_phase= CombinedPhase[7-1];
            break;
        }
    case 5:
        combined_phase= CombinedPhase[2-1];
        break;
    case 6:
        {
            if(phaseStatus[2-1]==2 || phaseStatus[2-1]==7)
                combined_phase=-1;
            else
                combined_phase= CombinedPhase[1-1];
            break;
        }
    case 7:
        combined_phase= CombinedPhase[4-1];
        break;
    case 8:
        {
            if(phaseStatus[4-1]==2 || phaseStatus[4-1]==7)
                combined_phase=-1;
            else
                combined_phase= CombinedPhase[3-1];
            break;
        }
    default:
        //cout<<"***Error! Wrong Phase Information.***\n";
        outputlog("***Error finding Split Phase Information.***\n");
        //system("pause");
        break;
    }

    return combined_phase;
}

// Return the first position of the request phase in the ReqTimePtList
// In case there are more vehicles request the same phases.
int RequestPhaseInList(LinkedList<ReqEntry> ReqList,int phase)
{
    int posOfReq=-1;
    int i=0;
    ReqList.Reset();
    if(!ReqList.ListEmpty())
    {
        while(!ReqList.EndOfList())
        {
            if(ReqList.Data().Phase==phase)
            {
                posOfReq=i;
                break;
            }
            else
            {
                i++;
            }

            ReqList.Next();
        }
    }

    return posOfReq;
}


int FindRingNo(int phase)
{
    int RingNo;
    switch (phase%10)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        RingNo=0;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        RingNo=1;
        break;
    default:
        cout<<"******Error: No such phase!\n";
        RingNo=-1;
    }
    return RingNo;
}



void populateCP(double  CP[2][3][15], double *Ring1LeftCP,double *Ring1RightCP,int *phaseSeqRing1 ,int t1, double *Ring2LeftCP,double *Ring2RightCP,int *phaseSeqRing2 ,int t2)
{
	int iTemp=0;
	int iTotalPlanningPhaseRing1=t1;
	int iTotalPlanningPhaseRing2=t2;
	// Ring 1
	for (int j=0; j<iTotalPlanningPhaseRing1; j++)
	{
		CP[0][0][j]=Ring1LeftCP[j];
		CP[0][1][j]=Ring1RightCP[j];
		iTemp=phaseSeqRing1[j];
		CP[0][2][j]=iTemp%10;
	}
	//Ring 2
	for (int j=0; j<iTotalPlanningPhaseRing2; j++)
	{
		CP[1][0][j]=Ring2LeftCP[j];
		CP[1][1][j]=Ring2RightCP[j];
		iTemp=phaseSeqRing2[j];
		CP[1][2][j]=iTemp%10;
	}
}

void readOptPlanFromFile(char *filename,double  aCriticalPoints[2][3][15])
{
	LinkedList<PriorityRequest> PrioReqList[2];
	memset(aCriticalPoints,0,sizeof(aCriticalPoints));
	fstream fss;
    fss.open(filename,fstream::in);
    if (!fss)
    {
        cout<<"***********Error opening the Result.txt file!\n";
        sprintf(temp_log,"Error opening the Result.txt file!");
        outputlog(temp_log);
		exit(1); 
    }
    string lineread;
    int SP[2];
    double InitTime1[2],InitGrn[2];
    double dVLeftPoint[3][8],dVRightPoint[3][8],dGLeftPoint[3][8],dGRightPoint[3][8];
    int ReqNo=0;
    int ReqPhaseNo=0;
    double ReqRl=0.0;
    double ReqRu=0.0;
    double ReqDelay=0.0;
    double dTotalDelay=0.0;
    int ReqType=0;
    //-------------------------------Begin of Read Plan from Result.txt----------------------------//
    getline(fss,lineread);
    sscanf(lineread.c_str(),"%d %d",&SP[0],&SP[1]);
    getline(fss,lineread);
    sscanf(lineread.c_str(),"%lf %lf %lf %lf",&InitTime1[0],&InitTime1[1],&InitGrn[0],&InitGrn[1]);
    for(int i=0;i<3;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",
            &dVLeftPoint[i][0],&dVLeftPoint[i][1],&dVLeftPoint[i][2],&dVLeftPoint[i][3],
            &dVLeftPoint[i][4],&dVLeftPoint[i][5],&dVLeftPoint[i][6],&dVLeftPoint[i][7]);
    }
     for(int i=0;i<3;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",
            &dVRightPoint[i][0],&dVRightPoint[i][1],&dVRightPoint[i][2],&dVRightPoint[i][3],
            &dVRightPoint[i][4],&dVRightPoint[i][5],&dVRightPoint[i][6],&dVRightPoint[i][7]);
    }
    for(int i=0;i<3;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",
            &dGLeftPoint[i][0],&dGLeftPoint[i][1],&dGLeftPoint[i][2],&dGLeftPoint[i][3],
            &dGLeftPoint[i][4],&dGLeftPoint[i][5],&dGLeftPoint[i][6],&dGLeftPoint[i][7]);
    }
    for(int i=0;i<3;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",
            &dGRightPoint[i][0],&dGRightPoint[i][1],&dGRightPoint[i][2],&dGRightPoint[i][3],
            &dGRightPoint[i][4],&dGRightPoint[i][5],&dGRightPoint[i][6],&dGRightPoint[i][7]);
    }
	getline(fss,lineread);
    sscanf(lineread.c_str(),"%d",&ReqNo);
    cout<<"Number of Requests" <<ReqNo<<endl;
    for(int i=0;i<ReqNo;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%d %lf %lf %lf %d ",&ReqPhaseNo,&ReqRl,&ReqRu,&ReqDelay,&ReqType);
		ReqPhaseNo=ReqPhaseNo-1;
        int CurRing=FindRingNo(ReqPhaseNo);
        if (ReqPhaseNo<=10) // Double check if the req in next cycle is not being process in current cycl!!! 
		{
			if ( ReqPhaseNo<SP[CurRing]-1)
				ReqPhaseNo=ReqPhaseNo*10; 
		}
        int phase_in_ring=ReqPhaseNo-4*CurRing; // If in ring 2, phase should -4.
        PriorityRequest PriorityRequest_t=PriorityRequest(phase_in_ring,ReqRl,ReqRu,ReqDelay,ReqType);
        PrioReqList[CurRing].InsertRear(PriorityRequest_t);
	}
	getline(fss,lineread);
    sscanf(lineread.c_str(),"%lf ",&dTotalDelay);
    cout<<"Total Delay of Requests: "<< dTotalDelay<<endl;
    fss.close();
     //-------------------------------End of Reading the Plan from Result.txt----------------------------//
    
    // -----------------Processing the plan considering the missing phases and ConfigIS structure.------------//
    int R1No=ConfigIS.Ring1No;
    int R2No=ConfigIS.Ring2No;
    SP[0]=SP[0]-1;
    SP[1]=SP[1]-5;
    int SPIdx1=FindIndexArray<int>(ConfigIS.Phase_Seq_R1,R1No,SP[0]);
    int SPIdx2=FindIndexArray<int>(ConfigIS.Phase_Seq_R2,R2No,SP[1]);
    //cout <<"SP index 1 and 2:\t"<<(SPIdx1+1)<<"  "<<(SPIdx2+1)<<endl;
   
   // MZP commented on 5/4/17  we always have positive split phases timing for two cyckes ahead
    int t1=R1No*2;
    int t2=R2No*2;
    double *SplitRing1Left=new double[t1];
    double *SplitRing1Right=new double[t1];
    double *SplitRing2Left=new double[t2];
    double *SplitRing2Right=new double[t2];
    double *Ring1CriticalLeftPoint=new double[t1];
    double *Ring1CriticalRightPoint=new double[t1];
    double *Ring2CriticalLeftPoint=new double[t2];
    double *Ring2CriticalRightPoint=new double[t2];
	int *Phase1=GeneratePhaseArray(SP[0],ConfigIS.Phase_Seq_R1,R1No,t1,1); // TOTAL t1 & t2 "Array.h"
    int *Phase2=GeneratePhaseArray(SP[1],ConfigIS.Phase_Seq_R2,R2No,t2,1); // Phase1 including the cycle information
    int ii=0;
    // For left points in ring 1
    for (int i=0;i<3;i++)  // Cycle
    {
        for (int k=0;k<R1No;k++)  // Phase
        {
            int RP=ConfigIS.Phase_Seq_R1[k];
            if(dVLeftPoint[i][RP]!=0)
            {
				SplitRing1Left[ii]=dVLeftPoint[i][RP];
				ii++;
			}
			if (ii>=t1)
			  ii=t1-1;
        }
    }
    for(int i=0;i<t1;i++)
    {
        int phase_no=Phase1[i];
        int ring_no=phase_no/10;
        int real_phase=phase_no%10;
        Ring1CriticalLeftPoint[i]=SumArray<double>(SplitRing1Left,t1,0,i)+dGLeftPoint[ring_no][real_phase];
        Ring1CriticalLeftPoint[i]=Ring1CriticalLeftPoint[i]+InitTime1[0];
    }
    PrintArray<int>(Phase1,t1);
    PrintArray<double>(SplitRing1Left,t1);
    PrintArray<double>(Ring1CriticalLeftPoint,t1);
    
    // For right points in ring 1
    ii=0;
    for (int i=0;i<3;i++)  // Cycle
    {
        for (int k=0;k<R1No;k++)  // Phase
        {
            int RP=ConfigIS.Phase_Seq_R1[k];
            if(dVLeftPoint[i][RP]!=0)
            {
				SplitRing1Right[ii]=dVRightPoint[i][RP];
				ii++;
            }
            if (ii>=t1)
				ii=t1-1;
        }
    }
    
    for(int i=0;i<t1;i++)
    {
        int phase_no=Phase1[i];
        int ring_no=phase_no/10;
        int real_phase=phase_no%10;
        Ring1CriticalRightPoint[i]=SumArray<double>(SplitRing1Right,t1,0,i)+dGRightPoint[ring_no][real_phase];
        Ring1CriticalRightPoint[i]=Ring1CriticalRightPoint[i]+InitTime1[0];
    }
    PrintArray<int>(Phase1,t1);
    PrintArray<double>(SplitRing1Right,t1);
    PrintArray<double>(Ring1CriticalRightPoint,t1);
    
    
    // For left points in ring 2
	ii=0;
    for (int i=0;i<3;i++)  // Cycle
    {
        for (int k=0;k<R2No;k++)  // Phase
        {
            int RP=ConfigIS.Phase_Seq_R2[k]+4; //Ring 2 shuold +4; (should be real phase -1)
            if(dVLeftPoint[i][RP]!=0)
            {
				SplitRing2Left[ii]=dVLeftPoint[i][RP];
				ii++;
			}
            if (ii>=t2)
				ii=t2-1;
        }
    }
    for(int i=0;i<t2;i++)
    {
        int phase_no=Phase2[i];
        int ring_no=phase_no/10;
        int real_phase=phase_no%10+4; // Ring 2 shuold +4; (should be real phase -1)
        Ring2CriticalLeftPoint[i]=SumArray<double>(SplitRing2Left,t2,0,i)+dGLeftPoint[ring_no][real_phase];
        Ring2CriticalLeftPoint[i]=Ring2CriticalLeftPoint[i]+InitTime1[1];
    }
    
	PrintArray<int>(Phase2,t2);
    PrintArray<double>(SplitRing2Left,t2);
    PrintArray<double>(Ring2CriticalLeftPoint,t2);
	// For right points in ring 2
	ii=0;
    for (int i=0;i<3;i++)  // Cycle
    {
        for (int k=0;k<R2No;k++)  // Phase
        {
            int RP=ConfigIS.Phase_Seq_R2[k]+4; //Ring 2 shuold +4; (should be real phase -1)
            if(dVLeftPoint[i][RP]!=0)
            {
				SplitRing2Right[ii]=dVRightPoint[i][RP];
				ii++;
            }
            if (ii>=t2)
				ii=t2-1;
        }
    }
    for(int i=0;i<t2;i++)
    {
        int phase_no=Phase2[i];
        int ring_no=phase_no/10;
        int real_phase=phase_no%10+4; // Ring 2 shuold +4; (should be real phase -1)
        Ring2CriticalRightPoint[i]=SumArray<double>(SplitRing2Right,t2,0,i)+dGRightPoint[ring_no][real_phase];
        Ring2CriticalRightPoint[i]=Ring2CriticalRightPoint[i]+InitTime1[1];
    }
    
	PrintArray<int>(Phase2,t2);
    PrintArray<double>(SplitRing2Left,t2);
    PrintArray<double>(Ring2CriticalLeftPoint,t2);
 
       
    PrioReqList[0].Reset();
    PrioReqList[1].Reset();
    populateCP(aCriticalPoints,Ring1CriticalLeftPoint, Ring1CriticalRightPoint, Phase1,t1, Ring2CriticalLeftPoint, Ring2CriticalRightPoint, Phase2,t2);
	delete []SplitRing1Right;
	delete []SplitRing1Left;
	delete []SplitRing2Left;
    delete []SplitRing2Right;
    delete []Ring1CriticalLeftPoint;
    delete []Ring1CriticalRightPoint;
    delete []Ring2CriticalLeftPoint;
    delete []Ring2CriticalRightPoint;	
}

void readOptPlanFromFileForEV(char *filename,double aCriticalPoints[2][3][15], int omitPhases[8])
{
	LinkedList<PriorityRequest> PrioReqList[2];
	
	// Figuring out which phases are missing
	memset(omitPhases,0,sizeof(omitPhases));
	for (int j=0; j<8;j++)
		if (ConfigIS_EV.Gmax[j]==0 && ConfigIS.Gmax[j]>0 )
			omitPhases[j]=j+1;
			
	
	memset(aCriticalPoints,0,sizeof(aCriticalPoints));
	fstream fss;
    fss.open(filename,fstream::in);
    if (!fss)
    {
        cout<<"***********Error opening the Result.txt file! *********\n";
        sprintf(temp_log,"Error opening the Result.txt file!");
        outputlog(temp_log);
		exit(1); 
    }
    
    // --------------Reading the output of Optimizaition model from Result.txt -----------//
    string lineread;
    int SP[2];
    double InitTime1[2],InitGrn[2];
    double dVLeftPoint[3][8],dVRightPoint[3][8],dGLeftPoint[3][8],dGRightPoint[3][8];
    int ReqNo=0;
    int ReqPhaseNo=0;
    double ReqRl=0.0;
    double ReqRu=0.0;
    double ReqDelay=0.0;
    int ReqType=0;
    //-------------------------------Begin of Read Plan----------------------------//
    getline(fss,lineread);
    sscanf(lineread.c_str(),"%d %d",&SP[0],&SP[1]);
    getline(fss,lineread);
    sscanf(lineread.c_str(),"%lf %lf %lf %lf",&InitTime1[0],&InitTime1[1],&InitGrn[0],&InitGrn[1]);
    for(int i=0;i<3;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",
            &dVLeftPoint[i][0],&dVLeftPoint[i][1],&dVLeftPoint[i][2],&dVLeftPoint[i][3],
            &dVLeftPoint[i][4],&dVLeftPoint[i][5],&dVLeftPoint[i][6],&dVLeftPoint[i][7]);
    }
     for(int i=0;i<3;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",
            &dVRightPoint[i][0],&dVRightPoint[i][1],&dVRightPoint[i][2],&dVRightPoint[i][3],
            &dVRightPoint[i][4],&dVRightPoint[i][5],&dVRightPoint[i][6],&dVRightPoint[i][7]);
    }
    for(int i=0;i<3;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",
            &dGLeftPoint[i][0],&dGLeftPoint[i][1],&dGLeftPoint[i][2],&dGLeftPoint[i][3],
            &dGLeftPoint[i][4],&dGLeftPoint[i][5],&dGLeftPoint[i][6],&dGLeftPoint[i][7]);
    }
    for(int i=0;i<3;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf",
            &dGRightPoint[i][0],&dGRightPoint[i][1],&dGRightPoint[i][2],&dGRightPoint[i][3],
            &dGRightPoint[i][4],&dGRightPoint[i][5],&dGRightPoint[i][6],&dGRightPoint[i][7]);
    }
	getline(fss,lineread);
    sscanf(lineread.c_str(),"%d",&ReqNo);
    for(int i=0;i<ReqNo;i++)
    {
        getline(fss,lineread);
        sscanf(lineread.c_str(),"%d %lf %lf %lf %d ",&ReqPhaseNo,&ReqRl,&ReqRu,&ReqDelay,&ReqType);
		ReqPhaseNo=ReqPhaseNo-1;
        int CurRing=FindRingNo(ReqPhaseNo);
        int phase_in_ring=ReqPhaseNo-4*CurRing; // If in ring 2, phase should -4.
        PriorityRequest PriorityRequest_t=PriorityRequest(phase_in_ring,ReqRl,ReqRu,ReqDelay,ReqType);
        PrioReqList[CurRing].InsertRear(PriorityRequest_t);
	}
    fss.close();
    
    
    // When we have EV in the request table, the Left and right points are similar. There is only one path for optimal signal schedule. Therefore, we only consider the left point in the creating Critial points
    int R1No=ConfigIS_EV.Ring1No;
    int R2No=ConfigIS_EV.Ring2No;
    SP[0]=SP[0]-1;
    SP[1]=SP[1]-5;
    int SPIdx1=FindIndexArray<int>(ConfigIS_EV.Phase_Seq_R1,R1No,SP[0]);
    int SPIdx2=FindIndexArray<int>(ConfigIS_EV.Phase_Seq_R2,R2No,SP[1]);
   // cout <<"SP index 1 and 2:\t"<<(SPIdx1+1)<<"  "<<(SPIdx2+1)<<endl;
    
    
     int t1=-1;//R1No*2-SPIdx1;
     int t2=-1;//R2No*2-SPIdx2;
     double *Split1;
     double *Split2;
     double *mustChangeTime1;
     double *mustChangeTime2;
	 int *Phase1;//=GeneratePhaseArray(SP[0],ConfigIS_EV.Phase_Seq_R1,R1No,t1,1); // TOTAL t1 & t2 "Array.h"
     int *Phase2;//=GeneratePhaseArray(SP[1],ConfigIS_EV.Phase_Seq_R2,R2No,t2,1); // Phase1 including the cycle information
     
   
    //cout<<"R1No "<<R1No<<endl;
    //cout<<"R2No "<<R2No<<endl;
    int ii=0;
	if (R1No==1)
	{
		t1=1;    // only one phase
		Phase1=new int[t1];
		Split1=new double[t1];
        mustChangeTime1=new double[t1];
        Split1[0]=0.0;
        int *Phase11=GeneratePhaseArray(SP[0],ConfigIS_EV.Phase_Seq_R1,R1No,t1,1); // TOTAL t1=1
        Phase1[0]=Phase11[0];
        //------ IF only have one phase in a ring, we need to add 2 cycles' result up.
        ii=0;
        int RP1=ConfigIS_EV.Phase_Seq_R1[0]%10; // IS (Real phase -1)
        for (int i=0;i<3;i++)  // Cycle
        {
            Split1[ii]+=dVLeftPoint[i][RP1];
        }
        int CurPhase=Phase1[ii]%10;  // Ring 2 shuold +4;
        mustChangeTime1[ii]=Split1[ii]-ConfigIS_EV.Yellow[CurPhase]-ConfigIS_EV.Red[CurPhase];
        PrintArray<int>(Phase1,t1);
        PrintArray<double>(Split1,t1);
        PrintArray<double>(mustChangeTime1,t1);
	}else
	{
		// MZ commented on 5/4 , we always have two cycles ahead
		//t1=R1No*2-SPIdx1;
		t1=R1No*2;
		Phase1=new int[t1];
		Split1=new double[t1];
        mustChangeTime1=new double[t1];
        int *Phase11=GeneratePhaseArray(SP[0],ConfigIS_EV.Phase_Seq_R1,R1No,t1,1);
   
        for(int jj=0;jj<t1;jj++)
        {
			Phase1[jj]=Phase11[jj];
		}
        ii=0;
        for (int i=0;i<3;i++)  // Cycle
        {
            for (int k=0;k<R1No;k++)  // Phase
            {
                int RP=ConfigIS_EV.Phase_Seq_R1[k];
                if(dVLeftPoint[i][RP]!=0)
                {
                    Split1[ii]=dVLeftPoint[i][RP];
                    ii++;
                }
                if (ii>=t1)
					ii=t1-1;
            }
        }
        for(int i=0;i<t1;i++)
        {
            int phase_no=Phase1[i];
            int ring_no=phase_no/10; //Cycle
            int real_phase=phase_no%10;
            mustChangeTime1[i]=SumArray<double>(Split1,t1,0,i)+dGLeftPoint[ring_no][real_phase];
            mustChangeTime1[i]=mustChangeTime1[i]+InitTime1[0];
        }
        PrintArray<int>(Phase1,t1);
        PrintArray<double>(Split1,t1);
        PrintArray<double>(mustChangeTime1,t1);
	}

	if (R2No==1)
	{
		if(SPIdx2<0)
        {
            cout<<"***********Starting Phase is not in Ring 2 Sequence.***********\n";
            sprintf(temp_log,"***********Starting Phase is not in Ring 2 Sequence.***********");
            outputlog(temp_log);            exit(1);
        }
		t2=1;    // only one phase
		Phase2=new int[t2];
		Split2=new double[t2];
        mustChangeTime2=new double[t2];
		Split2[0]=0.0;
        int *Phase22=GeneratePhaseArray(SP[1],ConfigIS_EV.Phase_Seq_R2,R2No,t2,1); // TOTAL t1=1
        Phase2[0]=Phase22[0];
        //------ IF only have one phase in a ring, we need to add 2 cycles' result up.
        ii=0;
        int RP2=ConfigIS_EV.Phase_Seq_R2[0]%10+4; // IS (Real phase -1)
        for (int i=0;i<3;i++)  // Cycle
        {
            Split2[ii]+=dVLeftPoint[i][RP2];
        }
        int CurPhase=Phase2[ii]%10+4;  // Ring 2 shuold +4;
        mustChangeTime2[ii]=Split2[ii]-ConfigIS_EV.Yellow[CurPhase]-ConfigIS_EV.Red[CurPhase];
        PrintArray<int>(Phase2,t2);
        PrintArray<double>(Split2,t2);
        PrintArray<double>(mustChangeTime2,t2);
  	}else
	{
	     // MZ commented on 5/4
		//t2=R2No*2-SPIdx2;
		t2=R2No*2;
		Phase2=new int[t2];
		Split2=new double[t2];
        mustChangeTime2=new double[t2];
        int *Phase22=GeneratePhaseArray(SP[1],ConfigIS_EV.Phase_Seq_R2,R2No,t2,1);
        for(int jj=0;jj<t2;jj++)
			Phase2[jj]=Phase22[jj];
        ii=0;
        for (int i=0;i<3;i++)  // Cycle
        {
            for (int k=0;k<R2No;k++)  // Phase
            {
                int RP=ConfigIS_EV.Phase_Seq_R2[k]%10+4;
                if(dVLeftPoint[i][RP]!=0)
                {
                    Split2[ii]=dVLeftPoint[i][RP];
                    ii++;
                }
                if (ii>=t2)
				   ii=t2-1;
            }
        }
        for(int i=0;i<t2;i++)
        {
            int phase_no=Phase2[i];
            int ring_no=phase_no/10; //Cycle
            int real_phase=phase_no%10+4;
            mustChangeTime2[i]=SumArray<double>(Split2,t2,0,i)+dGLeftPoint[ring_no][real_phase];
            mustChangeTime2[i]=mustChangeTime2[i]+InitTime1[1];
        }
        PrintArray<int>(Phase2,t2);
        PrintArray<double>(Split2,t2);
        PrintArray<double>(mustChangeTime2,t2);
	}
	populateCP(aCriticalPoints, mustChangeTime1, mustChangeTime1, Phase1, t1, mustChangeTime2, mustChangeTime2, Phase2, t2);

	delete []mustChangeTime1;
    delete []mustChangeTime2;
    delete []Split1;
    delete []Split2;
    delete []Phase1;
    delete []Phase2;

}





void PrintList2File(char *Filename,LinkedList<ReqEntry> &ReqList)
{
	ofstream fs;
	//iostream fs;
	fs.open(Filename);
	if(fs.good())
	{
		fs<<"Num_req  "<<ReqList.ListSize()<<" "<<0<<endl;  //*** IMPORTANT: UpdateFlag changed to "0": means solved ***//
		if (!ReqList.ListEmpty())
		{
			ReqList.Reset();
			while(!ReqList.EndOfList())
			{
				fs<<RSUID<<" "<<ReqList.Data().VehID<<"  "<<ReqList.Data().VehClass<<"  "<<ReqList.Data().ETA<<"  "<<ReqList.Data().Phase                        
					<<" "<<ReqList.Data().MinGreen<<" "<<ReqList.Data().dSetRequestTime<<" "<<ReqList.Data().Split_Phase<<" "<<ReqList.Data().iInLane<<" "<<ReqList.Data().iOutLane
					<<" "<<ReqList.Data().iStrHour<<" "<<ReqList.Data().iStrMinute<<" "<<ReqList.Data().iStrSecond<<" "<<ReqList.Data().iEndHour
					<<" "<<ReqList.Data().iEndMinute<<" "<<ReqList.Data().iEndSecond<<" "<<ReqList.Data().iVehState<<" "<<ReqList.Data().iMsgCnt<< " " << endl;
				ReqList.Next();
			}
		}
	}
	else
	{
		perror("Error occurs when open  requests.txt  file to write the linked list");
	}
	cout<<"Rewrite the requests into requests.txt file successfully.\n";
	fs.close();
}


void PrintList2File_EV(char *Filename,LinkedList<ReqEntry> &ReqList)
{
	ofstream fs;
	fs.open(Filename);
	if(fs.good())
	{
		fs<<"Num_req  "<<ReqList.ListSize()<<" "<<0<<endl;  //*** IMPORTANT: UpdateFlag changed to "0": means solved ***//
		if (!ReqList.ListEmpty())
		{
			ReqList.Reset();
			while(!ReqList.EndOfList())
			{
				fs<<RSUID<<" "<<ReqList.Data().VehID<<"  "<<ReqList.Data().VehClass<<"  "<<ReqList.Data().ETA<<"  "<<ReqList.Data().Phase                        
					<<" "<<ReqList.Data().MinGreen<<" "<<ReqList.Data().dSetRequestTime<<" "<<ReqList.Data().iInLane<<" "<<ReqList.Data().iOutLane
					<<" "<<ReqList.Data().iStrHour<<" "<<ReqList.Data().iStrMinute<<" "<<ReqList.Data().iStrSecond<<" "<<ReqList.Data().iEndHour
					<<" "<<ReqList.Data().iEndMinute<<" "<<ReqList.Data().iEndSecond<<" "<<ReqList.Data().iVehState<<" "<<ReqList.Data().iMsgCnt<< " " << endl;
				ReqList.Next();
			}
		}
	}
	else
	{
		perror("Error occurs when open requests_combined.txt to write the linked list");
	}
	cout<<"Rewrite the requests into requests_combined.txt file successfully. \n";
	fs.close();
}

void ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List)
{
    fstream fss;
    fss.open(filename,fstream::in);
    ReqEntry req_temp;
    int ReqNo;
    char RSU_ID[128],OBU_ID[128];
    int Veh_Class,Req_Phase,Req_SplitPhase;
    double dsetRequestTime;
    float ETA,MinGrn;
	int iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt;	 

    string lineread;
    Req_List.ClearList();
    getline(fss,lineread);
    sscanf(lineread.c_str(),"%*s %d %d",&ReqNo,&ReqListUpdateFlag);
    if (ReqListUpdateFlag > 0)
    {
		sprintf(temp_log,"request number in requests.txt is: %d, Flag is{%d}\n",ReqNo,ReqListUpdateFlag);
		outputlog(temp_log);
	}
    while(!fss.eof())
    {
        getline(fss,lineread);
        if(lineread.size()!=0)
        {
            sscanf(lineread.c_str(),"%s %s %d %f %d %f %lf %d %d %d %d %d %d %d %d %d %d %d ",RSU_ID,OBU_ID,&Veh_Class,
                &ETA,&Req_Phase,&MinGrn,&dsetRequestTime,&Req_SplitPhase,
                &iInLane,&iOutLane,&iStrHour,&iStrMinute,&iStrSecond,&iEndHour,&iEndMinute,&iEndSecond,&iVehState,&iMsgCnt);
            ReqEntry req_temp(OBU_ID,Veh_Class,ETA,Req_Phase,MinGrn,dsetRequestTime,Req_SplitPhase,iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt);
            Req_List.InsertAfter(req_temp);
        }
    }
    fss.close();
}




void ReqListFromFile_EV(char *filename,LinkedList<ReqEntry>& Req_List)
{
    fstream fss;
    fss.open(filename,fstream::in);
    ReqEntry req_temp;
    int ReqNo;
    char RSU_ID[128],OBU_ID[128];
    int Veh_Class,Req_Phase;
    double dsetRequestTime;
    float ETA,MinGrn;
	int iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt;	
    string lineread;
    Req_List.ClearList();
    getline(fss,lineread);
    sscanf(lineread.c_str(),"%*s %d %d",&ReqNo,&ReqListUpdateFlag);
    if (ReqListUpdateFlag > 0)
    {
		sprintf(temp_log,"request number in requests_combined.txt is: %d, Flag is{%d}\n",ReqNo,ReqListUpdateFlag);
		outputlog(temp_log);
	}

    while(!fss.eof())
    {
        getline(fss,lineread);
        if(lineread.size()!=0)
        {
            // sscanf(lineread.c_str(),"%s %s %d %f %d %f %lf %d %d %d %d %d %d %d %d %d %d %d ",RSU_ID,OBU_ID,&Veh_Class,
            //     &ETA,&Req_Phase,&MinGrn,&dsetRequestTime,&iInLane,&iOutLane,&iStrHour,&iStrMinute,&iStrSecond,&iEndHour,&iEndMinute,&iEndSecond,&iVehState,&iMsgCnt);

			//Debashis:: No of formatter and specifier don't match. 
			sscanf(lineread.c_str(),"%s %s %d %f %d %f %lf %d %d %d %d %d %d %d %d %d %d ",RSU_ID,OBU_ID,&Veh_Class,
                &ETA,&Req_Phase,&MinGrn,&dsetRequestTime,&iInLane,&iOutLane,&iStrHour,&iStrMinute,&iStrSecond,&iEndHour,&iEndMinute,&iEndSecond,&iVehState,&iMsgCnt);

            ReqEntry req_temp(OBU_ID,Veh_Class,ETA,Req_Phase,MinGrn,dsetRequestTime,0,iInLane,iOutLane,iStrHour,iStrMinute,iStrSecond,iEndHour,iEndMinute,iEndSecond,iVehState, iMsgCnt);
            Req_List.InsertAfter(req_temp);
         //   cout<<lineread.c_str()<<endl;
        }
    }
    fss.close();
}






void LinkList2DatFile(LinkedList<ReqEntry> Req_List,char *filename,double InitTime[2],int InitPhase[2],double GrnElapse[2])
{	
    //-- Convert request linkedlist to the NewModel.dat file for GLPK solver.
    //-- Init1 and Init2 are the initial time for the two rings while current phases are in R or Y.
    //-- PassedGrn1 and PassedGrn2 are the elapsed time when current phase is in Green

    ofstream fs;
    fs.open(filename,ios::out);
    int R1No=ConfigIS.Ring1No;
    int R2No=ConfigIS.Ring2No;
	 //---------------According to priorityconfiguration file, some priority eligible vehicle may have weigth equal to zero. we should remove them from the list
	LinkedList<ReqEntry> Req_List_New;
	if(Req_List.ListEmpty()==0)
        removeZeroWeightReq(Req_List, Req_List_New);
    
    
    fs<<"data;\n";
    fs<<"param SP1:="<<InitPhase[0]<<";\n";  // This is the real phase [1-4]
    fs<<"param SP2:="<<InitPhase[1]<<";\n";  // This is the real phase [5-8]

    for(int i=0;i<2;i++)
    {
        if (InitTime[i]<0)
        {
            InitTime[i]=0;
        }
    }

    fs<<"param init1:="<<InitTime[0]<<";\n";
    fs<<"param init2:="<<InitTime[1]<<";\n";
    fs<<"param Grn1 :="<<GrnElapse[0]<<";\n";
    fs<<"param Grn2 :="<<GrnElapse[1]<<";\n";

    fs<<"param y          \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=ConfigIS.Phase_Seq_R1[i];
        fs<<"\t"<<(k+1)<<"  "<<ConfigIS.Yellow[k];
    }
    for(int i=0;i<R2No;i++)
    {
        int k=ConfigIS.Phase_Seq_R2[i];
        fs<<"\t"<<(k+5)<<"  "<<ConfigIS.Yellow[k+4];
    }
    fs<<";\n";

    fs<<"param red       \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=ConfigIS.Phase_Seq_R1[i];
        fs<<"\t"<<(k+1)<<"  "<<ConfigIS.Red[k];
    }
    for(int i=0;i<R2No;i++)
    {
        int k=ConfigIS.Phase_Seq_R2[i];
        fs<<"\t"<<(k+5)<<"  "<<ConfigIS.Red[k+4];
    }
    fs<<";\n";
	
	fs<<"param gmin      \t:=";
	for(int i=0;i<R1No;i++)
	{
		int k=ConfigIS.Phase_Seq_R1[i];
		fs<<"\t"<<(k+1)<<"  "<<ConfigIS.Gmin[k];
	}
	for(int i=0;i<R2No;i++)
	{
		int k=ConfigIS.Phase_Seq_R2[i];
		fs<<"\t"<<(k+5)<<"  "<<ConfigIS.Gmin[k+4];
	}
	fs<<";\n";
	
	
	// here we obtain parameter gmax 
	
	int iGmax[2][4]={};  
	double dGmax[8]={0.0};
	
	intitializdGmaxAndiGmax(dGmax, iGmax);
	// in coordination case, the maximum green time for the two coordinated phase is set to the cycle-sumation of minimum green time of othee phases and the clearance time.
	if (ConfigIS.dCoordinationWeight>0)
	{
		setMaxGrnTimeForCoordinatedPhases(dGmax,iGmax);       	 // Set the maximum green time of each phase equal to cycle length minus the summation of other non coordinated clearance time and minimum green time .
		doubleCheckMissingPhasesInTintersections(dGmax,iGmax);   	
	}
	addGrnExtToRequestedPhaseMaxGrnTime(Req_List_New, dGmax,iGmax);  // adding green extention portion to the max green time for the requested phases
	
	fs<<"param gmax      \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=ConfigIS.Phase_Seq_R1[i];
        fs<<"\t"<<(k+1)<<"  "<<dGmax[k];
    }
    for(int i=0;i<R2No;i++)
    {
        int k=ConfigIS.Phase_Seq_R2[i];
        fs<<"\t"<<(k+5)<<"  "<< dGmax[k+4];
	}

    fs<<";\n\n";


	Req_List_New.Reset();
	int NumberofRequests=0;
	int iNumberofTransitInList=1;
	int iNumberofTruckInList=1;
	char tempID[64]=" ";
	fs<<"param priorityType:= ";
	if(Req_List_New.ListEmpty()==0)
    {
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass!=6) // only consider non coordination request in this weighting list. THe coordination request weight is considered in the objective function 
			{
				NumberofRequests++;
				if (Req_List_New.Data().VehClass==2)
					iNumberofTransitInList++;
				if (Req_List_New.Data().VehClass==3)
					iNumberofTruckInList++;
				fs<<NumberofRequests;
				fs<<" ";
				fs<<Req_List_New.Data().VehClass;			
				fs<<" ";
				strcpy(tempID,Req_List_New.Data().VehID);
			}
			Req_List_New.Next();
		}
		while (NumberofRequests<10)
		{
			NumberofRequests++;
			fs<<NumberofRequests;
			fs<<" ";
			fs<<0;			
			fs<<" ";
		}
		fs<<" ;  \n";
	}else
	{
	    fs<<" 1 0 2 0 3 5 4 0 5 0 6 0 7 0 8 0 9 0 10 0 ; \n";	
	}
	
	
	if (iNumberofTransitInList>1)
		iNumberofTransitInList=iNumberofTransitInList-1;
	if (iNumberofTruckInList>1)
		iNumberofTruckInList=iNumberofTruckInList-1;
		
		
	fs<<"param PrioWeigth:=  1 1 2 ";
	fs<< ConfigIS.dTransitWeight/iNumberofTransitInList;
	fs<< " 3 " ;
	fs<< ConfigIS.dTruckWeight/iNumberofTruckInList;
	fs<< " 4 0 5 0 ";
	fs<< " 6 " ;
	fs<< ConfigIS.dCoordinationWeight;
	fs<< " 7 0 8 0 9 0 10 0 ; \n" ;
	
	
   

	int ReqSeq=1;
    double dTempCl=0.0;
    double dTempCu=0.0;
    int	iTempPhase=0;
    
    fs<<"param Rl (tr): 1 2 3 4 5 6 7 8:=\n";
    Req_List_New.Reset();
    			
    
    while(!Req_List_New.EndOfList())
    {
		if (Req_List_New.Data().VehClass!=6) // if it is a coordination request, we will check it in at it in the Cu and Cl 
        {
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{			
				if(Req_List_New.Data().Phase==j)
				{
					if (Req_List_New.Data().MinGreen>0) // in this case the vhicle is in the queue and we should set the Rl as less as possible!!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
					{
						int iRingOfTheRequest=FindRingNo(j-1); 
						fs<< 1 << "  " ;
						if ( iGmax[iRingOfTheRequest][(j-1)%4]<=0 )
						{
							if (j%2==0) 
								iGmax[iRingOfTheRequest][(j-1)%4]=iGmax[iRingOfTheRequest][(j-1)%4-1];
							else
								iGmax[iRingOfTheRequest][(j-1)%4]=iGmax[iRingOfTheRequest][(j-1)%4+1];
						}
					}
					else
						fs<<max(Req_List_New.Data().ETA-ROBUSTTIME_LOW,float(1.0))<<"  "; 
				}
				else
					fs<<".  ";
			}
			fs<<"\n";
			ReqSeq=ReqSeq+1;
		}
		Req_List_New.Next();
    }
    fs<<";\n";
    fs<<"param Ru (tr): 1 2 3 4 5 6 7 8:=\n";
    Req_List_New.Reset();
    ReqSeq=1;
    while(!Req_List_New.EndOfList())
    {
		if (Req_List_New.Data().VehClass!=6) // if it is a coordination request, we will check it in at it in the Cu and Cl 
        {
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if(Req_List_New.Data().Phase==j)
				{
					if (Req_List_New.Data().MinGreen>0) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
					{
						int iRingOfTheRequestt=FindRingNo(j-1);
						double dmaxGreenForThisPhase=0.0;
						
						if (iRingOfTheRequestt==0) // ring 1
						{
							for(int i=0;i<R1No;i++)
							{
								int kkk=ConfigIS.Phase_Seq_R1[i];
								if (kkk+1==j)
									dmaxGreenForThisPhase=dGmax[kkk];	
							}
						}
						if (iRingOfTheRequestt==1) // ring 2
						{
							for(int i=0;i<R2No;i++)
							{
								int kkk=ConfigIS.Phase_Seq_R2[i];
								if (kkk+5==j)
									dmaxGreenForThisPhase=dGmax[kkk+4];	
							}
						}

						if (Req_List_New.Data().MinGreen+ROBUSTTIME_UP >=dmaxGreenForThisPhase )
							fs<<dmaxGreenForThisPhase  <<"  ";
						else
								fs<<Req_List_New.Data().MinGreen+ROBUSTTIME_UP  <<"  ";		
					}
					else
						fs<<Req_List_New.Data().ETA+ROBUSTTIME_UP <<"  ";
				}
				else
					fs<<".  ";
			}
				fs<<"\n";
			ReqSeq=ReqSeq+1;
		}
		Req_List_New.Next();
    }
     fs<<";\n";
    if (HaveCoordInList==1) // in order to know in which 
	{
		fs<<"param Cl1 (tr): 1 2 3 4 5 6 7 8:=\n";
		Req_List_New.Reset();
		ReqSeq=1;
		fs<<ReqSeq<<"  ";
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass==6 &&  Req_List_New.Data().Phase < 5 )
			{
				for(int j=1;j<=8;j++)
				{
					if(Req_List_New.Data().Phase==j)
					{
						if (Req_List_New.Data().MinGreen>0) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
						{
							fs<< 1 << "  " ;
							dTempCl=ConfigIS.dCoordCycle-ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1]+Req_List_New.Data().MinGreen;
						}
						else 
						{
							fs<< Req_List_New.Data().ETA << "  " ;	
							dTempCl=ConfigIS.dCoordCycle+Req_List_New.Data().ETA ;
						}				
						iTempPhase=j;
						ReqSeq=ReqSeq+1;
					}
					else
						fs<<".  ";
				}
				fs<<"\n";			
			}
			Req_List_New.Next();
		}
		
		
		// setting the second coordination request in the second cycle of ring 1
		if (ReqSeq>1) // if there is one coordination request for a phase in ring 1
		{
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if (j==iTempPhase)
					fs<< dTempCl << "  " ;	
				else
					fs<<".  ";
			}
			fs<<"\n";
		}
		fs<<";\n";

     
		fs<<"param Cu1 (tr): 1 2 3 4 5 6 7 8:=\n";
		Req_List_New.Reset();
		ReqSeq=1;
		iTempPhase=0;
		fs<<ReqSeq<<"  ";
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass == 6 &&  Req_List_New.Data().Phase < 5) // for the phases in the second ring
			{
				for(int j=1;j<=8;j++)
				{
					if( Req_List_New.Data().Phase==j )
					{
						if (Req_List_New.Data().MinGreen>0 ) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
						{
							fs<<Req_List_New.Data().MinGreen  <<"  ";
							dTempCu= ConfigIS.dCoordCycle+Req_List_New.Data().MinGreen ; // Cu for the second cycle
						}	
						else 
						{
							fs<< Req_List_New.Data().ETA + ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1]<< "  " ;
							dTempCu= ConfigIS.dCoordCycle + Req_List_New.Data().ETA + ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1];
						}
						iTempPhase=j;
						ReqSeq=ReqSeq+1;
					}
					else
						fs<<".  ";
				}
				fs<<"\n";
			}
			Req_List_New.Next();
		}
		
		// setting the second coordination request in the second cycle of ring1
		if (ReqSeq>1) // if there is one coordination request for a phase in ring 1
		{
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if (j==iTempPhase)
					fs<< dTempCu << "  " ;	
				else
					fs<<".  ";
			}
			fs<<"\n";
		}
		
		
		fs<<";\n";
		
		fs<<"param Cl2 (tr): 1 2 3 4 5 6 7 8:=\n";
		Req_List_New.Reset();
		ReqSeq=1;
		iTempPhase=0;
		dTempCu=0.0;
		dTempCl=0.0;
		fs<<ReqSeq<<"  ";
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass==6 &&  Req_List_New.Data().Phase > 4)
			{
				
				for(int j=1;j<=8;j++)
				{
					if(Req_List_New.Data().Phase==j)
					{
						if (Req_List_New.Data().MinGreen>0) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
						{
							fs<< 1 << "  " ;
							dTempCl=ConfigIS.dCoordCycle-ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[1]-1]+Req_List_New.Data().MinGreen;
						}
						else 
						{
							fs<< Req_List_New.Data().ETA << "  " ;	
							dTempCl=ConfigIS.dCoordCycle+Req_List_New.Data().ETA;
						}				
						iTempPhase=j;
						ReqSeq=ReqSeq+1;		
					}
					else
						fs<<".  ";
				}
				fs<<"\n";			
			}
			Req_List_New.Next();
		}
		
		// setting the second coordination request in the second cycle of ring2
		if (ReqSeq>1) // if there is one coordination request for a phase in ring 2
		{
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if (j==iTempPhase)
					fs<< dTempCl << "  " ;	
				else
					fs<<".  ";
			}
			fs<<"\n";
		}
		fs<<";\n";

     
		fs<<"param Cu2 (tr): 1 2 3 4 5 6 7 8:=\n";
		Req_List_New.Reset();
		ReqSeq=1;
		fs<<ReqSeq<<"  ";
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass==6 &&  Req_List_New.Data().Phase > 4)
			{
				for(int j=1;j<=8;j++)
				{
					if(Req_List_New.Data().Phase==j)
					{
						if (Req_List_New.Data().MinGreen>0 ) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
						{
							fs<<Req_List_New.Data().MinGreen  <<"  ";	
							dTempCu= ConfigIS.dCoordCycle+Req_List_New.Data().MinGreen ; // Cu for the second cycle
						}
						else 
						{
							fs<< Req_List_New.Data().ETA + ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[1]-1]<< "  " ;	
							dTempCu= ConfigIS.dCoordCycle + Req_List_New.Data().ETA + ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1];
						}
						iTempPhase=j;
						ReqSeq=ReqSeq+1;
					}
					else
						fs<<".  ";
				}
				fs<<"\n";
				
			}
			Req_List_New.Next();
		}
		// setting the second coordination request in the second cycle of ring2
		if (ReqSeq>1) // if there is one coordination request for a phase in ring 2
		{
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if (j==iTempPhase)
					fs<< dTempCu << "  " ;	
				else
					fs<<".  ";
			}
			fs<<"\n";
		}
		fs<<";\n";
	}
	
	
    fs<<"end;";
    fs.close();
}

void intitializdGmaxAndiGmax( double dGmax[8],int iGmax[2][4])
{
	int R1No=ConfigIS.Ring1No;
    int R2No=ConfigIS.Ring2No;           
	for(int i=0;i<R1No;i++)  //ring 1
    {
        int k=ConfigIS.Phase_Seq_R1[i];
			dGmax[k]=ConfigIS.Gmax[k];
			iGmax[0][k]=(int)dGmax[k];
	}
	for(int i=0;i<R2No;i++) // ring 2 
    {
        int k=ConfigIS.Phase_Seq_R2[i];
        dGmax[k+4]=ConfigIS.Gmax[k+4];//max(ConfigIS.Gmax[k+4],dGmax[k+4]);
		iGmax[1][k]=(int) dGmax[k+4];
	}
}

void addGrnExtToRequestedPhaseMaxGrnTime(LinkedList<ReqEntry> Req_List, double dGmax[8],int iGmax[2][4])
{
	// adding green extention portion to the max green time for the requested phases
	Req_List.Reset();
	int R1No=ConfigIS.Ring1No;
    int R2No=ConfigIS.Ring2No;           
	for(int i=0;i<R1No;i++)  //ring 1
    {
        int k=ConfigIS.Phase_Seq_R1[i];
        // We extend the max green for requsted phase. If the model consideres coodination, 
        // we only extend the max green for requested non-coordinated phases. The max green time for coordinated phases when the model
        // is on coordination is already set as cycle -sum{min(non coocrinated)}
		if( (RequestPhaseInList(Req_List,(k+1)) >= 0 ) && 
		  ( (ConfigIS.dCoordinationWeight == 0) || ( ( ConfigIS.dCoordinationWeight > 0 ) && 
		  ( k+1 != ConfigIS.iCoordinatedPhase[0] ) && ( ConfigIS.iCoordinatedPhase[0] > 0 ) ) ) )
        {
			dGmax[k]=(dGmax[k]*(1+MaxGrnExtPortion));
			if (ConfigIS.Gmax[k+4]>0) // e.g. if we increase the max green for phase 4 by MaxGrnExtPortion%, then the max green for phase 8 should also increased by the same amount. Unless there is no phase 8 in the signal controller
				dGmax[k+4]=dGmax[k];
			iGmax[0][k]=(int) dGmax[k];	
        }
    }
      
    for(int i=0;i<R2No;i++) // ring 2 
    {
        int k=ConfigIS.Phase_Seq_R2[i];
       if( ( RequestPhaseInList(Req_List,(k+5)) >= 0 ) && 
		  ( (ConfigIS.dCoordinationWeight == 0) || ( ( ConfigIS.dCoordinationWeight > 0 ) && 
		  ( k+5 != ConfigIS.iCoordinatedPhase[1] ) && ( ConfigIS.iCoordinatedPhase[1] >0 ) ) ) )   
        {
			dGmax[k+4]=(dGmax[k+4]*(1+MaxGrnExtPortion));
			if (ConfigIS.Gmax[k]>0) // e.g. if we increase the max green for phase 4 by MaxGrnExtPortion%, then the max green for phase 8 should also increased by the same amount. Unless there is no phase 8 in the signal controller
				dGmax[k]=dGmax[k+4];
			iGmax[1][k]=(int) dGmax[k+4];
        }
    }
}

void setMaxGrnTimeForCoordinatedPhases(double dGmax[8],int iGmax[2][4])	// Set the maximum green time of each phase equal to the split time of it
{
	int R1No=ConfigIS.Ring1No;
    int R2No=ConfigIS.Ring2No;
    //first, consider the split time for each phase as its maximum green time.
	for(int i=0;i<R1No;i++)
	{
		int k=ConfigIS.Phase_Seq_R1[i];
		dGmax[k] = ConfigIS.dCoordinationPhaseSplit[k] - ConfigIS.Red[k] - ConfigIS.Yellow[k];
		iGmax[0][k] = (int) dGmax[k];
	}
	
	for(int i=0;i<R2No;i++)
	{
		int k=ConfigIS.Phase_Seq_R2[i];
		dGmax[k+4] = ConfigIS.dCoordinationPhaseSplit[k+4] - ConfigIS.Red[k+4] - ConfigIS.Yellow[k+4];
		iGmax[1][k]=(int) dGmax[k+4];
	}
	
	// second, reset the max time for the coordinated phases: Calculate the maximum possible green time for coordinated phase ( cycle - sum { min (non coordnated phase + clrearance time) }
	double dMaxCoordPhase1GrnTime=0.0;
	double dMaxCoordPhase2GrnTime=0.0;
	double dSumOfMinAndClearanceTimeInRing1=0.0;   
	double dSumOfMinAndClearanceTimeInRing2=0.0; 
		   
	for(int i=0;i<R1No;i++)  
	{
		int k=ConfigIS.Phase_Seq_R1[i];
		dSumOfMinAndClearanceTimeInRing1 = dSumOfMinAndClearanceTimeInRing1 + ConfigIS.Gmin[k] + ConfigIS.Red[k] + ConfigIS.Yellow[k];
	}
	for(int i=0;i<R2No;i++) 
	{
		int k=ConfigIS.Phase_Seq_R2[i];
		dSumOfMinAndClearanceTimeInRing2 = dSumOfMinAndClearanceTimeInRing2 + ConfigIS.Gmin[k+4] + ConfigIS.Red[k+4] + ConfigIS.Yellow[k+4];
	}
	if ( ConfigIS.iCoordinatedPhase[0] > 0)
	{
		for(int i=0;i<R1No;i++)
		{
			int k=ConfigIS.Phase_Seq_R1[i];
			if ( k+1 == ConfigIS.iCoordinatedPhase[0] )
			{
				dMaxCoordPhase1GrnTime = ConfigIS.dCoordCycle - (dSumOfMinAndClearanceTimeInRing1 - (ConfigIS.Gmin[k]) );
				dGmax[ConfigIS.iCoordinatedPhase[0]-1] = dMaxCoordPhase1GrnTime;
				iGmax[0][k] = (int) dGmax[ConfigIS.iCoordinatedPhase[0]-1];
			}
		}
	}
	
	if ( ConfigIS.iCoordinatedPhase[1] >0)
	{
		for(int i=0;i<R2No;i++)
		{
			int k=ConfigIS.Phase_Seq_R2[i];
			if ( k+5 == ConfigIS.iCoordinatedPhase[1] )
			{
				dMaxCoordPhase2GrnTime = ConfigIS.dCoordCycle - (dSumOfMinAndClearanceTimeInRing2 - (ConfigIS.Gmin[k+4]) );
				dGmax[ConfigIS.iCoordinatedPhase[1]-1]=dMaxCoordPhase2GrnTime;
				iGmax[1][k]=(int) dGmax[ConfigIS.iCoordinatedPhase[1]-1];
			}
		}
	
	}
}

void doubleCheckMissingPhasesInTintersections(double dGmax[8],int iGmax[2][4]) //  double check for the missing phases. For T intersections. 
{	
	int R1No=ConfigIS.Ring1No;
    int R2No=ConfigIS.Ring2No;
	for(int i=0;i<R1No;i++)  
	{
		int k=ConfigIS.Phase_Seq_R1[i];
		if (ConfigIS.Gmax[k] > 0 && ConfigIS.dCoordinationPhaseSplit[k] == 0 )   // in the case we have a missing phase we need to consider Config.Gmax otherwise  dCoordinationPhaseSplit has the split time. e.g. assume the controller does not have phase 7 and 8 in second ring. we had considered missing phases in Config.Gmax and a dummy phase 8 was added to Config.Gmax structure.
		{
			dGmax[k] = ConfigIS.dCoordinationPhaseSplit[k+4];
			iGmax[0][k]=(int)dGmax[k];
		}
	}
	
	for(int i=0;i<R2No;i++) 
	{
		int k=ConfigIS.Phase_Seq_R2[i];
		if (ConfigIS.Gmax[k+4] > 0 && ConfigIS.dCoordinationPhaseSplit[k+4] == 0 )
		{
			dGmax[k+4] = ConfigIS.dCoordinationPhaseSplit[k]; 
			iGmax[1][k]=(int) dGmax[k+4];
		}
	}
}

// With EV case: will change the MaxGrn to a very large number for EV requested phases
void LinkList2DatFileForEV(LinkedList<ReqEntry> Req_List,char *filename,double InitTime[2],int InitPhase[2],double GrnElapse[2],RSU_Config configIS,int ChangeMaxGrn)
{
    //-- Convert request linkedlist to the NewModel.dat file for GLPK solver.
    //-- Init1 and Init2 are the initial time for the two rings while current phases are in R or Y.
    //-- PassedGrn1 and PassedGrn2 are the elapsed time when current phase is in Green
    //-- Add new dumy argument: ChangeMaxGrn: default value=0, no need to change the green time;
    //---otherwise, will change the max green time to a big number MaxGrnTime
	ofstream fs;

    fs.open(filename,ios::out);

    int R1No=configIS.Ring1No;
    int R2No=configIS.Ring2No;

    fs<<"data;\n";
    fs<<"param SP1:="<<InitPhase[0]<<";\n";  // This is the real phase [1-4]
    fs<<"param SP2:="<<InitPhase[1]<<";\n";  // This is the real phase [5-8]

    for(int i=0;i<2;i++)
    {
        if (InitTime[i]<0)
        {
            InitTime[i]=0;
        }
    }

    fs<<"param init1:="<<InitTime[0]<<";\n";
    fs<<"param init2:="<<InitTime[1]<<";\n";
    fs<<"param Grn1 :="<<GrnElapse[0]<<";\n";
    fs<<"param Grn2 :="<<GrnElapse[1]<<";\n";


	int MP[2];//=ConfigIS.MissPhase[i];// Missing phase
	int RlP[2]={-1,-1};//=ConfigIS.MP_Relate[i];// Missing Phase related
	int Found;

	MP[0]=ConfigIS.MissPhase[0];
	if(MP[0]>=0)
	{
		Found=FindIndexArray<int>(configIS.Phase_Seq_R1,configIS.Ring1No,MP[0]);
		if(Found<0)
			RlP[0]=ConfigIS.MP_Relate[0];
	}

	MP[1]=ConfigIS.MissPhase[1];
	if(MP[1]>=0)
	{
		Found=FindIndexArray<int>(configIS.Phase_Seq_R2,configIS.Ring2No,MP[1]-4);
		if(Found<0)
			RlP[1]=ConfigIS.MP_Relate[1];
	}



    //=================Add the information for Yellow, Red, GrnMin======//
    fs<<"param y             \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=configIS.Phase_Seq_R1[i];
        fs<<"\t"<<(k+1)<<"  "<<configIS.Yellow[k];
    }

    for(int i=0;i<R2No;i++)
    {
        int k=configIS.Phase_Seq_R2[i];
        fs<<"\t"<<(k+5)<<"  "<<configIS.Yellow[k+4];
    }
	//========================================================//
	for(int i=0;i<2;i++)
	{
		if(RlP[i]>=0)
		{
			int MP1=ConfigIS.MissPhase[i];// Missing phase
			int RlP1=ConfigIS.MP_Relate[i];// Missing Phase related
			fs<<"\t"<<(MP1+1)<<"  "<<configIS.Yellow[RlP1];
		}
	}
	//========================================================//
    fs<<";\n";

    fs<<"param red       \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=configIS.Phase_Seq_R1[i];
        fs<<"\t"<<(k+1)<<"  "<<configIS.Red[k];
    }
    for(int i=0;i<R2No;i++)
    {
        int k=configIS.Phase_Seq_R2[i];
        fs<<"\t"<<(k+5)<<"  "<<configIS.Red[k+4];
    }
	//========================================================//
	for(int i=0;i<2;i++)
	{
		if(RlP[i]>=0)
		{
			int MP1=ConfigIS.MissPhase[i];// Missing phase
			int RlP1=ConfigIS.MP_Relate[i];// Missing Phase related
			fs<<"\t"<<(MP1+1)<<"  "<<configIS.Red[RlP1];
		}
	}
	//========================================================//
    fs<<";\n";

    fs<<"param gmin      \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=configIS.Phase_Seq_R1[i];
        fs<<"\t"<<(k+1)<<"  "<<configIS.Gmin[k];
    }
    for(int i=0;i<R2No;i++)
    {
        int k=configIS.Phase_Seq_R2[i];
        fs<<"\t"<<(k+5)<<"  "<<configIS.Gmin[k+4];
    }
	//========================================================//
	for(int i=0;i<2;i++)
	{
		if(RlP[i]>=0)
		{
			int MP1=ConfigIS.MissPhase[i];// Missing phase
			int RlP1=ConfigIS.MP_Relate[i];// Missing Phase related
			fs<<"\t"<<(MP1+1)<<"  "<<configIS.Gmin[RlP1];
		}
	}
	//========================================================//
    fs<<";\n";

    //================End of the information for Yellow, Red, GrnMin======//


    // NEED to change the green max of the requested phases to a large number
    fs<<"param gmax      \t:=";

    for(int i=0;i<R1No;i++)
    {
        int k=configIS.Phase_Seq_R1[i];

        double temp_grnMax=configIS.Gmax[k]; //RequestPhaseInList
	    if(temp_grnMax>0)    // Phase
        {
            if(ChangeMaxGrn!=0) // EV: not only change phase
            {
                fs<<"\t"<<(k+1)<<"  "<<MaxGrnTime;
            }
            else
            {
                if(RequestPhaseInList(Req_List,(k+1)) >= 0)
                {
                    fs<<"\t"<<(k+1)<<"  "<<int(configIS.Gmax[k]*(1+MaxGrnExtPortion));
                }
                else
                {
                    fs<<"\t"<<(k+1)<<"  "<<configIS.Gmax[k];
                }
            }
        }
    }
    for(int i=0;i<R2No;i++)
    {
        int k=configIS.Phase_Seq_R2[i];   // k belongs to {0~3}
        double temp_grnMax=configIS.Gmax[k+4];

        if(temp_grnMax>0)    // Phase is used.
        {
            if(ChangeMaxGrn!=0) // EV: not only change phase
            {
                fs<<"\t"<<(k+5)<<"  "<<MaxGrnTime;
            }
            else
            {
                if(RequestPhaseInList(Req_List,(k+5)) >= 0)
                {
                    fs<<"\t"<<(k+5)<<"  "<<int(configIS.Gmax[k+4]*(1+MaxGrnExtPortion));
                }
                else
                {
                    fs<<"\t"<<(k+5)<<"  "<<configIS.Gmax[k+4];
                }
            }
        }
    }
	//====================MaxGreen=240======================//
	for(int i=0;i<2;i++)
	{
		if(RlP[i]>=0)
		{
			int MP1=ConfigIS.MissPhase[i];// Missing phase
			//int RlP1=ConfigIS.MP_Relate[i];// Missing Phase related
			fs<<"\t"<<(MP1+1)<<"  "<<MaxGrnTime;
		}
	}
	//========================================================//

    fs<<";\n\n";



	
	Req_List.Reset();
	int NumberofRequests=0;
	char tempID[64]=" ";
	fs<<"param priorityType:= ";
	if(Req_List.ListEmpty()==0)
    {
		while(!Req_List.EndOfList())
		{
				NumberofRequests++;
				fs<<NumberofRequests;
				fs<<" ";
				fs<<Req_List.Data().VehClass;			
				fs<<" ";
				strcpy(tempID,Req_List.Data().VehID);
			Req_List.Next();
		}
		while (NumberofRequests<10)
		{
			NumberofRequests++;
			fs<<NumberofRequests;
			fs<<" ";
			fs<<0;			
			fs<<" ";
		}
		fs<<" ;  \n";
	}
	fs<<"param PrioWeigth:=  1 1 2 0 3 0 4 0 5 0 6 0 7 0 8 0 9 0 10 0 ; \n";
	LinkedList<ReqEntry> Req_List_New;
    int priority;
	if(Req_List.ListEmpty()==0)
    {
        //------Here EV=1, TRANSIT=2. EV>TRANSIT-----//
        priority=FindListHighestPriority(Req_List);  //*** Req list should be not empty.***//
        //cout<<"priority"<<priority<<endl;
        FindReqWithSamePriority(Req_List,priority,Req_List_New);
    }

    fs<<"param Rl (tr): 1 2 3 4 5 6 7 8:=\n";
    Req_List_New.Reset();
    int ReqSeq=1;
    while(!Req_List_New.EndOfList())
    {
		//cout<<"Req_List_New.Data().VehClass"<<Req_List_New.Data().VehClass<<endl;
		fs<<ReqSeq<<"  ";
		for(int j=1;j<=8;j++)
		{
			if(Req_List_New.Data().Phase==j)
				fs<<max(Req_List_New.Data().ETA-ROBUSTTIME_LOW_EV,float(1.0))<<"  ";
			else
				fs<<".  ";
		}
		if(ReqSeq<Req_List_New.ListSize())
			fs<<"\n";
		ReqSeq=ReqSeq+1;
		Req_List_New.Next();
    }

    fs<<";\n";

    fs<<"param Ru (tr): 1 2 3 4 5 6 7 8:=\n";
    Req_List_New.Reset();
    ReqSeq=1;
    while(!Req_List_New.EndOfList())
    {
		fs<<ReqSeq<<"  ";
		for(int j=1;j<=8;j++)
		{
			if(Req_List_New.Data().Phase==j)
				if (Req_List_New.Data().MinGreen>0) 
				{
					fs<< ROBUSTTIME_UP_EV+Req_List_New.Data().MinGreen<<"  "; 
				}
				else
				fs<<Req_List_New.Data().ETA+ROBUSTTIME_UP_EV <<"  "; 
			else
				fs<<".  ";
		}
		if(ReqSeq<Req_List_New.ListSize())
			fs<<"\n";
		ReqSeq=ReqSeq+1;
		Req_List_New.Next();
    }
    fs<<";\n";

    fs<<"end;";
    fs.close();
}


void removeZeroWeightReq(LinkedList<ReqEntry> Req_List_, LinkedList<ReqEntry> &Req_List_New)
{
	Req_List_.Reset();
    Req_List_New.ClearList();
    if(Req_List_.ListEmpty()==0)
    {
        while(!Req_List_.EndOfList())
        {
			if ( Req_List_.Data().VehClass==TRANSIT )
			{
				if (ConfigIS.dTransitWeight>0)
					Req_List_New.InsertAfter(Req_List_.Data());
			}
            else if (Req_List_.Data().VehClass==TRUCK )
            {
				if (ConfigIS.dTruckWeight>0)
					Req_List_New.InsertAfter(Req_List_.Data());
			}else
					Req_List_New.InsertAfter(Req_List_.Data());
            Req_List_.Next();
        }
    }
    
}




void FindReqWithSamePriority(LinkedList<ReqEntry> Req_List_,int priority,LinkedList<ReqEntry> &Req_List_New)
{
    Req_List_.Reset();
    Req_List_New.ClearList();

    if(Req_List_.ListEmpty()==0)
    {
        while(!Req_List_.EndOfList())
        {
            if(Req_List_.Data().VehClass==priority)
            {
                Req_List_New.InsertAfter(Req_List_.Data());
            }

            Req_List_.Next();
        }
    }
}

int FindListHighestPriority(LinkedList<ReqEntry> ReqList)
{
    ReqList.Reset();

    int priority=10;  // TRANSIT(2) is lower than EV(1): a large number

    if(ReqList.ListEmpty())
    {
        return priority;
    }

    while(!ReqList.EndOfList())
    {
        if(ReqList.Data().VehClass<priority)
        {
            priority=ReqList.Data().VehClass;
        }
        else
        {
            ReqList.Next();
        }
    }

    return priority;
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




void PrintFile2Log(char *resultsfile)
{
    fstream fss;
    string lineread;
    fss.open(resultsfile,fstream::in);
    if (!fss)
    {
        sprintf(tmp_log,"***********Error opening the plan file in order to print to a log file!\n");   
        outputlog(tmp_log);
        exit(1);
    }
    while(!fss.eof())
    {
        getline(fss,lineread);
        strcpy(tmp_log,lineread.c_str());	
        outputlog(tmp_log);		
    }   
    fss.close();
}


void deductSolvingTimeFromCPs(double  aCriticalPoints[2][3][15],double tt2,double tt1)
{
	for (int k=0;k<2;k++)
	{
		for (int ii=0;ii<15;ii++)
		{
			if (adCriticalPoints[k][0][ii]>0)
				adCriticalPoints[k][0][ii] = max(0.0, adCriticalPoints[k][0][ii]-(tt2-tt1));
			if (adCriticalPoints[k][1][ii]>0)
				adCriticalPoints[k][1][ii] = max(0.0, adCriticalPoints[k][1][ii]-(tt2-tt1));
		}
	}
}
void GLPKSolver()
{
	double dAfterSolvingTime;
	double dBeforeSolvingTime=GetSeconds();
	// The argument should be the real phase 1-8.
	struct timeval start;
	gettimeofday(&start, NULL);

	char modFile[128]="/nojournal/bin/NewModel.mod";

	if(HaveEVInList==1)
	{
		strcpy(modFile,"/nojournal/bin/NewModel_EV.mod");
	}
	outputlog(modFile);
	outputlog("\n");

	glp_prob *mip;
	glp_tran *tran;
	int ret;
	mip = glp_create_prob();
	tran = glp_mpl_alloc_wksp();



	//ret = glp_mpl_read_model(tran, "./Mod/PriReq_26.mod", 1);
	ret = glp_mpl_read_model(tran, modFile, 1);

	if (ret != 0)
		{  fprintf(stderr, "Error on translating model!\n");
	sprintf(tmp_log,"Error on translating model!: [%.2f].\n",GetSeconds());
					outputlog(tmp_log);
	goto skip;
		}
	ret = glp_mpl_read_data(tran, "/nojournal/bin/NewModelData.dat");

	if (ret != 0)
		{  fprintf(stderr, "Error on translating data\n");
	sprintf(tmp_log,"Error on translating data: [%.2f].\n",GetSeconds());
					outputlog(tmp_log);
	goto skip;
		}
	ret = glp_mpl_generate(tran, NULL);

	if (ret != 0)
		{  fprintf(stderr, "Error on generating model\n");
	 sprintf(tmp_log,"Error on generating model: [%.2f].\n",GetSeconds());
					outputlog(tmp_log);
	goto skip;
		}

	dAfterSolvingTime=GetSeconds();  
	cout<<"TIME TO GENERATE THE OPTIMIZATION MODEL   "<<dAfterSolvingTime-dBeforeSolvingTime<<endl;  
		
	glp_mpl_build_prob(tran, mip);
	glp_simplex(mip, NULL);
	glp_iocp iocp;
	glp_init_iocp(&iocp);
	iocp.tm_lim=4000; // if after 4 seconds the optimizer can not find the best MIP solution, skip this time of solving
  
	glp_intopt(mip, &iocp);
	
    ret = glp_mpl_postsolve(tran, mip, GLP_MIP);

	if (ret != 0){
		fprintf(stderr, "Error on postsolving model\n");
		sprintf(tmp_log,"Error on postsolving model: [%.2f].\n",GetSeconds());
					outputlog(tmp_log);

				}
	skip: glp_mpl_free_wksp(tran);
	glp_delete_prob(mip);
}

void get_lane_no() 
{
    fstream fs;

    fs.open(Lane_No_File_Name);

    char temp[128];
	
	string temp_string;

    getline(fs,temp_string);  //First line is comment
	getline(fs,temp_string);  //Second line contains information


    if(temp_string.size()!=0)
    {
		char tmp[128];
		strcpy(tmp,temp_string.c_str());		
		sscanf(tmp,"%d %d %d %d %d %d %d %d",&LaneNo[0],&LaneNo[1],&LaneNo[2],&LaneNo[3],&LaneNo[4],&LaneNo[5],&LaneNo[6],&LaneNo[7]);
    }
    else
    {
        sprintf(temp,"Reading Lane_No_File problem");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }
    for(int e=0;e<8;e++)
    {
		TotalLanes+=LaneNo[e];
		cout<< "Phase"<< e+1<< "number of lane" << LaneNo[e]<<endl;
	}
		
    fs.close();
}

void get_lane_phase_mapping(int PhaseOfEachApproach[4][2], int PhaseOfEachLane[48]) 
{
    fstream fs;
    fs.open(Lane_Phase_File_Name);
    char temp[128];
	
	string temp_string;

    getline(fs,temp_string);  //First line is comment
	getline(fs,temp_string);  //Second line contains information

    if(temp_string.size()!=0)
    {
		char tmp[128];
		strcpy(tmp,temp_string.c_str());		
		sscanf(tmp,"%d %d %d %d %d %d %d %d",&PhaseOfEachApproach[0][0],&PhaseOfEachApproach[0][1],&PhaseOfEachApproach[1][0],&PhaseOfEachApproach[1][1],
											 &PhaseOfEachApproach[2][0],&PhaseOfEachApproach[2][1],&PhaseOfEachApproach[3][0],&PhaseOfEachApproach[3][1]);
    }
    else
    {
        sprintf(temp,"Reading Lane_Phase_Mapping_File problem");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }
    int lane=0;
	for(int j=0;j<4;j++)
	{
		//populating phase of each lane data structre
		if (PhaseOfEachApproach[j][0]!=PhaseOfEachApproach[j][1]) // in the case that left turn and through are not the same phase
		{
			for (int i=0;i<LaneNo[PhaseOfEachApproach[j][0]-1];i++)
			{
				PhaseOfEachLane[lane]=PhaseOfEachApproach[j][0];
				lane++;
			}
			for (int i=0;i<LaneNo[PhaseOfEachApproach[j][1]-1];i++)
			{
				PhaseOfEachLane[lane]=PhaseOfEachApproach[j][1];
				lane++;
			}
		}
		else 
		{
			for (int i=0;i<LaneNo[PhaseOfEachApproach[j][0]-1];i++)
			{
				PhaseOfEachLane[lane]=PhaseOfEachApproach[j][0];
				lane++;
			}	
		}
	}
	for(int j=0;j<4;j++)
	{
		lane=0;
		//populating LaneNo2 data structure to be used in calculateQ function
		if (PhaseOfEachApproach[j][0]!=PhaseOfEachApproach[j][1]) // in the case that left turn and through are not the same phase
		{
			LaneNo2[PhaseOfEachApproach[j][0]-1]=LaneNo[PhaseOfEachApproach[j][0]-1]; // LaneNo2 will record the total number of lanes per approach
			LaneNo2[PhaseOfEachApproach[j][1]-1]=LaneNo[PhaseOfEachApproach[j][0]-1]+LaneNo[PhaseOfEachApproach[j][1]-1]; 
		}
		else
		{
			LaneNo2[PhaseOfEachApproach[j][0]-1]=LaneNo[PhaseOfEachApproach[j][0]-1]; // LaneNo2 will record the total number of lanes per approach
		}
		
	}
	for (int jj=0;jj<TotalLanes;jj++)
		cout<<"lane "<< jj+1 <<" phase"<<PhaseOfEachLane[jj]<<endl;
	for (int jj=0;jj<8;jj++)	
		cout<<"phase "<< jj+1 <<" lane num per approach"<<LaneNo2[jj]<<endl;
    fs.close();
}




void calculateQ(int PhaseOfEachApproach[4][2],int PhaseOfEachLane[48],double dQsizeOfEachLane[48])
{
	ConnectedVehicle temp_CV;
	int templane=0;
	int tempapproach=0;
	int iMaxVehInQ=0;
	double dSizeOfQ=0.0;
	double dEndofQ=0.0;
	double dBeginingofQ=10000.0;
	int templ=0;
//	double dRecTime=GetSeconds();
	for (int e;e<8;e++)
	{
		for (int ee;ee<6;ee++)
		{
			NoVehInQ[e][ee]=0;
			QSize[e][ee]=0;
		}
	
	}
	for (int p=0;p<TotalLanes;p++)
		dQsizeOfEachLane[p]=0;
		
	for (int e=0;e<8;e++)
	{
		//Assign vehicles to the list of each lane of each phase. First of all, vehicles clusters by their phases and then cluster by each lane in the phase.
		trackedveh.Reset();
		vehlist_phase.ClearList();
		while(!trackedveh.EndOfList())  
		{
			if (trackedveh.Data().req_phase==e+1) //For each phase in hte list of vehicles
			{
				
				//{ cout<< "ID "<< trackedveh.Data().TempID<<"Lane "<< trackedveh.Data().lane<< "phase "<< trackedveh.Data().req_phase <<  "speed "<< trackedveh.Data().Speed<<"Dist "<<trackedveh.Data().stopBarDistance<< endl; }
				//if (trackedveh.Data().time_stop>0)
					
				temp_CV=trackedveh.Data();
				vehlist_phase.InsertRear(temp_CV);
			}
			
			//if (trackedveh.Data().req_phase>0) //Requested phase  
			//{
				//if(trackedveh.Data().Speed>1)
				//{
					//int ETA= int (trackedveh.Data().stopBarDistance/trackedveh.Data().Speed) ; //ceil(trackedveh.Data().stopBarDistance/trackedveh.Data().Speed);
					//if (ETA<50)
						//ArrivalTable[ETA][trackedveh.Data().req_phase-1]++;
				//}
				//else
					//ArrivalTable[0][trackedveh.Data().req_phase-1]++;
				
			//}
			trackedveh.Next();
		}
		// TEMPORARY!!!!!!!!!!!!!!!!!!!!!!!!!! // !!! check the total lines if we want to use the code for other intersections
		templ=0;
		if ((e+1)==1)
			templ=LaneNo[5];
		if ((e+1)==3)
			templ=LaneNo[7];
		if ((e+1)==5)
			templ=LaneNo[1];
		if ((e+1)==7)
			templ=LaneNo[3];
		//if (((e+1)==8)||((e+1)==2)||((e+1)==6))
			//templ=1;
			
			
		for (int ee=templ;ee<LaneNo[e]+templ;ee++)
		{
			vehlist_phase.Reset();
			vehlist_lane.ClearList();
			while(!vehlist_phase.EndOfList())  //Record the vehicles of each lane from each phase
			{
				//cout<<"PHAASE"<<vehlist_phase.Data().req_phase<<" LANEEEEE"<<	vehlist_phase.Data().lane<<endl; 
				if (vehlist_phase.Data().lane==ee+1) 
				{
					temp_CV=vehlist_phase.Data();
					vehlist_lane.InsertRear(temp_CV);
				}
				vehlist_phase.Next();
			}
			templane=0;
			tempapproach=0;
			iMaxVehInQ=0;
			dSizeOfQ=0.0;
			dEndofQ=0.0;
			dBeginingofQ=10000.0;
			vehlist_lane.Reset();
			while(!vehlist_lane.EndOfList())  // find the q length
			{
				//calculate queue size
				if (vehlist_lane.Data().Speed<1) 
				{
					iMaxVehInQ++;
					dEndofQ=max(dEndofQ,vehlist_lane.Data().stopBarDistance);
					dBeginingofQ=min(dBeginingofQ,vehlist_lane.Data().stopBarDistance);
					templane=vehlist_lane.Data().lane;
					tempapproach=vehlist_lane.Data().approach;
				}
				vehlist_lane.Next();
			}
			if (iMaxVehInQ==0) // no vehicle is in q
				dSizeOfQ=0.0;
			//else if (dEndofQ==dBeginingofQ) // only one vehicle in Q
				//dSizeOfQ=dEndofQ;
			else
			{
				if ((dBeginingofQ==dEndofQ) && (dEndofQ<3) ) //only one car in the queue and the car is the first car in the queue
					dSizeOfQ=3.5;
				else if (dBeginingofQ==dEndofQ) // only one connected car in the queue and there is a queue of non-connected cars infront of it
					dSizeOfQ=dEndofQ+3.5;
				else
					dSizeOfQ=dEndofQ-dBeginingofQ; 
			}
				
			NoVehInQ[e][ee]=iMaxVehInQ;
			QSize[e][ee]=dSizeOfQ;
		//		cout<<"Qsize phase "<<e+1<<"lane "<<ee+1<<  "  size "<<QSize[e][ee]<<endl;
			if (templane>1) // !!! check the total lines if we want to use the code for other intersections
			{
				if ((e+1==4)||(e+1==7))
					dQsizeOfEachLane[templane-1]=dSizeOfQ;
				else if ((e+1==1)||(e+1==6))
					dQsizeOfEachLane[templane+3-1]=dSizeOfQ;
				else if ((e+1==3)||(e+1==8))
					dQsizeOfEachLane[templane+3+5-1]=dSizeOfQ;
			 	else if ((e+1==2)||(e+1==5))
					dQsizeOfEachLane[templane+3+5+4-1]=dSizeOfQ;
			}
		
			//cout<<"Qsize"<<QSize[e][ee]<<endl;
			
			//cout<< "TEMPLATE LANE"<<templane<<endl;
			//if ((templane>0) && (e+1==PhaseOfEachApproach[(int)(tempapproach-1)/2][0]))
			//{
				//for (int cn=(int)(tempapproach-1)/2 ;cn>0;cn--)
				//{
					//if (PhaseOfEachApproach[cn][0]!=PhaseOfEachApproach[cn][1])
					//{
						//templane=LaneNo[PhaseOfEachApproach[cn][0]]+LaneNo[PhaseOfEachApproach[cn][1]]+templane;
					//}else
						//templane=LaneNo[PhaseOfEachApproach[cn][0]]+templane;
				//}
				//dQsizeOfEachLane[templane]=dSizeOfQ;
			//}
			//cout<<" LANE NUBMER" <<templane<< " SIZE " <<dQsizeOfEachLane[templane]<<endl;
			
		}
	}
	//for (int g=0;g<TotalLanes;g++) // !!! check the total lines if we want to use the code for other intersections
	//	cout<<"line"<<g+1<<" size "<<dQsizeOfEachLane[g]<<endl;
		
}
void extractOptModInputFromTraj(double dVehDistToStpBar[130],double dVehSpd[130],int iVehPhase[130],double Ratio[130],double indic[130],int laneNumber[130],double dQsizeOfEachLane[48],double Tq[130], int PhaseOfEachApproach[4][2],int PhaseOfEachLane[48],int InitPhase[2],double GrnElapse[2])
{
		trackedveh.Reset();
		int VehCnt=0;
		currentTotalVeh=0;
		for (int it=0;it<130;it++)
		{	
			dVehDistToStpBar[it]=0;
			dVehSpd[it]=0;
			iVehPhase[it]=0;
			Ratio[it]=0; 
			indic[it]=0; 
		}
		for (int it=0;it<TotalLanes;it++)
			bLaneSignalState[it]=0;
		
		while((!trackedveh.EndOfList() && VehCnt<130))  
		{
			
			if (trackedveh.Data().req_phase>0) 
			{
				if (trackedveh.Data().time_stop>0)
					Tq[VehCnt]=GetSeconds()-trackedveh.Data().time_stop;
				else
					Tq[VehCnt]=0.0;
				dVehDistToStpBar[VehCnt]=trackedveh.Data().stopBarDistance;
				if (trackedveh.Data().Speed>1)
					dVehSpd[VehCnt]=trackedveh.Data().Speed;
				else
					dVehSpd[VehCnt]=1;
				iVehPhase[VehCnt]=trackedveh.Data().req_phase;
				//cout<<"iVehPhase[VehCnt]"<<iVehPhase[VehCnt]<<endl;
				// obtaining the lane
				int temp= (int) (trackedveh.Data().approach-1)/2-1;
				int lane=0;
				for (int j=temp;j>0;j--)
				{
					
					lane =lane+LaneNo[PhaseOfEachApproach[j][0]];
					lane =lane+LaneNo[PhaseOfEachApproach[j][1]];
					
				}
				laneNumber[VehCnt]=lane+trackedveh.Data().lane;
				if ((0<laneNumber[VehCnt]) && (laneNumber[VehCnt]<=TotalLanes))
				{
					
					Ratio[VehCnt]=max(0.0, (dVehDistToStpBar[VehCnt]-dQsizeOfEachLane[laneNumber[VehCnt]])/(dQueuingSpd+dVehSpd[VehCnt]));
					indic[VehCnt]=Ratio[VehCnt]-dQsizeOfEachLane[laneNumber[VehCnt]]/(dQDischargingSpd-dQueuingSpd);
				}
				VehCnt++;
				currentTotalVeh++;
			}
			trackedveh.Next();
		}
		
		if (GrnElapse[0]>0)
		{
			for (int i=0;i<TotalLanes;i++)
			{
				if (PhaseOfEachLane[i]==InitPhase[0])
					bLaneSignalState[i]=1;
			}
		}
		
		
		if (GrnElapse[1]>0)
		{
			for (int i=0;i<TotalLanes;i++)
			{
				if (PhaseOfEachLane[i]==InitPhase[1])
					bLaneSignalState[i]=1;
					//cout<<"	bLaneSignalState[i]"<<bLaneSignalState[i]<<endl;
			}
		}
		
}

void identifyColor(int i_Color[2][8], int greenGroup, int redGroup, int yellowGroup, int nextPhase)
{
	
	// numbers 2, 4,5,6 and 7 were the output of ASC/3 proprietry NTCIP OID for different status of a phase.   ( PHASE_STA_TIME2_ASC in the MIB.h )
	// 4 is for red phase, 7 for green 6 for yellow, 2 for the next and 5 for a phase that just gets red after being yellow.
	// Since the application was originally written based on this proprietary OID, we tranfer the obtained signal status out of red, green, yellow next group NTCIP OIDs to it.
	for(int iii=0; iii<8; iii++)
	{
		if (ConfigIS.Gmax[iii]==0 || (( iii == ConfigIS.MissPhase[0] && ConfigIS.MissPhase[0]>=0) || (iii == ConfigIS.MissPhase[1] && ConfigIS.MissPhase[1]>=0) ) )  // in case phase iii is missing, the max is zero and i_Color gets 3. Since the Econolite proprietory OID, returns the  missing phase as 3, we need this conversion.  We do not want to use vendors proprietary OID
			i_Color[1][iii]=3;
		else
			i_Color[1][iii]=4;    	
	}
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


int GetSignalColor(int PhaseStatusNo)
{
    int ColorValue=1;

    switch (PhaseStatusNo)
    {
    case 2:
    case 3:
    case 4:
    case 5:
        ColorValue=1;
        break;
    case 6:
    case 11:
        ColorValue=4;
        break;
    case 7:
    case 8:
        ColorValue=3;
        break;
    default:
        ColorValue=0;
    }
    return ColorValue;
}


// WILL use the global parameter phase_read
void PhaseTimingStatusRead()
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
      
        int *out = new int[MAX_ITEMS];
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
        for(int i=0;i<8;i++)
        {
			phase_read.phaseColor[i]=phaseColor[i];
            CurPhaseStatus[i]=iColor[1][i];   
        }
/*		for(int i=0;i<8;i++)
        {
			cout<< phaseColor[i] << " ";   
        }
        cout<<endl;
         for(int i=0;i<8;i++)
        {
			cout<< iColor[1][i] << " ";   
        }
        cout<<"  "<<endl;
*/
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

int  CurTimingPlanRead()
{
    // USING the standard oid : ONLY read timing plan 1.
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
   
    int currentTimePlan; // return value

    init_snmp("RSU");   //Initialize the SNMP library

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
        snmp_sess_perror("RSU", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to GET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_GET); //DD: package data unit
    anOID_len = MAX_OID_LEN;

    //---#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

    char ctemp[50];

    sprintf(ctemp,"%s",CUR_TIMING_PLAN);   // JD need work
   
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
        int *out = new int[MAX_ITEMS];
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

        //CUR_TIMING_PLAN_NO=out[0];
        currentTimePlan=out[0];

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

    return currentTimePlan;

}


void IntersectionConfigRead(int CurTimingPlanNo,char *ConfigOutFile)
{

    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    
    /*
    * Initialize the SNMP library
    */
    init_snmp("RSU");

    /*
    * Initialize a "session" that defines who we're going to talk to
    */
    snmp_sess_init( &session );                   /* set up defaults */
    //char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
    //char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
    char ipwithport[64];
    strcpy(ipwithport,INTip);
 
    strcat(ipwithport,":");
    strcat(ipwithport,INTport); //for ASC get status, DO NOT USE port!!!
    session.peername = strdup(ipwithport);
    /* set the SNMP version number */
    session.version = SNMP_VERSION_1; //for ASC intersection
    //session.version = SNMP_VERSION_1; //for Rampmeter

    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
    {
        snmp_sess_perror("RSU", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to GET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;

    //// Phase options: last ".X" is phase, the last bit of return result is "0", the phase is not enabled.
    //#define PHASE_ENABLED       "1.3.6.1.4.1.1206.4.2.1.1.2.1.21."
    ////------The following from ASC3------------//
    //#define PHASE_MIN_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.9."   // need last "x.p" x is the timing plan number,p is the phase number: x get from CUR_TIMING_PLAN
    //#define PHASE_MAX_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.15."
    //#define PHASE_RED_CLR_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.19."
    //#define PHASE_YLW_XGE_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.18."
    char ctemp[50];

    for(int i=1;i<=8;i++) //PHASE_MIN_GRN_ASC
    {
        sprintf(ctemp,"%s%d.%d",PHASE_MIN_GRN_ASC,CurTimingPlanNo,i);        //  in seconds

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }

    for(int i=1;i<=8;i++)  //PHASE_YLW_XGE_ASC
    {
        sprintf(ctemp,"%s%d.%d",PHASE_YLW_XGE_ASC,CurTimingPlanNo,i);    // in tenth of seconds, for example if 35 should be 3.5 seconds

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }

    for(int i=1;i<=8;i++) //PHASE_RED_CLR_ASC
    {
        sprintf(ctemp,"%s%d.%d",PHASE_RED_CLR_ASC,CurTimingPlanNo,i);   // in tenth of seconds, for example if 35 should be 3.5 seconds

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }

    for(int i=1;i<=8;i++) // PHASE_MAX_GRN_ASC
    {
        sprintf(ctemp,"%s%d.%d",PHASE_MAX_GRN_ASC,CurTimingPlanNo,i);  // in seconds

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }
    for(int i=1;i<=8;i++)
    {
        sprintf(ctemp,"%s%d",PHASE_ENABLED,i);

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }

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
        int *out = new int[MAX_ITEMS];
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
        //FOR ASC INTERSECTIONS_ Draw the lights
        //UpdateIntersectionStatus(out[0],out[1]);
        //cout<<"Minimum Green Time for Phase 1 is:\t"<<out[0]<<" ****** "<<out[1]<<endl;
        // *** int PhaseSeq[8],MinGrn[8],Yellow[8],RedClr[8],GrnMax[8];***//

        int Result[5][8];  //*** Sequence: MinGrn[8],Yellow[8],RedClr[8],GrnMax[8],PhaseSeq[8],;***//
        /*
        Result[0][8]  ----> MinGrn[8]
        Result[1][8]  ----> Yellow[8]
        Result[2][8]  ----> RedClr[8]
        Result[3][8]  ----> GrnMax[8]
        Result[4][8]  ----> PhaseSeq[8]
        */
        for(int i=0;i<5;i++)
        {
            for(int j=0;j<8;j++)
            {
                Result[i][j]=out[j+i*8];
            }
        }
        int PhaseSeq[8],MinGrn[8],GrnMax[8];
        float   Yellow[8],RedClr[8];

        int TotalNo=0;

        for(int i=0;i<8;i++)
        {
            //*** Last bit is 1, odd number, the '&' is 0:
            if(Result[4][i] & 1)  //*** Be careful here: it is bit "&" not "&&". ***//
            {
                PhaseSeq[i]=i+1;

                MinGrn[i]=Result[0][i];
                Yellow[i]=float(Result[1][i]/10.0);
                RedClr[i]=float(Result[2][i]/10.0);
                GrnMax[i]=Result[3][i];

                TotalNo++;
            }

            else
            {
                PhaseSeq[i]=0;
                MinGrn[i]=0;
                Yellow[i]=0;
                RedClr[i]=0;
                GrnMax[i]=0;
            }
        }

        fstream fs_config;
        //TODO:,char *ConfigOutFile
        //fs_config.open(ConfigFile,ios::out);
        fs_config.open(ConfigOutFile,ios::out);


        fs_config<<"Phase_Num "<<TotalNo<<endl;

        fs_config<<"Phase_Seq";
        for(int i=0;i<8;i++) fs_config<<" "<<PhaseSeq[i];
        fs_config<<endl;

        fs_config<<"Gmin";
        for(int i=0;i<8;i++) fs_config<<"\t"<<MinGrn[i];
        fs_config<<endl;

        fs_config<<"Yellow";
        for(int i=0;i<8;i++) fs_config<<"\t"<<Yellow[i];
        fs_config<<endl;

        fs_config<<"Red";
        for(int i=0;i<8;i++) fs_config<<"\t"<<RedClr[i];
        fs_config<<endl;

        fs_config<<"Gmax";
        for(int i=0;i<8;i++) fs_config<<"\t"<<GrnMax[i];
        fs_config<<endl;


        fs_config.close();


    }
    else
    {
        /*
        * FAILURE: print what went wrong!
        */

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
    * Clean up:
    *  1) free the response.
    *  2) close the session.
    */
    if (response)        snmp_free_pdu(response);

    snmp_close(ss);

    SOCK_CLEANUP;

}

void IntersectionPhaseControl(int phase_control, int Total,char YES)
{
    char tmp_log[64];
    char tmp_int[16];
   
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    int count=1;
    int  failures = 0;

    //int number=pow(2.0,phaseNO-1);
    int number=Total;

    //itoa(number,buffer,2); // NOT WORKING

    sprintf(tmp_int,"%d",number);
    cout<<"CMD Applied to Phase: ";//<<buffer;
    binary(Total);
    cout<<"  "<<tmp_int<<"  "<<YES<<endl;

    /*
    * Initialize the SNMP library
    */
    init_snmp("RSU");

    /*
    * Initialize a "session" that defines who we're going to talk to
    */
    snmp_sess_init( &session );                   /* set up defaults */

    char ipwithport[64];
    strcpy(ipwithport,INTip);
    strcat(ipwithport,":");
    strcat(ipwithport,INTport);
    session.peername = strdup(ipwithport);

    /* set the SNMP version number */
    session.version = SNMP_VERSION_1;

    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
    {
        snmp_sess_perror("RSU", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to SET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_SET);
    anOID_len = MAX_OID_LEN;
    if (PHASE_HOLD==phase_control)
    {
        if (!snmp_parse_oid(MIB_PHASE_HOLD, anOID, &anOID_len))
        {
            snmp_perror(MIB_PHASE_HOLD);
            failures++;
        }
        sprintf(tmp_log,"HOLD control! Number (%d), AT time:[%.2f] \n ",Total,GetSeconds()); 
        outputlog(tmp_log);

    }
    else if (PHASE_FORCEOFF==phase_control)
    {
        if (!snmp_parse_oid(MIB_PHASE_FORCEOFF, anOID, &anOID_len))
        {
            snmp_perror(MIB_PHASE_FORCEOFF);
            failures++;
        }
        sprintf(tmp_log,"FORCEOFF control! Number (%d), AT time:[%.2f] \n ",Total,GetSeconds());
         outputlog(tmp_log);

    }
    else if (PHASE_OMIT==phase_control)
    {
        if (!snmp_parse_oid(MIB_PHASE_OMIT, anOID, &anOID_len))
        {
            snmp_perror(MIB_PHASE_OMIT);
            failures++;
        }
        sprintf(tmp_log,"OMIT control! Number (%d), AT time:[%.2f] \n ",Total,GetSeconds());
         outputlog(tmp_log);
    }
    else if (PHASE_VEH_CALL==phase_control)
    {
        if (!snmp_parse_oid(MIB_PHASE_VEH_CALL, anOID, &anOID_len))
        {
            snmp_perror(MIB_PHASE_VEH_CALL);
            failures++;
        }
        sprintf(tmp_log,"VEH CALL to ASC controller! Number (%d), AT time:%ld \n ",Total,time(NULL));
        outputlog(tmp_log);
    }


    //snmp_add_var() return 0 if success
    if (snmp_add_var(pdu, anOID, anOID_len,'i', tmp_int))
    {
        switch(phase_control)
        {
        case PHASE_FORCEOFF:
            snmp_perror(MIB_PHASE_FORCEOFF); break;
        case PHASE_OMIT:
            snmp_perror(MIB_PHASE_OMIT); break;
        case PHASE_VEH_CALL:
            snmp_perror(MIB_PHASE_VEH_CALL); break;
        case PHASE_HOLD:
            snmp_perror(MIB_PHASE_HOLD); break;
        }

        failures++;
    }

    if (failures)
    {
        snmp_close(ss);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Send the Request out.
    */
    status = snmp_synch_response(ss, pdu, &response);

    /*
    * Process the response.
    */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
        //------SUCCESS: Print the result variables
        int *out = new int[MAX_ITEMS];
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
                printf("value #%d is a string: %s\n", count++, sp);
                free(sp);
            }
            else
            {
                int *aa;
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                printf("value #%d is NOT a string! Ack!\n", count++);
            }
        }
    }
    else
    {
        // FAILURE: print what went wrong!
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet\nReason: %s\n",
            snmp_errstring(response->errstat));
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
            session.peername);
        else
            snmp_sess_perror("snmpdemoapp", ss);
    }
    //------Clean up:1) free the response. 2) close the session.
    if (response)
        snmp_free_pdu(response);
    snmp_close(ss);

    SOCK_CLEANUP;

}



/*   // NEED DEVELOPMENT
handleAdaptivePriorityCase()
{

	// Related to AdaptivePriority
	//Record current red elapse time information
	//Notes: since COP always runs at the beginning of the green time (2 phases), actually we should calculate the duration of previous red phase for the queue length estimation
	//for the 2 phases just turned red, the red duration time could be 0 (or in actually calculation they are very small)
	//for other four phases, just in the middle of red duration
	double red_start_time[8]={};
	int previous_signal_color[8]={};
	ConnectedVehicle temp_veh;
	int recv_data_len;
	int numbytes=0;
	double t_RequestTraj=0.0;  //Time to request Arrivale Table in case the code is used for adaptive control too
	int PhaseOfEachApproach[4][2]; // Use Lane_Phase_File_Name to populate this array
	int PhaseOfEachLane[48]; // assume there are 48 approaching lanes at most
	//input param for MILP model if codeusage is AdaptivePriority
	double dVehDistToStpBar[130]; // at most 130 vehicles is considered in the adaptive control optimization model
	double dVehSpd[130];
	int iVehPhase[130];
	double Ratio[130]; // one of the inputs of mathematical model
	double indic[130]; // one of the inputs of mathematical model
	int laneNumber[130]; // lane number of each car 
	double dQsizeOfEachLane[48];
	double Tq[130]; // The amount of time each vehicle has stopped 
	






















	if (t_2-t_RequestTraj>4) 	// if is it time to get trajectory from traj. aware componet and solve the problem. It is supposed to get the arrival table every 2 seconds
	{
		t_RequestTraj=GetSeconds();
		char buf[64]="Request Trajectory from signal control";   // sending the request to Trajectory Aware component 
		cout<<"Sent Request!"<<endl;
		//send request for trajectory data
		recvaddr.sin_port = htons(TrajRequestPort);
		numbytes = sendto(sockfd,buf,sizeof(buf) , 0,(struct sockaddr *)&recvaddr, addr_length);
		recv_data_len = recvfrom(sockfd, buf_traj, sizeof(buf_traj), 0,
						(struct sockaddr *)&sendaddr, (socklen_t *)&addr_length);
		if(recv_data_len<0)
		{
			printf("Receive Request failed\n");
			continue;
		}
		sprintf(temp_log,"Received Trajectory Data!\n");
		outputlog(temp_log); cout<<temp_log;
		trackedveh.ClearList(); //clear the list for new set of data
		
		//unpack the trajectory data and save to trackedveh list
		UnpackTrajData1(buf_traj);
		sprintf(temp_log,"The Number of Vehicle Received is: %d\n",trackedveh.ListSize());
		outputlog(temp_log); cout<<temp_log;
	//		sprintf(temp_log,"The red elapse time for each phase is:\n");
	//		outputlog(temp_log);
	//		sprintf(temp_log,"%f %f %f %f %f %f %f %f \n",red_elapse_time[0],red_elapse_time[1],red_elapse_time[2],red_elapse_time[3],red_elapse_time[4],red_elapse_time[5],red_elapse_time[6],red_elapse_time[7]);
	//		outputlog(temp_log);
		
		sprintf(temp_log,"Current time is: %f \n",t_RequestTraj);
		outputlog(temp_log);
			   
		if (penetration>=0.95)   //construct arrival table directly
		{
			trackedveh.Reset();
			cout<<"********************************"<<endl;
			calculateQ(PhaseOfEachApproach,PhaseOfEachLane,dQsizeOfEachLane);
			extractOptModInputFromTraj(dVehDistToStpBar,dVehSpd,iVehPhase,Ratio,indic,laneNumber,dQsizeOfEachLane,Tq,PhaseOfEachApproach,PhaseOfEachLane,InitPhase,GrnElapse);
		}else // calling EVLS 
		{
			//Algorithm to infer vehicle location and speed based on sample vehicle data and create arrival table
			cout<<"Estimating Location and Position of Unequipped Vehicle:"<<endl;
			for (int i=0;i<8;i++)  
			{	
			
				//Assign vehicles of each phase to the vehlist_each_phase
				trackedveh.Reset();
				vehlist_each_phase.ClearList();
				while(!trackedveh.EndOfList())  
				{
					if (trackedveh.Data().req_phase==i+1)  //assign to the same phase
					{
						temp_veh=trackedveh.Data();
						vehlist_each_phase.InsertRear(temp_veh);
					}
				trackedveh.Next();
				}
			
				
				sprintf(temp_log,"Phase: %d\n",i+1);
				outputlog(temp_log); cout<<temp_log;
				
				//sprintf(temp_log,"The input list is:\n");
				//outputlog(temp_log); cout<<temp_log;
				vehlist_each_phase.Reset();
				for(int j=0;j<vehlist_each_phase.ListSize();j++)
				{
				sprintf(temp_log,"%d %lf %lf %lf %d %d %d %lf\n",vehlist_each_phase.Data().TempID,vehlist_each_phase.Data().Speed,vehlist_each_phase.Data().stopBarDistance,vehlist_each_phase.Data().acceleration,vehlist_each_phase.Data().approach,vehlist_each_phase.Data().lane,vehlist_each_phase.Data().req_phase,vehlist_each_phase.Data().time_stop);
				outputlog(temp_log); //cout<<temp_log;
				vehlist_each_phase.Next();
				}
				
				cout<<"The number of lanes is "<<LaneNo[i]<<endl;
				
				if (vehlist_each_phase.ListSize()!=0)  //There is vehicle in the list 
					EVLS(i+1, t_RequestTraj, red_elapse_time[i], LaneNo[i]);
			}
			trackedveh.Reset();
			cout<<"********************************"<<endl;
			calculateQ(PhaseOfEachApproach,PhaseOfEachLane,dQsizeOfEachLane);
			extractOptModInputFromTraj(dVehDistToStpBar,dVehSpd,iVehPhase,Ratio,indic,laneNumber,dQsizeOfEachLane,Tq,PhaseOfEachApproach,PhaseOfEachLane,InitPhase,GrnElapse);

		}
		
		LinkList2DatFile(Req_List_Combined,prioritydatafile,InitTime,InitPhase,GrnElapse, ConfigIS.dTransitWeight, ConfigIS.dTruckWeight,ConfigIS.dCoordinationWeight,dVehDistToStpBar,dVehSpd,iVehPhase,Ratio,indic,laneNumber,dQsizeOfEachLane,Tq); // construct .dat file for the glpk
		
		PrintFile2Log(prioritydatafile); // Log the .dat file for glpk solver
		double t1_Solve=GetSeconds();
		GLPKSolver();  
		double t2_Solve=GetSeconds();
		sprintf(tmp_log,"Time for solving the problem is about: {%.3f}.\n",t2_Solve-t1_Solve); 
		outputlog(tmp_log);
		cout<< tmp_log<<endl;
		int success2=GLPKSolutionValidation(resultsfile);
		cout<<"success2"<<success2<<endl;
		if (success2==1)  
		{	
			
			Eventlist_R1.ClearList();
			Eventlist_R2.ClearList();				
			readOptPlanFromFile(resultsfile,adCriticalPoints);
			deductSolvingTimeFromCPs(adCriticalPoints,t2_Solve,t1_Solve);
			Construct_eventlist(adCriticalPoints,Req_List_Combined);
			
			byte tmp_event_data[500];
			int size=0;
			Pack_Event_List(tmp_event_data, size);
			char* event_data;
			event_data= new char[size];			
			for(int i=0;i<size;i++)
				event_data[i]=tmp_event_data[i];	
			recvaddr.sin_port = htons(TxEventListPort);
			if (sendto(sockfd,event_data,size+1 , 0,(struct sockaddr *)&recvaddr, addr_length))
			{
				sprintf(temp_log," The new Event List sent to SignalControllerInterface, The size is %d and time is : %.2f.......... \n", size, GetSeconds()); 
				outputlog(temp_log);
				cout<< temp_log<< endl;
			}
		}
	} //end of if (t_2-t_RequestTraj>4) 	// if is it time to get arrival table from traj. aware componet and solve the problem. It is supposed to get the arrival table every 2 seconds
}
*/






/*

void LinkList2DatFileForAdaptivePriority(LinkedList<ReqEntry> Req_List,char *filename,double InitTime[2],int InitPhase[2],double GrnElapse[2],double transitWeigth, double truckWeigth,double coordinationWeigth,double dVehDistToStpBar[130],double dVehSpd[130],int iVehPhase[130],double Ratio[130],double indic[130],int laneNumber[130],double dQsizeOfEachLane[48], double Tq[130])
{	
    //-- Convert request linkedlist to the NewModel.dat file for GLPK solver.
    //-- Init1 and Init2 are the initial time for the two rings while current phases are in R or Y.
    //-- PassedGrn1 and PassedGrn2 are the elapsed time when current phase is in Green

    ofstream fs;
    fs.open(filename,ios::out);

    int R1No=ConfigIS.Ring1No;
    int R2No=ConfigIS.Ring2No;
    int icurrent=0;
    
    fs<<"data;\n";
    if (HaveCoordInList==1) // in order to know in which 
	{
		Req_List.Reset();
		int iAlreadySetCurrent=0;
		if(Req_List.ListEmpty()==0)
		{
			while(!Req_List.EndOfList())
			{
				if (Req_List.Data().VehClass==6 && iAlreadySetCurrent==0) // if it is coordination request, we should know 
				{
					if (Req_List.Data().MinGreen>0)
					{
						fs<<"param current:="<<max(ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1],ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[1]-1])-Req_List.Data().MinGreen <<";\n";
						icurrent=max(ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1],ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[1]-1])-Req_List.Data().MinGreen;
					}
					else if (Req_List.Data().ETA>0)
					{
						fs<<"param current:="<<ConfigIS.dCoordCycle-Req_List.Data().ETA <<";\n";	
						icurrent=ConfigIS.dCoordCycle-Req_List.Data().ETA ;
					}
					iAlreadySetCurrent=1;			
				}
				Req_List.Next();
			}
		}
	}else
	fs<<"param current:=0;\n";

	// MZP commented on 5/17/17
	//if ((icurrent<ConfigIS.dCoordinationSplit[0]-1	)||(icurrent<ConfigIS.dCoordinationSplit[1]-1	))
	//	iCoordinatedPhaseRemainingMax=0;
	//if ( (icurrent>=ConfigIS.dCoordinationSplit[0]-1)&&(icurrent<=ConfigIS.dCoordinationSplit[0]+1)&&((InitPhase[0]!=ConfigIS.iCoordinatedPhase[0])&&(InitPhase[1]==ConfigIS.iCoordinatedPhase[1])))
	//	iCoordinatedPhaseRemainingMax=ConfigIS.dCoordinationSplit[0]+(ConfigIS.iCoordinationCycle-icurrent);
	//	cout<<"iCoordinatedPhaseRemainingMax"<<iCoordinatedPhaseRemainingMax<<"ConfigIS.dCoordinationSplit[0]"<<ConfigIS.dCoordinationSplit[0]<<"icurrent"<<icurrent<<endl;

	
    fs<<"param SP1:="<<InitPhase[0]<<";\n";  // This is the real phase [1-4]
    fs<<"param SP2:="<<InitPhase[1]<<";\n";  // This is the real phase [5-8]

    for(int i=0;i<2;i++)
    {
        if (InitTime[i]<0)
        {
            InitTime[i]=0;
        }
    }

    fs<<"param init1:="<<InitTime[0]<<";\n";
    fs<<"param init2:="<<InitTime[1]<<";\n";
    fs<<"param Grn1 :="<<GrnElapse[0]<<";\n";
    fs<<"param Grn2 :="<<GrnElapse[1]<<";\n";

    //=================Add the information for Yellow, Red======//
    fs<<"param y       \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=ConfigIS.Phase_Seq_R1[i];
        fs<<"\t"<<(k+1)<<"  "<<ConfigIS.Yellow[k];
    }
    for(int i=0;i<R2No;i++)
    {
        int k=ConfigIS.Phase_Seq_R2[i];
        fs<<"\t"<<(k+5)<<"  "<<ConfigIS.Yellow[k+4];
    }
    fs<<";\n";

    fs<<"param red       \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=ConfigIS.Phase_Seq_R1[i];
        fs<<"\t"<<(k+1)<<"  "<<ConfigIS.Red[k];
    }
    for(int i=0;i<R2No;i++)
    {
        int k=ConfigIS.Phase_Seq_R2[i];
        fs<<"\t"<<(k+5)<<"  "<<ConfigIS.Red[k+4];
    }
    fs<<";\n";
	// if the coordination priority is on, the minimum greent time for cordianted ohase should be split time.
	
	 
	//if (ConfigIS.dCoordinationWeight==0)
	//{
		fs<<"param gmin      \t:=";
		for(int i=0;i<R1No;i++)
		{
			int k=ConfigIS.Phase_Seq_R1[i];
			fs<<"\t"<<(k+1)<<"  "<<ConfigIS.Gmin[k];
		}
		for(int i=0;i<R2No;i++)
		{
			int k=ConfigIS.Phase_Seq_R2[i];
			fs<<"\t"<<(k+5)<<"  "<<ConfigIS.Gmin[k+4];
		}
		fs<<";\n";
//	}
//	else
//	{
//		fs<<"param gmin      \t:=";
//		for(int i=0;i<R1No;i++)
//		{
//			int k=ConfigIS.Phase_Seq_R1[i];
//			if (k+1==ConfigIS.iCoordinatedPhase[0]) 
//				fs<<"\t"<<(k+1)<<"  "<<ConfigIS.dCoordinationSplit[0];
//			else if (k+1==ConfigIS.iCoordinatedPhase[1]) 
//				fs<<"\t"<<(k+1)<<"  "<<ConfigIS.dCoordinationSplit[1];
//			else
//				fs<<"\t"<<(k+1)<<"  "<<ConfigIS.Gmin[k];
//		}
//		for(int i=0;i<R2No;i++)
//		{
//			int k=ConfigIS.Phase_Seq_R2[i];
//			if (k+5==ConfigIS.iCoordinatedPhase[0]) 
//				fs<<"\t"<<(k+5)<<"  "<<ConfigIS.dCoordinationSplit[0];
//			else if (k+5==ConfigIS.iCoordinatedPhase[1]) 
//				fs<<"\t"<<(k+5)<<"  "<<ConfigIS.dCoordinationSplit[1];
//			else
//				fs<<"\t"<<(k+5)<<"  "<<ConfigIS.Gmin[k+4];
//		}
//		cout<< "ConfigIS.dCoordinationSplit[1]"<<endl;
//		fs<<";\n";
//	}

//    fs<<"param gmax      \t:=";
//    for(int i=0;i<R1No;i++)
//   {
//        int k=ConfigIS.Phase_Seq_R1[i];

//        fs<<"\t"<<(k+1)<<"  "<<ConfigIS.Gmax[k];
//    }
//    for(int i=0;i<R2No;i++)
//    {
//        int k=ConfigIS.Phase_Seq_R2[i];

//      fs<<"\t"<<(k+5)<<"  "<<ConfigIS.Gmax[k+4];
//    }
//    fs<<";\n\n";
   
  

	// adding green extention portion to the max green time for the requested phases
	Req_List.Reset();
	int iGmax[2][4]={}; // max green time of each phase in ring 1
	for(int i=0;i<R1No;i++)
    {
        int k=ConfigIS.Phase_Seq_R1[i];
        // if the model is not for coordiantion, we extend the max green for requsted phase. If the mode is for coodination, 
        // we only extend the max green for requested non-coordinated phases. The max green time for coordinated phases when the model
        // is on coordination is already set as cycle -sum{min(non coocrinated)}
		if( (RequestPhaseInList(Req_List,(k+1))>0 ) && 
		  ( (ConfigIS.dCoordinationWeight == 0) || ( ( ConfigIS.dCoordinationWeight > 0 ) && 
		  ( k+1 != ConfigIS.iCoordinatedPhase[0] ) && ( ConfigIS.iCoordinatedPhase[0] > 0 ) ) ) )
        {
			dGlobalGmax[i]=(ConfigIS.Gmax[k]*(1+MaxGrnExtPortion));
			dGlobalGmax[i+4]=dGlobalGmax[i];
			iGmax[0][i]=(int) dGlobalGmax[i];
        }
        else
        {
			dGlobalGmax[i]=ConfigIS.Gmax[k];
			iGmax[0][i]=(int)dGlobalGmax[i];
        }
    }
    for(int i=0;i<R2No;i++)
    {
        int k=ConfigIS.Phase_Seq_R2[i];
		if( ( RequestPhaseInList(Req_List,(k+5))>0 ) && 
		  ( (ConfigIS.dCoordinationWeight == 0) || ( ( ConfigIS.dCoordinationWeight > 0 ) && 
		  ( k+5 != ConfigIS.iCoordinatedPhase[1] ) && ( ConfigIS.iCoordinatedPhase[1] >0 ) ) ) )
        
        {
			dGlobalGmax[i+4]=(ConfigIS.Gmax[k+4]*(1+MaxGrnExtPortion));
			dGlobalGmax[i]=dGlobalGmax[i+4];
			iGmax[1][i]=(int) dGlobalGmax[i+4];
        }
        else
        {
			dGlobalGmax[i+4]=max(ConfigIS.Gmax[k+4],dGlobalGmax[i+4]);
			iGmax[1][i]=(int) dGlobalGmax[i+4];
        }
    }


	// in coordination case, the maximum green time for the two coordinated phase is set to the cycle-sumation of minimum green time of othee phases and the clearance time.
	Req_List.Reset();
	if (ConfigIS.dCoordinationWeight>0)
	{
		// MZP added on 4/14/17
		// Calculate the maximum possible green time for coordinated phase ( cycle - sum { min (non coordnated phase + clrearance time) }
		double dMaxCoordPhase1GrnTime=0.0;
		double dMaxCoordPhase2GrnTime=0.0;
		if ( ConfigIS.iCoordinatedPhase[0] >0)
		{
			for(int i=0;i<R1No;i++)
			{
				int k=ConfigIS.Phase_Seq_R1[i];
				if ( k+1 != ConfigIS.iCoordinatedPhase[0] )
				{
					dMaxCoordPhase1GrnTime = dMaxCoordPhase1GrnTime + ConfigIS.Gmin[k] + ConfigIS.Red[k] + ConfigIS.Yellow[k];
					dMaxCoordPhase1GrnTime = ConfigIS.dCoordCycle-dMaxCoordPhase1GrnTime;
					dGlobalGmax[k] = ConfigIS.dCoordinationPhaseSplit[k]-(ConfigIS.Red[k] + ConfigIS.Yellow[k]);
					dGlobalGmax[ConfigIS.iCoordinatedPhase[0]-1] = dMaxCoordPhase1GrnTime;
					iGmax[0][i] = (int) dGlobalGmax[ConfigIS.iCoordinatedPhase[0]-1];
				}
			}
		}
		
		
		if ( ConfigIS.iCoordinatedPhase[1] >0)
		{
			for(int i=0;i<R2No;i++)
			{
				int k=ConfigIS.Phase_Seq_R2[i];
				if ( k+5 != ConfigIS.iCoordinatedPhase[1] )
				{
					dMaxCoordPhase2GrnTime=dMaxCoordPhase2GrnTime + ConfigIS.Gmin[k+4] + ConfigIS.Red[k+4] + ConfigIS.Yellow[k+4];
					dMaxCoordPhase2GrnTime = ConfigIS.dCoordCycle-dMaxCoordPhase2GrnTime;
					dGlobalGmax[k+4] = ConfigIS.dCoordinationPhaseSplit[k+4]-(ConfigIS.Red[k+4] + ConfigIS.Yellow[k+4]);
					dGlobalGmax[ConfigIS.iCoordinatedPhase[1]-1]=dMaxCoordPhase2GrnTime;
					iGmax[1][i]=(int) dGlobalGmax[ConfigIS.iCoordinatedPhase[1]-1];
				}
			}
		
		}
		
	// MZP commented on 4/14/17
	
	//	int temp_indx1;
	//	double temp_val1;
	//	for(int i=0;i<R1No;i++)
	//	{
	//		int k=ConfigIS.Phase_Seq_R1[i];
	//		if (k+1 == ConfigIS.iCoordinatedPhase[0])
	//		{	
	//			temp_val1=dGlobalGmax[i]+iCoordinatedPhaseRemainingMax;
	//			temp_indx1=i;
	//		}
	//	}
	//	for(int i=0;i<R2No;i++)
	//	{
	//		int k=ConfigIS.Phase_Seq_R2[i];
	//		if (k+5 == ConfigIS.iCoordinatedPhase[1])
	//		{
	//			dGlobalGmax[i+4]= max(dGlobalGmax[i+4]+iCoordinatedPhaseRemainingMax,temp_val1);
	//			dGlobalGmax[temp_indx1]=max(dGlobalGmax[i+4],temp_val1);
	//			iGmax[1][i]=(int) dGlobalGmax[i+4];
	//			iGmax[0][temp_indx1]= (int) dGlobalGmax[i+4];
	//		}
	//	}
		
	}

	Req_List.Reset();
    fs<<"param gmax      \t:=";
    for(int i=0;i<R1No;i++)
    {
        int k=ConfigIS.Phase_Seq_R1[i];
       // if ( ( (InitPhase[0]==k+1) && (HaveCoordInList!=1) ) || ((InitPhase[0]==k+1) && (HaveCoordInList==1) && (InitPhase[0]!=ConfigIS.iCoordinatedPhase[0]) )) // if the current phase is not the coordinated phase. or if the coordiantion is not on
	//		dGlobalGmax[i]=dGlobalGmax[i]-GrnElapse[0];
	//	else if ((InitPhase[0]==k+1) && (HaveCoordInList==1) && (InitPhase[0]==ConfigIS.iCoordinatedPhase[0]) && (icurrent< ConfigIS.iCoordinationSplit)) // if we are in the coordinated split time interval, we should not reduce the length of max green time 
	//		dGlobalGmax[i]=dGlobalGmax[i];
		if (dGlobalGmax[i] <= ConfigIS.Gmin[k])	
			dGlobalGmax[i] = ConfigIS.Gmin[k];
		 fs<<"\t"<<(k+1)<<"  "<<dGlobalGmax[i];
    }
    for(int i=0;i<R2No;i++)
    {
        int k=ConfigIS.Phase_Seq_R2[i];
      //  if ( ( (InitPhase[1]==k+5) && (HaveCoordInList!=1) ) || ((InitPhase[1]==k+5) && (HaveCoordInList==1) && (InitPhase[1]!=ConfigIS.iCoordinatedPhase[1]) )) // if the current phase is not the coordinated phase. or if the coordiantion is not on
		//	dGlobalGmax[i+4]=dGlobalGmax[i+4]-GrnElapse[1];
		//else if ((InitPhase[1]==k+5) && (HaveCoordInList==1) && (InitPhase[1]==ConfigIS.iCoordinatedPhase[1]) && (icurrent< ConfigIS.iCoordinationSplit)) // if we are in the coordinated split time interval, we should not reduce the length of max green time 
		//	dGlobalGmax[i+4]=dGlobalGmax[i+4];
		if (dGlobalGmax[i+4] <= ConfigIS.Gmin[k+4])	
			dGlobalGmax[i+4] = ConfigIS.Gmin[k+4];
        fs<<"\t"<<(k+5)<<"  "<< dGlobalGmax[i+4];
	}

    fs<<";\n\n";


	Req_List.Reset();
	int NumberofRequests=0;
	int iNumberofTransitInList=1;
	int iNumberofTruckInList=1;
	char tempID[64]=" ";
	fs<<"param priorityType:= ";
	if(Req_List.ListEmpty()==0)
    {
		while(!Req_List.EndOfList())
		{
			//cout<<"Req_List.Data().VehID" << Req_List.Data().VehID<< endl;
			if (Req_List.Data().VehClass!=6) // only consider non coordination request in this weighting list. THe coordination request weight is considered in the objective function 
			{
				NumberofRequests++;
				if (Req_List.Data().VehClass==2)
					iNumberofTransitInList++;
				if (Req_List.Data().VehClass==3)
					iNumberofTruckInList++;
				//cout<< "NumberofRequests"<<NumberofRequests<<endl;
				fs<<NumberofRequests;
				fs<<" ";
				fs<<Req_List.Data().VehClass;			
				fs<<" ";
				//cout<<"fs<<Req_List.Data().VehClass;			"<<Req_List.Data().VehClass<<endl;
				strcpy(tempID,Req_List.Data().VehID);
			}
			Req_List.Next();
		}
		while (NumberofRequests<10)
		{
			NumberofRequests++;
			fs<<NumberofRequests;
			fs<<" ";
			fs<<0;			
			fs<<" ";
		}
		fs<<" ;  \n";
	}else
	{
	    fs<<" 1 0 2 0 3 5 4 0 5 0 6 0 7 0 8 0 9 0 10 0 ; \n";	
	}
	
	
	if (iNumberofTransitInList>1)
		iNumberofTransitInList=iNumberofTransitInList-1;
	if (iNumberofTruckInList>1)
		iNumberofTruckInList=iNumberofTruckInList-1;
		
		
	fs<<"param PrioWeigth:=  1 1 2 ";
	fs<< transitWeigth/iNumberofTransitInList;
	fs<< " 3 " ;
	fs<< truckWeigth/iNumberofTruckInList;
	fs<< " 4 0 5 0 ";
	fs<< " 6 " ;
	fs<< coordinationWeigth;
	fs<< " 7 0 8 0 9 0 10 0 ; \n" ;
	
	
    //================End of the information for Yellow, Red======//

    //---------------According to priorityconfiguration file, some priority eligible vehicle may have weigth equal to zero. we should remove them from the list
	LinkedList<ReqEntry> Req_List_New;
	if(Req_List.ListEmpty()==0)
    {
        removeZeroWeightReq(Req_List, transitWeigth, truckWeigth,Req_List_New);
    }

	int ReqSeq=1;
    double dTempCl=0.0;
    double dTempCu=0.0;
    int	iTempPhase=0;
    
    fs<<"param Rl (tr): 1 2 3 4 5 6 7 8:=\n";
    Req_List_New.Reset();
    			
    
    while(!Req_List_New.EndOfList())
    {
		if (Req_List_New.Data().VehClass!=6) // if it is a coordination request, we will check it in at it in the Cu and Cl 
        {
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{			
				if(Req_List_New.Data().Phase==j)
				{
					if (Req_List_New.Data().MinGreen>0) // in this case the vhicle is in the queue and we should set the Rl as less as possible!!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
					{
						int iRingOfTheRequest=FindRingNo(j-1); 
						fs<< 1 << "  " ;
						if ( iGmax[iRingOfTheRequest][(j-1)%4]<=0 )
						{
							if (j%2==0) 
								iGmax[iRingOfTheRequest][(j-1)%4]=iGmax[iRingOfTheRequest][(j-1)%4-1];
							else
								iGmax[iRingOfTheRequest][(j-1)%4]=iGmax[iRingOfTheRequest][(j-1)%4+1];
						}
					}
					else
						fs<<max(Req_List_New.Data().ETA-ROBUSTTIME_LOW,float(1.0))<<"  "; 
				}
				else
					fs<<".  ";
			}
			fs<<"\n";
			ReqSeq=ReqSeq+1;
		}
		Req_List_New.Next();
    }
    fs<<";\n";
    fs<<"param Ru (tr): 1 2 3 4 5 6 7 8:=\n";
    Req_List_New.Reset();
    ReqSeq=1;
    while(!Req_List_New.EndOfList())
    {
		if (Req_List_New.Data().VehClass!=6) // if it is a coordination request, we will check it in at it in the Cu and Cl 
        {
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if(Req_List_New.Data().Phase==j)
				{
					if (Req_List_New.Data().MinGreen>0) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
					{
						int iRingOfTheRequestt=FindRingNo(j-1);
						double dmaxGreenForThisPhase=0.0;
						if (iRingOfTheRequestt==1)
						{
							for(int i=0;i<R2No;i++)
							{
								int kkk=ConfigIS.Phase_Seq_R2[i];
								if (kkk+5==j)
								dmaxGreenForThisPhase=dGlobalGmax[i+4];	
							}
						}
						if (iRingOfTheRequestt==0)
						{
							for(int i=0;i<R1No;i++)
							{
								int kkk=ConfigIS.Phase_Seq_R1[i];
								if (kkk+1==j)
									dmaxGreenForThisPhase=dGlobalGmax[i];	
							}
						}

						if (Req_List_New.Data().MinGreen+ROBUSTTIME_UP >=dmaxGreenForThisPhase )
							fs<<dmaxGreenForThisPhase  <<"  ";
						else
								fs<<Req_List_New.Data().MinGreen+ROBUSTTIME_UP  <<"  ";		
					}
					else
						fs<<Req_List_New.Data().ETA+ROBUSTTIME_UP <<"  ";
				}
				else
					fs<<".  ";
			}
				fs<<"\n";
			ReqSeq=ReqSeq+1;
		}
		Req_List_New.Next();
    }
     fs<<";\n";
    if (HaveCoordInList==1) // in order to know in which 
	{
		fs<<"param Cl1 (tr): 1 2 3 4 5 6 7 8:=\n";
		Req_List_New.Reset();
		ReqSeq=1;
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass==6)
			{
				fs<<ReqSeq<<"  ";
				for(int j=1;j<=8;j++)
				{
					if((Req_List_New.Data().Phase==j) && (j<5))
					{
						if (Req_List_New.Data().MinGreen>0) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
						{
							fs<< 1 << "  " ;
							dTempCl=ConfigIS.dCoordCycle-ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1]+Req_List_New.Data().MinGreen;
						}
						else 
						{
							fs<< Req_List_New.Data().ETA << "  " ;	
							dTempCl=ConfigIS.dCoordCycle+Req_List_New.Data().ETA ;
						}				
						iTempPhase=j;
					}
					else
						fs<<".  ";
				}
				fs<<"\n";
				ReqSeq=ReqSeq+1;
			}
			Req_List_New.Next();
		}
		
		// setting the second coordination request in the second cycle of ring 1
		if (ReqSeq>1) // if there is one coordination request for a phase in ring 1
		{
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if (j==iTempPhase)
					fs<< dTempCl << "  " ;	
				else
					fs<<".  ";
			}
			fs<<"\n";
		}
		fs<<";\n";

     
		fs<<"param Cu1 (tr): 1 2 3 4 5 6 7 8:=\n";
		Req_List_New.Reset();
		ReqSeq=1;
		iTempPhase=0;
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass==6)
			{
				fs<<ReqSeq<<"  ";
				for(int j=1;j<=8;j++)
				{
					if( (Req_List_New.Data().Phase==j) && (j<5) )
					{
						if (Req_List_New.Data().MinGreen>0 ) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
						{
							fs<<Req_List_New.Data().MinGreen  <<"  ";
							dTempCu= ConfigIS.dCoordCycle+Req_List_New.Data().MinGreen ; // Cu for the second cycle
						}	
						else 
						{
							fs<< Req_List_New.Data().ETA + ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1]<< "  " ;
							dTempCu= ConfigIS.dCoordCycle + Req_List_New.Data().ETA + ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1];
						}
						iTempPhase=j;
					}
					else
						fs<<".  ";
				}
				fs<<"\n";
				ReqSeq=ReqSeq+1;
			}
			Req_List_New.Next();
		}
		
		// setting the second coordination request in the second cycle of ring1
		if (ReqSeq>1) // if there is one coordination request for a phase in ring 1
		{
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if (j==iTempPhase)
					fs<< dTempCu << "  " ;	
				else
					fs<<".  ";
			}
			fs<<"\n";
		}
		
		
		fs<<";\n";
		
		fs<<"param Cl2 (tr): 1 2 3 4 5 6 7 8:=\n";
		Req_List_New.Reset();
		ReqSeq=1;
		iTempPhase=0;
		dTempCu=0.0;
		dTempCl=0.0;
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass==6)
			{
				fs<<ReqSeq<<"  ";
				for(int j=1;j<=8;j++)
				{
					if((Req_List_New.Data().Phase==j) && (j>4))
					{
						if (Req_List_New.Data().MinGreen>0) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
						{
							fs<< 1 << "  " ;
							dTempCl=ConfigIS.dCoordCycle-ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[1]-1]+Req_List_New.Data().MinGreen;
						}
						else 
						{
							fs<< Req_List_New.Data().ETA << "  " ;	
							dTempCl=ConfigIS.dCoordCycle+Req_List_New.Data().ETA;
						}				
						iTempPhase=j;		
					}
					else
						fs<<".  ";
				}
				fs<<"\n";
				ReqSeq=ReqSeq+1;
			}
			Req_List_New.Next();
		}
		
		// setting the second coordination request in the second cycle of ring2
		if (ReqSeq>1) // if there is one coordination request for a phase in ring 2
		{
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if (j==iTempPhase)
					fs<< dTempCl << "  " ;	
				else
					fs<<".  ";
			}
			fs<<"\n";
		}
		fs<<";\n";

     
		fs<<"param Cu2 (tr): 1 2 3 4 5 6 7 8:=\n";
		Req_List_New.Reset();
		ReqSeq=1;
		while(!Req_List_New.EndOfList())
		{
			if (Req_List_New.Data().VehClass==6)
			{
				fs<<ReqSeq<<"  ";
				for(int j=1;j<=8;j++)
				{
					if((Req_List_New.Data().Phase==j)&& (j>4))
					{
						if (Req_List_New.Data().MinGreen>0 ) // in this case the vhicle is in the queue and we should set the Ru at most less than maximum remaining green time!!  // MZ Added to hedge against the worst case that may happen when the vehicle is in the queue
						{
							fs<<Req_List_New.Data().MinGreen  <<"  ";	
							dTempCu= ConfigIS.dCoordCycle+Req_List_New.Data().MinGreen ; // Cu for the second cycle
						}
						else 
						{
							fs<< Req_List_New.Data().ETA + ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[1]-1]<< "  " ;	
							dTempCu= ConfigIS.dCoordCycle + Req_List_New.Data().ETA + ConfigIS.dCoordinationPhaseSplit[ConfigIS.iCoordinatedPhase[0]-1];
						}
					iTempPhase=j;
					}
					else
						fs<<".  ";
				}
				fs<<"\n";
				ReqSeq=ReqSeq+1;
			}
			Req_List_New.Next();
		}
		// setting the second coordination request in the second cycle of ring2
		if (ReqSeq>1) // if there is one coordination request for a phase in ring 2
		{
			fs<<ReqSeq<<"  ";
			for(int j=1;j<=8;j++)
			{
				if (j==iTempPhase)
					fs<< dTempCu << "  " ;	
				else
					fs<<".  ";
			}
			fs<<"\n";
		}
		fs<<";\n";
	}
	if ((codeUsage==2) && (currentTotalVeh>0)) // if we consider adaptive control and the number of vehicles in the trajectory is greater than zero
	{
		//if (currentTotalVeh>31) // !!!! increase solution time!!!!
			//currentTotalVeh=31;
		float t=0.0;
		fs<<"param s:=" << dQDischargingSpd << "; \n";
		fs<<"param qSp:=" << dQueuingSpd << "; \n";
		fs<<"param Ar:=";
		for (int i=0;i<currentTotalVeh;i++)
		{
			t=( int(dVehDistToStpBar[i]*10) )/10;
			fs<< i+1 <<" " << t<<" ";
		}
		fs<<"; \n";
		fs<<"param Ve:=";
		for (int i=0;i<currentTotalVeh;i++)
		{
			t=( int(dVehSpd[i]*10) )/10;
			fs<< i+1 <<" " <<t<<" ";
		}
		fs<<"; \n";
		fs<<"param Ln:=";
		for (int i=0;i<currentTotalVeh;i++)
		{
			fs<< i+1 <<" " <<laneNumber[i]<<" ";
		}
		fs<<"; \n";
		fs<<"param Ratio:=";
		for (int i=0;i<currentTotalVeh;i++)
		{
			t=( int(Ratio[i]*10) )/10;
			fs<< i+1 <<" " <<t<<" ";
		}
		fs<<"; \n";
		
		fs<<"param Ph:=";
		for (int i=0;i<currentTotalVeh;i++)
		{
			fs<< i+1 <<" " <<iVehPhase[i]<<" ";
		}
		fs<<"; \n";
		
		fs<<"param Tq:=";
		for (int i=0;i<currentTotalVeh;i++)
		{
			t=(int (Tq[i]*10) )/10;
			fs<< i+1 <<" " <<t<<" ";
		}
		fs<<"; \n";
		
		fs<<"param LaPhSt:=";
		for (int i=0;i<TotalLanes;i++)
		{
			fs<< i+1 <<" " <<bLaneSignalState[i]<<" ";
		}
		fs<<"; \n";
		
		fs<<"param L0:=";
		for (int i=0;i<TotalLanes;i++)
		{
			t=( int(dQsizeOfEachLane[i]*10) )/10;
			fs<< i+1 <<" " <<t<<" ";
		}
		fs<<"; \n";
	}
	
    fs<<"end;";
    fs.close();
}

*/






