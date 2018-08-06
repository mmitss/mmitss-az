// Receive RNDF map, Request List and Signal status from RSU through DSRC
// Need "rsu_msg_transmitter" running in RSUs


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <istream>
#include "math.h"
#include <sys/time.h>
#include <iomanip>

#include "LinkedList.h"
#include <time.h>

#include "NMAP.h"
#include "geoCoord.h"

#include <asn_application.h>
#include <asn_internal.h>/* for _ASN_DEFAULT_STACK_MAX */

#include <SPAT.h>
#include <MAP.h>


#define MAX_BUFLEN_MAP  1999
#define NUM_MM_STS 8

#ifndef byte
    #define byte  char  // needed if byte not defined
#endif

#define FLOAT2INT 10.0

using namespace std;

char logfilename[256] = "/nojournal/bin/log/obu_listener_V2_";

char recv_buf[MAX_BUFLEN_MAP];

//socket settings
//#define PORT 5799
#define PORT 15030
#define PORT_ART 15040
#define BROADCAST_ADDR "192.168.101.255"

#define numPhases 8
//*** The ACTIVE is for RNDF file. ***//
#define EV  1
#define TRANSIT 2

#define ACTIVE 1
#define LEAVE 0
#define NOT_ACTIVE -1

#ifndef UNIT_CHANGE_VALUE
    #define UNIT_CHANGE_VALUE  1000000  // needed if byte not defined
#endif


int Global;
const long timeinterval=30;   // For determining if the rndf entry is obsolete

char predir [64] = "/nojournal/bin/";

char rndf_file[128]="/nojournal/bin/RNDF.txt";
char active_rndf_file[128]="/nojournal/bin/ActiveRNDF.txt";
char signalfilename[128] = "/nojournal/bin/signal_status.txt";
char temp_log[1024];

char map_files[128]="/nojournal/bin/Intersection_maps.txt";
char active_map_file[64]="/nojournal/bin/ActiveMAP.txt";

char ART_File_Name[64]= "/nojournal/bin/psm.txt";


char tmp_log[512];


class Intersection_ID
{
	public:
	long ID;
	long Time;  //The time this map is received
};



LinkedList <Intersection_ID> Intersection_ID_list;    //Stores the intersection ID of all intersections


double GetSeconds();   // Get the current time: seconds in float point number
char *GetDate();     // Get the current date as: "Sat May 20 15:21:51 2000"

void split(string &s1, string &s2, char delim);

void xTimeStamp( char * pc_TimeStamp_ );
int  outputlog(char *output);


void printmap_to_file(MAP_t *MapType,char *filename);
int print_Sig_Status(SPAT_t *spatType, char *filename);

//int printspat(J2735SPATMsg_t *spat);

int FindActiveMap();    //find the active map id

int field_flag=0; //0: in the lab  1: in the field

//unpack ART message and save to requests_combined.txt
void Unpack_ART(byte* ablob, char * filename);

bool is_empty(std::ifstream& pFile);

void flipEndian (char *buf, int size) {
       int start, end;
       for (start = 0, end = size - 1; start < end; start++, end--) {
               buf[start] = buf[start] ^ buf[end];
               buf[end] = buf[start] ^ buf[end];
               buf[start] = buf[start] ^ buf[end];
       }
}

int active_map_id=0;

int main ( int argc, char* argv[] )
{
	
	if (argc>=2)	{sscanf(argv[1],"%d",&field_flag);}
	
	
	//Struct for UDP socket timeout: 1s
	struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    
    //Struct for UDP socket timeout: 1s
	struct timeval tv_ART;
    tv_ART.tv_sec = 0;
    tv_ART.tv_usec = 10000;
	
	
	int i,j;
	//Init log file
	//------log file name with Time stamp-----------------------//
	char timestamp[128];
	char tt[256];
	char strbuf[256];
	xTimeStamp(timestamp);
	strcat(logfilename,timestamp);
	strcat(logfilename,".log");
	//------end log file name-----------------------------------//

	//----init requests.txt and RNDF.txt-------------------------//
	char temp2[64];
	sprintf(temp2,"Num_req -1 0");
	//--strcpy(tt,"");	strcat(tt,predir);	strcat(tt,"requests.txt");
	fstream fs_req;
	fs_req.open(ART_File_Name, ios::out | ios::trunc );
	fs_req<<temp2;
	fs_req.close();

	//----Delete all old ***.rndf files----------//
	system("\\rm -f /nojournal/bin/*.rndf");

	fstream fs_rndf;
	fs_rndf.open(rndf_file, ios::out | ios::trunc );
	fs_rndf.close();

	fstream fs_active_rndf;
	fs_active_rndf.open(active_rndf_file, ios::out | ios::trunc );
	fs_active_rndf.close();

	//------------------------------------------------------//
	fstream fs;
	fs.open(logfilename, ios::out | ios::trunc );
	fs << "obugid is running...." << GetDate();

	cout << "obugid is running....\n";

	int sockfd;
	char pBuf[256];
	char sendMSG[256];
	struct sockaddr_in sendaddr;
	struct sockaddr_in recvaddr;
	int numbytes;
	int numbytes_ART;
	int addr_len;
	int broadcast=1;

	if((sockfd = socket(PF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd");
		exit(1);
	}
	
	//Setup time out
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Error");
	}
	

	if((setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,
		&broadcast,sizeof broadcast)) == -1)
	{
		perror("setsockopt - SO_SOCKET ");
		exit(1);
	}


	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = htons(PORT);
	sendaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr(BROADCAST_ADDR) ; //INADDR_BROADCAST;

	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);

	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");
		exit(1);
	}
	addr_len = sizeof ( sendaddr );
	
	
	int sockfd_ART;
	struct sockaddr_in sendaddr_ART;
    if((sockfd_ART = socket(PF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd");
		exit(1);
	}
	
	//Setup time out
	if (setsockopt(sockfd_ART, SOL_SOCKET, SO_RCVTIMEO,&tv_ART,sizeof(tv_ART)) < 0) {
      perror("Error");
	}
	sendaddr_ART.sin_family = AF_INET;
	sendaddr_ART.sin_port = htons(PORT_ART);
	sendaddr_ART.sin_addr.s_addr = INADDR_ANY;//inet_addr(BROADCAST_ADDR) ; //INADDR_BROADCAST;
	
	memset(sendaddr_ART.sin_zero,'\0',sizeof sendaddr_ART.sin_zero);
	if(bind(sockfd_ART, (struct sockaddr*) &sendaddr_ART, sizeof sendaddr_ART) == -1)
	{
		perror("bind");
		exit(1);
	}
	
  

	int ret;
	
	int count=0;
	
	while ( true )
	{
		
		//Find active map
		
		active_map_id=FindActiveMap();
		
		cout<<"Active map is: "<<active_map_id<<endl;
		
		int flag_map=0;
		
		//cout<<"Receive the data from RSU"<<endl;
	
		// -------------------------Receive the data from RSU-------------------------//
		numbytes = recvfrom(sockfd, recv_buf, sizeof recv_buf, 0,(struct sockaddr *)&sendaddr, (socklen_t *)&addr_len);
		
		if(numbytes>=0)
		{
			cout<<"Received data number: "<<count++<<endl;
		}
		
			SPAT_t * spatType_decode=0;
			spatType_decode=(SPAT_t *)calloc(1,sizeof(SPAT_t));
			asn_dec_rval_t rval;
			
			rval=ber_decode(0, &asn_DEF_SPAT,(void **)&spatType_decode, recv_buf, MAX_BUFLEN_MAP);
		
			if ( rval.code==RC_OK) 
			{			
				printf("SPAT: Decode Sucess\n");
			//	printspat(&spat);
				//print signal status
				flag_map=0;
				//get intersection id
				unsigned char ByteA=spatType_decode->intersections.list.array[0]->id.buf[0];
				unsigned char ByteB=spatType_decode->intersections.list.array[0]->id.buf[1];
				int intersection_id= (int) (((ByteB << 8) + (ByteA)));				
				if (intersection_id==active_map_id)
					print_Sig_Status(spatType_decode,signalfilename);                        //need to be changed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (Intersection_ID_list.ListSize()==1)
					print_Sig_Status(spatType_decode,signalfilename);                        //need to be changed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			} 
			else 
			{
				printf("SPAT: Decode Failure\n");
				flag_map=1;
			}

			
		if(flag_map==1 && numbytes>=0)  //Received MAP Message, not a SPAT
		{
			//sprintf(temp_log,"Received MAP Message size %d\n",numbytes);
			//outputlog(temp_log); cout<<temp_log;
			
			MAP_t * Map_decoder=0;
				
			Map_decoder=(MAP_t *)calloc(1,sizeof(MAP_t));		
			
			int decode_map_flag=0;               //indicate whether the map message is decoded successfully
			
			
			//~ for(int ii=0;ii<MAX_BUFLEN_MAP;ii++)
			//~ {
				//~ printf("%2x ",(unsigned char)recv_buf[ii]);
				//~ if(ii%16==15 && ii>0){
					//~ printf("\n");}
			//~ }
			
			
			
			// Calculating the length of the message taking the 2nd and 3rd bytes of the buffer. (after 0th and 1st bytes)
			//numbytes = *(int *)(recv_buf);
			//flipEndian ((char *)(&numbytes), 4);
			//numbytes &= 0xFFFF;
			//numbytes += 4;
		
			sprintf(temp_log,"Parsing MAP Message size %d\n",numbytes);
			outputlog(temp_log); cout<<temp_log;
			

			
			asn_dec_rval_t rval;
			rval=ber_decode(0, &asn_DEF_MAP,(void **)&Map_decoder, recv_buf, numbytes);
			
			if (rval.code==RC_OK) {
			printf("MAP: Decode Success!\n");
			decode_map_flag=1;
			} else {
			printf("MAP: Decode Failure\n");
			decode_map_flag=0;
			}
			
			if(decode_map_flag==1) //only decode map successfully
			{
				int found=0;
				//printmap(&j2735_map);
				
				
				//get intersection id
				unsigned char ByteA=Map_decoder->intersections.list.array[0]->id.buf[0];
				unsigned char ByteB=Map_decoder->intersections.list.array[0]->id.buf[1];
				int intersection_id= (int) (((ByteB << 8) + (ByteA)));
				
				//Determine whether this is a new MAP
				Intersection_ID_list.Reset();
				while(!Intersection_ID_list.EndOfList())
				{
					if (Intersection_ID_list.Data().ID==intersection_id)   //old map
					{
						found=1;
						Intersection_ID_list.Data().Time=GetSeconds();					
						cout<<"This is not a new map!"<<endl;
						break;
					}
					Intersection_ID_list.Next();
				}			
				if(found==0)   //This is a new map!!
				{
					//Add new map;
					Intersection_ID tmp_Inter;
					tmp_Inter.ID=intersection_id;
					tmp_Inter.Time=GetSeconds();
					Intersection_ID_list.InsertRear(tmp_Inter);
					cout<<"Add new MAP!!!!"<<endl;
					//Write the map structure to a text file
					
					//Create the new map_file Name;
					char map_file_name[128]="/nojournal/bin/Intersection_MAP_";
					char tmp[16];
					sprintf(tmp,"%d",intersection_id);
					strcat(map_file_name,tmp);	
					strcat(map_file_name,".nmap");
					cout<<"Intersection map name: "<<map_file_name<<endl;
					printmap_to_file(Map_decoder,map_file_name);   //Create a map description file.       need to be changed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					
				}			
				
				//Delete the MAP if we didn't receive for 5 seconds
				long current_time=GetSeconds();
				Intersection_ID_list.Reset();
				while(!Intersection_ID_list.EndOfList())
				{
					if (current_time-Intersection_ID_list.Data().Time>5)
						Intersection_ID_list.DeleteAt();
					Intersection_ID_list.Next();
				}
				
				cout<<"The current map list size is:"<<Intersection_ID_list.ListSize()<<endl;
				
				//Write current map list to Intersection_maps.txt
				fstream fs;
				fs.open(map_files, ios::out);
				if (!fs || !fs.good())
				{
					cout << "could not open file!\n";
				}				
				Intersection_ID_list.Reset();
				while(!Intersection_ID_list.EndOfList())
				{
					sprintf(tmp_log,"%d %ld -1\n",Intersection_ID_list.Data().ID,Intersection_ID_list.Data().Time);
					fs<<tmp_log;
					Intersection_ID_list.Next();
				}
				fs.close();
			}	
		
		}
		
		
		numbytes_ART = recvfrom(sockfd_ART, recv_buf, sizeof recv_buf, 0,(struct sockaddr *)&sendaddr_ART, (socklen_t *)&addr_len);	
		//cout<<"numbytes_ART "<<numbytes_ART<<endl;
		if (numbytes_ART>0) //Received ART message
		{
			
			cout<<"Received ART Message!"<<endl;		
			//unpack received ART message and write to the file if the id is matched to the active map id.
			Unpack_ART(recv_buf,ART_File_Name);
		}
		
		if (numbytes<0)  //Didn't receive anything for 3 seconds
		{
			memset(&recv_buf[0], 0, sizeof(recv_buf)); //clear the buf if nothing received			
			Intersection_ID_list.ClearList();  //Clear the map list
			fstream fs;   //Clear the Intersection_maps.txt file
			fs.open(map_files, ios::out|ios::trunc);
			fs.close();
			//clear the active_map.txt file
			fs.open(active_map_file, ios::out|ios::trunc);
			fs.close();
			
			//clear the signal_status file if there is no active map
			//ifstream f_active_map;
			//f_active_map.open(active_map_file,ios::in);

			//if(is_empty(f_active_map))
			//{
				fstream f_signal_status_file;
				f_signal_status_file.open(signalfilename,ios::out|ios::trunc);
				f_signal_status_file.close();
				
				//Clear the request combined file
				fstream req_com_file;
				req_com_file.open(ART_File_Name,ios::out|ios::trunc);
				req_com_file.close();
				
				
			//}
			//f_active_map.close();
			
		}
		

		

	}//while(true)

	close(sockfd); 
	
	
	return 0;
}

bool is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
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


int outputlog(char *output)
{
	FILE * stream = fopen( logfilename, "r" );
	fseek( stream, 0L, SEEK_END );
	long endPos = ftell( stream );
	fclose( stream );

	fstream fs;
	if (endPos <1000000)
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



void split(string &s1, string &s2, char delim)
{
	size_t i;

	i=s1.find_first_of(delim);
	s2.append(s1, i+1, s1.size());
	s1.erase(i, s1.size());
}


double GetSeconds()
{
	struct timeval tv_tt;
	gettimeofday(&tv_tt, NULL);
	return (tv_tt.tv_sec+tv_tt.tv_usec/1.0e6);
}

char *GetDate()
{
	time_t rawtime;

	time ( &rawtime );

	return ctime(&rawtime);
}



void printmap_to_file(MAP_t *MapType,char *filename)
{
	int i,j,k,l;
	char log[128];
	fstream fs;
	fs.open(filename, ios::out);
	
	if (!fs || !fs.good())
	{
		cout << "could not open file!\n";
	}
	
	fs<<"MAP_Name\t"<< "Gaisy_Gav.nmap"<<endl;
	fs<<"RSU_ID\t"<< "Gaisy_Gav"<<endl;
	
	//Do intersection ID
	unsigned char ByteA=MapType->intersections.list.array[0]->id.buf[0];
	unsigned char ByteB=MapType->intersections.list.array[0]->id.buf[1];
	int intersection_id= (int) (((ByteB << 8) + (ByteA)));
	
	fs<<"IntersectionID\t"<<intersection_id<<endl;
	fs<<"Intersection_attributes\t"<<"00110011"<<endl;
	//Do reference Point
	double latitude= (double)(MapType->intersections.list.array[0]->refPoint->lat)/UNIT_CHANGE_VALUE;
	double longitude= (double) (MapType->intersections.list.array[0]->refPoint->Long)/UNIT_CHANGE_VALUE;
	ByteA=MapType->intersections.list.array[0]->refPoint->elevation->buf[0];
	ByteB=MapType->intersections.list.array[0]->refPoint->elevation->buf[1];
	
	printf("%x %x ",ByteA,ByteB);
	unsigned short elevation= (int) ((((ByteB+1) << 8) + (ByteA)));	
	sprintf(log,"Reference_point\t %lf %lf %d\n",latitude,longitude,elevation);
	
	
	//init geo coord
	geoCoord geoPoint;
	double g_long, g_lat, g_altitude ;
	double x_grid, y_grid, z_grid ;
	
	double ecef_x,ecef_y,ecef_z;					
	//Important!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Note the rectangular coordinate system: local_x is N(+) S(-) side, and local_y is E(+) W(-) side 
	//This is relative distance to the ref point!!!!
	//local z can be considered as 0 because no elevation change.
	double local_x,local_y,local_z;
	geoPoint.init(longitude, latitude, elevation/10);
	
	fs<<log;
	fs<<"No_Approach\t"<<"8"<<endl;
	
	
	cout<<"Num_approaches "<<MapType->intersections.list.array[0]->approaches.list.count<<endl;
	
	int missing_approach=0;
	
	//find missing approach: for T intersections
	if(MapType->intersections.list.array[0]->approaches.list.count<4) //There is missing approaches
	{	
		
		for(i=0;i<MapType->intersections.list.array[0]->approaches.list.count;i++) 
		{
			if(*MapType->intersections.list.array[0]->approaches.list.array[i]->approach->id!=i*2+1)
				{
					missing_approach=i*2+1;
					break;
				}
		}
		if(missing_approach==0)  //less than 4 approaches, but here missing approach is zero, which means missing the last approach!!!!!!!!!!
		{
			missing_approach=7;
		}
	}
	
	cout<<"Missing approach is:"<<missing_approach<<endl;
	
	//Do each Approach
	for(i=0;i<MapType->intersections.list.array[0]->approaches.list.count;i++)   //There are only 4 approaches in J2735 MAP
	{
		
		int approach_id=*MapType->intersections.list.array[0]->approaches.list.array[i]->approach->id;
		
		//add missing approach information to the map file here
		if(missing_approach==i*2+1)
		{
			sprintf(log,"Approach\t %d\n",i*2+1);
			fs<<log;
			sprintf(log,"Approach_type\t 0\n");
			fs<<log;
			sprintf(log,"No_lane\t 0\n");
			fs<<log;
			fs<<"end_approach"<<endl;
			sprintf(log,"Approach\t %d\n",i*2+2);
			fs<<log;
			sprintf(log,"Approach_type\t 0\n");
			fs<<log;
			sprintf(log,"No_lane\t 0\n");
			fs<<log;
			fs<<"end_approach"<<endl;
		}
		
		
		//Ingress lanes Odd Approaches
		sprintf(log,"Approach\t %d\n",approach_id);
		fs<<log;
		sprintf(log,"Approach_type\t 1\n");
		fs<<log;
		int lane_no=MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.count;
		sprintf(log,"No_lane\t %d\n",lane_no);
		
		cout<<"No of Lane is: "<<lane_no<<endl;
		fs<<log;
		//Do each ingress lane
		
		for(j=0;j<lane_no;j++)
		{
			sprintf(log,"Lane\t %d.%d\n",approach_id,j+1);

			fs<<log;
			//Do lane ID;
			ByteA=MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneNumber.buf[0];
			ByteB=MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneNumber.buf[1];
			
			unsigned short Lane_ID= (int) (((ByteB << 8) + (ByteA)));
			sprintf(log,"Lane_ID\t %d\n",Lane_ID);
			fs<<log;
			sprintf(log,"Lane_type\t 1\n");
			fs<<log;
			if (MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneAttributes==2)
				sprintf(log,"Lane_attributes\t 0000000000100010\n");
			else if (MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneAttributes==4)
				sprintf(log,"Lane_attributes\t 000000000010100\n");
			else if (MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneAttributes==8)
				sprintf(log,"Lane_attributes\t 0000000000101000\n");	
			else if (MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneAttributes==6)
				sprintf(log,"Lane_attributes\t 000000000010110\n");
			else if (MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneAttributes==12)
				sprintf(log,"Lane_attributes\t 0000000000101100\n");
			else if (MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneAttributes==10)
				sprintf(log,"Lane_attributes\t 0000000000101010\n");	
			else if (MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneAttributes==14)
				sprintf(log,"Lane_attributes\t 0000000000101110\n");
			else
				sprintf(log,"Lane_attributes\t 0000000000100000\n"); //No movement avaialbe
			fs<<log;
			sprintf(log,"Lane_width\t %d\n",*MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneWidth);
			fs<<log;
			
			int num_way_points=MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.count;
			
			sprintf(log,"No_nodes\t %d\n",num_way_points);
			fs<<log;
			//do lane nodes
			for(k=0;k<num_way_points;k++)
			{
			//transfer from local coordinates to GPS coordinates	
			unsigned char B0,B1,B2,B3;
			B0=MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[0];
			B1=MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[1];
			B2=MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[2];
			B3=MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[3];
			
			sprintf(tmp_log,"%x %x %x %x\n",MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[0],
				MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[1],
				MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[2],
				MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[3]);
				
				cout<<tmp_log;
			
			
			int N_Offset_int, E_Offset_int;
			N_Offset_int=(int)B0;
			N_Offset_int=(N_Offset_int<<8)+(int)B1;
			E_Offset_int=(int)B2;
			E_Offset_int=(E_Offset_int<<8)+(int)B3;
			
			sprintf(tmp_log,"N_Offset %d E_offset %d\n",N_Offset_int,E_Offset_int);
				cout<<tmp_log;
			
			double N_Offset=(double)(N_Offset_int-32768)/10;
			double E_Offset=(double)(E_Offset_int-32768)/10;
			
			geoPoint.local2ecef(N_Offset, E_Offset, elevation/10, &x_grid, &y_grid, &z_grid) ; // Third argument z=0.0 for vissim
			geoPoint.ecef2lla(x_grid, y_grid, z_grid, &g_long, &g_lat, &g_altitude) ;
								
			sprintf(log,"%d.%d.%d\t %lf %lf\n",approach_id,j+1,k+1,g_lat,g_long);
			fs<<log;
			}
			
			
			//do connection lanes - only for approaching lanes
			unsigned char C[4];
			C[0]= MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->connectsTo->buf[0];
			C[1]= MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->connectsTo->buf[1];
			C[2]= MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->connectsTo->buf[2];
			C[3]= MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->connectsTo->buf[3];
			//No. of connection lane
			int conn_lane_no=(int)C[0];
			sprintf(log,"No_Conn_lane\t %d\n",conn_lane_no);
			fs<<log;
			for(int l=0;l<conn_lane_no;l++)
			{
				//printf("Connection lane: %d",mapdata->approaches[i].ingress_lanes[j].connects_to_lanes[l]);
				int ten,one; //two digits of the connection lane
				ten= (int) (C[l+1]/10);
				one= C[l+1]%10;
				sprintf(log,"%d.%d\t 0\n",ten,one);  //THe movement manuevue is missing in the J2735 MAP, put 0
				fs<<log;
			}
			fs<<"end_lane"<<endl;		
		}
		fs<<"end_approach"<<endl;
		//Do each egress lane
		sprintf(log,"Approach\t %d\n",(*MapType->intersections.list.array[0]->approaches.list.array[i]->approach->id)+1);
		fs<<log;
		sprintf(log,"Approach_type\t 2\n");
		fs<<log;
		int lane_no_egress=MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.count;
		
		sprintf(log,"No_lane\t %d\n",lane_no_egress);
		fs<<log;
		for(j=0;j<lane_no_egress;j++)
		{
			sprintf(log,"Lane\t %d.%d\n",approach_id+1,j+1);
			fs<<log;
			//Do lane ID;
			ByteA=MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->laneNumber.buf[0];
			ByteB=MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->laneNumber.buf[1];
			unsigned short Lane_ID_Egress= (int) (((ByteB << 8) + (ByteA)));
			sprintf(log,"Lane_ID\t %d\n",Lane_ID_Egress);
			fs<<log;
			sprintf(log,"Lane_type\t 1\n");
			fs<<log;
			sprintf(log,"Lane_attributes\t 0000000001100011\n");
			fs<<log;
			sprintf(log,"Lane_width\t %d\n",*MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->laneWidth);
			fs<<log;
			
			int num_way_points_egress=MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->nodeList.list.count;
			sprintf(log,"No_nodes\t %d\n",num_way_points_egress);
			fs<<log;
			//do lane nodes
			for(k=0;k<num_way_points_egress;k++)
			{
				//transfer from local coordinates to GPS coordinates				
				unsigned char B0,B1,B2,B3;
				B0=MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[0];
				B1=MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[1];
				B2=MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[2];
				B3=MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[3];
			
				int N_Offset_int, E_Offset_int;
				N_Offset_int=(int)B0;
				N_Offset_int=(N_Offset_int<<8)+(int)B1;
				E_Offset_int=(int)B2;
				E_Offset_int=(E_Offset_int<<8)+(int)B3;
			
				double N_Offset= (double)(N_Offset_int-32768)/10;
				double E_Offset= (double)(E_Offset_int-32768)/10;
				

			
				geoPoint.local2ecef(N_Offset, E_Offset, elevation/10, &x_grid, &y_grid, &z_grid) ; // Third argument z=0.0 for vissim
				geoPoint.ecef2lla(x_grid, y_grid, z_grid, &g_long, &g_lat, &g_altitude) ;
							
				sprintf(log,"%d.%d.%d\t %lf %lf\n",approach_id+1,j+1,k+1,g_lat,g_long);
				fs<<log;
			}
			//do connection lanes
			sprintf(log,"No_Conn_lane\t 0\n");
			fs<<log;
			fs<<"end_lane"<<endl;
		}
		fs<<"end_approach"<<endl;
		
	}
	
	if(MapType->intersections.list.array[0]->approaches.list.count<4 && missing_approach==7)
	{
		sprintf(log,"Approach\t %d\n",7);
		fs<<log;
		sprintf(log,"Approach_type\t 0\n");
		fs<<log;
		sprintf(log,"No_lane\t 0\n");
		fs<<log;
		fs<<"end_approach"<<endl;
		sprintf(log,"Approach\t %d\n",8);
		fs<<log;
		sprintf(log,"Approach_type\t 0\n");
		fs<<log;
		sprintf(log,"No_lane\t 0\n");
		fs<<log;
		fs<<"end_approach"<<endl;
	}
	
	
	fs<<"end_map";
		
	fs.close();	
}

int print_Sig_Status(SPAT_t *spatType,char *filename)
{
	fstream fs;
	fs.open(filename, ios::out);
	
	if (!fs || !fs.good())
	{
		cout << "could not open file!\n";
		exit(0);
	}
	
	//get intersection id
	unsigned char ByteA=spatType->intersections.list.array[0]->id.buf[0];
	unsigned char ByteB=spatType->intersections.list.array[0]->id.buf[1];
	int intersection_id= (int) (((ByteB << 8) + (ByteA)));
	
	fs<<intersection_id<<" Signal_status ";
	
	int i;
	//printf("The signal status are:\n");
	for(i=0;i<NUM_MM_STS;i++)
	{
		if(*spatType->intersections.list.array[0]->states.list.array[i]->currState == 0)
		{
			//printf("N/A ");
			fs<<"0 ";
		}	
		if(*spatType->intersections.list.array[0]->states.list.array[i]->currState & (1<<0))
		{
			//printf("G ");
			fs<<"3 ";
		}
		if(*spatType->intersections.list.array[0]->states.list.array[i]->currState & (1<<1))
		{
			//printf("Y ");
			fs<<"4 ";
		}
		if(*spatType->intersections.list.array[0]->states.list.array[i]->currState & (1<<2))
		{
			//printf("R ");
			fs<<"1 ";
		}
	}
	//printf("\n");
	
	//printf("The Ped signal status are:\n");
	//for(i=0;i<NUM_MM_STS;i++)
	//{
		//if (mm_st[i].ped_state==J2735SPAT_PED_WALK)
		//printf("W ");
		//if (mm_st[i].ped_state==J2735SPAT_PED_CAUTION)
		//printf("F ");
		//if (mm_st[i].ped_state==J2735SPAT_PED_STOP)
		//printf("D ");
		//if (mm_st[i].ped_state==J2735SPAT_PED_UNAVAILABLE)
		//printf("N/A ");
	//}
	//printf("\n");
		
	//printf("The Ped Remaining time are:\n");
	//for(i=0;i<NUM_MM_STS;i++)
	//{
		//printf("%d ",mm_st[i].next_time_remaining);
	//}
	//printf("\n");
	
		
	return 0;
}


int gettimestamp(uint64_t *seconds, uint64_t *microsecs)
{
    struct timeval tv;

    gettimeofday(&tv, 0);

    *seconds = tv.tv_sec;
    *microsecs = tv.tv_usec;

    return 0;
}




void Unpack_ART(byte* ablob, char *filename)
{
			
	int intersection_id;
	int i;
	int offset=0;
	unsigned short   tempUShort; // temp values to hold data in final format
	long    tempLong;
	float tempfloat;
	unsigned char   byteA;  // force to unsigned this time,
	unsigned char   byteB;  // we do not want a bunch of sign extension 
	unsigned char   byteC;  // math mucking up our combine logic
	unsigned char   byteD;
	
	
	
	//Intersection ID;
	byteA = ablob[offset+0];
	byteB = ablob[offset+1];
	byteC = ablob[offset+2];
	byteD = ablob[offset+3];
	intersection_id = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
	offset = offset + 4;
	
	
	
	if (intersection_id==active_map_id)    //only write the request table from the active map to the file
	{
		
		fstream fs;
		fs.open(filename, ios::out);
		
		if (!fs || !fs.good())
		{
			cout << "could not open file!\n";
			exit(0);
		}		
		sprintf(temp_log,"Received ART!\n");
		outputlog(temp_log);
		
		//Number of Requests
		byteA = ablob[offset+0];
		byteB = ablob[offset+1];
		int NumReq = (int)(((byteA << 8) + (byteB))); // in fact unsigned
		offset = offset + 2;
		fs<<"Num_req "<<NumReq<<endl;
		//cout<<"The number of requests is: "<<NumReq<<endl;
		
		sprintf(temp_log,"The number of requests is %d\n",NumReq);
		outputlog(temp_log); cout<<temp_log;
		
		//Do Active Request Table
		for (i=0;i<NumReq;i++)
		{
			//Do Veh_ID
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			byteC = ablob[offset+2];
			byteD = ablob[offset+3];
			tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
			offset = offset + 4;
			fs<<tempLong<<" ";
			//Do Veh_Class
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//Do ETA
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			tempfloat=tempUShort/FLOAT2INT;
			offset = offset + 2;
			fs<<setprecision(3)<<tempfloat<<" ";
			//Do phase
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//Do Tq
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			tempfloat=tempUShort/FLOAT2INT;		
			offset = offset + 2;
			fs<<setprecision(3)<<tempfloat<<" ";
			//Do abs_time
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			byteC = ablob[offset+2];
			byteD = ablob[offset+3];
			tempLong = (unsigned long)((byteA << 24) + (byteB << 16) + (byteC << 8) + (byteD));
			offset = offset + 4;
			fs<<tempLong<<" ";
			sprintf(temp_log,"Absolute time is %ld\n",tempLong);
			outputlog(temp_log);
			
			
			//Do split phase
		//	byteA = ablob[offset+0];
		//	byteB = ablob[offset+1];
		//	tempUShort= (int)(((byteA << 8) + (byteB)));
		//	offset = offset + 2;
		//	fs<<tempUShort<<" ";
			//Do inlane
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//Do outlane
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//do shour
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//do smin
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//do ssec
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//do ehour
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//do emin
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//do esec
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort<<" ";
			//do veh states
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			//fs<<tempUShort<<" ";
					
			if(tempUShort==1)
			//fs<<"Approaching ";
			fs<<"1 ";
			else if(tempUShort==2)
			//fs<<"Leaving ";
			fs<<"2 ";
			else if(tempUShort==3)
			//fs<<"InQueue ";
			fs<<"3 ";
			else
			//fs<<"N/A ";
			fs<<"0 ";
			//do request sequence
			byteA = ablob[offset+0];
			byteB = ablob[offset+1];
			tempUShort= (int)(((byteA << 8) + (byteB)));
			offset = offset + 2;
			fs<<tempUShort;
			
			sprintf(temp_log,"Sequence is %d\n",tempUShort);
			outputlog(temp_log);
			
			if(i<NumReq-1)
			fs<<endl;
		}	
		fs.close();
	}
}

int FindActiveMap()
{
	int active_id=0;
	char temp[128];	
	ifstream f_active_map;
	f_active_map.open(active_map_file,ios::in);
	if(!is_empty(f_active_map))    //if not empty, read the active map id
	{
		string temp_string;
		getline(f_active_map,temp_string);
		if(temp_string.size()!=0)
		{
			char tmp[128];
			//cout<<temp_string<<endl;
			strcpy(tmp,temp_string.c_str());		
			sscanf(tmp,"%*d %d %*d",&active_id);
			
		//	cout<<"active id is:"<<active_id<<endl;
			
		}
		else
		{
			sprintf(temp,"Reading active map file error!\n");
			cout<<temp<<endl;
			outputlog(temp);
			exit(0);
		}
	}
	else    //if the file is empty
	{
		if(field_flag==1)   //in the field, request gps location, not done yet!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{
			//gps_init ();
			//savari_gps_read(&gps, gps_handle);
			//savari_gps_close(gps_handle);
		}
	}
	f_active_map.close();
	return active_id;
	
}
