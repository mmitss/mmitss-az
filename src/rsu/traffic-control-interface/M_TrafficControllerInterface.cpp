/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   



/* MMITSS_MRP_TrafficControllerInterface.cpp
*  Created by :Yiheng Feng
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.

*  Revision History:
*  1. Two errors were found by Byungho Beak and fixed. " if (ceil(currenttime-begintime)>=Eventlist_R1.Data().time && ceil(currenttime-begintime)<=Eventlist_R1.Data().time+2)   "   
*	and also at   " if  (ceil(currenttime-begintime)<Eventlist_R1.Data().time)   and 
*  2. Mehdi Zamanipour added the logic to get VISSIM time in case the application is running in simulation. 

*/

  

//MMITSS MRP Traffic Controller Interface
//This component is reponsible to receive signal timing schedule from 
//MRP_TrafficControl and MRP_PriorityRequestServer and send control 
//commands (NTCIP: FORCE_OFF, VEH_CALL, PHASE_OMIT, PHASE_HOLD) to
//Econolite ASC3/Cobalt controller

//Created by: Yiheng Feng 10/28/2014

//Department of Systems and Industrial Engineering
//University of Arizona
           
        

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
#include <string.h>
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
#include "Mib.h"
#include "LinkedList.h"
#include "ListHandle.h"
#include "Signal.h"
#include "Array.h"
#include "Config.h"   // <vector>
#include "Print2Log.h"
#include "GetInfo.h"
#include "Schedule.h"
          
using namespace std;


//#define OBU_ADDR "192.168.1.25"
#define BROADCAST_ADDR "192.168.101.255"   //DSRC
#define SIMULATION_N0N_COORDINATED_INTERSECTION 3
#define SIMULATION_COORDINATED_INTERSECTION 2
#define FIELD 1

#ifndef DEG2ASNunits
    #define DEG2ASNunits  (1000000.0)  // used for ASN 1/10 MICRO deg to unit converts
#endif


//define log file name
char predir [64] 		  = "/nojournal/bin/";
char logfilename[256] 	  = "/nojournal/bin/log/MMITSS_MRP_TrafficControllerInterface_";
char ConfigInfo[256]	  = "/nojournal/bin/ConfigInfo.txt";
char rsuid_filename[64]   = "/nojournal/bin/rsuid.txt";
char IPInfo[64]           ="/nojournal/bin/IPInfo.txt";  //stores the virtual asc3 Controller IP address



char temp_log[512];
PhaseStatus phase_read={0};   // value would be {1,3,4}
int CurPhaseStatus[8]={0};    // when do phaseReading: the same results as phase_read withoug conversion
char ConfigFile[256];
char INTip[64];
char INTport[8];   //this port is for the virtual controller
int CombinedPhase[8]={0};
Phase Phases;  //----Important one----//
string RSUID;
int hold_timer;  //timer to hold the phase;
int currenttime;
LinkedList <Schedule> Eventlist_R1;
LinkedList <Schedule> Eventlist_R2;
int PhaseDisabled[8];
int occupancy[8];
int Ped_Info[2];
int CurrPhase[2];
int Phase_Num;
int phase_seq[8];
RSU_Config ConfigIS;
int iRecvOptTimingPort=44444;
int iClockPort=12345;  // to get VISSIM time in case the code is running for simulation testing and intersection is on coordination
char buf[8192];   //Received Signal Control Schedule should be within 8k.
int icodeUsage=1;   // by default, the application will be used for field testing  (icodeUsage=1 -> FIELD)     (icodeUsage=2 -> SIMULATION_COORDINATED_INTERSECTION)  icodeUsage=3 -> SIMULATION_N0N_COORDINATED_INTERSECTION

// socket to get optimal plan from algorithm (ISIG or Priority)
int sockfd;
struct sockaddr_in sendaddr;
int addr_length;
int recv_data_len;
//Parameters for setting up socket for VISSIM time
int sockfd_clock;
struct sockaddr_in sendaddr_clock;
int numbytes_clock, addr_len_clock;
char buf_clock[1024];
int addr_length_clock;

int   outputlog(char *output);
void  UnpackEventData(char* ablob);  //This function unpack the signal event data and save to event lists;
char  Signal_Configure_COP_File[256]= "/nojournal/bin/Signal_Config_COP.txt";
void  ReadSignalParameters(char *ConfigFile);
int   Checkconflict(int CurPhase[2]);
int   ModifyCurPhase(int CurrPhase[2], int phase_seq[8]);
void  initializLogging();
int   handleArguments(int argc, char* argv[]);
void  testConnectionToController();
float Simtime();                     //Get simulation time from VISSIM
int   msleep_sim(int duration);        //sleep simulation of time of duration
void  setupSocketConnection();
void  setupConfiguration();

int main ( int argc, char* argv[] )
{
	int New_Schedule_flag=0;   //flag to find out whether a new signal timing schedule is received
	double currenttime;
	double begintime;   //the current event list beginning time
	double hold_timer_R1=0.0;  //Hold timer, control the No. hold command send to ASC controller
	double hold_timer_R2=0.0;
	double omit_timer_R1=0.0;  //Omit timer, control the No. hold command send to ASC controller
	double omit_timer_R2=0.0;
	int flag_r1=0;
	int flag_r2=0;
	handleArguments(argc, argv);
	initializLogging();
	setupConfiguration();
	testConnectionToController();
	setupSocketConnection();
	if (icodeUsage==SIMULATION_COORDINATED_INTERSECTION)
		begintime=Simtime();
	else
		begintime=GetSeconds();
	while (true)
	{	
		//cout<<"Waiting for Signal Timing Schedule..."<<endl;
		recv_data_len = recvfrom(sockfd, buf, sizeof(buf), 0,
							(struct sockaddr *)&sendaddr, (socklen_t *)&addr_length);
		if (icodeUsage==SIMULATION_COORDINATED_INTERSECTION)
			currenttime=Simtime();
		else
			currenttime=GetSeconds();
	
												   
		if(recv_data_len<0)
		{
			memset(&buf[0], 0, sizeof(buf)); //clear the buf if nothing received
			New_Schedule_flag=0; //no new schedule is received
			//continue;
		}
		else
		{
			New_Schedule_flag=1; //new schedule is received
			//Re-create the event list
			Eventlist_R1.ClearList();
			Eventlist_R2.ClearList();
			UnpackEventData(buf);
			sprintf(temp_log,"Received New Schedule at time %.2lf\n",currenttime);
			outputlog(temp_log); cout<<temp_log;
			//print the new schedule
			Eventlist_R1.Reset();
			Eventlist_R2.Reset();
			while(!Eventlist_R1.EndOfList())
			{
				sprintf(temp_log,"Time: %d Phase: %d Action: %d\n",Eventlist_R1.Data().time,Eventlist_R1.Data().phase,Eventlist_R1.Data().action);
				outputlog(temp_log); cout<<temp_log;
				Eventlist_R1.Next();
			}
			while(!Eventlist_R2.EndOfList())
			{
				sprintf(temp_log,"Time: %d Phase: %d Action: %d\n",Eventlist_R2.Data().time,Eventlist_R2.Data().phase,Eventlist_R2.Data().action);
				outputlog(temp_log); cout<<temp_log;
				Eventlist_R2.Next();
			}
		}             
		
		if(New_Schedule_flag==1)  //Reset the current time
		{
			if (icodeUsage==SIMULATION_COORDINATED_INTERSECTION)
				begintime=currenttime;
			else
				begintime=GetSeconds();
			hold_timer_R1=0.0;  //Reset Hold timer
			hold_timer_R2=0.0;
		}
		 
		Eventlist_R1.Reset();
		Eventlist_R2.Reset();

		int forceoff_cmd=0;
		int vehcall_cmd=0;
		int hold_cmd=0;
		int omit_cmd=0;
		int pedcall_cmd=0;
		int pedclear_cmd=0;
		while(!Eventlist_R1.EndOfList())
		{
			//!!! B.H : test
			//sprintf(temp_log,"R1 Test 1 currenttime : %.2lf begintime : %.2lf Eventlist_R1.Data().time :%d \n",currenttime, begintime, Eventlist_R1.Data().time);
			//outputlog(temp_log); cout<<temp_log;
			
			//!!! B.H : Error fixed : if (currenttime-begintime<Eventlist_R1.Data().time)
			if (ceil(currenttime-begintime)<Eventlist_R1.Data().time)   //used as hold command or omit
			{
				//!!! B.H : test
				//sprintf(temp_log,"Test 1-1 ceil(currenttime-begintime) : %.2lf Eventlist_R1.Data().time :%d \n",ceil(currenttime-begintime), Eventlist_R1.Data().time);
				//outputlog(temp_log); cout<<temp_log;
				
				if (Eventlist_R1.Data().action==3 && currenttime-hold_timer_R1>1.0)  //hold the current phase every one second
				{
					hold_cmd+=(int) pow(2.0,Eventlist_R1.Data().phase-1);
					sprintf(temp_log,"HOLD Phase %d at time %.2lf\n",Eventlist_R1.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
					hold_timer_R1=currenttime;
				}
				if (Eventlist_R1.Data().action==1)  //omit the current phase every one second
				{
					omit_cmd+=(int) pow(2.0,Eventlist_R1.Data().phase-1);
					sprintf(temp_log,"OMIT Phase %d at time %.2lf\n",Eventlist_R1.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
					omit_timer_R1=currenttime;
				}
			}
			//!!! B.H : Error fixed : if (currenttime-begintime>=Eventlist_R1.Data().time && currenttime-begintime<=Eventlist_R1.Data().time+2) 
			if (ceil(currenttime-begintime)>=Eventlist_R1.Data().time && ceil(currenttime-begintime)<=Eventlist_R1.Data().time+2)   //trigger event in the the time list
			{
				//!!! B.H : test
				//sprintf(temp_log,"Test 1-2 ceil(currenttime-begintime) : %0.2lf Eventlist_R1.Data().time :%d \n",ceil(currenttime-begintime), Eventlist_R1.Data().time);
				//outputlog(temp_log); cout<<temp_log;
				
				if(Eventlist_R1.Data().action==0)  // force off
				{
					forceoff_cmd+=(int) pow(2.0,Eventlist_R1.Data().phase-1);
					sprintf(temp_log,"FORCE_OFF Phase %d at time %.2lf\n",Eventlist_R1.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
				}
				if(Eventlist_R1.Data().action==2)  // vehicle call
				{

                    vehcall_cmd = vehcall_cmd | (int)pow(2.0, Eventlist_R1.Data().phase - 1);   

					sprintf(temp_log,"VEH_CALL Phase %d- vehcall_cmd (%d) at time %.2lf\n",Eventlist_R1.Data().phase, vehcall_cmd, currenttime);
					outputlog(temp_log); cout<<temp_log;
				}
				if(Eventlist_R1.Data().action==4)  // ped call
				{
					pedcall_cmd+=(int) pow(2.0,Eventlist_R1.Data().phase-1);
					sprintf(temp_log,"PED_CALL Phase %d at time %.2lf\n",Eventlist_R1.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
				}
				if(Eventlist_R1.Data().action==5)  // ped clear
				{
					pedclear_cmd+=(int) pow(2.0,Eventlist_R1.Data().phase-1);
					sprintf(temp_log,"PED_CLEAR Phase %d at time %.2lf\n",Eventlist_R1.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
				}
			}
			Eventlist_R1.Next();
		}
		while(!Eventlist_R2.EndOfList())
		{
			//!!! B.H : test
			//sprintf(temp_log,"R1 Test 2 currenttime : %.2lf begintime : %.2lf Eventlist_R2.Data().time :%d \n",currenttime, begintime, Eventlist_R2.Data().time);
			//outputlog(temp_log); cout<<temp_log;
			//!!! B.H : Error fixed : if (currenttime-begintime<Eventlist_R2.Data().time)
			if (ceil(currenttime-begintime)<Eventlist_R2.Data().time)   //used as hold command
			{
				//!!! B.H : test
				//sprintf(temp_log,"Test 2-1 ceil(currenttime-begintime) : %.2lf Eventlist_R2.Data().time :%d \n",ceil(currenttime-begintime), Eventlist_R2.Data().time);
				//outputlog(temp_log); cout<<temp_log;
				
				if (Eventlist_R2.Data().action==3 && currenttime-hold_timer_R2>1.0)  //hold the current phase every one second
				{
					hold_cmd+=(int) pow(2.0,Eventlist_R2.Data().phase-1);
					sprintf(temp_log,"HOLD Phase %d at time %.2lf\n",Eventlist_R2.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
					hold_timer_R2=currenttime;
				}
				if (Eventlist_R2.Data().action==1)  //omit the current phase every one second
				{
					omit_cmd+=(int) pow(2.0,Eventlist_R2.Data().phase-1);
					sprintf(temp_log,"OMIT Phase %d at time %.2lf\n",Eventlist_R2.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
					omit_timer_R2=currenttime;
				}
			}				
			//!!! B.H : Error fixed : if (currenttime-begintime>=Eventlist_R2.Data().time && currenttime-begintime<=Eventlist_R2.Data().time+2)
			if (ceil(currenttime-begintime)>=Eventlist_R2.Data().time && ceil(currenttime-begintime)<=Eventlist_R2.Data().time+2)   //trigger event in the the time list
			{
				//!!! B.H : test
				//sprintf(temp_log,"Test 2-2 ceil(currenttime-begintime) : %0.2lf Eventlist_R2.Data().time :%d \n",ceil(currenttime-begintime), Eventlist_R2.Data().time);
				//outputlog(temp_log); cout<<temp_log;
				
				if(Eventlist_R2.Data().action==0)  // force off
				{
					forceoff_cmd+=(int) pow(2.0,Eventlist_R2.Data().phase-1);
					sprintf(temp_log,"FORCE_OFF Phase %d at time %2lf\n",Eventlist_R2.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
				}
				if(Eventlist_R2.Data().action==2)  // vehicle call
				{
                    vehcall_cmd = vehcall_cmd | (int)pow(2.0, Eventlist_R2.Data().phase - 1);

					sprintf(temp_log,"VEH_CALL Phase %d - vehcall_cmd (%d) at time %2lf\n",Eventlist_R2.Data().phase, vehcall_cmd, currenttime);
					outputlog(temp_log); cout<<temp_log;
				}
				if(Eventlist_R2.Data().action==4)  // ped call
				{
					pedcall_cmd+=(int) pow(2.0,Eventlist_R2.Data().phase-1);
					sprintf(temp_log,"PED_CALL Phase %d at time %2lf\n",Eventlist_R2.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
				}
				if(Eventlist_R2.Data().action==5)  // ped clear
				{
					pedclear_cmd+=(int) pow(2.0,Eventlist_R2.Data().phase-1);
					sprintf(temp_log,"PED_CLEAR Phase %d at time %.2lf\n",Eventlist_R2.Data().phase,currenttime);
					outputlog(temp_log); cout<<temp_log;
				}
			}
			Eventlist_R2.Next();
		}

		if(vehcall_cmd!=0)
			IntersectionPhaseControl(PHASE_VEH_CALL,vehcall_cmd,'Y');

		if(forceoff_cmd!=0)
			IntersectionPhaseControl(PHASE_FORCEOFF,forceoff_cmd,'Y');		
		
		if(hold_cmd!=0)
			IntersectionPhaseControl(PHASE_HOLD,hold_cmd,'Y');	
			
		if(pedcall_cmd!=0)
			IntersectionPhaseControl(PHASE_PED_CALL,pedcall_cmd,'Y');
			
		if(pedclear_cmd!=0)
			IntersectionPhaseControl(PHASE_PED_CALL,0,'Y');		
			
		
		
		if(Eventlist_R1.ListSize()>0)
		{
			Eventlist_R1.Reset(Eventlist_R1.ListSize()-1);
			
			if(currenttime-begintime<=Eventlist_R1.Data().time+2)  //already reach the end of the barrier, back to COP
			{
				flag_r1=1;
			}
		}
		
		if(Eventlist_R2.ListSize()>0)
		{
			Eventlist_R2.Reset(Eventlist_R2.ListSize()-1);
			if (currenttime-begintime<=Eventlist_R2.Data().time+2)
			{
				flag_r2=1;
			}		
		}
		
		if(flag_r1==1 || flag_r2==1)
		{
			if(pedcall_cmd==0 && pedclear_cmd==0)
				IntersectionPhaseControl(PHASE_OMIT,omit_cmd,'Y');	
			flag_r1=0;
			flag_r2=0;
		}
		//!!! B.H : msleep(500) delayed the process of solution from Solver  			
		msleep(200);
      	
	}// End of while(true)
	
	return 0;

} // ---End of main()

//**********************************************************************************//

int outputlog(char *output)
{
	FILE * stream = fopen( logfilename, "r" );

	if (stream==NULL)
	{
		perror ("Error opening file");
	}

	fseek( stream, 0L, SEEK_END );
	long endPos = ftell( stream );
	fclose( stream );

	fstream fs;
	if (endPos <10000000)
		fs.open(logfilename, ios::out | ios::app);
	else
		fs.open(logfilename, ios::out | ios::trunc);

	//fstream fs;
	//fs.open("/nojournal/bin/OBU_logfile.txt", ios::out | ios::app);
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

	return 1;
}

void initializLogging()
{
	char timestamp[128];
    xTimeStamp(timestamp);
    strcat(logfilename,timestamp);    
    strcat(logfilename,".log");
	printf("%s\n",logfilename);
	fstream fs_log;
	fs_log.open(logfilename,fstream::out | fstream::trunc);
	fs_log.close();
}

int handleArguments(int argc, char* argv[])
{
	 int ch;
     int iExtention=0;
     while ((ch = getopt(argc, argv, "c:p:")) != -1) 
     {
		switch (ch) 
		{
		    case 'c':
				icodeUsage=atoi (optarg);
				if (icodeUsage==SIMULATION_COORDINATED_INTERSECTION)
					printf ("Code usage is for testing in simulation for a coordinated signal -> VISSIM should have been setup to broadcast time \n");
				else if (icodeUsage==SIMULATION_N0N_COORDINATED_INTERSECTION)
					printf ("Code usage is for  simulation with no coordinated intersection \n");
				else
					printf ("Code usage is for field testing \n");
				break;
			case 'p':
				iClockPort=atoi (optarg);
				printf ("The VISSIM clock is received from port %d \n", iClockPort);
				break;
            default:
			     return 0;
		 }
    }
}
void setupSocketConnection()
{
	//Struct for UDP socket timeout: 0.001s
	struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
	//------------init: Begin of Network connection------------------------------------
	
	if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd");
		exit(1);
	}	
	//Setup time out
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Error");
	}  
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = htons(iRecvOptTimingPort);  //*** IMPORTANT: the trajectory pushing code should also have this port. ***//
	
	if (icodeUsage==SIMULATION_COORDINATED_INTERSECTION)
		sendaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//inet_addr(OBU_ADDR);//INADDR_ANY; 
	else
		sendaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr(OBU_ADDR);//INADDR_ANY;


	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);
	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");        exit(1);
	}
	addr_length = sizeof ( sendaddr );
	
	
	//-----------------------End of Network Connection------------------//
	if (icodeUsage==SIMULATION_COORDINATED_INTERSECTION)
	{
		//Network Connection for receiving the system clcok from VISSIM
		if((sockfd_clock = socket(AF_INET,SOCK_DGRAM,0)) == -1)
		{
			perror("sockfd");
			exit(1);
		}						 
		int UdpBufSize = 1024;
		setsockopt(sockfd_clock, SOL_SOCKET, SO_RCVBUF, &UdpBufSize, sizeof(int));	
		int optval = 1;
		setsockopt(sockfd_clock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));	 
		sendaddr_clock.sin_family = AF_INET;
		sendaddr_clock.sin_port = htons(iClockPort);  //*** IMPORTANT: the vissim,signal control and performance observer should also have this port. ***//
		//sendaddr_clock.sin_addr.s_addr = inet_addr("10.254.56.255");//inet_addr(LOCAL_HOST_ADDR);//inet_addr(OBU_ADDR);//INADDR_ANY;
		sendaddr_clock.sin_addr.s_addr =INADDR_ANY;
		memset(sendaddr_clock.sin_zero,'\0',sizeof sendaddr_clock.sin_zero);
		if(bind(sockfd_clock, (struct sockaddr*) &sendaddr_clock, sizeof sendaddr_clock) == -1)
		{
			perror("bind");        
			exit(1);
		}
		addr_length_clock = sizeof ( sendaddr_clock );
	}
}

void setupConfiguration()
{
	get_ip_address();           // READ the ASC controller IP address into INTip from "ntcipIP.txt"
	get_configfile();
	//-----------------Read in the ConfigIS Every time in case of changing plan-----------------------//
    int curPlan=CurTimingPlanRead();
    sprintf(temp_log,"Current timing plan is:\t%d\n",curPlan);    //cout<<temp_log;
    outputlog(temp_log);
    //PhaseTimingStatusRead();  // Find the disabled phases.
    IntersectionConfigRead(curPlan,ConfigFile);  // Generate: configinfo_XX.txt
    IntersectionPedConfigRead(curPlan,ConfigFile);   //Read configured ped walking time and clearance time
    PrintPlan2Log(ConfigFile);
    ReadInConfig(ConfigFile); // Find the missing phases of the signal table, save to ConfigIS
    PrintRSUConfig();
	//Read Signal Parameters for traffic control to phase_seq, MinGreen, MaxGreen, Yellow and Red
    ReadSignalParameters(Signal_Configure_COP_File);
    sprintf(temp_log,"Phase Sequence: %d %d %d %d %d %d %d %d\n",phase_seq[0],phase_seq[1],phase_seq[2],phase_seq[3],phase_seq[4],phase_seq[5],phase_seq[6],phase_seq[7]);
}	

void testConnectionToController()
{	
	for(int i=4;i>0;i--) //warm up...
    {
        PhaseTimingStatusRead();
        Phases.UpdatePhase(phase_read);
        Phases.Display();
        //Phases.RecordPhase(signal_plan_file);
        for (int j=0;j<2;j++)
		{
			CurrPhase[j]=Phases.CurPhase[j]+1;
			cout<<"Current phase is: "<<CurrPhase[0]<<" "<<CurrPhase[1]<<endl;
		}
		msleep(1000);
    }
}

void UnpackEventData(char* ablob)
{
	int offset=0;
	unsigned short   tempUShort; // temp values to hold data in final format
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
	//message id
	temp = (char)ablob[offset];
	offset = offset + 1; // move past to next item
	
	//Do No. Event in R1
	int No_R1;
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	No_R1 = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;
	
	//Do each event in R1
	for(int i=0;i<No_R1;i++)
	{
		Schedule TempSche;
		//do time
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempSche.time = (tempLong /  DEG2ASNunits); // convert and store as float
		offset = offset + 4;
		//do phase
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempSche.phase = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		//do action
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempSche.action = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		Eventlist_R1.InsertRear(TempSche);   //add the new event to the list	
	}
	
	//Do No. Event in R2
	int No_R2;
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	No_R2 = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;
	
	//Do each event in R1
	for(int i=0;i<No_R2;i++)
	{
		Schedule TempSche;
		//do time
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempSche.time = (tempLong /  DEG2ASNunits); // convert and store as float
		offset = offset + 4;
		//do phase
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempSche.phase = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		//do action
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempSche.action = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		Eventlist_R2.InsertRear(TempSche);   //add the new event to the list	
	}	
}

void ReadSignalParameters(char *ConfigFile)
{
	fstream fs;
	fs.open(ConfigFile);
	string temp_string;
	char temp[128];
	//read number of phases
	getline(fs,temp_string);
	strcpy(temp,temp_string.c_str());
	sscanf(temp,"%*s %d",&Phase_Num);
	//read phase sequence
	getline(fs,temp_string);
	strcpy(temp,temp_string.c_str());
	sscanf(temp,"%*s %d %d %d %d %d %d %d %d",&phase_seq[0],&phase_seq[1],&phase_seq[2],&phase_seq[3],&phase_seq[4],&phase_seq[5],&phase_seq[6],&phase_seq[7]);
	fs.close();
}
int Checkconflict(int CurPhase[2])
{
	int conflict=0;
	if(CurPhase[0]==1 ||CurPhase[0]==2)  //P11
	{
		if (CurPhase[1]!=5 && CurPhase[1]!=6)
			conflict=1;
	}
	if(CurPhase[0]==3 ||CurPhase[0]==4)  //P11
	{
		if (CurPhase[1]!=7 && CurPhase[1]!=8)
			conflict=1;
	}
	if(CurPhase[0]==5 ||CurPhase[0]==6)  //P11
	{
		if (CurPhase[1]!=1 && CurPhase[1]!=2)
			conflict=1;
	}
	if(CurPhase[0]==7 ||CurPhase[0]==8)  //P11
	{
		if (CurPhase[1]!=3 && CurPhase[1]!=4)
			conflict=1;
	}
	
	return conflict;
}

int ModifyCurPhase(int CurrPhase[2], int phase_seq[8])
{
	if(phase_seq[CurrPhase[0]+4-1]==0)  //missing phase is in ring 2
		CurrPhase[1]=CurrPhase[0]+4;    //change phase in ring 2
	if(phase_seq[CurrPhase[1]-4-1]==0)  //missing phase is in ring 1
		CurrPhase[0]=CurrPhase[1]-4;    //change phase in ring 1
}

float Simtime()
{
	//recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
    //                   (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);
    recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
                        (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);    
	unsigned char byteA, byteB, byteC, byteD;
    byteA = buf_clock[0];
    byteB = buf_clock[1];
    byteC = buf_clock[2];
    byteD = buf_clock[3];
    
	long  DSecond = (long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD)); // in fact unsigned
	
	float sim_time=DSecond/10.0;
	return sim_time;
}

int msleep_sim(int duration)
{
	float t1,t2;
	//recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
    //                    (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);
	recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
                        (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);    
	
	
	unsigned char byteA, byteB, byteC, byteD;
    byteA = buf_clock[0];
    byteB = buf_clock[1];
    byteC = buf_clock[2];
    byteD = buf_clock[3];
    
	long  DSecond = (long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD)); // in fact unsigned
	
	t1=DSecond/10.0;
	t2=t1;
	while(t2-t1<duration/1000)
	{
		recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
                        (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);
		recvfrom(sockfd_clock, buf_clock, sizeof(buf_clock), 0,
                        (struct sockaddr *)&sendaddr_clock, (socklen_t *)&addr_length_clock);    

		byteA = buf_clock[0];
		byteB = buf_clock[1];
		byteC = buf_clock[2];
		byteD = buf_clock[3];
    
		DSecond = (long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD)); // in fact unsigned
	
		t2=DSecond/10.0;
	}
	return 0;	
}
