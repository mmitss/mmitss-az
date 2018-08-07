
/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  MMITSS_rsu_BSM_receiver_Gavilan_Savari.cpp  
*  Created by Yiheng Feng on 7/11/14.
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
  
   
// Work with Vissim 6 DriverModel through UDP
// Need drivermodel_udp_R.dll from "DriverModel_DLL_UDP_InFusion" running

//2014.4.16: Added function
//Receive request from Signal_Control and Performance Measure to send trajectory data

//2014.7.11: Changed Function
//BSM unpacking is now using Savari function.


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

//#include "Mib.h"
#include "LinkedList.h"
#include "NMAP.h"
#include "geoCoord.h"
#include "BasicVehicle.h"
#include "ListHandle.h"
#include "ConnectedVehicle.h"



#include <asn_application.h>
#include <asn_internal.h> /* for _ASN_DEFAULT_STACK_MAX */
#include <BSM.h>

    
using namespace std;

#ifndef BSM_MSG_SIZE
    #define BSM_MSG_SIZE  (46)
#endif

#ifndef DEG2ASNunits
    #define DEG2ASNunits  (1000000.0)  // used for ASN 1/10 MICRO deg to unit converts
#endif

#ifndef byte
    #define byte  char  // needed if byte not defined
#endif

//socket settings
#define PRPORT 15020
//#define OBU_ADDR "192.168.1.25"
#define BROADCAST_ADDR "192.168.101.255"   //DSRC

#define LOCAL_HOST_ADDR "127.0.0.1"

#define EV  1
#define TRANSIT 2

#define ACTIVE 1
#define NOT_ACTIVE -1
#define LEAVE 0

#define PI 3.14159265

char vehicleid [64];
char buf[256];

char temp_log[512];
int continue_while=0;


//define log file name
char predir [64] = "/nojournal/bin/";
char logfilename[256] = "/nojournal/bin/log/MMITSS_rsu_BSM_receiver_";
char rndf_file[64]="/nojournal/bin/RNDF.txt";
char active_rndf_file[128]="/nojournal/bin/ActiveRNDF.txt";
char ConfigInfo[256]	  = "/nojournal/bin/ConfigInfo.txt";
char arrivaltablefile[256]= "/nojournal/bin/ArrivalTable.txt";
char trajdata[256]= "/nojournal/bin/trajectorydata.txt";
char MAP_File_Name[64]  = "/nojournal/bin/nmap_name.txt";
char Lane_Phase_Mapping_File_Name[64]  = "/nojournal/bin/Lane_Phase_Mapping.txt";
char DSRC_Range_file[64] = "/nojournal/bin/DSRC_Range.txt";

//!!! B.H : To find out whether the transit passes the far side but stop
char lanepeermapping[64]  = "/nojournal/bin/Lane_Peer_Mapping.txt"; 
void readlanepeermapping();
double lat_busStop_east[2];
double long_busStop_east[2];
double lat_busStop_west[2];
double long_busStop_west[2];
int pre_approach_long[2]; 
int curr_approach_long[2]; 
int peer_reqPhase[2];
double ETA_east;
char eastbound_IP;
double ETA_west;
char westbound_IP;
int flag_busStopEast=0;
int flag_busStopWest=0;
//double temp_R = 0.0;
double local_x_bus[2];
double local_y_bus[2];
double local_z_bus[2];


string nmap_name;



//new map file
//char MAP_File_Name[128]="/nojournal/bin/Daysi_Gav_Reduced.nmap";  //file stored in default folder
vector<LaneNodes> MAP_Nodes;

int  outputlog(char *output);
//void get_ip_address();           // READ the virtual ASC controller IP address into INTip from "IPInfo.txt"
void FindVehInMap(double Speed, double Heading,int nFrame,double N_Offset1,double E_Offset1,double N_Offset2,double E_Offset2, MAP NewMap, int pre_app, double &Dis_curr, double &est_TT, int &request_phase, int &previous_requested_phase, int &approach, int &lane);

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
int T_approach;        
int T_lane;
int requested_phase=-1;  //requested phase
int previous_requested_phase=0;


//for NTCIP signal control

char IPInfo[64]="/nojournal/bin/ntcipIP.txt";  //stores the virtual asc3 Controller IP address
char INTip[64];
char *INTport = "501";   //this port is for the virtual controller
int CombinedPhase[8]={0};


string RSUID;
char rsuid_filename[64]   = "/nojournal/bin/rsuid.txt";
char ConfigFile[256] = "/nojournal/bin/ConfigInfo_Daisy_Mout_Traj.txt";  //this is just the temporary file for trajectory based test

//For sending the trajectory data
int traj_data_size;  //in byte

LinkedList <ConnectedVehicle> trackedveh;

//Pack trajectory data if the request is from performance observer
void PackTrajData(byte* tmp_traj_data,int &size);
//Pack trajectory data if the request is from signal control
void PackTrajData1(byte* tmp_traj_data,int &size);
//Pack trajectory data if the request is from coordinator
void PackTrajData2(byte* tmp_traj_data,int &size);
//Pack trajectory data if the request is from SAC
void PackTrajData3(byte* tmp_traj_data,int &size);


//data structure to store the lane nodes and phase mapping

int LaneNode_Phase_Mapping[8][8][20];           //8 approaches, at most 8 lanes each approach, at most 20 lane nodes each lane
												// the value is just the requested phase of this lane node;
int phase_mapping[8];    //the corresponding phases sequence: approach 1 3 5 7 through left
float DSRC_Range[8]; 	 //Range of the DSRC radio on each leg of the intersection
int rect_width, rect_height; //The width and Height of the central Rectangle

//Parameters for trajectory control
int ArrivalTable[121][8];     //maximum planning time horizon * number of phases
int currenttime;
int smooth_timer1;
int smooth_timer2; 

double GetSeconds();
void get_map_name(); //Yiheng add 07/18/2014
void xTimeStamp( char * pc_TimeStamp_ );
void get_lane_phase_mapping();
void get_DSRC_Range(); //Shayan added 10.7.14


double FindDistanceToSegment(double x1, double y1, double x2, double y2, double pointX, double pointY);
//!!! B.H : For peer control, double &temp_R, double &temp_S were added
void DistanceFromLine(double cx, double cy, double ax, double ay ,double bx, double by, double &distanceSegment, double &distanceLine, double &temp_R, double &temp_S);



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

int main ( int argc, char* argv[] )
{
	
	//Struct for UDP socket timeout: 0.5s
	struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

	//double lower_ETA=6; //default value
	//double upper_ETA=8;
	
	int traj_send_freq=10;
	
	int port=3333;
	
	float speed_coif=1;
	
	int flag_field=0;     //flag to decide whether this is run in the field or in the lab; 0: in the lab; 1: in the field
	
	//self defined value for sending frequency
	if (argc>=2)	{sscanf(argv[1],"%d",&port);} 
	if (argc>=3)	{sscanf(argv[2],"%f",&speed_coif);} 
	if (argc>=4)    {sscanf(argv[3],"%d",&flag_field);}

	
	//get_ip_address();           // READ the ASC controller IP address into INTip from "ntcipIP.txt"
	
	
	//gps_init ();
	
	
	//~ for(int i=0;i<20;i++)
	//~ {
		//~ gps_init ();
		//~ int result=savari_gps_read(&gps, gps_handle);
		//~ cout<<"result is:"<<result<<endl;
		//~ sprintf(temp_log,"%lf %lf %.1lf\n",gps.latitude,gps.longitude,gps.time);
		//~ cout<<temp_log;
		//~ savari_gps_close(gps_handle);
		//~ msleep(200);
	//~ }
	
	char timestamp[128];
    xTimeStamp(timestamp);
    strcat(logfilename,timestamp);    
    strcat(logfilename,".log");
	
	printf("%s\n",logfilename);
	
	fstream fs_log;
	fs_log.open(logfilename,fstream::out | fstream::trunc);
	
	int i,j,k;
	
	//read the new map from the .nmap file and save to NewMap;

	MAP NewMap;
	NewMap.ID=1;
	NewMap.Version=1;
	// Parse the MAP file
	get_map_name();
	char mapname[128];
	sprintf(mapname,"%s",nmap_name.c_str());
	printf("%s",mapname);
	NewMap.ParseIntersection(mapname);

	sprintf(temp_log,"Read the map successfully At (%d).\n",time(NULL));
	outputlog(temp_log); cout<<temp_log;
	
	//Initialize the ref point
	double ref_lat=NewMap.intersection.Ref_Lat;
	double ref_long=NewMap.intersection.Ref_Long;
	double ref_ele=NewMap.intersection.Ref_Ele/10;
	geoPoint.init(ref_long, ref_lat, ref_ele);
	
	sprintf(temp_log,"%lf %lf %lf \n",ref_lat,ref_long,ref_ele);
	outputlog(temp_log); cout<<temp_log;
	
	int temp_phase=0;
	
	for(i=0;i<8;i++)
{
	for(j=0;j<8;j++)
	{
		for(k=0;k<20;k++)
		{
			LaneNode_Phase_Mapping[i][j][k]=0;
		}
	}
}
	
	
	//store all nodes information to MAP_Nodes after parsing the message
	//This is used for calculating vehicle positions in the MAP
	for (i=0;i<NewMap.intersection.Approaches.size();i++)
		for(j=0;j<NewMap.intersection.Approaches[i].Lanes.size();j++)
			for(k=0;k<NewMap.intersection.Approaches[i].Lanes[j].Nodes.size();k++)
			{	
				LaneNodes temp_node;
				temp_node.index.Approach=NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].index.Approach;
				temp_node.index.Lane=NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].index.Lane;
				temp_node.index.Node=NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].index.Node;
				temp_node.Latitude=NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].Latitude;
				temp_node.Longitude=NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].Longitude;
				geoPoint.lla2ecef(temp_node.Longitude,temp_node.Latitude,ref_ele,&ecef_x,&ecef_y,&ecef_z);
				geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x,&local_y,&local_z);
				temp_node.N_Offset=local_x;
				temp_node.E_Offset=local_y;

				MAP_Nodes.push_back(temp_node);
												
				sprintf(temp_log,"%d %d %d %lf %lf %lf %lf \n",temp_node.index.Approach,temp_node.index.Lane,temp_node.index.Node,temp_node.N_Offset,temp_node.E_Offset,temp_node.Latitude,temp_node.Longitude);
				//outputlog(temp_log); cout<<temp_log;
			}

//Construct the lane node phase mapping matrix
get_lane_phase_mapping();

for(int iii=0;iii<MAP_Nodes.size();iii++)
{
	int flag=0;
	for (i=0;i<NewMap.intersection.Approaches.size();i++)
	{
		for(j=0;j<NewMap.intersection.Approaches[i].Lanes.size();j++)
		{
			for(k=0;k<NewMap.intersection.Approaches[i].Lanes[j].Nodes.size();k++)
			{
				if(NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].index.Approach==MAP_Nodes[iii].index.Approach &&
					NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].index.Lane==MAP_Nodes[iii].index.Lane)
				{
					//determine requesting phase
					if (MAP_Nodes[iii].index.Approach==1)  //south bound
					{
						if (NewMap.intersection.Approaches[i].Lanes[j].Attributes[1]==1)  //through
							LaneNode_Phase_Mapping[MAP_Nodes[iii].index.Approach][MAP_Nodes[iii].index.Lane][MAP_Nodes[iii].index.Node]=phase_mapping[0];
						if (NewMap.intersection.Approaches[i].Lanes[j].Attributes[2]==1)  //left turn
							LaneNode_Phase_Mapping[MAP_Nodes[iii].index.Approach][MAP_Nodes[iii].index.Lane][MAP_Nodes[iii].index.Node]=phase_mapping[1];
					}
					if (MAP_Nodes[iii].index.Approach==3)
					{
						if (NewMap.intersection.Approaches[i].Lanes[j].Attributes[1]==1)  //through
							LaneNode_Phase_Mapping[MAP_Nodes[iii].index.Approach][MAP_Nodes[iii].index.Lane][MAP_Nodes[iii].index.Node]=phase_mapping[2];
						if (NewMap.intersection.Approaches[i].Lanes[j].Attributes[2]==1)  //left turn
							LaneNode_Phase_Mapping[MAP_Nodes[iii].index.Approach][MAP_Nodes[iii].index.Lane][MAP_Nodes[iii].index.Node]=phase_mapping[3];
					}
					if (MAP_Nodes[iii].index.Approach==5)
					{
						if (NewMap.intersection.Approaches[i].Lanes[j].Attributes[1]==1)  //through
							LaneNode_Phase_Mapping[MAP_Nodes[iii].index.Approach][MAP_Nodes[iii].index.Lane][MAP_Nodes[iii].index.Node]=phase_mapping[4];
						if (NewMap.intersection.Approaches[i].Lanes[j].Attributes[2]==1)  //left turn
							LaneNode_Phase_Mapping[MAP_Nodes[iii].index.Approach][MAP_Nodes[iii].index.Lane][MAP_Nodes[iii].index.Node]=phase_mapping[5];
					}
					if (MAP_Nodes[iii].index.Approach==7)
					{
						if (NewMap.intersection.Approaches[i].Lanes[j].Attributes[1]==1)  //through
							LaneNode_Phase_Mapping[MAP_Nodes[iii].index.Approach][MAP_Nodes[iii].index.Lane][MAP_Nodes[iii].index.Node]=phase_mapping[6];
						if (NewMap.intersection.Approaches[i].Lanes[j].Attributes[2]==1)  //left turn
							LaneNode_Phase_Mapping[MAP_Nodes[iii].index.Approach][MAP_Nodes[iii].index.Lane][MAP_Nodes[iii].index.Node]=phase_mapping[7];
					}
					flag=1;
					break;
				}
			}
			if(flag==1)
				break;
		}
		if(flag==1)
			break;
	}
}

cout<<"Lane Nodes Headings:"<<endl;
int tmp_pos=0;
for (unsigned int i=0;i<NewMap.intersection.Approaches.size();i++)
	for(unsigned int j=0;j<NewMap.intersection.Approaches[i].Lanes.size();j++)
		for(unsigned k=0;k<NewMap.intersection.Approaches[i].Lanes[j].Nodes.size();k++)
		{
			
			if(MAP_Nodes[tmp_pos].index.Approach%2==1)  //odd approaches, approching lanes
			{
				if(k<NewMap.intersection.Approaches[i].Lanes[j].Nodes.size()-1)
				{
					MAP_Nodes[tmp_pos].Heading=atan2(MAP_Nodes[tmp_pos].N_Offset-MAP_Nodes[tmp_pos+1].N_Offset,MAP_Nodes[tmp_pos].E_Offset-MAP_Nodes[tmp_pos+1].E_Offset)*180.0/PI;
					MAP_Nodes[tmp_pos].Heading=90.0-MAP_Nodes[tmp_pos].Heading;
				}
				else
				{
					MAP_Nodes[tmp_pos].Heading=MAP_Nodes[tmp_pos-1].Heading;	
					//cout<<MAP_Nodes[tmp_pos].Heading<<" "<<MAP_Nodes[tmp_pos-1].Heading<<endl;
				}

				sprintf(temp_log,"Appraoching heading %d is %f\n",tmp_pos,MAP_Nodes[tmp_pos].Heading);
				cout<<temp_log;
			}
			if(MAP_Nodes[tmp_pos].index.Approach%2==0)  //even approaches, leaving lanes
			{
				if(k<NewMap.intersection.Approaches[i].Lanes[j].Nodes.size()-1)
				{
					MAP_Nodes[tmp_pos].Heading=atan2(MAP_Nodes[tmp_pos+1].N_Offset-MAP_Nodes[tmp_pos].N_Offset,MAP_Nodes[tmp_pos+1].E_Offset-MAP_Nodes[tmp_pos].E_Offset)*180.0/PI;
					MAP_Nodes[tmp_pos].Heading=90.0-MAP_Nodes[tmp_pos].Heading;
				}
				else
				{
					MAP_Nodes[tmp_pos].Heading=MAP_Nodes[tmp_pos-1].Heading;	
				}
				
					sprintf(temp_log,"Leaving heading %d is %f\n",tmp_pos,MAP_Nodes[tmp_pos].Heading);
					cout<<temp_log;
			}
											
			if(MAP_Nodes[tmp_pos].Heading<0)
			 MAP_Nodes[tmp_pos].Heading+=360;
			
			//sprintf(temp_log,"%d %d %d %lf %lf %lf %lf %f\n",NewMap.MAP_Nodes[tmp_pos].index.Approach,NewMap.MAP_Nodes[tmp_pos].index.Lane,NewMap.MAP_Nodes[tmp_pos].index.Node,NewMap.MAP_Nodes[tmp_pos].N_Offset,NewMap.MAP_Nodes[tmp_pos].E_Offset,NewMap.MAP_Nodes[tmp_pos].Latitude,NewMap.MAP_Nodes[tmp_pos].Longitude,NewMap.MAP_Nodes[tmp_pos].Heading);
			//outputlog(temp_log); cout<<temp_log;										
			tmp_pos++;								
		}

	
				
//for(i=0;i<8;i++)
//{
//	for(j=0;j<8;j++)
//	{
//		for(k=0;k<20;k++)
//		{
//			sprintf(temp_log,"The requested phase for node %d.%d.%d is %d\n",i,j,k,LaneNode_Phase_Mapping[i][j][k]);
//			outputlog(temp_log);
//		}
//	}
//}		
	
	get_DSRC_Range(); //call the function to read the DSRC ranges and central rectangle attributes
	//List to store the vehicle trajectory
	
	//!!! B.H : To find out whether the transit passes the far side but stop
	readlanepeermapping();
	if (lat_busStop_east[0] > 0)
	{
		flag_busStopEast =1;
	}
	if (lat_busStop_west[0] > 0)
	{
		flag_busStopWest =1;
	}
	
	
	ConnectedVehicle TempVeh;


	char BSM_buf[BSM_MSG_SIZE]={0};  //buffer to store the BSM
	//BasicVehicle vehIn;

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
	
	//Setup time out
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Error");
	}

//	if((setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,
//		&broadcast,sizeof broadcast)) == -1)
//	{
//		perror("setsockopt - SO_SOCKET ");
//		exit(1);
//	}
     
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = htons(port);  //*** IMPORTANT: the vissim,signal control and performance observer should also have this port. ***//
	sendaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr(LOCAL_HOST_ADDR);//inet_addr(OBU_ADDR);//INADDR_ANY;

	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);

	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");        exit(1);
	}

//	recvaddr.sin_family = AF_INET;
//	recvaddr.sin_port = htons(PRPORT);
//	recvaddr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR) ; //INADDR_BROADCAST;
//	memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);

	int addr_length = sizeof ( recvaddr );
	int recv_data_len;
	//-----------------------End of Network Connection------------------//

	cout<<"About to get data from Vissim...............\n";

	int rolling_horizon=1;  //every 1 seconds, write the arrival table to a file
	int begintime=time(NULL);
	currenttime=time(NULL);
    smooth_timer1=time(NULL);
    smooth_timer2=time(NULL); 
  
  
	int control_timer;
	
	int counter=0;
	
	double t_1,t_2; //---time stamps used to calculate the time needed
	double traj_send_timer;
	
	traj_send_timer=GetSeconds();
	
	//Whether the data is requested by other components. 1:yes; 0:no
	int traj_requested=0;
	
	sprintf(temp_log,"The format of the veh traj data is: ID Lat Long Speed Heading GPS_Time\n");
	outputlog(temp_log); cout<<temp_log;
	
	while (true)
	{
		//savari_gps_read(&gps, gps_handle);
		
		//t_1=GetSeconds();
		
		recv_data_len = recvfrom(sockfd, buf, sizeof(buf), 0,
                        (struct sockaddr *)&sendaddr, (socklen_t *)&addr_length);
					
		BSM_t * bsmType=0;
		bsmType = (BSM_t *) calloc(1, sizeof(BSM_t));

		if(recv_data_len<0)
		{
						
			//printf("Receive Request failed\n");
			memset(&buf[0], 0, sizeof(buf)); //clear the buf if nothing received
			traj_requested=-1; //means nothing is received
			//sprintf(temp_log,"Receive Request failed\n");
			//cout<<temp_log;  outputlog(temp_log);
			
			//continue;
		}	
		else
		{	
						
			//if(recv_data_len==BSM_MSG_SIZE)  //if received is a BSM data
			//{
			//for(i=0;i<BSM_MSG_SIZE;i++)  //copy the first 45 bytes to BSM_buf
			//{
			//	BSM_buf[i]=buf[i];
			//}	
			
			//Received BSM

			//unit32_t oob;
			unsigned int oob;
			
			
			asn_dec_rval_t rval;
			rval=ber_decode(0, &asn_DEF_BSM,(void **)&bsmType, buf, sizeof(buf));
			if ( rval.code==RC_OK)
			{
			traj_requested=0;
			}
			else
			{
			printf("BSM Decode Error!\n");
			traj_requested=1;
			}

		//	int ii;
		//	for (ii = 0; ii < 70; ii++)
		//		printf ("%02x ", (unsigned char)buf[ii]);
		//	cout << endl;
		}	
		//	for (i=0;i<45;i++)
		//	{
		//		printf("%X  ",buf[i]);
		//	}
		//	printf("\n");
			
			
			//printbsmcsv (&bsm);
			//printbsm(&bsm);
			
			
							//outputlog(temp_log); 
					//		cout<<temp_log;
			
			
			
			//vehIn.BSM2Vehicle(BSM_buf);  //change from BSM to Vehicle information
			
			//t_2=GetSeconds();
			
			//sprintf(temp_log,"The time for unpacking the BSM is: %lf second \n",t_2-t_1);
			//outputlog(temp_log); cout<<temp_log;
			
			//cout<<setprecision(8)<<vehIn.pos.latitude<<endl;
			
			
			
		if(traj_requested==0)
		{
						
			//process the BSM data with the map information
			
			 //Unpack the BSM blob
			BasicVehicle vehIn;
			
			//Modified for veh type (B.H)
			char bsmdata[39];
			for(int ii=0;ii<39;ii++)
			{
			bsmdata[ii]=bsmType->bsmBlob.buf[ii];
			}
			vehIn.BSMToVehicle(bsmdata);
			
			
			
			int Veh_pos_inlist;
			
			trackedveh.Reset();
			
			int found=0;   //flag to indicate whether a vehicle is found in the list
			int pro_flag=0;  //indicate whether to map the received data in the MAP
			//save vehicle trajactory
			
			//t_1=GetSeconds();
			
			double t1=GetSeconds();
			
			while(!trackedveh.EndOfList())  //match vehicle according to vehicle ID
				{
					//if ID is a match, store trajectory information
					//If ID will change, then need an algorithm to do the match!!!!
					if(trackedveh.Data().TempID==vehIn.TemporaryID)  //every 0.5s processes a trajectory 
					{
						//sprintf(temp_log,"Current time is %lf\n",t1);
						//cout<<temp_log;
						if (t1-trackedveh.Data().receive_timer>0.5)
						{
							
							pro_flag=1;   //need to map the point						
							trackedveh.Data().receive_timer=t1;  //reset the timer 
							trackedveh.Data().traj[trackedveh.Data().nFrame].latitude=vehIn.pos.latitude;
							trackedveh.Data().traj[trackedveh.Data().nFrame].longitude=vehIn.pos.longitude;
							trackedveh.Data().traj[trackedveh.Data().nFrame].elevation=vehIn.pos.elevation;
							//change GPS points to local points
							geoPoint.lla2ecef(vehIn.pos.longitude,vehIn.pos.latitude,vehIn.pos.elevation,&ecef_x,&ecef_y,&ecef_z);
							geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x,&local_y,&local_z);
							trackedveh.Data().N_Offset[trackedveh.Data().nFrame]=local_x;
							trackedveh.Data().E_Offset[trackedveh.Data().nFrame]=local_y;
							trackedveh.Data().acceleration=(vehIn.motion.speed-trackedveh.Data().Speed)*2;		//instantaneously acceleration added 4/20/2014					
							trackedveh.Data().Speed=vehIn.motion.speed/speed_coif;       //Speed from Savari BSM decoder is KM/H, need to change to m/s here 
							trackedveh.Data().heading=vehIn.motion.heading;
							//cout<<"Veh heading is: "<<bsm.heading<<endl;
							trackedveh.Data().Dsecond=vehIn.DSecond;
							//trackedveh.Data().active_flag=14;  //reset active_flag every time RSE receives BSM from the vehicle
							trackedveh.Data().time[trackedveh.Data().nFrame]=t1;
							
							
							
							
							//cout<<"Veh Speed is: "<<trackedveh.Data().Speed<<endl;				
							
							//!!!! Modified (B.H) : for 1 second solving
							if(trackedveh.Data().Speed <=1 && trackedveh.Data().stop_flag==0)  //if(trackedveh.Data().Speed<0.8 && trackedveh.Data().stop_flag==0) //vehicle has not stopped before, for the ELVS algorithm
							{
								trackedveh.Data().stop_flag=1;
								trackedveh.Data().queue_flag=1; //for Performance Obserevr
								trackedveh.Data().time_stop=GetSeconds();
							}
							
							if(trackedveh.Data().Speed>1 && trackedveh.Data().stop_flag==1)  //if(trackedveh.Data().Speed>0.8 && trackedveh.Data().stop_flag==1) // Added by Shayan to keep track of the last stopped vehicle 
							{
								trackedveh.Data().stop_flag=0;
							}
							
							if (flag_field==1)  //in the field log the received BSM data
							{
								//gps_init();
								//savari_gps_read(&gps, gps_handle);
								sprintf(temp_log,"Timestamp %.2lf: %d %lf %lf %f %f\n",GetSeconds(),trackedveh.Data().TempID,
								trackedveh.Data().traj[trackedveh.Data().nFrame].latitude,trackedveh.Data().traj[trackedveh.Data().nFrame].longitude,trackedveh.Data().Speed,trackedveh.Data().heading);								
								//sprintf(temp_log,"%d %lf %lf %f %f %.1lf\n",trackedveh.Data().TempID,
								//trackedveh.Data().traj[trackedveh.Data().nFrame].latitude,trackedveh.Data().traj[trackedveh.Data().nFrame].longitude,trackedveh.Data().Speed,trackedveh.Data().heading,gps.time);								
								//sprintf(temp_log,"VehID= %d lat= %lf long= %lf Speed= %f Heading= %f Dsecond= %d local_x=%f loacal_y=%f\n",trackedveh.Data().TempID,
								//bsm.latitude,bsm.longitude,bsm.speed,bsm.heading,bsm.secmark, local_x,local_y);
								//savari_gps_close(gps_handle);
								outputlog(temp_log); 
								//cout<<temp_log;
							}
							//cout<<"VehID= "<<trackedveh.Data().TempID<<" Speed= "<<trackedveh.Data().Speed<<" Heading= "<<trackedveh.Data().heading<<endl;

							//cout<<"Latitude= "<<trackedveh.Data().traj[trackedveh.Data().nFrame].latitude<<endl;
							//cout<<"Longitude= "<<trackedveh.Data().traj[trackedveh.Data().nFrame].latitude<<endl;
							//cout<<"Elevation= "<<trackedveh.Data().traj[trackedveh.Data().nFrame].elevation<<endl;
							
							
					//		fstream traj;
					//		traj.open(trajdata,ios::out|ios::app);	
					//		int temp_frame=trackedveh.Data().nFrame;
							//for (i=1;i<=temp_frame;i++)
							//{
							
					//			traj<<trackedveh.Data().TempID<<" ";
					//			traj<<fixed<<setprecision(2)<<trackedveh.Data().time[temp_frame];
					//			traj<<" "<<trackedveh.Data().N_Offset[temp_frame]<<" "<<trackedveh.Data().E_Offset[temp_frame]<<" "<<trackedveh.Data().req_phase<<endl;
							//}
					//		traj.close();
							
							
							
							trackedveh.Data().nFrame++; 
							
							//if reached the end of the trajectory, start over
							if(trackedveh.Data().nFrame==5000)
								trackedveh.Data().nFrame=0;
							
							
							Veh_pos_inlist=trackedveh.CurrentPosition();  //store this vehicle's position in the tracked vehicle list, start from 0
						}
						found=1;
						break;
					}
				trackedveh.Next();
				}
			if(found==0)  //this is a new vehicle
				{
					TempVeh.TempID=vehIn.TemporaryID;
					TempVeh.traj[0].latitude=vehIn.pos.latitude;
					TempVeh.traj[0].longitude=vehIn.pos.longitude;
					TempVeh.traj[0].elevation=vehIn.pos.elevation;
					//change GPS points to local points
					geoPoint.lla2ecef(vehIn.pos.longitude,vehIn.pos.latitude,vehIn.pos.elevation,&ecef_x,&ecef_y,&ecef_z);
					geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x,&local_y,&local_z);
					TempVeh.N_Offset[0]=local_x;
					TempVeh.E_Offset[0]=local_y;
					TempVeh.Speed=vehIn.motion.speed/speed_coif;
					TempVeh.acceleration=0;
					TempVeh.nFrame=1;
					TempVeh.Phase_Request_Counter=0;
					TempVeh.active_flag=14;  //initilize the active_flag
					TempVeh.receive_timer=GetSeconds();
					TempVeh.time[0]=TempVeh.receive_timer;
					TempVeh.stop_flag=0;
					TempVeh.queue_flag=0;
					TempVeh.processed=0;
					TempVeh.entry_time=GetSeconds();
					TempVeh.previous_approach = 0; //To keep track of the approach for Performance by movement
					
					TempVeh.temp_approach = 0;
					TempVeh.previous_lane = 0; //To keep track of the lane for performance by movement
					TempVeh.temp_lane = 0;
					
					TempVeh.time_stop=0;
					//!!! Added for veh type (B.H)
					//TempVeh.VehType=vehIn.Vehicletype;
					
					//!!!! for offset refiner
					//TempVeh.offsetRefiner_flag=1;
					
					//cout<<"Local x:"<<local_x<<" "<<"local y:"<<local_y<<endl;
					
					//-----------------------------------Vehicle Entrance into the Geo Fenced Area-------------------------------//

					//Find the Nearest Lane info from the current vehicle location
					int N_App,N_Lane,N_Node; //Nearest Approach, Lane, and Node
					int N_pos; //node position in the vector
					double Tempdis = 10000000.0;
					unsigned int s;
					//The output of DistanceFromLine Function:
					double distanceSegment, distanceLine;
					
					//!!! B.H : far side bus stop
					double temp_R = -99.0;
					double temp_S = -99.0;
					
					double dis_to_ref; //Distance to the Refernce Point
						//find the nearest lane node
					for(s = 0; s < MAP_Nodes.size(); s++)
					{
						double Dis = sqrt(pow((local_x - MAP_Nodes[s].N_Offset),2) + pow((local_y - MAP_Nodes[s].E_Offset),2));
						if (Dis<Tempdis)
						{
							Tempdis = Dis;
							N_App = MAP_Nodes[s].index.Approach;
							N_Lane = MAP_Nodes[s].index.Lane;
							N_Node = MAP_Nodes[s].index.Node;
							N_pos = s;
						}
					}
					
					//cout<<"nearest node is: "<<N_App<<"."<<N_Lane<<"."<<N_Node<<endl;
					
					if(Tempdis<1000000.0)  // Find a valid node
					{
						//cout<<N_App<<"."<<N_Lane<<"."<<N_Node<<endl;						
						if(N_Node == 1) //Case when the nearest node is the first node of the lane: The line bw MAP_Nodes[s] and MAP_Nodes[s+1]
						{
							DistanceFromLine(local_x, local_y, MAP_Nodes[N_pos].N_Offset, MAP_Nodes[N_pos].E_Offset , MAP_Nodes[N_pos+1].N_Offset, MAP_Nodes[N_pos+1].E_Offset, distanceSegment, distanceLine, temp_R, temp_S);
							//cout<<"Two points are: "<<MAP_Nodes[N_pos].index.Approach<<"."<<MAP_Nodes[N_pos].index.Lane<<"."<<MAP_Nodes[N_pos].index.Node<<" and "<<MAP_Nodes[N_pos+1].index.Approach<<"."<<MAP_Nodes[N_pos+1].index.Lane<<"."<<MAP_Nodes[N_pos+1].index.Node<<endl; 
						}
						else
						{
							//line bw MAP_Nodes[s] and MAP_Nodes[s-1]
							DistanceFromLine(local_x, local_y, MAP_Nodes[N_pos].N_Offset, MAP_Nodes[N_pos].E_Offset , MAP_Nodes[N_pos-1].N_Offset, MAP_Nodes[N_pos-1].E_Offset, distanceSegment, distanceLine, temp_R, temp_S);
							//cout<<"Two points are: "<<MAP_Nodes[N_pos].index.Approach<<"."<<MAP_Nodes[N_pos].index.Lane<<"."<<MAP_Nodes[N_pos].index.Node<<" and "<<MAP_Nodes[N_pos-1].index.Approach<<"."<<MAP_Nodes[N_pos-1].index.Lane<<"."<<MAP_Nodes[N_pos-1].index.Node<<endl; 
						}
						//cout<<"Distance To Line is: "<<distanceLine<<endl;
						
						//Add the vehcile into the list only when it is inside the GeoFenced area
						
						dis_to_ref = sqrt(pow(local_x,2) + pow(local_y,2));
						

 
								if (dis_to_ref <= DSRC_Range[N_App-1] && distanceLine <= 8)  
								{
									trackedveh.InsertRear(TempVeh);   //add the new vehicle to the tracked list
						
									//sprintf(temp_log,"Add Vehicle No. %d at %lf, the list size is %d\n",TempVeh.TempID, GetSeconds(),trackedveh.ListSize());
									//outputlog(temp_log); cout<<temp_log;

									//sprintf(temp_log,"The added location is %lf %lf\n",TempVeh.traj[0].latitude,TempVeh.traj[0].longitude);
									//outputlog(temp_log); cout<<temp_log;
									
									Veh_pos_inlist=trackedveh.ListSize()-1;  //start from 0
								}
								//else
									//cout<<"Vehicle ID of "<<TempVeh.TempID<<" is Out of Range!!! The vehicle is not considered!"<<endl;
							
					}
					else
					{
						sprintf(temp_log,"Not valid GPS points !\n");
						outputlog(temp_log); cout<<temp_log;
					}
				}
		   		
				
		//Find the vehicle in the map: output: distance, eta and requested phase
		
		
		//t_2=GetSeconds();
			
		//sprintf(temp_log,"The time for Adding vehicle to the list is: %lf second \n",t_2-t_1);
		//outputlog(temp_log); cout<<temp_log;
					

		if(pro_flag==1)
		{
									
			//t_1=GetSeconds();
			trackedveh.Reset(Veh_pos_inlist);
			double tmp_speed=trackedveh.Data().Speed;
			double tmp_heading=trackedveh.Data().heading;
			int tmp_nFrame=trackedveh.Data().nFrame;
			double tmp_N_Offset1=trackedveh.Data().N_Offset[trackedveh.Data().nFrame-1];
			double tmp_E_Offset1=trackedveh.Data().E_Offset[trackedveh.Data().nFrame-1];
			double tmp_N_Offset2=trackedveh.Data().N_Offset[trackedveh.Data().nFrame-2];
			double tmp_E_Offset2=trackedveh.Data().E_Offset[trackedveh.Data().nFrame-2];
			
	//-----------------------------------Vehicle Exit form the Geo Fenced Area-------------------------------//

            //Find the Nearest Lane info from the current vehicle location
			int N_App,N_Lane,N_Node; //Nearest Approach, Lane, and Node
			int N_pos; //node position in the vector
			double Tempdis = 1000000.0;
			unsigned int s;
			//The output of DistanceFromLine Function:
			double distanceSegment, distanceLine;
			double dis_to_ref; //Distance to the refernce point
			//!!! B.H : far side bus stop
			double temp_R = -99.0;
			double temp_S = -99.0;
			double Dis_bus = 0.0;
			 
			//find the nearest lane node
			for(s = 0; s < MAP_Nodes.size(); s++)
			{
				double Dis = sqrt(pow((local_x - MAP_Nodes[s].N_Offset),2) + pow((local_y - MAP_Nodes[s].E_Offset),2));
				if (Dis<Tempdis)
				{
					Tempdis = Dis;
					N_App = MAP_Nodes[s].index.Approach;
					N_Lane = MAP_Nodes[s].index.Lane;
					N_Node = MAP_Nodes[s].index.Node;
					N_pos = s;
				}
				
			}
			
			if (Tempdis<1000000.0)
			{
				  
				//cout<<N_App<<"."<<N_Lane<<"."<<N_Node<<endl;
					
				if(N_Node == 1) //Case when the nearest node is the first node of the lane: The line bw MAP_Nodes[s] and MAP_Nodes[s+1]
				{
					DistanceFromLine(local_x, local_y, MAP_Nodes[N_pos].N_Offset, MAP_Nodes[N_pos].E_Offset , MAP_Nodes[N_pos+1].N_Offset, MAP_Nodes[N_pos+1].E_Offset, distanceSegment, distanceLine, temp_R, temp_S);
					//cout<<"Two points are: "<<MAP_Nodes[N_pos].index.Approach<<"."<<MAP_Nodes[N_pos].index.Lane<<"."<<MAP_Nodes[N_pos].index.Node<<" and "<<MAP_Nodes[N_pos+1].index.Approach<<"."<<MAP_Nodes[N_pos+1].index.Lane<<"."<<MAP_Nodes[N_pos+1].index.Node<<endl; 
				}
				else
				{
					//line bw MAP_Nodes[s] and MAP_Nodes[s-1]
					DistanceFromLine(local_x, local_y, MAP_Nodes[N_pos].N_Offset, MAP_Nodes[N_pos].E_Offset , MAP_Nodes[N_pos-1].N_Offset, MAP_Nodes[N_pos-1].E_Offset, distanceSegment, distanceLine, temp_R, temp_S);
					//cout<<"Two points are: "<<MAP_Nodes[N_pos].index.Approach<<"."<<MAP_Nodes[N_pos].index.Lane<<"."<<MAP_Nodes[N_pos].index.Node<<" and "<<MAP_Nodes[N_pos-1].index.Approach<<"."<<MAP_Nodes[N_pos-1].index.Lane<<"."<<MAP_Nodes[N_pos-1].index.Node<<endl; 
				}
				
				//cout<<"Distance To Line is: "<<distanceLine<<endl;
				
				dis_to_ref = sqrt(pow(tmp_N_Offset1,2) + pow(tmp_E_Offset1,2));
				
										
				if (dis_to_ref >= DSRC_Range[N_App-1] || distanceLine >= 8)  //Outside the GeoFence Area
				{
					
					trackedveh.Data().req_phase=-1;
					trackedveh.Data().ETA=9999;
					trackedveh.Data().stopBarDistance=9999;
					//cout<<"Vehicle ID of "<<trackedveh.Data().TempID<<" is outside the boundaries of GeoFence"<<endl;
					//!!! Test : remove later
					//sprintf(temp_log,"Vehicle No. %d is outside the boundaries of GeoFence with dis_to_ref %f and DistanceLine of %f \n",trackedveh.Data().TempID,dis_to_ref,distanceLine);
					//cout<<temp_log;
					//outputlog(temp_log);
				}
				else    
				{
					
					trackedveh.Data().leaving_time=GetSeconds();
					
					int pre_app = trackedveh.Data().previous_approach;
					int pre_lane = trackedveh.Data().previous_lane;
					
					//sprintf(temp_log,"!!!!!!!!!!!!!!!!!!!!!!! Veh ID %d Approach %d Lane %d trackedveh.Data().req_phase %d dis_to_ref %f\n",trackedveh.Data().TempID,T_approach,T_lane,trackedveh.Data().req_phase, dis_to_ref);
					//cout<<temp_log;outputlog(temp_log);
					
					if(trackedveh.Data().req_phase < 0 && dis_to_ref < 30) // if the vehicle passes the stopbar, set the approach and lane to be previous approach and lane
					{
						
						//sprintf(temp_log,"BEFORE Veh ID %d Approach %d Lane %d trackedveh.Data().req_phase %d dis_to_ref %f is within the middle of the intersection!\n",trackedveh.Data().TempID,T_approach,T_lane,trackedveh.Data().req_phase, dis_to_ref);
						//cout<<temp_log;outputlog(temp_log);
						
						trackedveh.Data().approach=trackedveh.Data().previous_approach; //trackedveh.Data().approach = pre_app;
						trackedveh.Data().lane=trackedveh.Data().previous_lane; //trackedveh.Data().lane = pre_lane;
						
						//trackedveh.Data().active_flag =5;
						
						//!!! For offset refiner
						//trackedveh.Data().offsetRefiner_flag=2;
						
						//sprintf(temp_log,"AFTER Vehicle No. %d Approach %d Lane %d trackedveh.Data().req_phase %d dis_to_ref %f is within the middle of the intersection!\n",trackedveh.Data().TempID,trackedveh.Data().approach,trackedveh.Data().lane,trackedveh.Data().req_phase, dis_to_ref);
						//cout<<temp_log;
						//outputlog(temp_log);
					}
					else //if(trackedveh.Data().req_phase >= 0 || dis_to_ref >= 35) //Inside the GeoFence but not within the intersection
					{
						//sprintf(temp_log,"BEFORE Veh ID %d Approach %d Lane %d Req_Phase %d trackedveh.Data().req_phase %d dis_to_ref %f\n",trackedveh.Data().TempID,T_approach,T_lane,requested_phase,trackedveh.Data().req_phase, dis_to_ref);
						//cout<<temp_log;outputlog(temp_log);
						
						
						FindVehInMap(tmp_speed,tmp_heading,tmp_nFrame,tmp_N_Offset1,tmp_E_Offset1,tmp_N_Offset2,tmp_E_Offset2,NewMap,pre_app,Dis_curr,ETA,requested_phase,previous_requested_phase,T_approach,T_lane);		
						trackedveh.Data().req_phase=requested_phase;
						trackedveh.Data().pre_requested_phase=previous_requested_phase;
						trackedveh.Data().ETA=ETA;
						trackedveh.Data().stopBarDistance=Dis_curr;
						trackedveh.Data().approach=T_approach;
						trackedveh.Data().lane=T_lane;
						trackedveh.Data().processed=1;   //already go through the find vehicle in the mAP function
						trackedveh.Data().active_flag =14;
						
						
						//if   ( (trackedveh.Data().req_phase==2 || trackedveh.Data().req_phase==6) && ((trackedveh.Data().stopBarDistance < 100 && trackedveh.Data().stopBarDistance > 0 && trackedveh.Data().Speed < 3) || (trackedveh.Data().stopBarDistance < 30 && trackedveh.Data().stopBarDistance > 0 )) ) 
						//{
						//	trackedveh.Data().offsetRefiner_flag=3;
						//}
						
						
						if(trackedveh.Data().req_phase < 0 && dis_to_ref < 30) //For the first times after passimg the stop bar ignore what FindVehInMap did, because it was not correct!
						{
							trackedveh.Data().approach=trackedveh.Data().previous_approach;
							trackedveh.Data().lane=trackedveh.Data().previous_lane;
							
							//!!! For offset refiner
							//trackedveh.Data().offsetRefiner_flag=2;
							
						}
						
						  
						//sprintf(temp_log,"Vehicle No. %d Approach %d Lane %d req_phase %d veh_type %d\n",trackedveh.Data().TempID,T_approach,T_lane,trackedveh.Data().req_phase, trackedveh.Data().VehType);
						//cout<<temp_log; outputlog(temp_log);
						
						if(trackedveh.Data().temp_approach != trackedveh.Data().approach)
						{   
												
							trackedveh.Data().previous_approach=trackedveh.Data().temp_approach;
							trackedveh.Data().temp_approach=trackedveh.Data().approach;
							
							if(trackedveh.Data().previous_approach == 0)
								trackedveh.Data().previous_approach = T_approach;
								
							//----------------------------Dealing with U-Turn Movements------------------------------
							//!!! B.H : Not necessary for peer priority control
							/*
							if (trackedveh.Data().previous_approach != 0)  
							{
								if (trackedveh.Data().approach%2==1) //U-turn after passing the intersection 
								{
									if (trackedveh.Data().previous_approach==trackedveh.Data().approach+1)
									 {
																				
										//cout<<"Vehicle ID of "<<trackedveh.Data().TempID<<" is Deleted!"<<endl;
										trackedveh.DeleteAt();
														  
									 }
								}
								else //U-turn at the signal
								{
									if (trackedveh.Data().previous_approach==trackedveh.Data().approach-1)
									{
																				
										//cout<<"Vehicle ID of "<<trackedveh.Data().TempID<<" is Deleted!"<<endl;
										trackedveh.DeleteAt();
								
									  
									}
								}
							}
							*/ 
								
						}  
						
						if(trackedveh.Data().temp_lane != trackedveh.Data().lane)
						{   
							trackedveh.Data().previous_lane=trackedveh.Data().temp_lane;
							trackedveh.Data().temp_lane=trackedveh.Data().lane;
							
							if(trackedveh.Data().previous_lane == 0)
								trackedveh.Data().previous_lane = T_lane;
						}	
						
					}
					
					//!!! B.H : To find out whether the transit passes the far side but stop
					if ((flag_busStopEast == 1) && (trackedveh.Data().approach == curr_approach_long[0]))  //eastbound
					{
									
						//ref_ele =1292;
						//first point
						geoPoint.lla2ecef(long_busStop_east[0],lat_busStop_east[0],ref_ele,&ecef_x,&ecef_y,&ecef_z);
						geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x_bus[0],&local_y_bus[0],&local_z_bus[0]);
						//second point
						geoPoint.lla2ecef(long_busStop_east[1],lat_busStop_east[1],ref_ele,&ecef_x,&ecef_y,&ecef_z);
						geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x_bus[1],&local_y_bus[1],&local_z_bus[1]);
						
						DistanceFromLine(local_x, local_y, local_x_bus[0], local_y_bus[0], local_x_bus[1], local_y_bus[1], distanceSegment, distanceLine, temp_R, temp_S);
			
												
						if (temp_R > 0.0)  //!!! if ((temp_R > 1.0) && (Dis_bus <= 50))
						{
													
							trackedveh.Data().busStop_S = 1;
							//sprintf(temp_log,"Test eastbound ID : %d busStop_S : %d temp_R : %lf temp_S : %lf\n", trackedveh.Data().TempID,trackedveh.Data().busStop_S, temp_R, temp_S);
							//outputlog(temp_log); //cout<<temp_log;
						
						}	
					}
					
					if ((flag_busStopWest == 1) && (trackedveh.Data().approach == curr_approach_long[1]))   //westbound
					{
						//ref_ele =1292;
						//first point
						geoPoint.lla2ecef(long_busStop_west[0],lat_busStop_west[0],ref_ele,&ecef_x,&ecef_y,&ecef_z);
						geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x_bus[0],&local_y_bus[0],&local_z_bus[0]);
						//second point
						geoPoint.lla2ecef(long_busStop_west[1],lat_busStop_west[1],ref_ele,&ecef_x,&ecef_y,&ecef_z);
						geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x_bus[1],&local_y_bus[1],&local_z_bus[1]);
						
						DistanceFromLine(local_x, local_y, local_x_bus[0], local_y_bus[0], local_x_bus[1], local_y_bus[1], distanceSegment, distanceLine, temp_R, temp_S);
						
						//Dis_bus = sqrt(pow((local_x - local_x_bus[1]),2) + pow((local_y - local_y_bus[1]),2));
						if (temp_R > 0.0)  //!!! ((temp_R > 1.0) && (Dis_bus <= 50))
						{
							trackedveh.Data().busStop_S = 1;
							//sprintf(temp_log,"Test westbound ID : %d busStop_S : %d temp_R : %lf temp_S : %lf \n", trackedveh.Data().TempID,trackedveh.Data().busStop_S, temp_R, temp_S);
							//outputlog(temp_log); //cout<<temp_log;
						}
					}
					
				}
						
			}
			else
			{
				sprintf(temp_log,"Not valid GPS points !\n");
				outputlog(temp_log); cout<<temp_log;
			}
				
			pro_flag=0;
			
			//t_2=GetSeconds();	
			//sprintf(temp_log,"The time for Processing BSM and Mapping for vehicle is: %lf second \n",t_2-t_1);
			//outputlog(temp_log); cout<<temp_log;
			
			//t_2=GetSeconds();	
			//sprintf(temp_log,"The time for locating the vehicle to the MAP is: %lf second \n",t_2-t_1);
			//outputlog(temp_log); cout<<temp_log;
		
		}
	}
		
		//go through the tracked list to create the arrival table every rolling-horizon time
		currenttime=time(NULL);
		//cout<<"current time is: "<<currenttime<<endl;
		
		int difference=currenttime-begintime;
		
		//sprintf(temp_log,"The Difference is %d\n",difference);
		//outputlog(temp_log); cout<<temp_log;
		
		if (difference>=rolling_horizon)
		{	
		
			//t_1=GetSeconds();
		    
		    	
			//go throught the vehicle list and delete the already left vehicle
			trackedveh.Reset();
			while(!trackedveh.EndOfList())  //match vehicle according to vehicle ID
			{
							
				trackedveh.Data().active_flag--;
				//sprintf(temp_log, "The active_flag of vehicle %d is %d\n",trackedveh.Data().TempID,trackedveh.Data().active_flag);
				//outputlog(temp_log); cout<<temp_log;
				
				if (trackedveh.Data().active_flag<-5)
				{
					//first write the vehicle trajectory to a file
					
					//then delete the vehicle
					sprintf(temp_log,"Delete Vehicle No. %d at %lf \n",trackedveh.Data().TempID, GetSeconds());
					outputlog(temp_log); cout<<temp_log;
					trackedveh.DeleteAt();
				}
				trackedveh.Next();
			}
		
		
			//reset begintime;
			begintime=currenttime;
			//clear current arrival table
			//~ for (i=0;i<121;i++)
				//~ for(j=0;j<8;j++)
				//~ {
					//~ ArrivalTable[i][j]=0;
				//~ }
			//~ 
			//~ //construct the arrival table
			//~ trackedveh.Reset();
				//~ while(!trackedveh.EndOfList())
				//~ {
					//~ if(trackedveh.Data().req_phase>0)  //only vehicle request phase, then record the arrival time
					//~ {
						//~ int att=(int) floor(trackedveh.Data().ETA+0.5);
						//~ ArrivalTable[att][trackedveh.Data().req_phase-1]++;
					//~ }
				//~ trackedveh.Next();
				//~ }
				//~ 
			//~ //write arrival table to the file
			//~ fstream arr;
			//~ arr.open(arrivaltablefile,ios::out)	;	
			//~ 
			//~ for (i=0;i<121;i++)
			//~ {
				//~ for(j=0;j<7;j++)
				//~ {
					//~ arr<<ArrivalTable[i][j]<<" ";
				//~ }
					//~ arr<<ArrivalTable[i][7];
				//~ if(i<120)
				//~ arr<<endl;
			//~ }
			//~ arr.close();
		}
	//}  //end of processing BSM
	
	
	
	//Push Trajectory data when requested
	if (traj_requested==1)
	{
		
		traj_requested=0;
		
		//trajectory request from performance observer
		if(strcmp(buf,"Request Trajectory from performance observer")==0)
		{
			cout<<"Receive Request from Performance Observer!"<<endl;
			//Set-up receive address as local host
			recvaddr.sin_family = AF_INET;
			recvaddr.sin_port = htons(22222);  //to performance observer the port is 22222!!!!!!!!!!!!!!!!!!!!!!!!!!
			recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1") ; //Send to local host;
			memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
			
			//Pack the trajectory data to a octet stream
			byte tmp_traj_data[500000];  // 500k is the maximum can be sent
			//Pack the data from trackedveh to a octet string
			t_1=GetSeconds();
			PackTrajData(tmp_traj_data,traj_data_size);
			t_2=GetSeconds();
			
			cout<<"Pack time is: "<<t_2-t_1<<endl;
			
			char* traj_data;
			traj_data= new char[traj_data_size];
			
			for(i=0;i<traj_data_size;i++)
			traj_data[i]=tmp_traj_data[i];
			//Send trajectory data
			
			t_1=GetSeconds();
			numbytes = sendto(sockfd,traj_data,traj_data_size+1 , 0,(struct sockaddr *)&recvaddr, addr_length);
			
			t_2=GetSeconds();
			
			sprintf(temp_log,"Send trajectory data to Performance Observer! The size is %d and sending time is %lf. \n",traj_data_size,t_2-t_1);
			outputlog(temp_log); cout<<temp_log;
			
			delete[] traj_data;
		}
		
		//trajectory request from signal control
		if(strcmp(buf,"Request Trajectory from signal control")==0)
		{
			//Set-up receive address as local host
			recvaddr.sin_family = AF_INET;
			recvaddr.sin_port = htons(33333);  //to signal control the port is 33333!!!!!!!!!!!!!!!!!!!!!!!!!!
			recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1") ; //Send to local host;
			memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
			
			//Pack the trajectory data to a octet stream
			byte tmp_traj_data[500000];  // 500k is the maximum can be sent
			//Pack the data from trackedveh to a octet string
			t_1=GetSeconds();
			PackTrajData1(tmp_traj_data,traj_data_size);
			t_2=GetSeconds();
			
			cout<<"Pack time is: "<<t_2-t_1<<endl;
			
			char* traj_data;
			traj_data= new char[traj_data_size];
			
			for(i=0;i<traj_data_size;i++)
			traj_data[i]=tmp_traj_data[i];
			//Send trajectory data
			
			t_1=GetSeconds();
			numbytes = sendto(sockfd,traj_data,traj_data_size+1 , 0,(struct sockaddr *)&recvaddr, addr_length);
			
			t_2=GetSeconds();
			
			//print out sent data
			trackedveh.Reset();
			while(!trackedveh.EndOfList())
			{
			sprintf(temp_log,"%d %lf %lf %lf %d %d %d %.2f %d\n",trackedveh.Data().TempID,trackedveh.Data().Speed,trackedveh.Data().stopBarDistance,trackedveh.Data().acceleration,trackedveh.Data().approach,trackedveh.Data().lane,trackedveh.Data().req_phase,trackedveh.Data().time_stop,trackedveh.Data().processed);
			//outputlog(temp_log); //cout<<temp_log;
			trackedveh.Next();	
			}
			
			
			sprintf(temp_log,"Send trajectory data to Signal Control! The size is %d and sending time is %lf. \n",traj_data_size,t_2-t_1);
			//outputlog(temp_log); cout<<temp_log;
			
			delete[] traj_data;
		}
		
		//trajectory request from coordinator
		if(strcmp(buf,"Request Trajectory from coordinator")==0)
		{
			//Set-up receive address as local host
			recvaddr.sin_family = AF_INET;
			recvaddr.sin_port = htons(66666);  //to signal control the port is 66666!!!!!!!!!!!!!!!!!!!!!!!!!!
			recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1") ; //Send to local host;
			memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
			
			//Pack the trajectory data to a octet stream
			byte tmp_traj_data[500000];  // 500k is the maximum can be sent
			//Pack the data from trackedveh to a octet string
			t_1=GetSeconds();
			PackTrajData2(tmp_traj_data,traj_data_size);
			t_2=GetSeconds();
			
			cout<<"Pack time is: "<<t_2-t_1<<endl;
			
			char* traj_data;
			traj_data= new char[traj_data_size];
			
			for(i=0;i<traj_data_size;i++)
			traj_data[i]=tmp_traj_data[i];
			//Send trajectory data
			
			t_1=GetSeconds();
			numbytes = sendto(sockfd,traj_data,traj_data_size+1 , 0,(struct sockaddr *)&recvaddr, addr_length);
			
			t_2=GetSeconds();
			
			//!!!!!!!!!!TEST
			//sprintf(temp_log,"Size 1 of buf : %d  Size 2 of buf : %d \n", sizeof(traj_data), traj_data_size); 
			//outputlog(temp_log);
					
			//for( int i = 0; i < traj_data_size; i++ )
			//{
			//	sprintf(temp_log,"%x ",traj_data[i]); outputlog(temp_log);
			//}
			
			
			
			//print out sent data
			//trackedveh.Reset();
			//while(!trackedveh.EndOfList())
			//{
			//	sprintf(temp_log,"sent to coordinator %d %lf %lf %d \n",trackedveh.Data().TempID,trackedveh.Data().Speed,trackedveh.Data().stopBarDistance,trackedveh.Data().req_phase);
			//	outputlog(temp_log); cout<<temp_log;
			//	trackedveh.Next();	
			//}
			
			
			sprintf(temp_log,"Send trajectory data to coordinator!! The size is %d and sending time is %lf. \n",traj_data_size,t_2-t_1);
			outputlog(temp_log); cout<<temp_log;
			
			delete[] traj_data;
		}
		
		//trajectory request from Long Term Planning
		if(strcmp(buf,"Request Trajectory from Long_Term_Planning")==0)
		{	
			sprintf(temp_log,"Received request from LTP \n");
			outputlog(temp_log); cout<<temp_log;
			
			//Set-up receive address as local host
			recvaddr.sin_family = AF_INET;
			recvaddr.sin_port = htons(99999);  //to signal control the port is 66666!!!!!!!!!!!!!!!!!!!!!!!!!!
			recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1") ; //Send to the first intersection because SAC is located in there;
			memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
			
			//Pack the trajectory data to a octet stream
			byte tmp_traj_data[500000];  // 500k is the maximum can be sent
			//Pack the data from trackedveh to a octet string
			t_1=GetSeconds();
			PackTrajData3(tmp_traj_data,traj_data_size);
			t_2=GetSeconds();
			
			cout<<"Pack time is: "<<t_2-t_1<<endl;
			
			char* traj_data;
			traj_data= new char[traj_data_size];
			
			for(i=0;i<traj_data_size;i++)
			traj_data[i]=tmp_traj_data[i];
			//Send trajectory data
			
			//t_1=GetSeconds();
			numbytes = sendto(sockfd,traj_data,traj_data_size+1 , 0,(struct sockaddr *)&recvaddr, addr_length);
			
			//t_2=GetSeconds();
			
			//sprintf(temp_log,"Send trajectory data to LPT!! The size is %d and sending time is %lf. \n",traj_data_size,t_2-t_1);
			//outputlog(temp_log); //cout<<temp_log;
			
			delete[] traj_data;
		}
		
		
		
		
		
	}
	
	
	//!!!! Byungho : for smoothness !!!!!!!!!!!!!!
	/*
	smooth_timer2=time(NULL); 
	if ((smooth_timer2-smooth_timer1)>2)
	{
	    smooth_timer1=smooth_timer2;	
		trackedveh.Reset();
		while(!trackedveh.EndOfList())
		{
			if (trackedveh.Data().pre_requested_phase == 2 || trackedveh.Data().pre_requested_phase == 6 )
			{
				sprintf(temp_log,"%d %lf %lf %lf %lf %lf %d %d %d %.2f %d\n",trackedveh.Data().TempID,trackedveh.Data().Speed,trackedveh.Data().stopBarDistance,trackedveh.Data().entry_time, trackedveh.Data().leaving_time, trackedveh.Data().acceleration,trackedveh.Data().approach,trackedveh.Data().lane,trackedveh.Data().pre_requested_phase,trackedveh.Data().time_stop,trackedveh.Data().processed);
				outputlog(temp_log); //cout<<temp_log;
			}
			trackedveh.Next();	
		}
	}
	*/
	
	}// End of while(true)
	//*/
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
	if (endPos <100000000)
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



void FindVehInMap(double Speed, double Heading,int nFrame,double N_Offset1,double E_Offset1,double N_Offset2,double E_Offset2, MAP NewMap, int pre_app, double &Dis_curr, double &est_TT, int &request_phase,int &previous_requested_phase, int &approach, int &lane)
{
            int i,j,k;
			//Find Vehicle position in the map
            //find the nearest Lane nodes from the vehicle location
			int t_App,t_Lane,t_Node;
			int t_pos; //node position in the vector
			double lane_heading;
			//temp vehicle point
			
			//calculate the vehicle in the MAP
			//find lane, requesting phase and distance 
				double tempdis=1000000.0;
				//find the nearest lane node
				for(j=0;j<MAP_Nodes.size();j++)
				{
					double dis=sqrt((local_x-MAP_Nodes[j].N_Offset)*(local_x-MAP_Nodes[j].N_Offset)+(local_y-MAP_Nodes[j].E_Offset)*(local_y-MAP_Nodes[j].E_Offset));
					if (dis<tempdis)
					{
						tempdis=dis;
						t_App=MAP_Nodes[j].index.Approach;
						t_Lane=MAP_Nodes[j].index.Lane;
						t_Node=MAP_Nodes[j].index.Node;
						t_pos=j;
					}
				}

				approach=t_App;
				lane=t_Lane;
				//trackedveh.Reset(Veh_pos_inlist);
				//sprintf(temp_log,"The nearest node is: %d %d %d \n",t_App,t_Lane,t_Node);
				//outputlog(temp_log); 
				//cout<<temp_log;
				//sprintf(temp_log,"local_x is: %f local_y is:%f \n",local_x,local_y);
				//outputlog(temp_log); cout<<temp_log;
				
				
				if(nFrame>=2) //start from second frame
			{
				// determine it is approaching the intersection or leaving the intersection or in queue
				// The threshold for determing in queue: 89.4 cm = 2mph
				//calculate the distance from the reference point here is 0,0;
				int veh_state; 		// 1: appraoching; 2: leaving; 3: queue
				double N_Pos;  //current vehicle position
				double E_Pos;
				double E_Pos2; //previous vehicle position
				double N_Pos2;

				int match=0;  //whether the vehicle's heading match the lanes heading  

				//find the first node (nearest of intersection) of the lane
				double inter_pos_N=MAP_Nodes[t_pos-t_Node+1].N_Offset;
				double inter_pos_E=MAP_Nodes[t_pos-t_Node+1].E_Offset;


				N_Pos=N_Offset1;//current position
				E_Pos=E_Offset1;
				N_Pos2=N_Offset2;//previous frame position
				E_Pos2=E_Offset2;

				//double veh_heading=atan2(N_Pos-N_Pos2,E_Pos-E_Pos2)*180/PI;
				double veh_heading=Heading;
				if (veh_heading<0)
					veh_heading+=360;
					
					
				lane_heading=MAP_Nodes[t_pos].Heading;
					
				//~ //calculate lane heading
				//~ if (t_Node>1)  //not the nearest node
				//~ {
					//~ double app_node_N=MAP_Nodes[t_pos-1].N_Offset; //the node to the intersection has smaller number
					//~ double app_node_E=MAP_Nodes[t_pos-1].E_Offset;
					//~ lane_heading=atan2(MAP_Nodes[t_pos-1].N_Offset-MAP_Nodes[t_pos].N_Offset,MAP_Nodes[t_pos-1].E_Offset-MAP_Nodes[t_pos].E_Offset)*180.0/PI;
					//~ lane_heading=90.0-lane_heading; //Changed by YF 6/25/2014: Heading 0 degree changed to north
				//~ }
				//~ else  //t_Node=1  //already the nearest node
				//~ {
					//~ double app_node_N=MAP_Nodes[t_pos+1].N_Offset; //the adjacent node has larger number
					//~ double app_node_E=MAP_Nodes[t_pos+1].E_Offset;
					//~ lane_heading=atan2(MAP_Nodes[t_pos].N_Offset-MAP_Nodes[t_pos+1].N_Offset,MAP_Nodes[t_pos].E_Offset-MAP_Nodes[t_pos+1].E_Offset)*180.0/PI;
					//~ lane_heading=90.0-lane_heading; //Changed by YF 6/25/2014: Heading 0 degree changed to north
				//~ }
				
				
				if(lane_heading<0)
				lane_heading+=360;
				
				
				if( abs(veh_heading - lane_heading)> 120 && abs(veh_heading - lane_heading) <240)
				{
					if(approach%2 == 1) //When the approach is 1, 3, 5,or 7
						approach = approach +1;
					else //When the approach number is 2, 4, 6,or 8
						approach = approach -1;
				}
							
				//cout<<"Lane Heading is: "<<lane_heading<<endl;
				//cout<<"Vehicle Heading is: "<<veh_heading<<endl;
				
				
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
				
				//else
				//approach = pre_app; //if the heading of the vehicle converges a lot from the lane heading (in the middle of the intersection) consider the approach to be the previous approach.
				

				

 
				
				double Dis_pre= sqrt((N_Pos2-inter_pos_N)*(N_Pos2-inter_pos_N)+(E_Pos2-inter_pos_E)*(E_Pos2-inter_pos_E));
				Dis_curr=sqrt((N_Pos-inter_pos_N)*(N_Pos-inter_pos_N)+(E_Pos-inter_pos_E)*(E_Pos-inter_pos_E));
				
				
				if(match==1 && approach%2==1)   //odd approach: igress approaches
				{
					if (fabs(Dis_pre-Dis_curr)<0.894/10)  //unit m/0.1s =2mph //Veh in queue is also approaching the intersection, leaving no queue
					{
						veh_state=3;  //in queue
					}
					if (fabs(Dis_pre-Dis_curr)>=0.894/10)
					{
						if (Dis_curr<Dis_pre)
							veh_state=1;  //approaching
						else
						{
							veh_state=2;  //leaving
							request_phase=-1;  //if leaving, no requested phase
						}
					}
				}
				if(match==1 && approach%2==0)   //even approach: engress approaches
				{
					veh_state=2;  //leaving
					request_phase=-1;
				}
				
				if (match==0)
				{
					veh_state=4;   // NOT IN THE MAP
					request_phase=-1;  
				}
				
				
				//cout<<"Veh status is: "<<veh_state<<endl;
				
				
				
				
				
				//~ if (fabs(Dis_pre-Dis_curr)<0.894/10 && match==1)  //unit m/0.1s =2mph //Veh in queue is also approaching the intersection, leaving no queue
				//~ {
					//~ veh_state=3;  //in queue
				//~ }
				//~ if (fabs(Dis_pre-Dis_curr)>=0.894/10 && match==1)
				//~ {
					//~ if (Dis_curr<Dis_pre)
						//~ veh_state=1;  //approaching
					//~ else
					//~ {
						//~ veh_state=2;  //leaving
						//~ request_phase=-1;  //if leaving, no requested phase
					//~ }
				//~ }
				//~ if (match==0)
				//~ {
					//~ veh_state=4;
					//~ request_phase=-1;  //if leaving, no requested phase
				//~ }
				
				//sprintf(temp_log,"Veh State %d ",veh_state);
				//outputlog(temp_log); cout<<temp_log;

				//cout<<" Veh State is "<<veh_state<<endl;

				if (veh_state==1 || veh_state==3) //only vehicle approaching intersection need to do something
				{
						request_phase=LaneNode_Phase_Mapping[t_App][t_Lane][t_Node];
						previous_requested_phase= LaneNode_Phase_Mapping[t_App][t_Lane][t_Node];
					//sprintf(temp_log,"Request Phase is %d \n",request_phase);
					//outputlog(temp_log); cout<<temp_log;
					//cout<<"Request Phase is "<<request_phase;
					//cout<<" Current distance to stopbar is "<<Dis_curr;
					//calculate estimated travel time to stop bar
					if(Speed<1)
						est_TT=0;    //if the vehicle is in queue, assume ETA is 0
					else
						est_TT=Dis_curr/Speed;
					if(est_TT>9999)
						est_TT=9999;
					//sprintf(temp_log,"Dis_Stop_bar %f ETA %f Req_Phase %d appr %d lane %d\n \n",Dis_curr,est_TT,request_phase,approach,lane);
					//outputlog(temp_log); //cout<<temp_log;
					
					//cout<<" TT to stop bar is "<<est_TT<<endl;

				}
				else //Veh in State 2 or 4
				{
					est_TT=99999;
					Dis_curr=99999;
					previous_requested_phase= LaneNode_Phase_Mapping[t_App][t_Lane][t_Node];
				}

			}
}


double GetSeconds()
{
	struct timeval tv_tt;
	gettimeofday(&tv_tt, NULL);
	return (tv_tt.tv_sec+tv_tt.tv_usec/1.0e6);    
}

void PackTrajData(byte* tmp_traj_data,int &size)
{
	int i,j;
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    byte    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
    double  temp;
	//header 2 bytes
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x01 for trajectory data send to performance observer
	tmp_traj_data[offset]=0x01;
	offset+=1;
	//No. of Vehicles in the list
	int temp_veh_No=0;
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if(trackedveh.Data().processed==1)
			temp_veh_No++;
		trackedveh.Next();
	}
	
	tempUShort = (unsigned short)temp_veh_No;
	
	cout<<"send list number is: "<<tempUShort<<endl;
	cout<<"total list number is: "<<trackedveh.ListSize()<<endl;
	
	pByte = (byte* ) &tempUShort;
    tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
    tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	//for each vehicle
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if (trackedveh.Data().processed==1)
		{
			//vehicle temporary id
			tempUShort = (unsigned short)trackedveh.Data().TempID;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//the number of trajectory points: nFrame
			tempUShort = (unsigned short)trackedveh.Data().nFrame;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//Speed of the vehicle
			tempLong = (long)(trackedveh.Data().Speed* DEG2ASNunits); // to 1/10th micro degees units
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			
			//entry time
			tempLong = (long)(trackedveh.Data().entry_time); 
				pByte = (byte* ) &tempLong;
				tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
				tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
				tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
				tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
				offset = offset + 4;
			//leaving time
			tempLong = (long)(trackedveh.Data().leaving_time); 
				pByte = (byte* ) &tempLong;
				tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
				tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
				tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
				tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
				offset = offset + 4;
			
			//Added by Shayan for PerformanceObserver 08122014
				
			//vehicle approach
			tempUShort = (unsigned short)trackedveh.Data().approach;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//vehicle lane
			tempUShort = (unsigned short)trackedveh.Data().lane;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//Distance To Stop Bar
			tempLong = (long)(trackedveh.Data().stopBarDistance* DEG2ASNunits); // to 1/10th micro degees units
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			
			//vehicle stop_flag
			tempUShort = (unsigned short)trackedveh.Data().stop_flag;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//vehicle queue_flag
			tempUShort = (unsigned short)trackedveh.Data().queue_flag;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//Time when vehicle stops
			tempLong = (long)(trackedveh.Data().time_stop); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			
			//Previous approach of vehicle
			tempUShort = (unsigned short)trackedveh.Data().previous_approach;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//Previous lane of vehicle
			tempUShort = (unsigned short)trackedveh.Data().previous_lane;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//do each trajectory point
			//this following order: latitude,longitude,N_Offset,E_Offset
			for(j=trackedveh.Data().nFrame-2;j<trackedveh.Data().nFrame;j++)
			{
				/*
				//latitude
				tempLong = (long)(trackedveh.Data().traj[j].latitude * DEG2ASNunits); 
				pByte = (byte* ) &tempLong;
				tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
				tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
				tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
				tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
				offset = offset + 4;
				//longitude
				tempLong = (long)(trackedveh.Data().traj[j].longitude * DEG2ASNunits); // to 1/10th micro degees units
				pByte = (byte* ) &tempLong;
				tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
				tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
				tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
				tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
				offset = offset + 4;
				*/
				//N_Offset
				tempLong = (long)(trackedveh.Data().N_Offset[j]* DEG2ASNunits); // to 1/10th micro degees units
				pByte = (byte* ) &tempLong;
				tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
				tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
				tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
				tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
				offset = offset + 4;
				//E_Offset
				tempLong = (long)(trackedveh.Data().E_Offset[j] * DEG2ASNunits); // to 1/10th micro degees units
				pByte = (byte* ) &tempLong;
				tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
				tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
				tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
				tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
				offset = offset + 4;
				//cout<<"East offset "<<j<<" :"<<trackedveh.Data().E_Offset[j]<<endl;
			}
		}
	trackedveh.Next();	
	}
	size=offset;
}


void PackTrajData1(byte* tmp_traj_data,int &size)
{
	int i,j;
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    byte    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
    double  temp;
	//header 2 bytes
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x01 for trajectory data send to traffic control
	tmp_traj_data[offset]=0x02;
	offset+=1;
	//No. of Vehicles in the list
	int temp_veh_No=0;
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if(trackedveh.Data().processed==1)
			temp_veh_No++;
		trackedveh.Next();
	}
	
	tempUShort = (unsigned short)temp_veh_No;
	
	cout<<"send list number is: "<<tempUShort<<endl;
	cout<<"total list number is: "<<trackedveh.ListSize()<<endl;
	
	pByte = (byte* ) &tempUShort;
    tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
    tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	//for each vehicle
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if (trackedveh.Data().processed==1)
		{
			//vehicle temporary id
			tempUShort = (unsigned short)trackedveh.Data().TempID;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;

			//vehicle current speed
			tempLong = (long)(trackedveh.Data().Speed * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			//vehicle current distance to stop bar
			tempLong = (long)(trackedveh.Data().stopBarDistance * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			//vehicle current acceleration
			tempLong = (long)(trackedveh.Data().acceleration * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			//approach
			tempByte = (byte)trackedveh.Data().approach; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item
			//lane
			tempByte = (byte)trackedveh.Data().lane; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item
			//req_phase
			tempByte = (byte)trackedveh.Data().req_phase; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item
			//vehicle first stopped time (absolute time)
			tempLong = (long)(trackedveh.Data().time_stop); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;	
			//vehicle stop_flag
			tempByte = (byte)trackedveh.Data().stop_flag; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item	
		}		
		trackedveh.Next();	
	}
	size=offset;
}

//!!!! for coordinator and offset refiner 
void PackTrajData2(byte* tmp_traj_data,int &size)
{
	int i,j;
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    byte    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
    double  temp;
	//header 2 bytes
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x01 for trajectory data send to traffic control
	tmp_traj_data[offset]=0x02;
	offset+=1;
	//No. of Vehicles in the list
	int temp_veh_No=0;
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if(trackedveh.Data().processed==1)
			temp_veh_No++;
		trackedveh.Next();
	}
	
	tempUShort = (unsigned short)temp_veh_No;
	
	cout<<"send list number is: "<<tempUShort<<endl;
	cout<<"total list number is: "<<trackedveh.ListSize()<<endl;
	
	pByte = (byte* ) &tempUShort;
    tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
    tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	
	//for each vehicle
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if (trackedveh.Data().processed==1)
		{
			//vehicle temporary id
			tempUShort = (unsigned short)trackedveh.Data().TempID;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;

			//vehicle current speed
			tempLong = (long)(trackedveh.Data().Speed * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			//vehicle current distance to stop bar
			tempLong = (long)(trackedveh.Data().stopBarDistance * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			//vehicle current acceleration
			tempLong = (long)(trackedveh.Data().acceleration * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			//approach
			tempByte = (byte)trackedveh.Data().approach; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item
			//lane
			tempByte = (byte)trackedveh.Data().lane; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item
			//req_phase
			tempByte = (byte)trackedveh.Data().req_phase; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item
			//vehicle first stopped time (absolute time)
			tempLong = (long)(trackedveh.Data().time_stop); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;	
			
			//vehicle offset refiner flag
			//tempUShort = (unsigned short)trackedveh.Data().offsetRefiner_flag;
			//pByte = (byte* ) &tempUShort;
			//tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			//tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			//offset = offset + 2;
			
			//vehicle stop_flag
			tempUShort = (unsigned short)trackedveh.Data().stop_flag;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			
		}		
		trackedveh.Next();	
	}
	size=offset;
}

//!!!! for LTP
void PackTrajData3(byte* tmp_traj_data,int &size)
{
	int i,j;
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    byte    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
    double  temp;
	//header 2 bytes
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x01 for trajectory data send to traffic control
	tmp_traj_data[offset]=0x02;
	offset+=1;
	//No. of Vehicles in the list
	int temp_veh_No=0;
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if(trackedveh.Data().processed==1)
			temp_veh_No++;
		trackedveh.Next();
	}
	
	tempUShort = (unsigned short)temp_veh_No;
	
	cout<<"send list number is: "<<tempUShort<<endl;
	cout<<"total list number is: "<<trackedveh.ListSize()<<endl;
	
	pByte = (byte* ) &tempUShort;
    tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
    tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	//for each vehicle
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if (trackedveh.Data().processed==1)
		{
			//vehicle temporary id
			tempUShort = (unsigned short)trackedveh.Data().TempID;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
            
            //vehicle current speed
			tempLong = (long)(trackedveh.Data().Speed * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
            
			//vehicle current distance to stop bar
			tempLong = (long)(trackedveh.Data().stopBarDistance * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			
			//vehicle current acceleration
			tempLong = (long)(trackedveh.Data().acceleration * DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			
			//req_phase
			tempByte = (byte)trackedveh.Data().req_phase; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item
			
			//Previous approach of vehicle
			tempUShort = (unsigned short)trackedveh.Data().previous_approach;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//vehicle approach
			tempUShort = (unsigned short)trackedveh.Data().approach;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//parameter r for bus stop
			tempUShort = (unsigned short)trackedveh.Data().busStop_S;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
		}		
		trackedveh.Next();	
	}
	size=offset;
}




void get_map_name() //Yiheng add 07/18/2014
{
    fstream fs;

    fs.open(MAP_File_Name);

    char temp[128];

    getline(fs,nmap_name);

    if(nmap_name.size()!=0)
    {
        //std::cout<< "Current Vehicle ID:" << RSUID <<std::endl;
        sprintf(temp,"Current MAP is %s\n",nmap_name.c_str());
        cout<<temp<<endl;
        outputlog(temp);
    }
    else
    {
        sprintf(temp,"Reading MAP_File problem");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }

    fs.close();
}


void get_lane_phase_mapping() //Yiheng add 07/18/2014
{
    fstream fs;

    fs.open(Lane_Phase_Mapping_File_Name);

    char temp[128];
	
	string temp_string;

    getline(fs,temp_string);  //First line is comment
	getline(fs,temp_string);  //Second line contains information


    if(temp_string.size()!=0)
    {
		char tmp[128];
		strcpy(tmp,temp_string.c_str());		
		sscanf(tmp,"%d %d %d %d %d %d %d %d",&phase_mapping[0],&phase_mapping[1],&phase_mapping[2],&phase_mapping[3],&phase_mapping[4],&phase_mapping[5],&phase_mapping[6],&phase_mapping[7]);
    }
    else
    {
        sprintf(temp,"Reading Lane_Phase_Mapping_File problem");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }

    fs.close();
}

void get_DSRC_Range() //Shayan Added 10.7.14
{
	fstream sh;
	sh.open(DSRC_Range_file);
	
	char temp[128];
	
	string temp_string;

    getline(sh,temp_string);  //Reading the first line which explains the legs of the intersection assignment
	getline(sh,temp_string);  //Reading the second line which has the values for each leg
	
	if(temp_string.size()!=0)
    {
		char tmp[128];
		strcpy(tmp,temp_string.c_str());		
		sscanf(tmp,"%f %f %f %f %f %f %f %f",&DSRC_Range[0],&DSRC_Range[1],&DSRC_Range[2],&DSRC_Range[3],&DSRC_Range[4],&DSRC_Range[5],&DSRC_Range[6],&DSRC_Range[7]);
    }
    else
    {
        sprintf(temp,"Reading DSRC_Range_File problem!!");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }
    
    getline(sh,temp_string);  //Reading the third line which explains the central rectangle
	getline(sh,temp_string);  //Reading the fourth line which has the values for each side of the rectangle
	
	if(temp_string.size()!=0)
    {
		char tmp[128];
		strcpy(tmp,temp_string.c_str());		
		sscanf(tmp,"%d %d",&rect_width,&rect_height);
    }
    else
    {
        sprintf(temp,"Reading Rectangle attributes problem!!");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }

    sh.close();
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


//To find the Shortest Distance between the vehicle location and two nearest lane nodes ----> GeoFencing
//Shayan Added 10.3.14  
void DistanceFromLine(double cx, double cy, double ax, double ay ,double bx, double by, double &distanceSegment, double &distanceLine, double &temp_R, double &temp_S)
{

	//
	// find the distance from the point (cx,cy) to the line
	// determined by the points (ax,ay) and (bx,by)
	//
	// distanceSegment = distance from the point to the line segment
	// distanceLine = distance from the point to the line (assuming
	//					infinite extent in both directions
	//

/*
How do I find the distance from a point to a line?


    Let the point be C (Cx,Cy) and the line be AB (Ax,Ay) to (Bx,By).
    Let P be the point of perpendicular projection of C on AB.  The parameter
    r, which indicates P's position along AB, is computed by the dot product 
    of AC and AB divided by the square of the length of AB:
    
    (1)     AC dot AB
        r = ---------  
            ||AB||^2
    
    r has the following meaning:
    
        r=0      P = A
        r=1      P = B
        r<0      P is on the backward extension of AB
        r>1      P is on the forward extension of AB
        0<r<1    P is interior to AB
    
    The length of a line segment in d dimensions, AB is computed by:
    
        L = sqrt( (Bx-Ax)^2 + (By-Ay)^2 + ... + (Bd-Ad)^2)

    so in 2D:   
    
        L = sqrt( (Bx-Ax)^2 + (By-Ay)^2 )
    
    and the dot product of two vectors in d dimensions, U dot V is computed:
    
        D = (Ux * Vx) + (Uy * Vy) + ... + (Ud * Vd)
    
    so in 2D:   
    
        D = (Ux * Vx) + (Uy * Vy) 
    
    So (1) expands to:
    
            (Cx-Ax)(Bx-Ax) + (Cy-Ay)(By-Ay)
        r = -------------------------------
                          L^2

    The point P can then be found:

        Px = Ax + r(Bx-Ax)
        Py = Ay + r(By-Ay)

    And the distance from A to P = r*L.

    Use another parameter s to indicate the location along PC, with the 
    following meaning:
           s<0      C is left of AB
           s>0      C is right of AB
           s=0      C is on AB

    Compute s as follows:

            (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
        s = -----------------------------
                        L^2


    Then the distance from C to P = |s|*L.

*/


	double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
	double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
	double r = r_numerator / r_denomenator;
//
    double px = ax + r*(bx-ax);
    double py = ay + r*(by-ay);
//     
    double s =  ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / r_denomenator;

	distanceLine = fabs(s)*sqrt(r_denomenator);
    
    //!!! B.H : far side bus stop
    temp_R = r;
    temp_S = s;
//
// (xx,yy) is the point on the lineSegment closest to (cx,cy)
//
	double xx = px;
	double yy = py;

	if ( (r >= 0) && (r <= 1) )
	{
		distanceSegment = distanceLine;
	}
	else
	{

		double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
		double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
		if (dist1 < dist2)
		{
			xx = ax;
			yy = ay;
			distanceSegment = sqrt(dist1);
		}
		else
		{
			xx = bx;
			yy = by;
			distanceSegment = sqrt(dist2);
		}


	}

	return;
}


//!!! B.H : To read Lane_Peer_Mapping.txt
void readlanepeermapping() 
{
    fstream fs;

    fs.open(lanepeermapping);

	if (!fs)
    {
        cout<<"***********Error opening lane peer mapping file!\n";
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
}
