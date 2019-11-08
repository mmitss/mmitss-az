#pragma once

#include "SignalRequest.h"
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


// We need to connect to the Controller when there is an EV in the list. This is required to know the split phase when an EV is among requests. 
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
#define SIM_BROADCAST_ADDR "10.254.56.5"   //used for getting VISSIM Simulation Time. It is the IP of the machine that VISSIM is running on

// This application can be applied only for priority eligible vehicles OR can be integrated into COP (To Do)
#define PRIORITY 1


// This file contains the mode weights, coordination phase split time and the coordination weight.
#define PRIORITY_CONFIG_FILE "/nojournal/bin/priorityConfiguration.txt" 

// Stores the IntersectionID, RegionalID, and Mapfile name for the intersection
#define INTERSECTION_CONFIG_FILE_JSON "/nojournal/bin/IntersectionConfig.json"

// This file stores the number of requests and the ReqListUpdateFlag and the request's information
// This file will be modified whenever the request list is updated. The file is being read by Solver. 
// The Solver will make ReqListUpdateFlag flag zero and rewrite the content in request.txt
#define REQUESTFILENAME "/nojournal/bin/requests.txt"

// This file stores the number of requests and the ReqListUpdateFlag and the request's information
// it will be modified whenever the request list is updated. The file is being read by Solver. 
// The Solver will make ReqListUpdateFlag flag zero and rewrite the content in request.txt
// The difference between requests.txt and requests_combined.txt is that requests_combined includes 
// the requested split_phase when there is EV in the request list. 
#define REQUESTFILENAME_COMBINED "/nojournal/bin/requests_combined.txt" 

#define CONFIG_INFO_FILE "/nojournal/bin/ConfigInfo.txt"
#define RSUID_FILENAME "/nojournal/bin/rsuid.txt"
#define LOG_FILENAME "/nojournal/bin/log/MMITSS_MRP_PriorityRequestServer_"
#define LANEPHASE_FILENAME "/nojournal/bin/InLane_OutLane_Phase_Mapping.txt"
#define IPINFO_FILENAME "/nojournal/bin/IPInfo.txt" // the file to read the traffic signal controller IP and port

double getSimulationTime(const char *);  //from VISSIM 

void obtainInLaneOutLane(int srmInLane, int srmOutLane, int &inApproach, int &outApproach, int &iInlane, int &Outlane);

void calculateETA(int beginMin, int beginSec, int endMin, int endSec, int &iETA);

// To send clear commands to the controller when the last request passes the intersection
void sendClearCommandsToInterface();

void packEventList(char *tmp_event_data, int &size);  


void setupConnection(int &,long); 

// To set up udp socket to get SRM (FIELD case), or SRM and VISSIM time (SIMULATION case)
void getControllerIPaddress();

void getSignalConfigFile(char *, int *);

// We need to get signal status to determine the split phase in case one priority vehicle is an EV
void readPhaseTimingStatus(int PhaseStatus[8]);  

int getSignalColor(int PhaseStatusNo);

int outputlog(char *output);

void printReqestFile2Log(const char *resultsfile);

void xTimeStamp(char *pc_TimeStamp_);

int msleep(unsigned long milisec);

string getRSUid();

void clearRequestFiles();

void creatLogFile();

void setupConfigurationAndConnection();

int FindVehClassInList(LinkedList <ReqEntry> Req_List, int VehClass);

void startUpdateETAofRequestsInList(const string &, LinkedList <ReqEntry> &, int&, bool&);

void processRxMessage(const char *rxMsgBuffer, char tempMsg[], string &Rsu_id, const IntLanePhase lanePhase);

void sendSSM(LinkedList<ReqEntry> ReqList, const int IntersectionID, UdpSocket MsgReceiverSocket);

double getSystemTime();

void identifyColor(int color[2][8], int greenGroup, int redGroup, int yellowGroup, int PhaseNext);

void whichPhaseIsGreen(int phase_Color[8], int greenGroup, int redGroup,
                       int yellowGroup); // this function returns the color of the first argument which is phaseNo.

int getIntersectionID(void);

int getPhaseInfo(SignalRequest signalRequest);

