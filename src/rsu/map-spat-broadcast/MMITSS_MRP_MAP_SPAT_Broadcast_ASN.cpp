#include <getopt.h>
#include <unistd.h>
#include "SignalController.h"
#include "GetInfo.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include <vector>
#include "geoCoord.h"
//#include <libgps.h>


//#include "j2735spat.h"
//#include "j2735common.h"
#include "NMAP.h"

#include <asn_application.h>
#include <asn_internal.h>/* for _ASN_DEFAULT_STACK_MAX */

#include <SPAT.h>
#include <MAP.h>

#define LOCAL_HOST_ADDR "127.0.0.1"

#define MAX_BUFLEN_SPAT 500
#define MAX_BUFLEN_MAP  1999

#define FLOAT2INT 10.00001


#define NUM_MM_STS 8       // Number of Maximum phases
#define NUM_LANES 10        // 6 lanes maximum per phase

#ifndef byte
    #define byte  char  // needed if byte not defined
#endif

#ifndef UNIT_CHANGE_VALUE
    #define UNIT_CHANGE_VALUE  1000000  // needed if byte not defined
#endif


#define CONTROLLER_IP "10.254.56.23"

char INTip[64];
char INTport[16];

int PORT_MAP = 15030;
int PORT_ART = 15040;

char tmp_log[512];

char BROADCAST_ADDR_ATH1[64]="192.168.101.255";
char BROADCAST_ADDR_ATH0[64]="192.168.1.255";



char spat_buf[MAX_BUFLEN_SPAT];  //buf used to receive SPAT data from controller



uint8_t j2735_spat_buf[MAX_BUFLEN_SPAT]; //buf used to encode J2735 SPAT message
char j2735_map_buf[MAX_BUFLEN_MAP]; //buf used to encode J2735 MAP message
unsigned char j2735_map_buf_recv[MAX_BUFLEN_MAP]; //buf used to decode J2735 MAP message

//MAP parameters
char MAP_File_Name[64]  = "/nojournal/bin/nmap_name.txt";
char Lane_Phase_Mapping_File_Name[64]  = "/nojournal/bin/Lane_Phase_Mapping.txt";
char Inlane_Outlane_Phase_Mapping_File_Name[64] = "/nojournal/bin/InLane_OutLane_Phase_Mapping.txt";

char ART_File_Name[64]= "/nojournal/bin/requests_combined.txt";

string nmap_name;
int phase_mapping[8];    //the corresponding phases sequence: approach 1 3 5 7 through left
int appr_phase_mapping [8];  //map each phase to the approach order: appr_phase_mapping[0] -> phase1
int LaneNode_Phase_Mapping[8][8][20];           //8 approaches, at most 8 lanes each approach, at most 20 lane nodes each lane
												// the value is just the requested phase of this lane node;
NMAP NewMap;
vector<N_LaneNodes> MAP_Nodes;

void get_map_name(); //Yiheng add 07/18/2014
void get_lane_phase_mapping();

void get_appr_phase_mapping();

void write_inlane_outlane_phase_mapping_to_file(char *filename);

int msleep(unsigned long milisec);
void PrintPhases(PhaseStatus currentPhase);
void xTimeStamp( char * pc_TimeStamp_ );



int fillspat_ASN();
int fillmap_ASN();

int gettimestamp(uint64_t *seconds, uint64_t *microsecs);


//Read ART from file requested_combined.txt and form a octet stream to be sent
void Pack_ART(byte* ART_data, char * filename, int &size);

char Ped_Status_File_Name[64]  = "/nojournal/bin/Ped_status.txt";
int print_Ped_Status(SPAT_t *spatType, char *filename);

int print_Signal_Status(SPAT_t *spat);

SignalController Daisy_Mnt;

//define log file name
char logfilename[256]   = "/nojournal/bin/log/MMITSS_MRP_MAP_SPAT_BROADCAST_";
char IPInfo[64]			= "/nojournal/bin/ntcipIP.txt";

int field_flag=0; //0: in the lab  1: in the field



//~ int gps_init ();

static void usage(char *name)
{
    printf("%s [-hofc] [Number] Y\n"
           "   -d(0): Green HOLD on number\n"
           "   -o(2): Phase OMIT on number\n"
           "   -f(1): FORCEOFF ring on number\n"
           "   -c(3): phase CALL on number\n"
		   "The [Number] should be the binary value of the phase is controlled in(xxxx xxxx)\n "
           "\n", name);
}

//Savari GPS data structure
//~ gps_data_t *gps_handle;
//~ savari_gps_data_t gps;


int main ( int argc, char* argv[] )
    {
	int i,j,k;
		int ret;

    unsigned int type = 0;
    
    if (argc > 1) sscanf(argv[1],"%s",BROADCAST_ADDR_ATH1);   // Obtain the broadcasting address from user. 
    if (argc > 2) sscanf(argv[2],"%s",BROADCAST_ADDR_ATH0);
    if (argc > 3) sscanf(argv[3],"%d",&field_flag);
    if (argc > 4) sscanf(argv[4],"%d",&PORT_MAP);
	
    //------log file name with Time stamp---------------------------
    char timestamp[128];
    //char tmp_log[64];
    xTimeStamp(timestamp);
    strcat(logfilename,timestamp);strcat(logfilename,".log");

	std::fstream fs;
	fs.open(logfilename, fstream::out | fstream::trunc);

    //------end log file name-----------------------------------
    //-----------------Beginning of Wireless communication------------//
    sprintf(tmp_log,"Now to Read the Signal Controller IP information:\n");
	cout<<tmp_log;  outputlog(tmp_log);
    
	get_ip_address();


    int SleepTime=1000; // in ms

    for(int i=0;i>0;i--) //warm up...
        {
        Daisy_Mnt.PhaseRead();//IntersectionPhaseRead();
        Daisy_Mnt.UpdatePhase();//Phases.UpdatePhase(phase_read);
        Daisy_Mnt.Phases.Display();//Phases.Display();
        msleep(1000);
        }
	
	
	//Set controller to broadcast SPAT
	int Setvalue=6;   //with pedestrain data
	Daisy_Mnt.SPATSet(Setvalue);  //Set to broadcast the SPAT with a value
	
	//------------init: Begin of Network connection------------------------------------
	int sockfd;
	int broadcast=1;

	struct sockaddr_in sendaddr;
	struct sockaddr_in recvaddr;
	int numbytes, addr_len;

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
	sendaddr.sin_port = htons(6053);  //*** IMPORTANT: the vissim,signal control and performance observer should also have this port. ***//
	sendaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr(LOCAL_HOST_ADDR);//inet_addr(OBU_ADDR);//INADDR_ANY;

	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);

	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");        exit(1);
	}

	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(PORT_MAP);
	recvaddr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR_ATH1) ; //INADDR_BROADCAST;
	memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
	int addr_length = sizeof ( recvaddr );
	int recv_data_len;
	//-----------------------End of Network Connection------------------//
	
	int sockfd_ART;
	struct sockaddr_in recvaddr_ART;
	int addr_len_ART;
	
	if((sockfd_ART = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd_ART");
		exit(1);
	}
	
	if((setsockopt(sockfd_ART,SOL_SOCKET,SO_BROADCAST,
		&broadcast,sizeof broadcast)) == -1)
	{
		perror("setsockopt - SO_SOCKET ");
		exit(1);
	}
		
	recvaddr_ART.sin_family = AF_INET;
	recvaddr_ART.sin_port = htons(PORT_ART);
	recvaddr_ART.sin_addr.s_addr = inet_addr(BROADCAST_ADDR_ATH0) ; //INADDR_BROADCAST;
	memset(recvaddr_ART.sin_zero,'\0',sizeof recvaddr_ART.sin_zero);
	
	
/////////////////////////////////////////////Read the MAP description file and do the lane-phase mapping

	NewMap.ID=1;
	NewMap.Version=1;
	// Parse the MAP file
	get_map_name();
	char mapname[128];
	sprintf(mapname,"%s",nmap_name.c_str());
	printf("%s",mapname);
	NewMap.ParseIntersection(mapname);

	sprintf(tmp_log,"Read the map successfully At (%d).\n",time(NULL));
	outputlog(tmp_log); cout<<tmp_log;

	
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
				N_LaneNodes temp_node;
				temp_node.index.Approach=NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].index.Approach;
				temp_node.index.Lane=NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].index.Lane;
				temp_node.index.Node=NewMap.intersection.Approaches[i].Lanes[j].Nodes[k].index.Node;
				MAP_Nodes.push_back(temp_node);
												
				//sprintf(tmp_log,"%d %d %d\n",temp_node.index.Approach,temp_node.index.Lane,temp_node.index.Node);
				//outputlog(tmp_log); cout<<tmp_log;
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

//Read Map data and contruct the J2735MAP Message
//J2735MapData_t j2735_map,j2735_map_de;
//memset(&j2735_map, 0, sizeof(j2735_map));
//fillmap(&j2735_map);


//Fill the map message and encode to a buf

fillmap_ASN();



//j2735_free_mapdata(&j2735_map);
//int enc_len;
//enc_len = j2735_encode_mapdata(&j2735_map,j2735_map_buf, MAX_BUFLEN_MAP);
//    if (enc_len < 0) {
//        printf("MAP: Encode Failure\n");
//    } else {
//        printf("MAP: Encode Success\n");
//    }

//printf("The MAP size is is: %d\n",enc_len);

//j2735_dump_hex("MAP:", (unsigned char*)j2735_map_buf, enc_len);
//    memset(&j2735_map_de, 0, sizeof(j2735_map_de));
//    ret = j2735_decode_mapdata(&j2735_map_de,(unsigned char*)j2735_map_buf, enc_len);
//    if (ret < 0) {
//        printf("MAP: Decode Failure\n");
//    } else {
//        printf("MAP: Decode Success\n");
//    }
//printmap(&j2735_map_de);
//j2735_free_mapdata(&j2735_map_de);


//map each phase to approach number
get_appr_phase_mapping();


//Construct the inlane-outlane-phase mapping file for PRS
write_inlane_outlane_phase_mapping_to_file(Inlane_Outlane_Phase_Mapping_File_Name);

////////////////////////End of reading map and lane phase mapping

sprintf(tmp_log,"Signal Status Data: \n");
outputlog(tmp_log);

msleep(1000);


int count=0;
    while ( true )
    {

        sprintf(tmp_log,"\n..........In the While LOOP........:\n");
        cout<<tmp_log;       // outputlog(tmp_log);

		//Get SPAT from UDP port 6053	
		recv_data_len = recvfrom(sockfd, spat_buf, sizeof(spat_buf), 0,
                        (struct sockaddr *)&sendaddr, (socklen_t *)&addr_length);
                                            	
		//cout<<"The Received SPAT data bytes are: "<<recv_data_len<<endl;
		cout<<"Message Count: "<<count<<endl;
		
		//Read the SPAT Data and MAP Data to Create SPAT Message from Savari library
		//J2735SPATMsg_t spat;
		//J2735MovementState_t  mm_st[NUM_MM_STS];
		//uint8_t lane_sets[NUM_MM_STS][NUM_LANES];
		//fillspat(&spat, mm_st, lane_sets);
		
		
		//create SPAT message from ASN library
		fillspat_ASN();
		
		//xer_fprint(stdout, &asn_DEF_SPAT, spatType);
		
		

		
		sendto(sockfd,j2735_spat_buf,MAX_BUFLEN_SPAT, 0,(struct sockaddr *)&recvaddr, addr_length);
		cout<<"Broadcast J2735 SPAT Message!"<<endl;		
	
		count++;
		
		if(count%10==0)
		{
			
			//Send MAP every 1 second			
			sendto(sockfd,j2735_map_buf,MAX_BUFLEN_MAP, 0,(struct sockaddr *)&recvaddr, addr_length);
			cout<<"Broadcast J2735 MAP Message!"<<endl;
						
			//Send ART every 1 second			
			//Read ART from file requested_combined.txt and form a octet stream to be sent
			int ART_size=0;
			byte ART_buf[1024];
			Pack_ART(ART_buf, ART_File_Name,ART_size);
			char *ART_buf_send;
			ART_buf_send=new char[ART_size];
			for (i=0;i<ART_size;i++)
			{
				ART_buf_send[i]=ART_buf[i];
			}
			sendto(sockfd_ART,ART_buf_send,ART_size, 0,(struct sockaddr *)&recvaddr_ART, addr_length);
			cout<<"Broadcast Active Request Table with size "<<ART_size<<endl;;
			
		}
		
	
		//memset(&spat, 0, sizeof(spat));
		
		//~ ret = j2735_decode_spat(&spat, j2735_spat_buf, MAX_BUFLEN_SPAT);
		//~ if ( ret < 0 ) {
			//~ printf("SPAT: Decode Failure\n");
		//~ } else {
			//~ printf("SPAT: Decode Sucess\n");
			//~ //printspat(&spat);
        //~ j2735_free_spat_contents_only(&spat);
		//~ }
		
		
		if(count>=10000)
		count=0;
    }

    fs.close();
   
    return 0;
}



void PrintPhases(PhaseStatus currentPhase)
    {
    char tmp_log[128]="Phase Information: ";
    char tmp_phase[16];
    for(int i=0;i<numPhases;i++)
        {
        sprintf(tmp_phase," %d",currentPhase.phaseColor[i]);
        strcat(tmp_log,tmp_phase);
        }
    outputlog(tmp_log);
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

int gettimestamp(uint64_t *seconds, uint64_t *microsecs)
{
    struct timeval tv;

    gettimeofday(&tv, 0);

    *seconds = tv.tv_sec;
    *microsecs = tv.tv_usec;

    return 0;
}

void get_appr_phase_mapping()
{
	int i;
	for (i=0;i<8;i++)
	{	
		if(phase_mapping[i]!=0)
		{
			if(i%2==0)
				appr_phase_mapping[phase_mapping[i]-1]= (i+1);
			if(i%2==1)
				appr_phase_mapping[phase_mapping[i]-1]= i;
		}	
	}
	
	cout<<"appr_phase_mapping:"<<endl;
	for(i=0;i<8;i++)
	cout<<appr_phase_mapping[i]<<" ";
	cout<<endl;

}


int fillspat_ASN()
{
	unsigned char byteA;
	unsigned char byteB;
	unsigned short tempUShort;
	long tempLong;
	int Status_red;
	int Status_green;
	int Status_yellow;
	int Ped_status_walk;
	int Ped_status_flash;
	int Ped_status_dontwalk;
	
    int i,j,k;
    uint64_t secs, usecs;
    
    long lane_no[NUM_MM_STS]={0};  //the number of lanes of one phase
    int *laneset[NUM_MM_STS]={0};      //lane set
    long SignalLightState[NUM_MM_STS]={0};
	long SignalLightState_Next[NUM_MM_STS]={0};
	long Ped_States[NUM_MM_STS]={0};
	long Ped_States_Next[NUM_MM_STS]={0};
	long stateconfidence[NUM_MM_STS]={0};
	long time_remaining_ped[NUM_MM_STS]={0};
	long stateconfidence_ped[NUM_MM_STS]={0};
	long objectcount[NUM_MM_STS]={0};
	long peddetect[NUM_MM_STS]={0};
	long ped_objectcount[NUM_MM_STS]={0};
	long specialstate[NUM_MM_STS]={0};
    
    
    int tmp_lane_id;
    
    //memset(spatType, 0, sizeof(*spatType));
    
    SPAT_t * spatType=0;
    
    IntersectionState_t * Inter=0;

	MovementState_t * MoveState[NUM_MM_STS]={0};

	Inter= (IntersectionState_t *) calloc(1,sizeof(IntersectionState_t));
	
	
	spatType=(SPAT_t *)calloc(1,sizeof(SPAT_t));
	
	spatType->msgID=13;  //SPAT message ID
	
	
	char namebuf[4]={'a','b','c','d'};  //This is the descriptive name of the spat/intersections, currently is not used.
	
	spatType->name=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,namebuf,4);
	
	//Sequence of IntersectionState
	asn_sequence_add(&spatType->intersections,Inter);
	//Intersection name
	spatType->intersections.list.array[0]->name=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,namebuf,4);
	
	//printf("InterName=%x ",spatType->intersections.list.array[0]->name.buf[0]);
	//printf("%x ",spatType->intersections.list.array[0]->name.buf[1]);
	//printf("%x ",spatType->intersections.list.array[0]->name.buf[2]);
	//printf("%x\n",spatType->intersections.list.array[0]->name.buf[3]);
	
    //Intersection id
    long temp_inter_id=NewMap.intersection.ID;
	spatType->intersections.list.array[0]->id=*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&temp_inter_id,-1);  //IntersectionID

	
	 //printf("Interid=%x ",spatType->intersections.list.array[0]->id.buf[0]);

	//Intersection status
	long interstatus=1;      //  1  = J2735SPAT_STATUS_MANUAL_CONTROL
	spatType->intersections.list.array[0]->status=*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&interstatus,-1);  //IntersectionStatusObject
	
	
	//TimeMark
	long timeM;
	gettimestamp(&secs, &usecs);
	timeM=(secs*1000) + (usecs/1000);
	spatType->intersections.list.array[0]->timeStamp=&timeM;
	
	//lanesCnt: Number of Movement State to follow
	long lanecnt=NUM_MM_STS;
	spatType->intersections.list.array[0]->lanesCnt=&lanecnt;
	
	
	
    //Movement state is defined as phases e.g. mm_st[0] -> phase 1   mm_st[7] -> phase 8      
    for(i=0; i <  NUM_MM_STS; i++)
    {
		
		//cout<<"Phase: "<<i+1<<endl;

		if (appr_phase_mapping[i]!=0)   //Phase exist!!!!!!!!!!
		{
			int temp_laneset[10];  //most have 10 lanes.
			MoveState[i]=(MovementState_t *) calloc(1,sizeof(MovementState_t));
			asn_sequence_add(&spatType->intersections.list.array[0]->states,MoveState[i]);
			spatType->intersections.list.array[0]->states.list.array[i]->movementName=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,namebuf,4);  //movementName			
			
			///////////////////////Number of lanes and Lane Set -> from MAP
			lane_no[i]=0;
			int k_left=0;
			int k_through=0;
			
			for (j=0;j<NewMap.intersection.Approaches[appr_phase_mapping[i]-1].Lane_No;j++)
			{
				
				if (i==0 ||i==2 ||i==4 ||i==6)   //phase 1 3 5 7, left turn phases
				{
					if (NewMap.intersection.Approaches[appr_phase_mapping[i]-1].Lanes[j].Attributes[2]==1)
					{
						lane_no[i]++;
						tmp_lane_id=NewMap.intersection.Approaches[appr_phase_mapping[i]-1].Lanes[j].ID;
						temp_laneset[k_left]=tmp_lane_id;
						//cout<<"Lane ID for Phase "<<i+1<<" is: "<<temp_laneset[k_left]<<endl;
						k_left++;			
					}
				}
				else //phase 2 4 6 8, through phases
				{
					if (NewMap.intersection.Approaches[appr_phase_mapping[i]-1].Lanes[j].Attributes[1]==1)
					{
						lane_no[i]++;
						tmp_lane_id=NewMap.intersection.Approaches[appr_phase_mapping[i]-1].Lanes[j].ID;
						temp_laneset[k_through]=tmp_lane_id;
						//cout<<"Lane ID for Phase "<<i+1<<" is: "<<temp_laneset[k_through]<<endl;
						k_through++;
					}
				}			
			}

			
			//Do total number of lanes
			spatType->intersections.list.array[0]->states.list.array[i]->laneCnt=lane_no+i;  //total number of lanes
			//cout<<"lane_no "<<lane_no[i]<<endl;
			
			//Do lane set
			if(i==0 ||i==2 ||i==4 ||i==6)
			{
				laneset[i]= new int[k_left];
				for(int iii=0;iii<k_left;iii++)
					laneset[i][iii]=temp_laneset[iii];
				OCTET_STRING_fromBuf(&spatType->intersections.list.array[0]->states.list.array[i]->laneSet,(char*)laneset[i],k_left);  //lane set
			}
			
			if(i==1 ||i==3 ||i==5 ||i==7)
			{
				laneset[i]= new int[k_through];
				for(int iii=0;iii<k_through;iii++)
					laneset[i][iii]=temp_laneset[iii];
				OCTET_STRING_fromBuf(&spatType->intersections.list.array[0]->states.list.array[i]->laneSet,(char*)laneset[i],k_through);  //lane set
			}
			//free(laneset);
			
			
			//printf("Laneset=%x \n",spatType->intersections.list.array[0]->states.list.array[i]->laneSet.buf[0]);
			
			
			///////////////////////Signal Status -> from controller SPAT data	
			
			
			//Note: the ASN compiler doesn't have signal present
			//mm_st[i].signal_state.present = 1;
			//mm_st[i].next_signal_state.present = 1;
			

			
			//do signal status byte: 210-215 of SPAT
			byteA=spat_buf[210];
			byteB=spat_buf[211];
			Status_red=(int)(((byteA << 8) + (byteB)));
			byteA=spat_buf[212];
			byteB=spat_buf[213];
			Status_yellow=(int)(((byteA << 8) + (byteB)));
			byteA=spat_buf[214];
			byteB=spat_buf[215];
			Status_green=(int)(((byteA << 8) + (byteB)));			
			int tmp_sig_status=1<<i;  //get the bit value of this phase
			if(Status_green & tmp_sig_status)
			{
				
				SignalLightState[i] |=1<<0;  //Greenball
				spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
				
				SignalLightState_Next[i] |=1<<1;  //Yellowball
				spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				
				
				if (i==0 ||i==2 ||i==4 ||i==6)   //phase 1 3 5 7, left turn phases
				{
					SignalLightState[i] |=1<<4;  //Green Arrow left
					spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
					
					
					SignalLightState_Next[i] |=1<<5;  //Yellow Arraw left
					spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
					
				}
				else
				{
					SignalLightState[i] |=1<<12;  //Green Arrow through
					spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
					
					SignalLightState_Next[i] |=1<<13;  //Yellow Arraw through
					spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				}
				
			}
			else if (Status_yellow &tmp_sig_status)
			{
				
				SignalLightState[i] |=1<<1;  //Yellowball
				spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
				
				SignalLightState_Next[i] |=1<<2;  //Redball
				spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				
				if (i==0 ||i==2 ||i==4 ||i==6)   //phase 1 3 5 7, left turn phases
				{
					SignalLightState[i] |=1<<5;  //Yellow Arrow left
					spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
					
					
					SignalLightState_Next[i] |=1<<6;  //Red Arraw left
					spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				}
				else
				{
					SignalLightState[i] |=1<<13;  //Yellow Arrow through
					spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
					
					SignalLightState_Next[i] |=1<<14;  //Red Arraw through
					spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				}
			}
			else if (Status_red &tmp_sig_status)
			{
				SignalLightState[i] |=1<<2;  //Redball
				spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
				
				SignalLightState_Next[i] |=1<<0;  //Greenball
				spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				
				if (i==0 ||i==2 ||i==4 ||i==6)   //phase 1 3 5 7, left turn phases
				{
					SignalLightState[i] |=1<<6;  //Red Arrow left
					spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
					
					
					SignalLightState_Next[i] |=1<<4;  //Green Arraw left
					spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				}
				else
				{
					SignalLightState[i] |=1<<14;  //Red Arrow through
					spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
					
					SignalLightState_Next[i] |=1<<12;  //Green Arraw through
					spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				}
			}
			else
			{
				SignalLightState[i]=0;  //Dark, no state
				spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
				SignalLightState_Next[i]=0;  //no next state
				spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
				
			}
			//printf("signal status: %ld\n",*spatType->intersections.list.array[0]->states.list.array[i]->currState);
			//////////////////////////////End of signal status
			
			//do Ped status byte: 216-221 of SPAT
			byteA=spat_buf[216];
			byteB=spat_buf[217];
			Ped_status_dontwalk=(int)(((byteA << 8) + (byteB)));
			byteA=spat_buf[218];
			byteB=spat_buf[219];
			Ped_status_flash=(int)(((byteA << 8) + (byteB)));
			byteA=spat_buf[220];
			byteB=spat_buf[221];
			Ped_status_walk=(int)(((byteA << 8) + (byteB)));
			
			

			
			int tmp_ped_sig_status=1<<i;  //get the bit value of this phase
			if(Ped_status_walk & tmp_ped_sig_status)
			{
				Ped_States[i] |=1<<1;   //walk=3
				Ped_States[i] |=1<<0;
				spatType->intersections.list.array[0]->states.list.array[i]->pedState=Ped_States+i;
				
				Ped_States_Next[i] |=1<<1;   //caution, flash do not walk
				spatType->intersections.list.array[0]->states.list.array[i]->yellPedState=Ped_States_Next+i;
			}
			else if (Ped_status_flash &tmp_ped_sig_status)
			{
				Ped_States[i] |=1<<1;   //caution, flash do not walk
				spatType->intersections.list.array[0]->states.list.array[i]->pedState=Ped_States+i;
				
				Ped_States_Next[i] |=1<<0;   //do not walk
				spatType->intersections.list.array[0]->states.list.array[i]->yellPedState=Ped_States_Next+i;
			}
			else if (Ped_status_dontwalk &tmp_ped_sig_status)
			{
				Ped_States[i] |=1<<0;   //do not walk
				spatType->intersections.list.array[0]->states.list.array[i]->pedState=Ped_States+i;
				
				Ped_States[i] |=1<<1;   //walk=3
				Ped_States[i] |=1<<0;
				spatType->intersections.list.array[0]->states.list.array[i]->yellPedState=Ped_States_Next+i;
			}
			else
			{
				Ped_States[i]=0;   //unavailable
				spatType->intersections.list.array[0]->states.list.array[i]->pedState=Ped_States+i;
				
				Ped_States_Next[i]=0;   //unavailable
				spatType->intersections.list.array[0]->states.list.array[i]->yellPedState=Ped_States_Next+i;
			}			
			//cout<<"ped status: "<<mm_st[i].ped_state<<endl;
			//////////////////////////////////////////End of Ped signal status
			
			specialstate[i]=0;
			spatType->intersections.list.array[0]->states.list.array[i]->specialState=specialstate+i;
	
			//do time remaining and state confidence: always do the minimum time to change (may not be accurate!!!!)
			byteA=spat_buf[3+i*13];   //MinTimeTo Change
			byteB=spat_buf[4+i*13];
			
			long time_remaining=(long)(((byteA << 8) + (byteB)));
			spatType->intersections.list.array[0]->states.list.array[i]->timeToChange=time_remaining;
			
			//cout<<"Signal remaining time is:"<<spatType->intersections.list.array[0]->states.list.array[i]->timeToChange<<endl;
			
			stateconfidence[i] |=1<<0;    //Min time
			spatType->intersections.list.array[0]->states.list.array[i]->stateConfidence=stateconfidence+i;
			
			
			byteA=spat_buf[7+i*13];   //Ped MinTimeTo Change
			byteB=spat_buf[8+i*13];
			
			//The next_time_remaining is used as the ped remaining time!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			time_remaining_ped[i]=(long)(((byteA << 8) + (byteB)));
			spatType->intersections.list.array[0]->states.list.array[i]->yellTimeToChange=time_remaining_ped+i;
			
			stateconfidence_ped[i]=0; //don't know
			spatType->intersections.list.array[0]->states.list.array[i]->yellStateConfidence=stateconfidence_ped+i;
			//cout<<"ped remaining time: "<<time_remaining<<endl;
			//printf("ped remaining time: %d\n",mm_st[i].next_time_remaining);
			///////////////////////////////end of time remaining and state confidence

			objectcount[i]=20;
			spatType->intersections.list.array[0]->states.list.array[0]->vehicleCount=objectcount+i;
			//ped detect
			long peddetect=0;
			//peddetect |=1<<1;
			spatType->intersections.list.array[0]->states.list.array[0]->pedDetect=&peddetect;
			//ped count
			ped_objectcount[i]=0;
			spatType->intersections.list.array[0]->states.list.array[0]->pedCount=ped_objectcount+1;		
		}	
		else   //Phase doesn't exist!!!!!!!!!!!!!!!!!
		{
			MoveState[i]=(MovementState_t *) calloc(1,sizeof(MovementState_t));
			asn_sequence_add(&spatType->intersections.list.array[0]->states,MoveState[i]);
			spatType->intersections.list.array[0]->states.list.array[i]->movementName=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,namebuf,4);  //movementName
			
			lane_no[i]=0;
			spatType->intersections.list.array[0]->states.list.array[i]->laneCnt=lane_no+i;  //total number of lanes

			SignalLightState[i]=0; 
			spatType->intersections.list.array[0]->states.list.array[i]->currState=SignalLightState+i;
				
			SignalLightState_Next[i]=0; 
			spatType->intersections.list.array[0]->states.list.array[i]->yellState=SignalLightState_Next+i;
			
			Ped_States[i]=0;
			spatType->intersections.list.array[0]->states.list.array[i]->pedState=Ped_States+i;
				
			Ped_States_Next[i]=0; 
			spatType->intersections.list.array[0]->states.list.array[i]->yellPedState=Ped_States_Next+i;
			
			specialstate[i]=0;
			spatType->intersections.list.array[0]->states.list.array[i]->specialState=specialstate+i;
			
            long time_remaining=999;
			spatType->intersections.list.array[0]->states.list.array[i]->timeToChange=time_remaining;
			
			stateconfidence[i]=0;
			spatType->intersections.list.array[0]->states.list.array[i]->stateConfidence=stateconfidence+i;
			
			time_remaining_ped[i]=999;
			spatType->intersections.list.array[0]->states.list.array[i]->yellTimeToChange=time_remaining_ped+i;
			
			stateconfidence_ped[i]=0; //don't know
			spatType->intersections.list.array[0]->states.list.array[i]->yellStateConfidence=stateconfidence_ped+i;
			
			objectcount[i]=0;
			spatType->intersections.list.array[0]->states.list.array[0]->vehicleCount=objectcount+i;
			//ped detect
			peddetect[i]=0;
			spatType->intersections.list.array[0]->states.list.array[0]->pedDetect=peddetect+i;
			//ped cunt
			ped_objectcount[i]=0;
			spatType->intersections.list.array[0]->states.list.array[0]->pedCount=ped_objectcount+i;
		}
    }
    
    
    asn_enc_rval_t ec; /* Encoder return value */
	asn_dec_rval_t rval;
	size_t buf_size=MAX_BUFLEN_SPAT;
	
	
	//Encode SPAT message
	cout<<"Call Encoder..."<<endl;
	ec=der_encode_to_buffer(&asn_DEF_SPAT,spatType,j2735_spat_buf,buf_size);
	
	for(i=0;i<NUM_MM_STS;i++)
		free(laneset[i]);
	
	//xer_fprint(stdout, &asn_DEF_SPAT, spatType);
	
	//for(int ii=0;ii<8;ii++)
	//{
	//	printf("signal status: %ld\n",*spatType->intersections.list.array[0]->states.list.array[ii]->currState);
	//}
	
	print_Signal_Status(spatType);
	
	//printf("Call Decoder...:\n");
	
	//SPAT_t * spatType_decode=0;
	//spatType_decode=(SPAT_t *)calloc(1,sizeof(SPAT_t));

	//rval=ber_decode(0, &asn_DEF_SPAT,(void **)&spatType_decode, j2735_spat_buf, buf_size);
	//xer_fprint(stdout, &asn_DEF_SPAT, spatType_decode);
    
    return 0;
}


int fillmap_ASN()
{
	int i,j,k;
	
	int ret;
	asn_enc_rval_t ec; /* Encoder return value */
	size_t buf_size=MAX_BUFLEN_MAP;

	
	char namebuf[1]={'a'};
	long layerType=3;
	long layerID=77;
	long intersectionid=NewMap.intersection.ID;
	long latitude=NewMap.intersection.Ref_Lat*UNIT_CHANGE_VALUE;
	long longitude=NewMap.intersection.Ref_Long*UNIT_CHANGE_VALUE;
	long elevation=NewMap.intersection.Ref_Ele;
	
	cout<<"lat long ele "<<latitude<<" "<<longitude<<" "<<elevation<<endl;
	
	//long latitude=331234567;
	//long longitude=1221234567;
	//long elevation=100;
	long inter_heading=0;
	long Landwidth=365;
	char interstatus[1]={0x01};
	long approachid[8]={1,2,3,4,5,6,7,8};
	long lanenumber[4][10]={0};
	long lanenumber_engress[4][10]={0};
	long Laneattributes_ingress[4][10]={0};
	long Laneattributes_engress[4][10]={0};
	char connect_lane[2]={0x10,0x02};
	char connect_lane_egress[2]={0x11,0x03};

	double N_offset= 0;
	double E_offset= 0;
	int  N_offset_int=0;
	int  E_offset_int=0;
	unsigned long both_Offset=0;

	char offset_bytes[4];
	char offset_bytes_Egress[4];
	char * pByte;
	
	geoCoord geoPoint;
	double ecef_x,ecef_y,ecef_z;					
	//Important!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Note the rectangular coordinate system: local_x is N(+) S(-) side, and local_y is E(+) W(-) side 
	//This is relative distance to the ref point!!!!
	//local z can be considered as 0 because no elevation change.
	double local_x,local_y,local_z;
	geoPoint.init(NewMap.intersection.Ref_Long, NewMap.intersection.Ref_Lat, NewMap.intersection.Ref_Ele);
	
	MAP_t * MapType=0;
	Intersection_t * Inter=0;
	ApproachObject_t * Approaches[4]={0};     //four approaches
	VehicleReferenceLane_t * Lanes=0;
	Offsets_t *Offsets[20]={0};              //maximum 20 waypoints
	Position3D_t * ref_point=0;
	Approach_t * Appr[4]={0};     
	Approach_t * Egress[4]={0};   
	NodeList_t *Node_list=0;
	NodeList_t *Node_list_engress=0;
	ConnectsTo_t *ConnLane=0;
	VehicleReferenceLane_t * EgressLanes[4][10]={0};	//maximum 10 egress lanes
	VehicleReferenceLane_t * IngressLanes[4][10]={0};	//maximum 10 approaches lanes
	
	MAP_t  * Map_decoder=0;
	
	Map_decoder=(MAP_t *)calloc(1,sizeof(MAP_t));
	
	MapType=(MAP_t *)calloc(1,sizeof(MAP_t));
	Inter= (Intersection_t *) calloc(1,sizeof(Intersection_t));
	
	MapType->msgID=7;  //MAP message ID

	MapType->msgCnt=100;  //MAP message count

	MapType->name=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,namebuf,1);  //message name

	MapType->layerType=&layerType; // layertype 3= intersection data

	MapType->layerID=&layerID;     // layerID
	
	//add one intersection
 	asn_sequence_add(&MapType->intersections,Inter);
	
	MapType->intersections.list.array[0]->name=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,namebuf,1); //intersection name

	MapType->intersections.list.array[0]->id=*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&intersectionid,-1);  //IntersectionID;

	//reference point: latitude, longitude and elevation

	ref_point=(Position3D_t *) calloc(1,sizeof(Position3D_t));

	asn_sequence_add(&MapType->intersections.list.array[0]->refPoint,ref_point);
	MapType->intersections.list.array[0]->refPoint->lat=latitude;
	MapType->intersections.list.array[0]->refPoint->Long=longitude;
	MapType->intersections.list.array[0]->refPoint->elevation=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&elevation,-1); //elevation
	
	//refInterNum
	MapType->intersections.list.array[0]->refInterNum=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&intersectionid,-1);

	//Intersection heading 
	MapType->intersections.list.array[0]->orientation=&inter_heading;

	//Lane width
	MapType->intersections.list.array[0]->laneWidth=&Landwidth;

	//Intersection status object
	MapType->intersections.list.array[0]->type=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,interstatus,1);
	
	
	//Start to populate approaches
	int approach_num=(int) NewMap.intersection.Appro_No/2;
	for (i=0;i<approach_num;i++)
	{
		//add approach
		Approaches[i]=(ApproachObject_t *) calloc(1,sizeof(ApproachObject_t));
		asn_sequence_add(&MapType->intersections.list.array[0]->approaches,Approaches[i]);
		
		Appr[i]=(Approach_t *) calloc(1,sizeof(Approach_t));
		MapType->intersections.list.array[0]->approaches.list.array[i]->approach=Appr[i];
		
		//approach name
		MapType->intersections.list.array[0]->approaches.list.array[i]->approach->name=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,namebuf,1);
		
		//Approach id
		MapType->intersections.list.array[0]->approaches.list.array[i]->approach->id=approachid+i*2;  //Approach number is 1,3,5,7 only the ingress lanes of the Batelle MAP
		
		//Do approach(Ingress) lanes
		int Ingress_Lane_No=NewMap.intersection.Approaches[i*2].Lane_No;   //Number of lanes of approach 1,3,5,7
		for(j=0;j<Ingress_Lane_No;j++)
		{
			IngressLanes[i][j]=(VehicleReferenceLane_t *) calloc(1,sizeof(VehicleReferenceLane_t));
			asn_sequence_add(&MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes,IngressLanes[i][j]);
			//lane number
			lanenumber[i][j]=NewMap.intersection.Approaches[i*2].Lanes[j].ID;
			MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneNumber=*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&lanenumber[i][j],-1);
			//lane width
			MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneWidth=&Landwidth;
			//lane attributes
			if(NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[1]==1 ) //straight permitted
			Laneattributes_ingress[i][j] |=1<<1;  //through permitted
			if(NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[2]==1) //left turn permitted
			Laneattributes_ingress[i][j] |=1<<2;  //left permitted
			if(NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[3]==1 && NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[1]==0) //right turn permitted
			Laneattributes_ingress[i][j] |=1<<3;  //right permitted
			MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->laneAttributes=Laneattributes_ingress[i][j];
			//Do lane nodes
			int node_no=NewMap.intersection.Approaches[i*2].Lanes[j].Node_No;
		
			
			
			
			for(k=0;k<node_no;k++)
			{
				Node_list=(NodeList_t *) calloc(1,sizeof(NodeList_t));
				asn_sequence_add(&MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList,Node_list);
				//change to local coordinates			
				geoPoint.lla2ecef(NewMap.intersection.Approaches[i*2].Lanes[j].Nodes[k].Longitude,NewMap.intersection.Approaches[i*2].Lanes[j].Nodes[k].Latitude,NewMap.intersection.Ref_Ele,&ecef_x,&ecef_y,&ecef_z);
				geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x,&local_y,&local_z);
				
				E_offset=local_y;
				N_offset=local_x;
				N_offset_int=32768+N_offset*10;
				E_offset_int=32768+E_offset*10;
				both_Offset=(N_offset_int<<16)+E_offset_int;
				
				sprintf(tmp_log,"N_Offset %d E_offset %d both_offset %ld\n",N_offset_int,E_offset_int,both_Offset);
				//cout<<tmp_log;
				
				char * pByte;
				pByte=(char*)&both_Offset;
				offset_bytes[0]= (char) *(pByte+3);
				offset_bytes[1]= (char) *(pByte+2);
				offset_bytes[2]= (char) *(pByte+1);
				offset_bytes[3]= (char) *(pByte+0);				
				MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,offset_bytes,4);				
				
				sprintf(tmp_log,"%x %x %x %x\n",MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[0],
				MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[1],
				MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[2],
				MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->nodeList.list.array[k]->buf[3]);
				
				//cout<<tmp_log;
			}
			
			//Do connected lane
			//get connection lane No. first
			char conn_bytes[4]={0};
			
			int conn_lane_no=NewMap.intersection.Approaches[i*2].Lanes[j].Connection_No;
			conn_bytes[0]=conn_lane_no;
			//printf("conn_bytes[0] %d\n",conn_bytes[0]);
			for(int kkk=0;kkk<conn_lane_no;kkk++) //add each connection lane
			{
				conn_bytes[kkk+1]=NewMap.intersection.Approaches[i*2].Lanes[j].Connections[kkk].ConnectedLaneName.Approach*10+NewMap.intersection.Approaches[i*2].Lanes[j].Connections[kkk].ConnectedLaneName.Lane;
				//printf("conn_bytes[kkk+1] %d\n",conn_bytes[kkk+1]);
			}			
			
			MapType->intersections.list.array[0]->approaches.list.array[i]->approach->drivingLanes.list.array[j]->connectsTo=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,conn_bytes,-1);			
						
		}
		
		//Do Engress approach
		//add approach
		Egress[i]=(Approach_t *) calloc(1,sizeof(Approach_t));
		MapType->intersections.list.array[0]->approaches.list.array[i]->egress=Egress[i];
		
		//approach name
		MapType->intersections.list.array[0]->approaches.list.array[i]->egress->name=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,namebuf,1);
		
		//Approach id
		MapType->intersections.list.array[0]->approaches.list.array[i]->egress->id=approachid+i*2+1;  //Approach number is 2,4,6,8 only the egress lanes of the Batelle MAP
		
		//Do Engress lanes
		int Egress_Lane_No=NewMap.intersection.Approaches[i*2+1].Lane_No;   //Number of lanes of approach 1,3,5,7
		for(j=0;j<Egress_Lane_No;j++)
		{
			EgressLanes[i][j]=(VehicleReferenceLane_t *) calloc(1,sizeof(VehicleReferenceLane_t));
			asn_sequence_add(&MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes,EgressLanes[i][j]);
			//lane number
			lanenumber_engress[i][j]=NewMap.intersection.Approaches[i*2+1].Lanes[j].ID;
			MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->laneNumber=*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,(char*)&lanenumber_engress[i][j],-1);
			//lane width
			MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->laneWidth=&Landwidth;
			//lane attributes
			Laneattributes_engress[i][j] |=1<<1;  //through permitted
			MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->laneAttributes=Laneattributes_engress[i][j];
			//Do lane nodes
			int node_no=NewMap.intersection.Approaches[i*2+1].Lanes[j].Node_No;
			for(k=0;k<node_no;k++)
			{
				Node_list_engress=(NodeList_t *) calloc(1,sizeof(NodeList_t));
				asn_sequence_add(&MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->nodeList,Node_list_engress);
				//change to local coordinates			
				geoPoint.lla2ecef(NewMap.intersection.Approaches[i*2+1].Lanes[j].Nodes[k].Longitude,NewMap.intersection.Approaches[i*2+1].Lanes[j].Nodes[k].Latitude,NewMap.intersection.Ref_Ele,&ecef_x,&ecef_y,&ecef_z);
				geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x,&local_y,&local_z);
				
				E_offset=local_y;
				N_offset=local_x;
				N_offset_int=32768+N_offset*10;
				E_offset_int=32768+E_offset*10;
				both_Offset=(N_offset_int<<16)+E_offset_int;
				
				char * pByte;
				pByte=(char*)&both_Offset;
				offset_bytes[0]= (char) *(pByte+3);
				offset_bytes[1]= (char) *(pByte+2);
				offset_bytes[2]= (char) *(pByte+1);
				offset_bytes[3]= (char) *(pByte+0);				
				MapType->intersections.list.array[0]->approaches.list.array[i]->egress->drivingLanes.list.array[j]->nodeList.list.array[k]=OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING,offset_bytes,4);				
				
				
				
			}
			//Do connected lane: no need to do connection lanes for engress lanes
			
		}
	}
	
	
	printf("Call Encoder...\n");
	ec=der_encode_to_buffer(&asn_DEF_MAP,MapType,j2735_map_buf,buf_size);
	
	for(int ii=0;ii<MAX_BUFLEN_MAP;ii++)
	{
		printf("%2x ",(unsigned char)j2735_map_buf[ii]);
		if(ii%16==15 && ii>0){
			printf("\n");}
	}
	
	
	asn_dec_rval_t rval;
	rval=ber_decode(0, &asn_DEF_MAP,(void **)&Map_decoder, j2735_map_buf, buf_size);
	
	if(rval.code==RC_OK)
	{
		printf("Map decode successfully!\n");
	}
	else
		printf("Map decode failure!\n");
	
	xer_fprint(stdout, &asn_DEF_MAP, Map_decoder);
	return 1;

}



void write_inlane_outlane_phase_mapping_to_file(char *filename)
{
	int i,j;
	char log[128];
	fstream fs;
	fs.open(filename, ios::out);
	
	if (!fs || !fs.good())
	{
		cout << "could not open file!\n";
	}
	
	sprintf(log,"IntersectionID\t%d\n",NewMap.intersection.ID);
	fs<<log;
	sprintf(log,"No_Approach\t%d\n",NewMap.intersection.Appro_No);
	fs<<log;


	int count=0;
	for(i=0;i<8;i++)
	{
		if(phase_mapping[i]!=0)
		count++;
	}	
	sprintf(log,"No_Phase\t%d\n",count);
	fs<<log;


	int No_Ingress=NewMap.intersection.Appro_No/2;
	sprintf(log,"No_Ingress\t%d\n",No_Ingress);
	fs<<log;

	for(i=0;i<No_Ingress;i++)
	{
		sprintf(log,"Approach\t%d\n",i*2+1);
		fs<<log;
		int lane_no=NewMap.intersection.Approaches[i*2].Lane_No;
		for(j=0;j<NewMap.intersection.Approaches[i*2].Lane_No;j++)
		{
			if(NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[3]==1 && NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[1]==0 && NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[2]==0) //only right turn lane
			lane_no--;
		}
		sprintf(log,"No_Lane\t%d\n",lane_no);
		fs<<log;
		
		for(j=0;j<NewMap.intersection.Approaches[i*2].Lane_No;j++)
		{
			int phase;
			if(NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[3]==1) //right turn lane
				phase=0;   //write turn lane doesn't require any phase
			if(NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[1]==1) //through lane
				phase=phase_mapping[i*2];   //put through lane after right turn lane, to make sure a through-right turn lane can be written successfully
			if(NewMap.intersection.Approaches[i*2].Lanes[j].Attributes[2]==1) //left turn lane
				phase=phase_mapping[i*2+1];		
			if(phase!=0)
			{	
				sprintf(log,"%s %d.%d %d\n",NewMap.intersection.Approaches[i*2].Lanes[j].Lane_Name.c_str(),NewMap.intersection.Approaches[i*2].Lanes[j].Connections[0].ConnectedLaneName.Approach,
					NewMap.intersection.Approaches[i*2].Lanes[j].Connections[0].ConnectedLaneName.Lane,phase);
				fs<<log;
			}
		}
		sprintf(log,"end_Approach\n");
		fs<<log;
	}
	sprintf(log,"end_file");
	fs<<log;
	fs.close();
}

void Pack_ART(byte* ART_data, char * filename, int &size)
{
	int i;
	byte* pByte;
	int offset=0;
	
	unsigned short   tempUShort;
    long    tempLong;
    double  temp;
	
	char tempString[20];
	
	fstream fs;
    fs.open(filename);
    char tmp[128];
	string temp_string;
    getline(fs,temp_string);  
    strcpy(tmp,temp_string.c_str());
	int NumReq=0;
	sscanf(tmp,"%*s %d",&NumReq);
	cout<<"NumReq is: "<<NumReq<<endl;
	
	if(NumReq==-1)   //-1 means at the beginning of the program, make it 0 here.
	NumReq=0;
	
	//Do Intersection ID;
	tempLong = (long) NewMap.intersection.ID; 
	pByte = (byte* ) &tempLong;
	ART_data[offset+0] = (byte) *(pByte + 3); 
	ART_data[offset+1] = (byte) *(pByte + 2); 
	ART_data[offset+2] = (byte) *(pByte + 1); 
	ART_data[offset+3] = (byte) *(pByte + 0); 
	offset = offset + 4;
	
	//Pack Number of Requests
	tempUShort = (unsigned short)NumReq;
	pByte = (byte* ) &tempUShort;
	ART_data[offset+0] = (byte) *(pByte + 1); 
    ART_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	//Start packing the active request table
	for(i=0;i<NumReq;i++)
	{
		getline(fs,temp_string);  //First line is comment
		strcpy(tmp,temp_string.c_str());
		long vehid=0;
		int veh_class=0;
		float ETA=0;
		int phase=0;
		float Tq=0;
		long abs_time=0;
		//int split_phase=0;
		int inlane=0;
		int outlane=0;
		int shour=0;
		int smin=0;
		int ssec=0;
		int ehour=0;
		int emin=0;
		int esec=0;
		int veh_state=0;    //1: approaching 2: leaving 3: Inqueue
		int req_sequence=0;
		sscanf(tmp,"%*s %ld %d %f %d %f %ld %d %d %d %d %d %d %d %d %d %d",&vehid,&veh_class,&ETA,&phase,&Tq,&abs_time,&inlane,&outlane,&shour,&smin,&ssec,&ehour,&emin,&esec,&veh_state,&req_sequence);
		//Do Veh ID;
		tempLong = (long) vehid; 
		pByte = (byte* ) &tempLong;
		ART_data[offset+0] = (byte) *(pByte + 3); 
		ART_data[offset+1] = (byte) *(pByte + 2); 
		ART_data[offset+2] = (byte) *(pByte + 1); 
		ART_data[offset+3] = (byte) *(pByte + 0); 
		offset = offset + 4;
		//Do veh_class
		tempUShort = (unsigned short)veh_class;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do ETA
		tempUShort = (unsigned short)(ETA*FLOAT2INT);
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//cout<<"ETA: "<<tempUShort<<endl;
		//Do phase
		tempUShort = (unsigned short)phase;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do Tq
		tempUShort = (unsigned short)(Tq*FLOAT2INT);
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do abs_time
		tempLong = (long) abs_time; 
		pByte = (byte* ) &tempLong;
		ART_data[offset+0] = (byte) *(pByte + 3); 
		ART_data[offset+1] = (byte) *(pByte + 2); 
		ART_data[offset+2] = (byte) *(pByte + 1); 
		ART_data[offset+3] = (byte) *(pByte + 0); 
		offset = offset + 4;
		//Do split phase
		//tempUShort = (unsigned short)split_phase;
		//pByte = (byte* ) &tempUShort;
		//ART_data[offset+0] = (byte) *(pByte + 1); 
		//ART_data[offset+1] = (byte) *(pByte + 0); 
		//offset = offset + 2;
		//Do inlane
		tempUShort = (unsigned short)inlane;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do outlane
		tempUShort = (unsigned short)outlane;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do shour
		tempUShort = (unsigned short)shour;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do smin
		tempUShort = (unsigned short)smin;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do ssec
		tempUShort = (unsigned short)ssec;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do ehour
		tempUShort = (unsigned short)ehour;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do emin
		tempUShort = (unsigned short)emin;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do esec
		tempUShort = (unsigned short)esec;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do veh_state
		//if (strcmp(tempString,"Approaching")==0)
		//veh_state=1;
		//if (strcmp(tempString,"Leaving")==0)
		//veh_state=2;
		//if (strcmp(tempString,"InQueue")==0)
		//veh_state=3;
		
		tempUShort = (unsigned short)veh_state;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		//Do request sequence
		tempUShort = (unsigned short)req_sequence;
		pByte = (byte* ) &tempUShort;
		ART_data[offset+0] = (byte) *(pByte + 1); 
		ART_data[offset+1] = (byte) *(pByte + 0); 
		offset = offset + 2;
		
				
	}
    size=offset;
}


int print_Ped_Status(SPAT_t *spatType,char *filename)
{
	fstream fs;
	fs.open(filename, ios::out);
	
	if (!fs || !fs.good())
	{
		cout << "could not open file!\n";
		exit(0);
	}
	
	int i;
	//printf("The Ped status are:\n");
	for(i=0;i<NUM_MM_STS;i++)
	{
		if(*spatType->intersections.list.array[0]->states.list.array[i]->pedState ==0)
		{
			//printf("N/A ");
			fs<<"0 ";
		}	
		if(*spatType->intersections.list.array[0]->states.list.array[i]->pedState ==3)
		{
			//printf("G ");
			fs<<"1 ";
		}
		if(*spatType->intersections.list.array[0]->states.list.array[i]->pedState ==2)
		{
			//printf("Y ");
			fs<<"2 ";
		}
		if(*spatType->intersections.list.array[0]->states.list.array[i]->pedState ==1)
		{
			//printf("R ");
			fs<<"3 ";
		}
	}
	//printf("\n");
	fs<<"\n";
		
	//printf("The Ped Remaining time are:\n");
	//~ for(i=0;i<NUM_MM_STS;i++)
	//~ {
		//~ printf("%d ",mm_st[i].next_time_remaining);
		//~ fs<<mm_st[i].next_time_remaining<<" ";
	//~ }
	//~ printf("\n");
	//~ fs<<"\n";
	
	fs.close();	
	return 0;
}

int print_Signal_Status(SPAT_t *spat)
{
	
		
	if(field_flag==1)
	{
		//savari_gps_read (&gps, gps_handle);
		sprintf(tmp_log,"At time: %.2lf, Signal Status is: ",GetSeconds());
	}
	else
	{
		sprintf(tmp_log,"At time: %.2lf Signal Status is: ",GetSeconds());
	}
	
	
	
	
	int i;
	for(i=0;i<NUM_MM_STS;i++)
	{
	//printf("signal status: %ld\n",*spat->intersections.list.array[0]->states.list.array[i]->currState);	
		if(*spat->intersections.list.array[0]->states.list.array[i]->currState == 0)
		{
			//printf("N/A ");
			strcat(tmp_log,"0 ");

		}	
		if(*spat->intersections.list.array[0]->states.list.array[i]->currState & (1<<0))
		{
			//printf("G ");
			strcat(tmp_log,"3 ");
		}
		if(*spat->intersections.list.array[0]->states.list.array[i]->currState & (1<<1))
		{
			//printf("Y ");
			strcat(tmp_log,"4 ");
		}
		if(*spat->intersections.list.array[0]->states.list.array[i]->currState & (1<<2))
		{
			//printf("R ");
			strcat(tmp_log,"1 ");
		}
	}
	strcat(tmp_log,"\n");
	outputlog(tmp_log);
}


//~ int gps_init () {
    //~ int is_async = 0;
    //~ int fd;
//~ 
    //~ gps_handle = savari_gps_open(&fd, is_async);
    //~ if (gps_handle == 0) {
        //~ printf("sorry no gpsd running\n");
        //~ return -1;
    //~ }
//~ 
    //~ return 0;
//}
