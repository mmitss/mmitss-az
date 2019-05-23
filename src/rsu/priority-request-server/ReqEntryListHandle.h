#pragma once


#define EV 1      ////*Emergency vehicle: will have two split phases requests and priority as 1
#define TRANSIT 2 ////*Transit bus: will have one request and priority as 2
#define TRUCK 3
#define COORDINATION 6


extern int iObsoleteTimeOfRemainedReq;
extern double dCountDownIntervalForETA;
extern int ReqListUpdateFlag;    // The Flag to identify the ReqList update
extern int flagForClearingInterfaceCmd;
extern string RSUID;    // will get from "rsuid.txt"
extern int outputlog(char *output);

extern char logfilename[256];
extern char temp_log[256];
extern int CombinedPhase[8];

extern PriorityConfig priorityConfig;
extern double dTime;
extern double dCurrentTimeInCycle;


//----------------------------------------------------------------------------------------------//
int FindInReqList(LinkedList <ReqEntry> ReqList, ReqEntry TestEntry);

void UpdateList(LinkedList <ReqEntry> &Req_List, char *RcvMsg, int phaseStatus[8], int&, int CombinedPhase[]);

void PrintList2File(const char *Filename, const string& rsu_id, LinkedList <ReqEntry> &ReqList, int, int IsCombined = 0);

void PrintList(LinkedList <ReqEntry> &ReqList);

//int ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List);
int getCurrentFlagInReqFile(const char *filename);

int ReqListFromFile(char *filename, LinkedList <ReqEntry> &Req_List);

int FindSplitPhase(int phase, int phaseStatus[8], int CombinedPhase[]);

int numberOfEVs(LinkedList <ReqEntry> Req_List);

int FindTimesInList(LinkedList <ReqEntry> Req_List, int Veh_Class);

void updateETAofRequestsInList(LinkedList <ReqEntry> &Req_List, int &);

void deleteThePassedVehicle(LinkedList <ReqEntry> &Req_List, int &);

int currentFlagInRequestFile(char *filename);
