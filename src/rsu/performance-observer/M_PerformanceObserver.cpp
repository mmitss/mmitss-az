/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  MMITSS_rsu_PerformanceObserver.cpp  
 *  Created by Shayan Khoshmagham on 7/11/14.
 *  University of Arizona
 *  ATLAS Research Center
 *  College of Engineering
 *
 *  This code was developed under the supervision of Professor Larry Head
 *  in the ATLAS Research Center.
 *
 *  Revision History:
 *  
 */



// Work with Vissim 6 DriverModel through UDP
// Need drivermodel_udp_R.dll from "DriverModel_DLL_UDP_InFusion" running


//#include <libeloop.h>

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
#include "NMAP.h"
#include "geoCoord.h"
#include "BasicVehicle.h"
#include "ListHandle.h"
#include "ConnectedVehicle.h"

//#include <libgps.h> djc

#include "Signal.h"
#include "Array.h"
//#include "Config.h"   // <vector>
//#include "Print2Log.h"
#include "GetInfo.h"
//#include "COP.h"

#include "Performance.h"

#include <algorithm>
#include <numeric>  
using namespace std;

#ifndef BSM_MSG_SIZE
#define BSM_MSG_SIZE  (45)
#endif

#ifndef DEG2ASNunits
#define DEG2ASNunits  (100000.0)  // used for ASN 1/10 MICRO deg to unit converts
#endif

//socket settings
#define PRPORT 15020

#define ACTIVE 1
#define NOT_ACTIVE -1
#define LEAVE 0

#define PI 3.14159265

char buf_traj[20000];  //The Trajectory message from MRP_EquippedVehicleTrajecotryAware

char temp_log[512];
int continue_while=0;

int lane_mapping[8]; //The corresponding lane used for movement identification
int movement_mapping[12]; //The corresponding movements for TT&D purposes!

//define log file name
char predir [64] = "/nojournal/bin/";
char logfilename[256] = "/nojournal/bin/log/MMITSS_rsu_PerformanceObserver_TT_";
char ConfigInfo[256]	  = "/nojournal/bin/ConfigInfo.txt";
char Performance_data_file[256] = "/nojournal/bin/Performance_webserver.txt";
char Lane_Movement_Mapping_File_Name[64]  = "/nojournal/bin/Lane_Movement_Mapping.txt";
char Queue_data_file[256] = "/nojournal/bin/log/MMITSS_rsu_PerformanceObserver_QLE_";
char Performance_Acc_file[256] ="/nojournal/bin/log/MMITSS_rsu_PerformanceObserver_TT_Acc_";
char smoothness_file[256] ="/nojournal/bin/log/MMITSS_rsu_PerformanceObserver_Smoothness_";
char arrival_file[256] = "/nojournal/bin/log/MMITSS_rsu_PerformanceObserver_Arrival_";

char IPInfo[64]= "/nojournal/bin/ntcipIP.txt";
char Det_Numbers_file[64] = "/nojournal/bin/Det_Numbers.txt";
    
int tot_num; //Total number of detectors
int *Det_Number; //Indicate the detector numbers
float Detector_stopbar[20]; //Array of the distances from system detectors to the associated stopbar
int Lane_phase[20]; //Array of the phases associated with each lane
float Turning_proportion[20][3]; //Array of the Turning Movement Proportions
void get_Det_Numbers();
 
int  outputlog(char *output);
//void get_ip_address();           // READ the virtual ASC controller IP address into INTip from "IPInfo.txt"
  
geoCoord geoPoint;
double ecef_x,ecef_y,ecef_z;					
//Important!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Note the rectangular coordinate system: local_x is N(+) S(-) side, and local_y is E(+) W(-) side 
//This is relative distance to the ref point!!!!
//local z can be considered as 0 because no elevation change.
double local_x,local_y,local_z;

//This is the output of the FindVehInMap
double Dis_curr=0;  //current distance to the stop bar
double ETA=0;       //estimated travel time to stop bar
int requested_phase=-1;  //requested phase
 
//for NTCIP signal control

//char IPInfo[64]="/nojournal/bin/ntcipIP.txt";  //stores the virtual asc3 Controller IP address
char INTip[64];
char *INTport = "501";   //this port is for the virtual controller
int CombinedPhase[8]={0};
int out[20];
  
string RSUID;
char rsuid_filename[64]   = "/nojournal/bin/rsuid.txt";
char ConfigFile[256] = "/nojournal/bin/ConfigInfo_Daisy_Mout_Traj.txt";  //this is just the temporary file for trajectory based test

LinkedList <ConnectedVehicle> trackedveh;
LinkedList <ConnectedVehicle> received_list; //This is the list from BSM_reciever for updating the trajectory 

int perf_data_size_1s;
int perf_data_size_15s;
int perf_data_size_5m;

Performance TempPerf;


class Unequipped_veh{
	public:
	double time_join;
};
  
LinkedList <Unequipped_veh> Unequipped_veh_list[20]; //A list of unequipped vehicle, useful for detectors data

void UnpackTrajData(byte* ablob);  //This function unpacks the trajectory message and save to received_list;

void obtainEndofService( int ihour,int imin,int isec, int iETA,int &iHour,int &iMin,int &iSec);
void get_lane_movement_mapping();

void Push_1s_Frequency(byte* perf_traj_data,int &size);

PhaseStatus phase_read={0}; //values would be 1:Red	3:Yellow 4:Green
Phase Phases;
int CurPhaseStatus[8]={0};
int occupancy[8];
RSU_Config ConfigIS;

char BROADCAST_ADDR_DB[64]="10.254.56.6"; //This is the ip address of the database server machine

vector <double> speed_EB;


int main ( int argc, char* argv[] )
{
	int port = 30000; //This is the default port in case no port was inputed as an argument
	//For simulation: port = 30000			For Field: port = 3333
	
	float PR[8][5]; //Market Penetration Rate for each lane
	float MPR; //Market Penetration Rate to be given by the user in command line
	//Initialize the Penetration Rate to be 50% for now
	for(int x=0; x<8 ;x++)
	{
		for(int y=0; y<5; y++)
			PR[x][y] = 0.5;
	} 

	//Self defined value for sending frequency; to get the desired port from the user
	if (argc==4)
	{   
		sscanf(argv[1],"%d", &port);
		sscanf(argv[2],"%f", &MPR);
		for(int xx=0; xx<8 ;xx++)
		{for(int yy=0; yy<5; yy++)
				PR[xx][yy] = MPR;
		}
		sscanf(argv[3],"%s", &BROADCAST_ADDR_DB); 
	}
	else if (argc==2)
		sscanf(argv[1],"%d", &port);


	int i,j,k;
	get_ip_address(); 
	get_configfile();
	get_Det_Numbers();
	get_lane_movement_mapping();
	ReadInConfig(ConfigFile); // Find the missing phases of the signal table, save to ConfigIS
	for(i=4;i>0;i--) //warm up...
    {
        PhaseTimingStatusRead();
        Phases.UpdatePhase(phase_read);
        msleep(1000);
    }

	//------------init: Begin of Network connection------------------------------------
	int sockfd;

	struct sockaddr_in sendaddr;
	struct sockaddr_in recvaddr;
	int numbytes, addr_len;
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
	sendaddr.sin_port = htons(22222);  //*** IMPORTANT: the trajectory pushing code should also have this port. ***//
	sendaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr(OBU_ADDR);//INADDR_ANY;

	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);

	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");        exit(1);
	}

	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(port);
	recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1") ; 
	memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);

	int addr_length = sizeof ( recvaddr );
	int recv_data_len;
	//-----------------------End of Network Connection------------------//
	
	
	//------------init: Begin of Network connection for sending to Data Archiver------------------------------------
	int sockfd_db;

	struct sockaddr_in recvaddr_db;

	if((sockfd_db = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd_db");
		exit(1);
	}

	recvaddr_db.sin_family = AF_INET;
	recvaddr_db.sin_port = htons(3344); //The same port in Data Archiver program that listens to!
	recvaddr_db.sin_addr.s_addr = inet_addr("10.254.56.6") ; //IP of the database server!! could be set in the cmnd line
	memset(recvaddr_db.sin_zero,'\0',sizeof recvaddr_db.sin_zero);

	int addr_length_db = sizeof ( recvaddr_db );
	
	//-----------------------End of Network Connection for sending to Data Archiver------------------//
	
	int frequency=1; // Shows how often we send request for the trajectory data 
	int calc_frequency = 15; //Shows how often we calculate the performance measures
	int agg_frequency = 300; //Shows how often we aggregate data
	double send_timer=GetSeconds();
	double calc_timer=GetSeconds();
	double agg_timer=GetSeconds();
	
	int previous_signal_color[8];
	
	double red_start_time[8];
	double green_start_time[8];
	
	//initialize red and green start times
    for(i=0;i<8;i++)
    {
        previous_signal_color[i]=phase_read.phaseColor[i];
        if(phase_read.phaseColor[i]==1)
            red_start_time[i]=GetSeconds();
        else
            red_start_time[i]=0;
        
        if(phase_read.phaseColor[i]==4)
            green_start_time[i]=GetSeconds();
        else
            green_start_time[i]=0;    
    }  
 
	int two_minute_counter = 0;
	
	char timestamp[128];
    xTimeStamp(timestamp);
    
    //write to a file only for _TT_Acc
    strcat(Performance_Acc_file,timestamp);    
    strcat(Performance_Acc_file,".log");
	printf("%s\n",Performance_Acc_file);
	
	//write to afile only for _QLE
	strcat(Queue_data_file,timestamp);    
    strcat(Queue_data_file,".log");
	printf("%s\n",Queue_data_file);
  
	//write to a file only for _TT
    strcat(logfilename,timestamp);    
    strcat(logfilename,".log");
	printf("%s\n",logfilename);
	
	//write to a file only for Smoothness
    strcat(smoothness_file,timestamp);    
    strcat(smoothness_file,".log");
	printf("%s\n",smoothness_file);
	
    //write to a file only for Arrivals on Green/Red
    strcat(arrival_file,timestamp);    
    strcat(arrival_file,".log");
	printf("%s\n",arrival_file);
	
	fstream fs_log;
	fs_log.open(logfilename,fstream::out | fstream::trunc);
	
	int arrival_on_green[8];
	int arrival_on_red[8];
	
	fstream queue_fs;
	queue_fs.open(Queue_data_file,ios::out | ios::app);
	sprintf(temp_log,"The format of 1st line: Queue Length in clockwise order for Approaches 1,3,5,7 considering 5 lanes for each\n");
	queue_fs<<temp_log;
	sprintf(temp_log,"The format of 2nd line: Number of vehicles in the Queue for Approaches 1,3,5,7 considering 5 lanes for each\n");
	queue_fs<<temp_log;
	queue_fs.close();
	
	fstream smoothness_fs;
	smoothness_fs.open(smoothness_file,ios::out | ios::app);
	sprintf(temp_log,"The format of this log file: Vehicle ID, TimeStamp, Speed\n");
	smoothness_fs<<temp_log;
	smoothness_fs.close();
	
	fstream arrival_fs;
	arrival_fs.open(arrival_file,ios::out | ios::app);
	sprintf(temp_log,"The format of this log file: Vehicle ID, TimeStamp, Arrivals \n");
	arrival_fs<<temp_log;
	arrival_fs.close();

	float sum_dsrc[12]={0};
	
	while (true)
	{
		
		double currtime=GetSeconds();
		if (currtime-send_timer>frequency)  //every 1 second
		{
			send_timer=currtime;
			char buf[64]="Request Trajectory from performance observer";

			cout<<"Sent Request!"<<endl;

			//send requested
			int numbytes = sendto(sockfd,buf,sizeof(buf) , 0,(struct sockaddr *)&recvaddr, addr_length);

			recv_data_len = recvfrom(sockfd, buf_traj, sizeof(buf_traj), 0,
					(struct sockaddr *)&sendaddr, (socklen_t *)&addr_length);
					
			if(recv_data_len<0)
			{
				printf("Receive Request failed\n");
				continue;
			}

			sprintf(temp_log,"Received Trajectory Data!\n");
			cout<<temp_log; //outputlog(temp_log);
			
			received_list.ClearList();
			//trackedveh.ClearList(); //clear the list for new set of data    
			UnpackTrajData(buf_traj);
			//write to a file only for queue length
			fstream queue_fs;
			queue_fs.open(Queue_data_file,ios::out | ios::app);	
			
			//Distance-wise
			double maximum[8][5] = {{0}}; //To identify the maximum Queue Length
			double maximum2[8][5] = {{0}}; //To identify the second maximum Queue Length
			
			//Time-wise
			long max[8][5] = {{0}}; //To identify the last stopped vehicle
			long max2[8][5] = {{0}}; //To identify the second last stopped vehicle
			
			//Delta Objects for Distance and Time
			double delta_d[8][5];
			long delta_t[8][5];
			
			double I[8][5]; //Parameter to take into consideration of vehicle arrivals			
			double q_speed[8][5]={{0}}; //Queuing Speed based on MPR
			double EQL[8][5];

			//Getting Red Elapse Time from the controller  
			double red_elapse_time[8];

			PhaseTimingStatusRead();
			Phases.UpdatePhase(phase_read);
			DetRead();

			int Det_Vol_Out[tot_num]; // The complete output of the detectors volume data
			int jj=0;
			for(int i=0; i<tot_num; i++)
			{
				if(Detector_stopbar[i]>0)
					{
						Det_Vol_Out[i] = out[jj];
						jj++;
					}
				else
					Det_Vol_Out[i] = 0;
			}

			//Getting Red Elapse Time for Each Phase: if signal gets green, keep the previous red duration--------------REMEMBER TO GENERALIZE------------------
			for(int i=0; i<8; i++)
			{
				if(previous_signal_color[i] != 3 && phase_read.phaseColor[i] == 3) //Signal just turned Green
				{
					
					green_start_time[i] = GetSeconds();
					if(i==1)
					{
						arrival_on_green[1]=0;
						arrival_on_red[1]=0;
					}
				}
				
				if(previous_signal_color[i] != 1 && phase_read.phaseColor[i] == 1) //Signal just turned red
				{
					red_start_time[i] = GetSeconds();
				}
				if(phase_read.phaseColor[i] == 1) //Meaning that signal was already red
				{
					red_elapse_time[i] = GetSeconds() - red_start_time[i];	
				}
				previous_signal_color[i] = phase_read.phaseColor[i];	
			}			
			time_t theTime = time(NULL);
			struct tm *aTime = localtime(&theTime);
			int iHour=0;
			int iMin=0;
			int iSec=0;
			obtainEndofService(aTime->tm_hour,aTime->tm_min,aTime->tm_sec,frequency, iHour,iMin,iSec);
			
			sprintf(temp_log,"Observation_Period_is: %d %d %d %d %d %d\n", aTime->tm_hour, aTime->tm_min, aTime->tm_sec, iHour,iMin, iSec);
			queue_fs<<temp_log; //perf_fs<<temp_log; 

			int connect_veh_flag[20]={0};   //Whether the vehicle that passing the detector is a connected vehcile or not 0: not 1:yes
			//initialize the update_flag before going throught the received list: 0:Delete; 1:Update; 2:Add
			for (trackedveh.Reset(); !trackedveh.EndOfList(); trackedveh.Next())
			{
				trackedveh.Data().update_flag=0;
			}
			//----------------------------------Begin of doing Update or Add vehicle to the trackedveh list and dealing with received_list-------------------------//
			for (received_list.Reset(); !received_list.EndOfList(); received_list.Next())
			{
				ConnectedVehicle TempVeh;
				TempVeh = received_list.Data();
				int found = 0; //To decide whether the vehicle was found in the list or not
				int veh_id = received_list.Data().TempID;

				for (trackedveh.Reset(); !trackedveh.EndOfList(); trackedveh.Next())
				{
					if(trackedveh.Data().TempID == veh_id)
					{
						int temp_delay = trackedveh.Data().delay;
						double temp_distance = trackedveh.Data().Distance;
						int temp_passed_flag = trackedveh.Data().passed_flag;
						int temp_pre_requested_phase=trackedveh.Data().pre_requested_phase;
						//double temp_dsrc = trackedveh.Data().dsrc_range;

						trackedveh.Data() = received_list.Data(); //Put whatever in the received list to the trackeveh list

						trackedveh.Data().delay = temp_delay;
						trackedveh.Data().Distance = temp_distance;
						trackedveh.Data().passed_flag=temp_passed_flag;
						trackedveh.Data().pre_requested_phase=temp_pre_requested_phase;

						if(currtime - trackedveh.Data().entry_time <= 2) // To understand if the vehicle was deleted and added again because of a U-Turn. So set the delay and Distance back to zero!
						{
							trackedveh.Data().Distance = 0;
							trackedveh.Data().delay = 0;
							temp_distance = 0;
							trackedveh.Data().passed_flag = 0;
							trackedveh.Data().pre_requested_phase = -1;
						}

						//Travel Time Calculation 
						trackedveh.Data().TT = trackedveh.Data().leaving_time - trackedveh.Data().entry_time;
						
						//Distance Traversed Calculation
						if(trackedveh.Data().nFrame >= 2) //In order to calculate the Distance Traversed at least two points are needed! 
						{
							if(GetSeconds() - trackedveh.Data().leaving_time < 2)
							{
								double x1 = trackedveh.Data().N_Offset[1];
								double x2 = trackedveh.Data().N_Offset[0];
								double y1 = trackedveh.Data().E_Offset[1];
								double y2 = trackedveh.Data().E_Offset[0];
								trackedveh.Data().Distance = temp_distance + sqrt (((x1-x2)*(x1-x2)) + ((y1-y2)*(y1-y2)));
							}
						}

						//Delay Calculation
						if(trackedveh.Data().Speed<0.8 && trackedveh.Data().stop_flag==0 && trackedveh.Data().approach%2 == 1)  //vehicle has not stopped before
						{
							trackedveh.Data().delay+=1;
						}

						if(trackedveh.Data().stop_flag==1 && trackedveh.Data().approach%2 == 1)
						{
							trackedveh.Data().delay+=1;
						}

						if(trackedveh.Data().Speed>0.8 && trackedveh.Data().stop_flag==1)  //to keep track of the last stopped vehicle 
						{
							trackedveh.Data().stop_flag=0;
						}

						trackedveh.Data().update_flag=1;  //updated
						found=1;

						break; //to jump out of the current for loop and not go through the end of the trackedveh list!! Because the vehicle was just found.
					}				
				}

				if (found==0) //It's a new vehicle which should be added to the trackedveh list
				{
					trackedveh.InsertRear(TempVeh);
					trackedveh.Data().update_flag=2;
					trackedveh.Data().delay = 0;
					trackedveh.Data().Distance =0; 
					trackedveh.Data().passed_flag=0;
					trackedveh.Data().pre_requested_phase=-1;
				}
			} 
		  
			for (trackedveh.Reset(); !trackedveh.EndOfList(); trackedveh.Next())
			{
				//-------------------------------Arrivals on Green-----------------------------------------
				if(trackedveh.Data().req_phase>=0)
				{
					trackedveh.Data().pre_requested_phase=trackedveh.Data().req_phase;
					//cout<<"Pre Requested Phase: "<<trackedveh.Data().pre_requested_phase<<endl;
				}
				if(trackedveh.Data().req_phase<0)
				{				
					if(trackedveh.Data().passed_flag==0)
						{
							trackedveh.Data().passed_flag=1;
							if(trackedveh.Data().time_stop<=0.1) //The vehicle didn't stop and arrived on Green
							{
								if (trackedveh.Data().pre_requested_phase==2)
								{
									arrival_on_green[1]++;
									//cout<<"The number of arrivals on Green phase 2: "<<arrival_on_green[1]<<endl;
									//To log the necessary data for Arrivals on Green
									fstream arrival_fs;
									arrival_fs.open(arrival_file,ios::out | ios::app);
									sprintf(temp_log, "Arrivals on Green %d; %.2lf; %d\n", trackedveh.Data().TempID, GetSeconds(), arrival_on_green[1]);
									arrival_fs<<temp_log;
									arrival_fs.close();
								}
							}
							else //The vehicle stopped and arrived on Red
							{
								if (trackedveh.Data().pre_requested_phase==2)
								{
									arrival_on_red[1]++;
									//cout<<"The number of arrivals on Red phase 2: "<<arrival_on_red[1]<<endl;
									//To log the necessary data for Arrivals on Red
									fstream arrival_fs;
									arrival_fs.open(arrival_file,ios::out | ios::app);
									sprintf(temp_log, "Arrivals on Red %d; %.2lf; %d\n", trackedveh.Data().TempID, GetSeconds(), arrival_on_red[1]);
									arrival_fs<<temp_log;
									arrival_fs.close();
								}
							}		
						}
					
				}
					//To log the necessary data for smoothness
					fstream smoothness_fs;
					smoothness_fs.open(smoothness_file,ios::out | ios::app);
					sprintf(temp_log, " %d; %.2lf; %.2f\n", trackedveh.Data().TempID, GetSeconds(), trackedveh.Data().Speed);
					smoothness_fs<<temp_log;
					smoothness_fs.close();
				
				
				//-----------------------To deal with QLE---------------------------
				for(int i=0; i<tot_num; i++) //going through approaches 1,3,5,7 and lanes 1,2,3,4,5
				{	
					if(Detector_stopbar[i]>0 && fabs(trackedveh.Data().stopBarDistance - Detector_stopbar[i])<20) //Build based on the matrices of Approach*lane & stopbar distance of the associated detectors.
					{
						connect_veh_flag[i]=1;				
					}	
				}
				//cout<<"Vehicle ID of "<<trackedveh.Data().TempID<<" with stop flag of: "<<trackedveh.Data().stop_flag<<endl;
				if(trackedveh.Data().stop_flag == 1) //if the vehicle is stopped
				{ 
					//to get the distance difference for the last two vehicles
					if(trackedveh.Data().stopBarDistance > maximum[trackedveh.Data().approach][trackedveh.Data().lane -1])
						{
						maximum2[trackedveh.Data().approach][trackedveh.Data().lane -1] = maximum[trackedveh.Data().approach][trackedveh.Data().lane -1]; //To keep track of the second last vehicle
						maximum[trackedveh.Data().approach][trackedveh.Data().lane -1] = trackedveh.Data().stopBarDistance;
						TempPerf.LQV[trackedveh.Data().approach][trackedveh.Data().lane -1] = trackedveh.Data().TempID; //This is the last queued vehicle
						}  
						
					//to get the time difference for the last two vehicles
					if(trackedveh.Data().time_stop > max[trackedveh.Data().approach][trackedveh.Data().lane -1])
						{
						max2[trackedveh.Data().approach][trackedveh.Data().lane -1] = max[trackedveh.Data().approach][trackedveh.Data().lane -1];
						max[trackedveh.Data().approach][trackedveh.Data().lane -1] = trackedveh.Data().time_stop;
						}  
					if(currtime-trackedveh.Data().time_stop<3.1) //----------------------Attention-----------------
					{
							TempPerf.maximum_estimation[trackedveh.Data().approach][trackedveh.Data().lane -1]=maximum[trackedveh.Data().approach][trackedveh.Data().lane -1]+3; //SHOULD BE CALIBRATED...!!!3 was added to consider the rear position of the vehicle instead of the center
							TempPerf.maximum_estimation2[trackedveh.Data().approach][trackedveh.Data().lane -1]=maximum2[trackedveh.Data().approach][trackedveh.Data().lane -1]+3; //For the second last vehicle
					}

				}
				//Tracking the last queued vehicle as it starts moving
				if(trackedveh.Data().TempID == TempPerf.LQV[trackedveh.Data().approach][trackedveh.Data().lane -1]) 
				{
					TempPerf.LQV_stop_flag[trackedveh.Data().approach][trackedveh.Data().lane -1]=trackedveh.Data().stop_flag;
				}	
				//-----------------------------------To remove from the list------------------------------
				if(trackedveh.Data().update_flag==0)
				{
					int movement=0;
					switch(trackedveh.Data().approach)
					{
						case 8:
							switch(trackedveh.Data().previous_approach)
							{
								//NBLT
								case 5:
									movement=movement_mapping[2];
									break;
									//SBRT
								case 1:
									movement=movement_mapping[7];
									break;
									//WBTh
								case 3:
									movement=movement_mapping[3];
									break;
							}
							break;
							//EBRT, SBTh, WBLT
						case 6:
							switch(trackedveh.Data().previous_approach)
							{
								//WBLT
								case 3:
									movement=movement_mapping[5];
									break;
									//SBTh
								case 1:
									movement=movement_mapping[6];
									break;
									//EBRT
								case 7:
									movement=movement_mapping[10];
									break;

							}
							break;
							//NBRT, EBTh, SBLT
						case 4:
							switch(trackedveh.Data().previous_approach)
							{
								//SBLT
								case 1:
									movement=movement_mapping[8];
									break;
									//NBRT
								case 5:
									movement=movement_mapping[1];
									break;
									//EBTh
								case 7:
									movement=movement_mapping[9];
									break;
							}
							break;
							//WBRT, NBTh, EBLT
						case 2:
							switch(trackedveh.Data().previous_approach)
							{
								//EBLT
								case 7:
									movement=movement_mapping[11];
									break;
									//WBRT
								case 3:
									movement=movement_mapping[4];
									break;
									//NBTh
								case 5:
									movement=movement_mapping[0];
									break;
							}
							break;
					}		
					//Write to a file for TT&D Accumulation Data
					if(movement != 0)
					{
						TempPerf.App_Ext_counter[movement-1]++;
						float temp_tardity = trackedveh.Data().TT/trackedveh.Data().Distance;
						TempPerf.vehicle_tardity[movement-1] += temp_tardity;
						
						TempPerf.total_distance[movement-1] +=  trackedveh.Data().Distance;
						TempPerf.App_Ext_Tardity[movement-1] = (TempPerf.vehicle_tardity[movement-1] * TempPerf.total_distance[movement-1])/((TempPerf.App_Ext_counter[movement-1])*(TempPerf.App_Ext_counter[movement-1]));
						
						TempPerf.num_observation[movement-1]++;
					}					
					
					fstream tt_fs;
					tt_fs.open(Performance_Acc_file,ios::out | ios::app);
					sprintf(temp_log,"%.2lf; %d; %d; %d; %.2f; %.2f\n",GetSeconds(), movement, trackedveh.Data().TempID, trackedveh.Data().TT,trackedveh.Data().delay, trackedveh.Data().Distance);
					tt_fs<<temp_log;
					
					if(currtime - agg_timer > agg_frequency) //Logs every 5 minutes
					{
						agg_timer = currtime;
						for(int i=0; i<12; i++)
						{
							sprintf(temp_log,"Extended Tardity with %d vehicles for movement %d is: %f at time %.2lf\n", TempPerf.num_observation[i], i+1, TempPerf.App_Ext_Tardity[i],GetSeconds());
							if(TempPerf.App_Ext_Tardity[i] >0)
							{tt_fs<<temp_log; cout<<temp_log;}
							TempPerf.num_observation[i]=0;
							TempPerf.App_Ext_Tardity[i]=0;	
						}
					}
					tt_fs.close();
					trackedveh.DeleteAt();
				}
				
			}
			//---------------------------------- End of Updating the trackedveh list and everything will be based on it from now on-----------------------------------------//
			
			//-----------------Queue Length Estimation Continuation---------------------------
			for(int i=0; i<tot_num; i++)
			{
				int temp_app = (i/5)*2 + 1; //mapping between the approach and i: i=[0 1 2 3 ... 19] ----> App=[1 1 1 1 1 3 3 3 3 3 5 5 5 5 5 7 7 7 7 7]
				int temp_lane = (i%5)+1; //mapping between the lane and i: i=[0 1 2 3 ... 19] ----> Lane=[1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5]
				int temp_phase = Lane_phase[i]; //Reading from the config file
				//cout<<"Approach: "<<temp_app<<" Lane: "<<temp_lane<<" Phase: "<<temp_phase<<endl;
	
				if(Det_Number[i]>0) //Case in which the lane has a detector
				{
					float r = ((double) rand()/(RAND_MAX)); //Generating a random number between 0 and 1 for comparing the turning movement probabilities

					if(connect_veh_flag[i] == 0)
					{
						//cout<<"Dealing with Unequipped Vehicles Started!"<<endl;
						//Yiheng and I QLE
						double *dis_det_q; //Distance from the System Detector to the back of the Queue
						dis_det_q = new double[tot_num];
						double *time_to_join; //Time to join the back of the queue for non-CVs
						time_to_join = new double[tot_num];
						Unequipped_veh temp_veh;

						if( Det_Vol_Out[i] > 0)
						{
							//cout<<"An unequipped vehicle passed the detector "<<Det_Number[i]<<endl;
							if(r < Turning_proportion[i][0]) //Vehicle Change one lane to the LEFT
							{
								dis_det_q[i+1] = Detector_stopbar[i+1] - TempPerf.maximum_estimation[temp_app][temp_lane]; //distance from detector to end of queue	
								time_to_join[i+1] = currtime + ((sqrt(144 + 0.77*4*dis_det_q[i+1])-12) / 1.55);									
								temp_veh.time_join=time_to_join[i+1];
								Unequipped_veh_list[i+1].InsertRear(temp_veh);
								//cout<<"Vehicle change its lane to the LEFT"<<endl;
							}
							if(r >= Turning_proportion[i][0] && r < Turning_proportion[i][0]+Turning_proportion[i][1]) //Vehicle does NOT change its lane and go THROUGH
							{
								dis_det_q[i] = Detector_stopbar[i] - TempPerf.maximum_estimation[temp_app][temp_lane-1]; //distance from detector to end of queue	
								time_to_join[i] = currtime + ((sqrt(144 + 0.77*4*dis_det_q[i])-12) / 1.55);									
								temp_veh.time_join=time_to_join[i];
								Unequipped_veh_list[i].InsertRear(temp_veh);
								//cout<<"Vehicle doesn't change its lane"<<endl;
								//cout<<"one Unequipped Vehicle will be added at "<<time_to_join<<" seconds"<<endl;	
							}
							if(r >= Turning_proportion[i][0]+Turning_proportion[i][1] && r < 1) //Vehicle Change one lane to the RIGHT
							{
								dis_det_q[i-1] = Detector_stopbar[i-1] - TempPerf.maximum_estimation[temp_app][temp_lane-2]; //distance from detector to end of queue	
								time_to_join[i-1] = currtime + ((sqrt(144 + 0.77*4*dis_det_q[i-1])-12) / 1.55);									
								temp_veh.time_join=time_to_join[i-1];
								Unequipped_veh_list[i-1].InsertRear(temp_veh);
								//cout<<"Vehicle change its lane to the RIGHT"<<endl;	
							}	
						}
							//cout<<"Distance from Detector to LQV is: "<< dis_det_q<<endl;
						
						for (Unequipped_veh_list[i].Reset(); !Unequipped_veh_list[i].EndOfList(); Unequipped_veh_list[i].Next())			
						{
							//cout<<"------------------------------Inside the for loop for unequipped lists---------------------------"<<endl;
							if(phase_read.phaseColor[temp_phase-1] == 1 && currtime >= Unequipped_veh_list[i].Data().time_join)
							{
								TempPerf.maximum_estimation[temp_app][temp_lane-1]+=6.75;  //add one vehicle
								//cout<<"add one vehicle to queue during Red, queue length is:"<<maximum_estimation[1][0]<<endl;
								Unequipped_veh_list[i].DeleteAt();
							}
							else if(phase_read.phaseColor[temp_phase-1] != 1 && currtime >= Unequipped_veh_list[i].Data().time_join && TempPerf.LQV_stop_flag[temp_app][temp_lane-1]==1)  //stopped
							{
								TempPerf.maximum_estimation[temp_app][temp_lane-1]+=6.75;  //add one vehicle
								//cout<<"add one vehicle to queue during Green, queue length is:"<<maximum_estimation[1][0]<<endl;
								Unequipped_veh_list[i].DeleteAt();
								//cout<<"Deleted vehicle during green stopped"<<endl;
							}
							else if(phase_read.phaseColor[temp_phase-1] != 1 && currtime >= Unequipped_veh_list[i].Data().time_join && TempPerf.LQV_stop_flag[temp_app][temp_lane-1]==0)  //not stopped
							{
								//cout<<"queue already moved, don't add vehicle"<<endl;
								Unequipped_veh_list[i].DeleteAt();
							}
							else if(phase_read.phaseColor[temp_phase-1] != 1 && currtime >= Unequipped_veh_list[i].Data().time_join )  // passing the green light
							{
								Unequipped_veh_list[i].DeleteAt();
							}
						}   
					}
				}
				
				else //Case in which there is no system detector
				{
					//cout<<"Inside Else Condition, w/o Detector"<<endl;
					if (TempPerf.maximum_estimation[temp_app][temp_lane-1] >= 0)
					{
						if(TempPerf.maximum_estimation2[temp_app][temp_lane-1] != 0)
						delta_d[temp_app][temp_lane-1] = TempPerf.maximum_estimation[temp_app][temp_lane-1] - TempPerf.maximum_estimation2[temp_app][temp_lane-1];
						else
						delta_d[temp_app][temp_lane-1] = TempPerf.maximum_estimation[temp_app][temp_lane-1];
					}
					
					if(max[temp_app][temp_lane-1] >= 0)
					{
					
						if(max2[temp_app][temp_lane-1] != 0)
						delta_t[temp_app][temp_lane-1] = max[temp_app][temp_lane-1] - max2[temp_app][temp_lane-1];
						else 
						//---------------------------------------------ATTENTION------------------------------------
						delta_t[temp_app][temp_lane-1] = max[temp_app][temp_lane-1] - (currtime - red_elapse_time[temp_phase-1]); // Last time stop - (beginning of Red of corresponding Phase for that Approach)
					}

					//Estimated Queue Length
					I[temp_app][temp_lane-1] = (1-PR[temp_app][temp_lane-1])*10;
					q_speed[temp_app][temp_lane-1] = I[temp_app][temp_lane-1]/14;

					if(delta_d[temp_app][temp_lane-1]/delta_t[temp_app][temp_lane-1] > q_speed[temp_app][temp_lane-1])
					{
						TempPerf.maximum_estimation[temp_app][temp_lane-1] = TempPerf.maximum_estimation[temp_app][temp_lane-1] + q_speed[temp_app][temp_lane-1];
					}
					else
					{
						TempPerf.maximum_estimation[temp_app][temp_lane-1] = TempPerf.maximum_estimation[temp_app][temp_lane-1] + delta_d[temp_app][temp_lane-1]/delta_t[temp_app][temp_lane-1];
					}	
				}
				  
				//LQV_discharging[temp_app][temp_lane-1] = 2 + green_start_time[temp_phase-1] + floor(maximum_estimation[temp_app][temp_lane-1]/6.75)*1.25; //SHOULD BE CALIBRATED...!!!!!
				TempPerf.LQV_discharging[temp_app][temp_lane-1] = 2 + green_start_time[temp_phase-1] + sqrt((TempPerf.maximum_estimation[temp_app][temp_lane-1]*2)/1.5); //SHOULD BE CALIBRATED...!!!!!
				TempPerf.queue_counter[temp_app][temp_lane-1] = ceil(TempPerf.maximum_estimation[temp_app][temp_lane-1]/6.75); //SHOULD BE CALIBRATED...!!!!!
				
				if(TempPerf.maximum_estimation[temp_app][temp_lane-1]<3) //To avoid getting wrong value for queue_counter because of the ceil function
					TempPerf.queue_counter[temp_app][temp_lane-1]=0;
				
				if(phase_read.phaseColor[temp_phase-1] != 1 && currtime >= TempPerf.LQV_discharging[temp_app][temp_lane-1] && TempPerf.maximum_estimation[temp_app][temp_lane-1]>0) //Added the last part to avoid getting negative values
				{
					//cout<<"Discharging part started"<<endl;
					TempPerf.maximum_estimation[temp_app][temp_lane-1]-=9.75; //SHOULD BE CALIBRATED...!!!!!
				}  
  
				if(TempPerf.maximum_estimation[temp_app][temp_lane-1]<0)
					TempPerf.maximum_estimation[temp_app][temp_lane-1]=0;
				
			//Logging the First line of the log file for the Queue Length
				sprintf(temp_log, "%.2f ", TempPerf.maximum_estimation[temp_app][temp_lane-1]);
				queue_fs<<temp_log; //cout<<temp_log; 
					
			}
			//Logging the Second line of the log file for the number of vehicles in the queue
			queue_fs<<endl;
			for(int i=0; i<tot_num; i++)
			{
				int temp_app = (i/5)*2 + 1; //mapping between the approach and i: i=[0 1 2 3 ... 19] ----> App=[1 1 1 1 1 3 3 3 3 3 5 5 5 5 5 7 7 7 7 7]
				int temp_lane = (i%5)+1; //mapping between the lane and i: i=[0 1 2 3 ... 19] ----> Lane=[1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5]
				sprintf(temp_log, "%d ", TempPerf.queue_counter[temp_app][temp_lane-1]);
				queue_fs<<temp_log;	//cout<<temp_log; 
			}
			queue_fs<<endl;
			
		//----------------------Going through the list every 15 seconds now for the purpose of webpage!!
		//printf ("ppooiinntt b: %.2f\n", GetSeconds());
			if(currtime - calc_timer > calc_frequency) //Every 15 seconds
			{
				calc_timer = currtime;
				
				//write to a file only for the sake of RSE webserver
				fstream perf_fs;
				perf_fs.open(Performance_data_file,ios::out);
				
				sprintf(temp_log,"The_Number_of_Vehicle_Received_at_time_%lf_is: %d\n",GetSeconds(),trackedveh.ListSize());
				//outputlog(temp_log); 
				cout<<temp_log;
				perf_fs<<temp_log; queue_fs<<temp_log;

				sprintf(temp_log,"The_Current_time_is: %lf\n",GetSeconds());
				outputlog(temp_log);

				//This part is for testing and validating the received data

				//Get the Observation Period
				time_t theTime = time(NULL);
				struct tm *aTime = localtime(&theTime);
				int iHour=0;
				int iMin=0;              
				int iSec=0;
				obtainEndofService(aTime->tm_hour,aTime->tm_min,aTime->tm_sec,calc_frequency, iHour,iMin,iSec);
				sprintf(temp_log,"Observation_Period_is: %d %d %d %d %d %d\n", aTime->tm_hour, aTime->tm_min, aTime->tm_sec, iHour,iMin, iSec);
				perf_fs<<temp_log; outputlog(temp_log);//queue_fs<<temp_log;

				
				float sum_TT[12]={0}; 
				float sum_Delay[12]={0};
				float sum_numstops[12]={0};
				float sum_dsrc[12]={0}; //Only the first 4 indices are used to specify the DSRC range for the 4 major movements

				int count[12]={0};


				vector <string> movement;
				movement.push_back("Northbound_Through");
				movement.push_back("Eastbound_Through");
				movement.push_back("Southbound_Through");
				movement.push_back("Westbound_Through");
				movement.push_back("Northbound_Left_Turn");
				movement.push_back("Northbound_Right_Turn");
				movement.push_back("Eastbound_Left_Turn");
				movement.push_back("Eastbound_Right_Turn");
				movement.push_back("Southbound_Left_Turn");
				movement.push_back("Southbound_Right_Turn");
				movement.push_back("Westbound_Left_Turn");
				movement.push_back("Westbound_Right_Turn");

				for (trackedveh.Reset(); !trackedveh.EndOfList(); trackedveh.Next())			
				{

					//TT & Delay Calculation for each individual vehicle
					trackedveh.Data().TT = trackedveh.Data().leaving_time - trackedveh.Data().entry_time;
					
					//DSRC Calculation
					if(trackedveh.Data().dsrc_range > 0)
					{
						if(trackedveh.Data().approach == trackedveh.Data().previous_approach)
						{	
							trackedveh.Data().dsrc_range_rsu = trackedveh.Data().dsrc_range; //ATTENTION: It can be calibrated later. 60 meters is added due to the lost range of counting the number of packets
							//cout<<"AFTER!!!!!!!!!!!!!!!! Vehicle No. "<< trackedveh.Data().TempID <<" On Approach "<< trackedveh.Data().approach <<" shows DSRC Range of "<< trackedveh.Data().dsrc_range_rsu <<endl;
						}
						else
						{
							trackedveh.Data().dsrc_range_rsu = trackedveh.Data().dsrc_range; //ATTENTION: It can be calibrated later. 60 meters is added due to the lost range of counting the number of packets
							//cout<<"AFTER!!!!!!!!!!!!!!!! Vehicle No. "<< trackedveh.Data().TempID <<" On Approach "<< trackedveh.Data().previous_approach <<" shows DSRC Range of "<< trackedveh.Data().dsrc_range_rsu <<endl;

						}
					}
					
					//only for the sake of RSE webserver
					sprintf(temp_log,"%d %d %d %.2f %d %.2f\n",trackedveh.Data().TempID, trackedveh.Data().approach, trackedveh.Data().lane, trackedveh.Data().Distance, trackedveh.Data().TT,trackedveh.Data().delay);
					perf_fs<<temp_log; cout<<temp_log; //outputlog(temp_log); 

					//Average TT & Delay for each individual approach 
					switch(trackedveh.Data().approach)
					{
						//NB
						case 5:
							if(trackedveh.Data().lane == lane_mapping[5]) //NBLT
							{	count[4] = count[4]+1;
								sum_TT[4] = sum_TT[4] + trackedveh.Data().TT; 
								sum_Delay[4] = sum_Delay[4] + trackedveh.Data().delay;
								sum_numstops[4] = sum_numstops[4] + trackedveh.Data().queue_flag;
								sum_dsrc[0] = sum_dsrc[0] + trackedveh.Data().dsrc_range_rsu; 
							}
							else if (trackedveh.Data().lane == lane_mapping[4]) //NBRT
							{	count[5] = count[5]+1;
								sum_TT[5] = sum_TT[5] + trackedveh.Data().TT; 
								sum_Delay[5] = sum_Delay[5] + trackedveh.Data().delay;
								sum_numstops[5] = sum_numstops[5] + trackedveh.Data().queue_flag;
								sum_dsrc[0] = sum_dsrc[0] + trackedveh.Data().dsrc_range_rsu;
							}
							else //NBTh
							{	count[0] = count[0]+1;
								sum_TT[0] = sum_TT[0] + trackedveh.Data().TT;
								sum_Delay[0] = sum_Delay[0] + trackedveh.Data().delay;
								sum_numstops[0] = sum_numstops[0] + trackedveh.Data().queue_flag;
								sum_dsrc[0] = sum_dsrc[0] + trackedveh.Data().dsrc_range_rsu;
							}
							break;
							//EB
						case 7:
							if(trackedveh.Data().lane == lane_mapping[7]) //EBLT
							{	count[6] = count[6]+1;
								sum_TT[6] = sum_TT[6] + trackedveh.Data().TT; 
								sum_Delay[6] = sum_Delay[6] + trackedveh.Data().delay;
								sum_numstops[6] = sum_numstops[6] + trackedveh.Data().queue_flag; 
								sum_dsrc[1] = sum_dsrc[1] + trackedveh.Data().dsrc_range_rsu;
							}
							else if (trackedveh.Data().lane == lane_mapping[6]) //EBRT
							{	count[7] = count[7]+1;
								sum_TT[7] = sum_TT[7] + trackedveh.Data().TT; 
								sum_Delay[7] = sum_Delay[7] + trackedveh.Data().delay;
								sum_numstops[7] = sum_numstops[7] + trackedveh.Data().queue_flag;
								sum_dsrc[1] = sum_dsrc[1] + trackedveh.Data().dsrc_range_rsu;
							}
							else //EBTh
							{	count[1] = count[1]+1;
								sum_TT[1] = sum_TT[1] + trackedveh.Data().TT;
								sum_Delay[1] = sum_Delay[1] + trackedveh.Data().delay;
								sum_numstops[1] = sum_numstops[1] + trackedveh.Data().queue_flag;
								sum_dsrc[1] = sum_dsrc[1] + trackedveh.Data().dsrc_range_rsu;
							}
							break;
							//SB
						case 1:
							if(trackedveh.Data().lane == lane_mapping[1]) //SBLT
							{	count[8] = count[8]+1;
								sum_TT[8] = sum_TT[8] + trackedveh.Data().TT; 
								sum_Delay[8] = sum_Delay[8] + trackedveh.Data().delay;
								sum_numstops[8] = sum_numstops[8] + trackedveh.Data().queue_flag; 
								sum_dsrc[2] = sum_dsrc[2] + trackedveh.Data().dsrc_range_rsu;
							}
							else //SBTh
							{	count[2] = count[2]+1;
								sum_TT[2] = sum_TT[2] + trackedveh.Data().TT;
								sum_Delay[2] = sum_Delay[2] + trackedveh.Data().delay;
								sum_numstops[2] = sum_numstops[2] + trackedveh.Data().queue_flag;
								sum_dsrc[2] = sum_dsrc[2] + trackedveh.Data().dsrc_range_rsu;
							}
							break;
							//WB   
						case 3: 
							if(trackedveh.Data().lane == lane_mapping[3]) //WBLT
							{	count[10] = count[10]+1;
								sum_TT[10] = sum_TT[10] + trackedveh.Data().TT; 
								sum_Delay[10] = sum_Delay[10] + trackedveh.Data().delay;
								sum_numstops[10] = sum_numstops[10] + trackedveh.Data().queue_flag; 
								sum_dsrc[3] = sum_dsrc[3] + trackedveh.Data().dsrc_range_rsu;
							}
							else if (trackedveh.Data().lane == lane_mapping[2]) //WBRT
							{	count[11] = count[11]+1;
								sum_TT[11] = sum_TT[11] + trackedveh.Data().TT; 
								sum_Delay[11] = sum_Delay[11] + trackedveh.Data().delay;
								sum_numstops[11] = sum_numstops[11] + trackedveh.Data().queue_flag;
								sum_dsrc[3] = sum_dsrc[3] + trackedveh.Data().dsrc_range_rsu;
							}
							else //WBTh
							{	count[3] = count[3]+1;
								sum_TT[3] = sum_TT[3] + trackedveh.Data().TT; 
								sum_Delay[3] = sum_Delay[3] + trackedveh.Data().delay;
								sum_numstops[3] = sum_numstops[3] + trackedveh.Data().queue_flag;
								sum_dsrc[3] = sum_dsrc[3] + trackedveh.Data().dsrc_range_rsu;
							}
							break;
							//NBLT, SBRT, WBTh
						case 8:
							switch(trackedveh.Data().previous_approach)
							{
								//NBLT
								case 5:
									count[4] = count[4]+1;
									sum_TT[4] = sum_TT[4] + trackedveh.Data().TT; 
									sum_Delay[4] = sum_Delay[4] + trackedveh.Data().delay;
									sum_numstops[4] = sum_numstops[4] + trackedveh.Data().queue_flag;
									sum_dsrc[0] = sum_dsrc[0] + trackedveh.Data().dsrc_range_rsu;
									break;
									//SBRT
								case 1:
									count[9] = count[9]+1;
									sum_TT[9] = sum_TT[9] + trackedveh.Data().TT; 
									sum_Delay[9] = sum_Delay[9] + trackedveh.Data().delay;
									sum_numstops[9] = sum_numstops[9] + trackedveh.Data().queue_flag;
									sum_dsrc[2] = sum_dsrc[2] + trackedveh.Data().dsrc_range_rsu;
									break;
									//WBTh
								case 3:
									count[3] = count[3]+1;
									sum_TT[3] = sum_TT[3] + trackedveh.Data().TT; 
									sum_Delay[3] = sum_Delay[3] + trackedveh.Data().delay;
									sum_numstops[3] = sum_numstops[3] + trackedveh.Data().queue_flag;
									sum_dsrc[3] = sum_dsrc[3] + trackedveh.Data().dsrc_range_rsu;
									break;
							}
							break;
							//EBRT, SBTh, WBLT
						case 6:
							switch(trackedveh.Data().previous_approach)
							{
								//WBLT
								case 3:
									count[10] = count[10]+1;
									sum_TT[10] = sum_TT[10] + trackedveh.Data().TT; 
									sum_Delay[10] = sum_Delay[10] + trackedveh.Data().delay;
									sum_numstops[10] = sum_numstops[10] + trackedveh.Data().queue_flag;
									sum_dsrc[3] = sum_dsrc[3] + trackedveh.Data().dsrc_range_rsu;
									break;
									//SBTh
								case 1:
									count[2] = count[2]+1;
									sum_TT[2] = sum_TT[2] + trackedveh.Data().TT; 
									sum_Delay[2] = sum_Delay[2] + trackedveh.Data().delay;
									sum_numstops[2] = sum_numstops[2] + trackedveh.Data().queue_flag;
									sum_dsrc[2] = sum_dsrc[2] + trackedveh.Data().dsrc_range_rsu;
									break;
									//EBRT
								case 7:
									count[7] = count[7]+1;
									sum_TT[7] = sum_TT[7] + trackedveh.Data().TT; 
									sum_Delay[7] = sum_Delay[7] + trackedveh.Data().delay;
									sum_numstops[7] = sum_numstops[7] + trackedveh.Data().queue_flag;
									sum_dsrc[1] = sum_dsrc[1] + trackedveh.Data().dsrc_range_rsu;
									break;

							}
							break;
							//NBRT, EBTh, SBLT
						case 4:
							switch(trackedveh.Data().previous_approach)
							{
								//SBLT
								case 1:
									count[8] = count[8]+1;
									sum_TT[8] = sum_TT[8] + trackedveh.Data().TT; 
									sum_Delay[8] = sum_Delay[8] + trackedveh.Data().delay;
									sum_numstops[8] = sum_numstops[8] + trackedveh.Data().queue_flag;
									sum_dsrc[2] = sum_dsrc[2] + trackedveh.Data().dsrc_range_rsu;
									break;
									//NBRT
								case 5:
									count[5] = count[5]+1;
									sum_TT[5] = sum_TT[5] + trackedveh.Data().TT; 
									sum_Delay[5] = sum_Delay[5] + trackedveh.Data().delay;
									sum_numstops[5] = sum_numstops[5] + trackedveh.Data().queue_flag;
									sum_dsrc[0] = sum_dsrc[0] + trackedveh.Data().dsrc_range_rsu;
									break;
									//EBTh
								case 7:
									count[1] = count[1]+1;
									sum_TT[1] = sum_TT[1] + trackedveh.Data().TT; 
									sum_Delay[1] = sum_Delay[1] + trackedveh.Data().delay;
									sum_numstops[1] = sum_numstops[1] + trackedveh.Data().queue_flag;
									sum_dsrc[1] = sum_dsrc[1] + trackedveh.Data().dsrc_range_rsu;
									break;
							}
							break;
							//WBRT, NBTh, EBLT
						case 2:
							switch(trackedveh.Data().previous_approach)
							{
								//EBLT
								case 7:
									count[6] = count[6]+1;
									sum_TT[6] = sum_TT[6] + trackedveh.Data().TT; 
									sum_Delay[6] = sum_Delay[6] + trackedveh.Data().delay;
									sum_numstops[6] = sum_numstops[6] + trackedveh.Data().queue_flag;
									sum_dsrc[1] = sum_dsrc[1] + trackedveh.Data().dsrc_range_rsu;
									break;
									//WBRT
								case 3:
									count[11] = count[11]+1;
									sum_TT[11] = sum_TT[11] + trackedveh.Data().TT; 
									sum_Delay[11] = sum_Delay[11] + trackedveh.Data().delay;
									sum_numstops[11] = sum_numstops[11] + trackedveh.Data().queue_flag;
									sum_dsrc[3] = sum_dsrc[3] + trackedveh.Data().dsrc_range_rsu;
									break;
									//NBTh
								case 5:
									count[0] = count[0]+1;
									sum_TT[0] = sum_TT[0] + trackedveh.Data().TT; 
									sum_Delay[0] = sum_Delay[0] + trackedveh.Data().delay;
									sum_numstops[0] = sum_numstops[0] + trackedveh.Data().queue_flag;
									sum_dsrc[0] = sum_dsrc[0] + trackedveh.Data().dsrc_range_rsu;
									break;
							}
							break;
					}		
				}

				//for logging the average travel times and delay for each approach
				for(int i=0; i<12; i++)
				{
					TempPerf.App_TT[i] = sum_TT[i]/count[i];
					TempPerf.App_Delay[i] = sum_Delay[i]/count[i];
					TempPerf.App_numstops[i] = sum_numstops[i]/count[i];
					TempPerf.App_dsrc[i] = (sum_dsrc[i]/count[i])+175;	
				}

				if (trackedveh.ListSize()>0) //Logs only when there is a vehicle in the range!
				{ 	
					for(int k=0; k<12; k++)
					{
						sprintf(temp_log,"Average_TT_for_%s_movement_is: %.2f\n", movement[k].c_str(), TempPerf.App_TT[k]);
						perf_fs<<temp_log; if(count[k] > 0) {outputlog(temp_log); cout<<temp_log;} //To only print the movements with positive values.

						sprintf(temp_log,"Average_Delay_for_%s_movement_is: %.2f\n", movement[k].c_str(), TempPerf.App_Delay[k]);
						perf_fs<<temp_log; if(count[k] > 0) outputlog(temp_log);

						sprintf(temp_log,"Average_Stops_for_%s_movement_is: %.2f\n", movement[k].c_str(), TempPerf.App_numstops[k]);
						perf_fs<<temp_log; if(count[k] > 0) outputlog(temp_log);

						sprintf(temp_log,"Vehicle_Throughput_for_%s_movement_is: %d\n", movement[k].c_str(), count[k]);
						perf_fs<<temp_log; if(count[k] > 0) outputlog(temp_log);
						

					} 
				}
				
				for(int k=0; k<4; k++) //only do it for the four through movements
				{
					sprintf(temp_log,"DSRC_Range_for_%s_movement_is: %.2f \n", movement[k].c_str(), TempPerf.App_dsrc[k]);
					cout<<temp_log;
					perf_fs<<temp_log; if(count[k] > 0) outputlog(temp_log);
				}	
				perf_fs.close();  
				//queue_fs.close();
			} //End of if(currtime - send_timer > calc_frequency) //Every 15 Seconds
			
			sprintf(temp_log, "Estimated Q Length for lane %d and lane %d is: %f %f \n", 1, 2, TempPerf.maximum_estimation[1][0],TempPerf.maximum_estimation[1][1]);
			cout<<temp_log; //queue_fs<<temp_log;
				
			sprintf(temp_log, "The number of Vehicles in the Queue is: %d\n", TempPerf.queue_counter[1][0]);
			//cout<<temp_log;  queue_fs<<temp_log;
				
			queue_fs.close();	
			
		//Push 1second frequency data
			char tmp_perf_data_1s[50000];
			
			
			Push_1s_Frequency(tmp_perf_data_1s,perf_data_size_1s);
			
			char* perf_data_1s;
			perf_data_1s= new char[perf_data_size_1s];
			
			for(i=0;i<perf_data_size_1s;i++)
			perf_data_1s[i]=tmp_perf_data_1s[i];
			//Send trajectory data
			
			recvaddr_db.sin_addr.s_addr = inet_addr(BROADCAST_ADDR_DB) ; //IP of the database server
			numbytes = sendto(sockfd_db,perf_data_1s,perf_data_size_1s+1,0,(struct sockaddr *)&recvaddr_db, addr_length_db);
			
		} //End of if(currtime - send_timer > frequency) //Every 1 Second

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

void UnpackTrajData(byte* ablob)
{
	int No_Veh;
	int i,j;
	int offset;
	offset=0;
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

	//cout<<temp<<endl;

	//id
	temp = (byte)ablob[offset];
	offset = offset + 1; // move past to next item
	//cout<<temp<<endl;


	//Do vehicle number
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	No_Veh = (int)(((byteA << 8) + (byteB))); // in fact unsigned
	offset = offset + 2;

	//cout<<No_Veh<<endl;

	//Do each vehicle
	for(i=0;i<No_Veh;i++)
	{

		ConnectedVehicle TempVeh;
		//Do Veh ID
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.TempID = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		//cout<<TempVeh.TempID;

		//Do nFrame
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.nFrame = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;

		//Do Speed
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.Speed = (tempLong/1000000.0); // convert and store as float
		offset = offset + 4;

		//Do entry_time
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.entry_time = (tempLong); // convert and store as float
		offset = offset + 4;

		//Do leaving_time
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.leaving_time = (tempLong); // convert and store as float
		offset = offset + 4;

		//Do Veh Approach
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.approach = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;

		//Do Veh lane
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.lane = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;

		//Do Distance to Stop Bar
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.stopBarDistance = (tempLong /  1000000.0); // convert and store as float
		offset = offset + 4;

		//Do Veh stop_flag
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.stop_flag = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;

		//Do Veh queue_flag
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.queue_flag = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;

		//Do time when vehicle stops
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.time_stop = (tempLong); // convert and store as float
		offset = offset + 4;	

		//Do Veh Previous Approach
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.previous_approach = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;

		//Do Veh Previous Lane
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.previous_lane = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		
		//Do Veh req_phase
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		TempVeh.req_phase = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		if(TempVeh.req_phase >= 65535)
			TempVeh.req_phase = -1;
		offset = offset + 2;
		
		//Do DSRC Range
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		byteC = ablob[offset+2];
		byteD = ablob[offset+3];
		tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
		TempVeh.dsrc_range = (tempLong /  1000000.0); // convert and store as float
		offset = offset + 4;

		//for(j=TempVeh.nFrame-2;j<TempVeh.nFrame;j++)
		for(j=0;j<2;j++)
		{
			//N_Offset
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			byteC = ablob[offset+2];
			byteD = ablob[offset+3];
			tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
			TempVeh.N_Offset[j] = (tempLong /  1000000.0); // convert and store as float
			offset = offset + 4;

			//cout<<"N_Offset["<<j<<"] is:"<<TempVeh.N_Offset[j]<<endl;

			//E_Offset
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			byteC = ablob[offset+2];
			byteD = ablob[offset+3];
			tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
			TempVeh.E_Offset[j] = (tempLong /  1000000.0); // convert and store as float
			offset = offset + 4;

			//cout<<"E_Offset["<<j<<"] is:"<<tempLong<<endl;
			//cout<<"E_Offset["<<j<<"] is:"<<TempVeh.E_Offset[j]<<endl;
		}
		//cout<<"Done with one vehicle"<<endl;
				
		received_list.InsertRear(TempVeh); //100% Market Penetration Rate
		
	}

}

void obtainEndofService( int ihour,int imin,int isec, int iETA,int &iHour,int &iMin,int &iSec) //Adde by Mehdi
{
	if ((iETA>=0) && (iETA<59))
	{
		if (imin ==59)
		{
			if ((isec + iETA )>=60)
			{
				if (ihour==23)
				{
					iHour   = 0;
					iMin = 0;
					iSec = isec + iETA- 60;
				}else
				{
					iHour   = ihour + 1;
					iMin = 0;
					iSec = isec + iETA- 60;
				}
			}else
			{
				iHour    = ihour;
				iMin  = imin;
				iSec  = isec +iETA;
			}
		}else
		{
			if ((isec + iETA)>=60)
			{
				iHour    = ihour;
				iMin  = imin+1;
				iSec  = isec +iETA- 60;
			}
			else
			{
				iHour    = ihour;
				iMin  = imin;
				iSec = isec + iETA;
			}
		}
	}
	if ((iETA>=60) && (iETA<119))
	{
		if (imin ==59) 
		{
			if ((isec + iETA )<120)
			{
				if (ihour==23)
				{
					iHour   = 0;
					iMin = 0;
					iSec = isec + iETA- 60;
				}else
				{
					iHour   = ihour + 1;
					iMin = 0;
					iSec = isec + iETA- 60;
				}
			}else
			{
				if (ihour==23)
				{
					iHour   = 0;
					iMin = 1;
					iSec = isec + iETA- 120;
				}else
				{
					iHour   = ihour + 1;
					iMin = 1;
					iSec = isec + iETA- 120;
				}
			}
		}else if (imin ==58)
		{
			if ((isec + iETA )<120)
			{
				iHour   = ihour ;
				iMin = 59;
				iSec = isec + iETA- 60;
			}else
			{
				if (ihour==23)
				{
					iHour   = 0;
					iMin = 0;
					iSec = isec + iETA- 120;
				}else
				{
					iHour   = ihour + 1;
					iMin = 0;
					iSec = isec + iETA- 120;
				}
			}
		}else if (imin <58)
		{
			if ((isec + iETA )<120)
			{
				iHour=ihour;
				iMin = imin+1;
				iSec = isec + iETA- 60;
			}else
			{
				iHour=ihour;
				iMin = imin+2;
				iSec = isec + iETA- 120;
			}
		}
	}
}	

void get_lane_movement_mapping() 
{
	fstream sh;

	sh.open(Lane_Movement_Mapping_File_Name);

	char temp[128];

	string temp_string;

	getline(sh,temp_string);  //First line is comment explaining the order of the lanes
	getline(sh,temp_string);  //Second line shows the corresponding lane numbers: SBRT, SBLT, WBRT, WBLT, NBRT, NBLT, EBRT, EBLT

	if(temp_string.size()!=0)
	{
		char tmp[128];
		strcpy(tmp,temp_string.c_str());		
		sscanf(tmp,"%d %d %d %d %d %d %d %d",&lane_mapping[0],&lane_mapping[1],&lane_mapping[2],&lane_mapping[3],&lane_mapping[4],&lane_mapping[5],&lane_mapping[6],&lane_mapping[7]);
	}
	else
	{
		sprintf(temp,"Reading Lane_Movement_Mapping_File problem");
		cout<<temp<<endl;
		outputlog(temp);
		exit(0);
	}

	getline(sh,temp_string);  //Third line is comment explaining the order of the Movements
	getline(sh,temp_string);  //Fourth line shows the corresponding movement numbers

	if(temp_string.size()!=0)
	{
		char tmp[128];
		strcpy(tmp,temp_string.c_str());		
		sscanf(tmp,"%d %d %d %d %d %d %d %d %d %d %d %d",&movement_mapping[0],&movement_mapping[1],&movement_mapping[2],&movement_mapping[3],&movement_mapping[4],&movement_mapping[5],&movement_mapping[6],&movement_mapping[7],&movement_mapping[8],&movement_mapping[9],&movement_mapping[10],&movement_mapping[11]);
	}
	else
	{
		sprintf(temp,"Reading Lane_Movement_Mapping_File problem");
		cout<<temp<<endl;
		outputlog(temp);
		exit(0);
	}

	sh.close();
}
    
void get_Det_Numbers() 
{
	fstream sh;
	sh.open(Det_Numbers_file);
	char temp[128];
	
	
	string temp_string;

    getline(sh,temp_string);  //Reading the first line which explains the detectors assignment
    getline(sh,temp_string);  //Read the second line which indicate the number of detectors
    char tmp[128];
		strcpy(tmp,temp_string.c_str());		
		sscanf(tmp,"%d",&tot_num);
		
	Det_Number = new int[tot_num];
		
	getline(sh,temp_string);  //Reading the third line which has the Detector Number for each lane
	
	if(temp_string.size()!=0)
    {
		strcpy(tmp,temp_string.c_str());	
		
		char * pch;
		pch=strtok(tmp," ");
		
			
		for(int i=0; i<tot_num ; i++)
		{
		sscanf(pch,"%d",&Det_Number[i]);
		//printf("%d\n",Det_Number[i]);
		pch=strtok(NULL," ,.-");
		}
    }
    else
    {
        sprintf(temp,"Reading Det_Numbers_file problem!!");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }
    
    getline(sh,temp_string);  //Reading the fourth line which explains the Approach & Lane Combination for Distance from System Detectors to stop bar
	getline(sh,temp_string);  //Reading the fifth line which has the values for the corresponding approach and lane
	
	if(temp_string.size()!=0)
    {
		strcpy(tmp,temp_string.c_str());	
		
		char * pch;
		pch=strtok(tmp," ");
		
			
		for(int i=0; i<tot_num; i++) //going through approaches 1,3,5,7 and lanes 1,2,3,4,5
		{	
			sscanf(pch,"%f",&Detector_stopbar[i]);
			//printf("%.2f\n",Detector_stopbar[i]);
			pch=strtok(NULL," ,-");
		}
    }
    else
    {
        sprintf(temp,"Reading Stop bar distance of detectors problem!!");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }
    
    getline(sh,temp_string);  //Reading the sixth line which explains the Association between the Approach & Lane Combination and Phases
	getline(sh,temp_string);  //Reading the seventh line which has the values for the corresponding phases
	
	if(temp_string.size()!=0)
    {
		strcpy(tmp,temp_string.c_str());	
		
		char * pch;
		pch=strtok(tmp," ");
		
			
		for(int i=0; i<tot_num; i++) //going through approaches 1,3,5,7 and lanes 1,2,3,4,5
		{	
			sscanf(pch,"%d",&Lane_phase[i]);
			//printf("%.2f\n",Lane_phase[i]);
			pch=strtok(NULL," ,-");
		}
    }
    else
    {
        sprintf(temp,"Reading Phase Lane Mapping problem!!");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }
    
    getline(sh,temp_string);  //Reading the eighth line which explains the Turning Movement Proportions
    for(int i=0; i<tot_num ;i++) //Reading line by line the next 20 lines which has the values for the corresponding TMPs
    {
		getline(sh,temp_string);  
		if(temp_string.size()!=0)
		{
			strcpy(tmp,temp_string.c_str());	
			
			char * pch;
			pch=strtok(tmp," ");
	
			for(int j=0;j<3;j++)
			{
				sscanf(pch,"%f",&Turning_proportion[i][j]);
				//printf("%f\n",Turning_proportion[i][j]);
				pch=strtok(NULL," ,-");
			}
			
			/*sscanf(pch,"%f %f %f",&Turning_proportion[i][0],&Turning_proportion[i][1],&Turning_proportion[i][2]);
			printf("%f %f %f\n",Turning_proportion[i][0],Turning_proportion[i][1],Turning_proportion[i][2]);
			pch=strtok(NULL," ,-");
			*/
		}
		else
		{
			sprintf(temp,"Reading Turning Movement Proportions problem!!");
			cout<<temp<<endl;
			outputlog(temp);
			exit(0);
		}	
	}
	
	//printf("%f\n",Turning_proportion[7][1]); //For checking if it reads properly or not

    sh.close();
}

void Push_1s_Frequency(byte* perf_traj_data,int &size)
{
	//Push Performance data when requested
	int i,j;
	int offset=0;
	char*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    char    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
    double  temp;
	//header 2 bytes
	perf_traj_data[offset]=0xFF;
	offset+=1;
	perf_traj_data[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x01 for 1s perforamnce data
	perf_traj_data[offset]=0x01;
	offset+=1;	
	
	for(i=0;i<8;i++)
		for(j=0;j<5;j++)
		{
			tempLong = (long)(TempPerf.maximum_estimation[i][j] * 100000.0); 
			pByte = (byte* ) &tempLong;
			perf_traj_data[offset+0] = (byte) *(pByte + 3); 
			perf_traj_data[offset+1] = (byte) *(pByte + 2); 
			perf_traj_data[offset+2] = (byte) *(pByte + 1); 
			perf_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
		}
	size=offset;
}
