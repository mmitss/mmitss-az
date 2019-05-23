#pragma once


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

// This file contains the mode weights, coordination phase split time and the coordination weight.
#define PRIORITY_CONFIG_FILE "/nojournal/bin/priorityConfiguration.txt" 

// This file that stores the number of requests and the ReqListUpdateFlag and the requests information
// This file will be modified whenever the request list is updated. The file is being read by Solver. 
// The Solver will make ReqListUpdateFlag flag zero and rewrite the content in request.txt
#define REQUESTFILENAME "/nojournal/bin/requests.txt"

// This file that stores the number of requests and the ReqListUpdateFlag and the requests information
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

double getSimulationTime(char *);  //from VISSIM 

void obtainInLaneOutLane(int srmInLane, int srmOutLane, int &inApproach, int &outApproach, int &iInlane, int &Outlane);

void calculateETA(int beginMin, int beginSec, int endMin, int endSec, int &iETA);

void packEventList(char *tmp_event_data,
                   int &size);  // To send clear commands to the controller when the last request passes the intersection
void sendClearCommandsToInterface();

void setupConnection(int &);                               // To set up udp socket to get SRM (FIELD case), or SRM and VISSIM time (SIMULATION case)
void getControllerIPaddress();

void getSignalConfigFile(char *, int *);

void readPhaseTimingStatus(
        int PhaseStatus[8]);  // We need to get signal status to determin the split phase in case one priority vehicle is EV
int getSignalColor(int PhaseStatusNo);

int outputlog(char *output);

void printReqestFile2Log(const char *resultsfile);

void xTimeStamp(char *pc_TimeStamp_);

int msleep(unsigned long milisec);

void getRSUid(string);

void clearRequestFiles();

void creatLogFile();

void setupConfigurationAndConnection();

void readSignalControllerAndGetActivePhases();

int FindVehClassInList(LinkedList <ReqEntry> Req_List, int VehClass);

int gpsInitialization();

int doWeNeedToSolveForCoordRequests(bool &, bool&, bool &);

void calculateETAofCoordRequests(bool &, bool&, bool &);

void updateCoordRequestsInList(LinkedList <ReqEntry> &,int &, int []);

void startUpdateETAofRequestsInList(const string &, LinkedList <ReqEntry> &, int&);

void setCoordinationPriorityRequests(LinkedList <ReqEntry> &, int&, int []);

void processRxMessage(LinkedList <ReqEntry> &Req_List, int&, int []);

double readGPStime();

double getSystemTime();

void identifyColor(int color[2][8], int greenGroup, int redGroup, int yellowGroup, int PhaseNext);

void whichPhaseIsGreen(int phase_Color[8], int greenGroup, int redGroup,
                       int yellowGroup); // this function returns the color of the first argument which is phaseNo.


