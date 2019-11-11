#pragma once


#define EV 1      ////*Emergency vehicle: will have two split phases requests and priority as 1
#define TRANSIT 2 ////*Transit bus: will have one request and priority as 2
#define TRUCK 3
#define COORDINATION 6
#define OBSOLETE_TIME_OF_REMAINED_REQ 300 //30   // if a request is not updated for iObsoleteTimeOfRemainingReq second in request list, it should be deleted ??????

#define PRIORITY_REQUEST 1
#define REQUEST_UPDATE 2
#define PRIORITY_CANCELLATION 3

// ReqListUpdateFlag values
#define NO_UPDATE 0
#define ADD_NEW_REQUEST 1 //ADD a new request
#define UPDATED_REQUEST 2  // UPDATED request (changing the speed, joining the queue, leaving the queue)
#define DELETE_OBSOLETE_REQUEST 3 // DELETE an obsolete request
#define CANCEL_REQUEST_LEAVING_INTERSECTION 4 // CANCEL a request due to leaving intersection

#define COUNT_DOWN_INTERVAL_FOR_ETA 1 // The time interval that the ETA of requests in the requests table is updated for the purpose of count down

extern int outputlog(char *output);

extern char logfilename[256];
extern double dTime;



//----------------------------------------------------------------------------------------------//
int FindInReqList(LinkedList <ReqEntry> ReqList, ReqEntry TestEntry);

void UpdateList(LinkedList <ReqEntry> &Req_List, char *RcvMsg, int phaseStatus[8], int&, int CombinedPhase[], bool &);

void PrintList2File(const char *Filename, const string& rsu_id, LinkedList <ReqEntry> &ReqList, int, int IsCombined = 0);

void PrintList(LinkedList <ReqEntry> &ReqList);

//int ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List);

int getCurrentFlagInReqFile(const char *filename);

int ReqListFromFile(char *filename, LinkedList <ReqEntry> &Req_List);

int FindSplitPhase(int phase, int phaseStatus[8], int CombinedPhase[]);

int numberOfEVs(LinkedList <ReqEntry> Req_List);

int FindTimesInList(LinkedList <ReqEntry> Req_List, int Veh_Class);

void updateETAofRequestsInList(LinkedList <ReqEntry> &Req_List, int &);

void deleteThePassedVehicle(LinkedList <ReqEntry> &Req_List, int &, bool &);

int currentFlagInRequestFile(char *filename);
