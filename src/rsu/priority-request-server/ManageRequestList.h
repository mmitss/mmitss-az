#pragma once


#define EV 1      ////*Emergency vehicle: will have two split phases requests and priority as 1
#define TRANSIT 2 ////*Transit bus: will have one request and priority as 2
#define TRUCK 3
#define COORDINATION 6
#define OBSOLETE_TIME_OF_REMAINED_REQ 30   // if a request is not updated for iObsoleteTimeOfRemainingReq second in request list, it should be deleted ??????

//**eliminate the stench of global vatiables**
//extern double dCountDownIntervalForETA;
//extern int ReqListUpdateFlag;    // The Flag to identify the ReqList update
//extern int flagForClearingInterfaceCmd;
//extern char temp_log[256];
//extern double dCurrentTimeInCycle;

extern string RSUID;    // will get from "rsuid.txt"
extern int outputlog(char *output);

extern char logfilename[256];

extern int CombinedPhase[8];

extern PriorityConfig priorityConfig;
extern double dTime;



//----------------------------------------------------------------------------------------------//
int FindInReqList(LinkedList <ReqEntry> ReqList, ReqEntry TestEntry);

void UpdateList(LinkedList <ReqEntry> &Req_List, char *RcvMsg, int phaseStatus[8], int&, int CombinedPhase[], int &);

void PrintList2File(const char *Filename, const string& rsu_id, LinkedList <ReqEntry> &ReqList, int, int IsCombined = 0);

void PrintList(LinkedList <ReqEntry> &ReqList);

//int ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List);

int getCurrentFlagInReqFile(const char *filename);

int ReqListFromFile(char *filename, LinkedList <ReqEntry> &Req_List);

int FindSplitPhase(int phase, int phaseStatus[8], int CombinedPhase[]);

int numberOfEVs(LinkedList <ReqEntry> Req_List);

int FindTimesInList(LinkedList <ReqEntry> Req_List, int Veh_Class);

void updateETAofRequestsInList(LinkedList <ReqEntry> &Req_List, int &, const double, const double);

void deleteThePassedVehicle(LinkedList <ReqEntry> &Req_List, int &, int &);

int currentFlagInRequestFile(char *filename);
