#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <istream>
#include <math.h>
#include "LinkedList.h"
#include "ReqEntry.h"
#include "msgEnum.h"
#include "ManageRequestList.h"

using namespace std;

int FindTimesInList(LinkedList<ReqEntry> Req_List, int Veh_Class)
{
    Req_List.Reset();
    int times = 0;

    while (!Req_List.EndOfList())
    {
        if (Req_List.Data().VehClass == Veh_Class)
            times++;

        Req_List.Next();
    }

    return times;
}

void UpdateList(LinkedList<ReqEntry> &Req_List, char *RcvMsg, int phaseStatus[8],
                int &ReqListUpdateFlag, int CombinedPhase[], bool &clearSignalControllerCommands)
{

    char temp_log[256];
    int iNewReqDiviser = 0;
    int iRecvReqListDiviser = 0;
    ReqEntry NewReq; // default NewReq.Split_Phase=-10;
    char RSU_ID[16]{};

    sscanf(RcvMsg, "%d %s %ld %d %f %d %f %lf %d %d %d %d %d %d %d %d %d %d %lf %ld", &NewReq.iRequestType, RSU_ID,
           &NewReq.VehID, &NewReq.VehClass,
           &NewReq.ETA, &NewReq.Phase, &NewReq.MinGreen, &NewReq.dSetRequestTime,
           &NewReq.iInLane, &NewReq.iOutLane, &NewReq.iStrHour, &NewReq.iStrMinute, &NewReq.iStrSecond,
           &NewReq.iEndHour, &NewReq.iEndMinute, &NewReq.iEndSecond, &NewReq.iVehState, &NewReq.iMsgCnt,
           &NewReq.dTimeInCycle, &NewReq.lIntersectionId);

    // DC - We never put anything from the SRM into MinGreen and solver does not use it...........
    //if (NewReq.MinGreen > 0) // means vehicle is in the queue, we need queue clearance to be considered in Solver
    //    NewReq.ETA = 0;

    NewReq.dUpdateTimeOfETA = dTime;

    // Handle split phases for EVs in list
    int iNumberOfEVinList = numberOfEVs(Req_List);
    int SplitPhase = 0;

    if ((NewReq.VehClass == EV) && (iNumberOfEVinList == 0)) // Only the first EV in the list gets a split phase
    {
        SplitPhase = FindSplitPhase(NewReq.Phase, phaseStatus, CombinedPhase);
        NewReq.Split_Phase = SplitPhase;
    }
    else
        NewReq.Split_Phase = 0;

    // Begin processing the received request

    int pos = FindInReqList(Req_List, NewReq);

    // the vehicle is approaching the intersection and requesting a phase (or it is in the queue... DC - old comment? not tested here? ) 
    if (NewReq.iRequestType == PRIORITY_REQUEST && NewReq.Phase > 0)
    {
        if (pos < 0) // Request is NOT in the List, problem should be solved by Solver, therefore ReqListUpdateFlag becomes positive
        {
            Req_List.InsertRear(NewReq);

            Req_List.Reset(0);

            sprintf(temp_log, "%ld %d %.2f %d %.2f %.2f %d %d %d %d %d %d %d %d %d %d %.2f \n", Req_List.Data().VehID,
                    Req_List.Data().VehClass, Req_List.Data().ETA, Req_List.Data().Phase, Req_List.Data().MinGreen,
                    Req_List.Data().dSetRequestTime, Req_List.Data().iInLane, Req_List.Data().iOutLane,
                    Req_List.Data().iStrHour, Req_List.Data().iStrMinute, Req_List.Data().iStrSecond,
                    Req_List.Data().iEndHour, Req_List.Data().iEndMinute, Req_List.Data().iEndSecond,
                    Req_List.Data().iVehState, Req_List.Data().iMsgCnt, NewReq.dUpdateTimeOfETA);

            Req_List.Data().iLeavingCounter = 0;

            ReqListUpdateFlag = ADD_NEW_REQUEST;

            #ifdef CONSOLE_OUTPUT
                cout << "Adding new request to list --" << endl;
                writeRequestData(Req_List.Data());
            #endif

            #ifdef LOGGING
                sprintf(temp_log, "*** Add New Request **** { %s }\t \t FLAG  %d at time (%.2f).\n", RcvMsg,
                        ReqListUpdateFlag, dTime);
                outputlog(temp_log);
            #endif
        }
        else // The request is already in the list.
        {
            Req_List.Reset(pos);

            //DJC we have already found the request by vehicleID so why are we doing this MsgCnt testing ??????????
            iRecvReqListDiviser = (int)Req_List.Data().iMsgCnt / 10;
            iNewReqDiviser = (int)NewReq.iMsgCnt / 10;

            // if the received SRM with identical MsgCnt type is not already in the list, we should  add/update the new req to the list.
            if (iRecvReqListDiviser != iNewReqDiviser)
            {
                // We resolve the problem for the new updated request if the new updated request is EV, or there is not an EV in the list.
                if ((NewReq.VehClass == EV) || (iNumberOfEVinList == 0))
                    ReqListUpdateFlag = UPDATED_REQUEST;

                Req_List.Data() = NewReq; // Update the existed entry.

                Req_List.Data().dUpdateTimeOfETA = dTime;
            }
        }
        //----------------End Update the Requests list according to the received time.--------//
    }
    else if (NewReq.iRequestType == REQUEST_UPDATE)
    {
        if (pos >= 0) 
        {
            Req_List.Reset(pos);

            Req_List.Data() = NewReq;

            ReqListUpdateFlag = UPDATED_REQUEST; //re-solve based on the update

            #ifdef LOGGING
                sprintf(temp_log, "Updated a request in the list %s at time (%.2f).\n", RcvMsg, dTime);
                outputlog(temp_log);
            #endif
        }
        else
        {
            #ifdef LOGGING
                sprintf(temp_log,
                        "A request update is received but the request is not in the list. The update is ignored.\n");
                outputlog(temp_log);
            #endif    
        }

    }
    else if (NewReq.iRequestType == PRIORITY_CANCELLATION)
    {
        if (pos >= 0) // this is the first time we received the clear request
        {
            Req_List.Reset(pos);

            if (Req_List.ListSize() > 1) // if there is another request in the table we should solve the problem again
                ReqListUpdateFlag = CANCEL_REQUEST_LEAVING_INTERSECTION;
            else
                clearSignalControllerCommands = true;

            Req_List.Data() = NewReq;

            #ifdef LOGGING
                sprintf(temp_log, "Set the clear request in the list %s at time (%.2f).\n", RcvMsg, dTime);
                outputlog(temp_log);
            #endif    
        }
        else
        {
            #ifdef LOGGING
                sprintf(temp_log,
                        "A new cancel request is received but the request is not in the list. The request is ignored.\n");
                outputlog(temp_log);
            #endif
        }
    }

    if (Req_List.ListSize() == 0 && ReqListUpdateFlag != CANCEL_REQUEST_LEAVING_INTERSECTION)
    {
        #ifdef LOGGING
            sprintf(temp_log, "*************Empty List at time (%.2f).\n", dTime);
            outputlog(temp_log);
        #endif

        ReqListUpdateFlag = NO_UPDATE;
    }
    //---------------End of Handling the Received Requests.----------------//
}

int numberOfEVs(LinkedList<ReqEntry> ReqList)
{
    int iNumber = 0;

    ReqList.Reset();

    while (!ReqList.EndOfList())
    {
        if (ReqList.Data().VehClass == EV)
            iNumber++;

        ReqList.Next();
    }

    return iNumber;
}

int FindSplitPhase(int phase, int phaseStatus[8], int CombinedPhase[])
{
    //*** From CombinedPhase[] to get the combined phase :get_configfile() generates CombinedPhase[]
    //*** If the phase exsits, the value is not 0; if not exists, the default value is '0'.; "-1"  means will not change later
    //*** The argument phase should be among {1..8}
    //int Phase_Seq[8]={1,2,3,4,5,6,7,8};
    //*** NOW also consider the case that if phase 6 is current or next, will only call 2, and 6 will be in the solver as well

    int combined_phase = 0;

    switch (phase)
    {
    case 1:
        combined_phase = CombinedPhase[6 - 1];
        break;

    case 2:                                                     // if current phase or next phase is 6: we should not call phase 5, because cannot reverse from 6 to 5;
        if (phaseStatus[6 - 1] == 2 || phaseStatus[6 - 1] == 7) // do not consider not enable case: phaseStatus[6-1]==3
            combined_phase = -1;
        else
            combined_phase = CombinedPhase[5 - 1];
        break;

    case 3:
        combined_phase = CombinedPhase[8 - 1];
        break;

    case 4:
        if (phaseStatus[8 - 1] == 2 || phaseStatus[8 - 1] == 7)
            combined_phase = -1;
        else
            combined_phase = CombinedPhase[7 - 1];
        break;

    case 5:
        combined_phase = CombinedPhase[2 - 1];
        break;

    case 6:
        if (phaseStatus[2 - 1] == 2 || phaseStatus[2 - 1] == 7)
            combined_phase = -1;
        else
            combined_phase = CombinedPhase[1 - 1];
        break;

    case 7:
        combined_phase = CombinedPhase[4 - 1];
        break;

    case 8:
        if (phaseStatus[4 - 1] == 2 || phaseStatus[4 - 1] == 7)
            combined_phase = -1;
        else
            combined_phase = CombinedPhase[3 - 1];
        break;

    default:
        cout << "*** Wrong Phase Information***" << endl;
        system("pause");
        break;
    }

    return combined_phase;
}

// Find if the TestEntry is in the list, return value is the position.
int FindInReqList(LinkedList<ReqEntry> ReqList, ReqEntry TestEntry)
{
    ReqList.Reset();
    int temp = -1;

    if (ReqList.ListEmpty())
        return temp;
    else
        while (!ReqList.EndOfList())
        {
            if (ReqList.Data().VehID == TestEntry.VehID)
                return ReqList.CurrentPosition();

            ReqList.Next();
        }

    return temp;
}

int currentFlagInRequestFile(char *filename)
{
    char temp_log[256];
    int iCurrentFlag = 0;
    int iRequestNumber = 0;
    char cTemp[256];
    string lineread;
    fstream fss;

    fss.open(filename, fstream::in);

    getline(fss, lineread); // Read first line: should be [Num_req] [No] [UpdateFlag]

    if (lineread.size() != 0)
        sscanf(lineread.c_str(), "%*s %d %d", &iRequestNumber, &iCurrentFlag);

    #ifdef LOGGING    
        sprintf(cTemp, " Number of Requests in the requests_combined.txt is:  %d", iRequestNumber);
        outputlog(temp_log);
    #endif

    fss.close();

    return iCurrentFlag;
}

int getCurrentFlagInReqFile(const char *filename)
{
    int iTempFlag = 0;
    int iTempReqNum = 0;
    string lineread{};

    fstream fss;
    fss.open(filename, fstream::in);

    getline(fss, lineread); // Read first line: should be [Num_req] [No] [UpdateFlag]
    if (lineread.size() != 0)
        sscanf(lineread.c_str(), "%*s %d %d", &iTempReqNum, &iTempFlag);

    fss.close();

    return iTempFlag;
}

void PrintList(LinkedList<ReqEntry> &ReqList)
{
    if (ReqList.ListEmpty())
        cout << "NO entry in the list!\n";
    else
    {
        ReqList.Reset();

        while (!ReqList.EndOfList())
        {
            cout << ReqList.Data();
            ReqList.Next();
        }

        cout << "!!!!List print OVER!!!" << endl;
    }
}

void PrintList2File(const char *Filename, const string &rsu_id, LinkedList<ReqEntry> &ReqList, int ReqListUpdateFlag, const bool IsCombinedFile)
{
    // phase_status is from the ASC controller

    FILE *pFile = fopen(Filename, "w");
    int TotalReqNum = 0;
    int CurPhase;
    int SplitPhase;

    if (!ReqList.ListEmpty() && pFile != NULL)
    {
        if (IsCombinedFile) // output to "requests_combined.txt"
        {

            ReqList.Reset();

            if (FindTimesInList(ReqList, EV) == 1) //ONLY have one EV will possiblly call split phase. JD 2012.3.27
            {
                while (!ReqList.EndOfList())
                {
                    if (ReqList.Data().VehClass == EV && ReqList.Data().Split_Phase > 0)
                        TotalReqNum += 2; // We need send two requests.
                    else
                        TotalReqNum += 1;

                    ReqList.Next();
                }

                fprintf(pFile, "Num_req %d %d\n", TotalReqNum, ReqListUpdateFlag);

                ReqList.Reset();

                while (!ReqList.EndOfList())
                {
                    CurPhase = ReqList.Data().Phase; // current requested phase
                    SplitPhase = ReqList.Data().Split_Phase;

                    if (SplitPhase <= 0 || ReqList.Data().VehClass != EV)
                        fprintf(pFile, "%s %ld %d %.2f %d %.2f %lf %d %d %d %d %d %d %d %d %d %d %.2lf\n",
                                rsu_id.c_str(), ReqList.Data().VehID, ReqList.Data().VehClass,
                                ReqList.Data().ETA, CurPhase, ReqList.Data().MinGreen, ReqList.Data().dSetRequestTime,
                                ReqList.Data().iInLane, ReqList.Data().iOutLane, ReqList.Data().iStrHour,
                                ReqList.Data().iStrMinute, ReqList.Data().iStrSecond,
                                ReqList.Data().iEndHour, ReqList.Data().iEndMinute, ReqList.Data().iEndSecond,
                                ReqList.Data().iVehState, ReqList.Data().iMsgCnt, ReqList.Data().dTimeInCycle);
                    else // write both the requested phase (CurPhase) and the split phase (SplitPhase).
                    {
                        fprintf(pFile, "%s %ld %d %.2f %d %.2f %lf %d %d %d %d %d %d %d %d %d %d %.2lf\n",
                                rsu_id.c_str(), ReqList.Data().VehID, ReqList.Data().VehClass,
                                ReqList.Data().ETA, CurPhase, ReqList.Data().MinGreen, ReqList.Data().dSetRequestTime,
                                ReqList.Data().iInLane, ReqList.Data().iOutLane, ReqList.Data().iStrHour,
                                ReqList.Data().iStrMinute, ReqList.Data().iStrSecond,
                                ReqList.Data().iEndHour, ReqList.Data().iEndMinute, ReqList.Data().iEndSecond,
                                ReqList.Data().iVehState, ReqList.Data().iMsgCnt, ReqList.Data().dTimeInCycle);

                        fprintf(pFile, "%s %ld %d %.2f %d %.2f %lf %d %d %d %d %d %d %d %d %d %d %.2lf\n",
                                rsu_id.c_str(), ReqList.Data().VehID, ReqList.Data().VehClass,
                                ReqList.Data().ETA, SplitPhase, ReqList.Data().MinGreen, ReqList.Data().dSetRequestTime,
                                ReqList.Data().iInLane, ReqList.Data().iOutLane, ReqList.Data().iStrHour,
                                ReqList.Data().iStrMinute, ReqList.Data().iStrSecond,
                                ReqList.Data().iEndHour, ReqList.Data().iEndMinute, ReqList.Data().iEndSecond,
                                ReqList.Data().iVehState, ReqList.Data().iMsgCnt, ReqList.Data().dTimeInCycle);
                    }

                    ReqList.Next();
                }
            }
            else // when we have multiple EV at the intersection, there is no split phase for them
            {
                TotalReqNum = ReqList.ListSize();
                fprintf(pFile, "Num_req %d %d\n", TotalReqNum, ReqListUpdateFlag);

                ReqList.Reset();

                while (!ReqList.EndOfList())
                {
                    fprintf(pFile, "%s %ld %d %.2f %d %.2f %.2f %d %d %d %d %d %d %d %d %d %d %.2lf\n", rsu_id.c_str(),
                            ReqList.Data().VehID, ReqList.Data().VehClass,
                            ReqList.Data().ETA, ReqList.Data().Phase, ReqList.Data().MinGreen,
                            ReqList.Data().dSetRequestTime,
                            ReqList.Data().iInLane, ReqList.Data().iOutLane, ReqList.Data().iStrHour,
                            ReqList.Data().iStrMinute, ReqList.Data().iStrSecond,
                            ReqList.Data().iEndHour, ReqList.Data().iEndMinute, ReqList.Data().iEndSecond,
                            ReqList.Data().iVehState, ReqList.Data().iMsgCnt, ReqList.Data().dTimeInCycle);

                    ReqList.Next();
                }
            }
        }
        else //not a combined file so we write to requests.txt instead
        {    // EV will have split phase requests so we will add split_phase in the sequence of data for each EV request
            TotalReqNum = ReqList.ListSize();
            fprintf(pFile, "Num_req %d %d\n", TotalReqNum, 0);

            if (!ReqList.ListEmpty())
            {
                ReqList.Reset();

                while (!ReqList.EndOfList())
                {
                    fprintf(pFile, "%s %ld %d %.2f %d %.2f %.2f %d %d %d %d %d %d %d %d %d %d %d %.2lf \n",
                            rsu_id.c_str(), ReqList.Data().VehID, ReqList.Data().VehClass,
                            ReqList.Data().ETA, ReqList.Data().Phase, ReqList.Data().MinGreen,
                            ReqList.Data().dSetRequestTime, ReqList.Data().Split_Phase,
                            ReqList.Data().iInLane, ReqList.Data().iOutLane, ReqList.Data().iStrHour,
                            ReqList.Data().iStrMinute, ReqList.Data().iStrSecond,
                            ReqList.Data().iEndHour, ReqList.Data().iEndMinute, ReqList.Data().iEndSecond,
                            ReqList.Data().iVehState, ReqList.Data().iMsgCnt, ReqList.Data().dTimeInCycle);
                    ReqList.Next();
                }
            }
        }
    } // endl of if (!Req_List.ListEmpty())

    fclose(pFile);
}

void deleteThePassedVehicle(LinkedList<ReqEntry> &Req_List, int &ReqListUpdateFlag, bool &clearSignalControllerCommands)
{
    char temp_log[256];

    Req_List.Reset();

    while (!Req_List.EndOfList())
    {
        // three time steps after the vehicle leaving request is received, the vehicle is deleted from the list.
        //if (Req_List.Data().iLeavingCounter >= 3)
        if (Req_List.Data().iLeavingCounter)
        {
            Req_List.DeleteAt();

            #ifdef LOGGING
                sprintf(temp_log, "Deleted the vehicle which has passed the intersection from the list\n\n");
                outputlog(temp_log);
            #endif

            if (Req_List.ListSize() > 0) // if there is another request in the table we should solve the problem again
                ReqListUpdateFlag = CANCEL_REQUEST_LEAVING_INTERSECTION;
            else // else the traffic control interface code will be told to clear the previous commands
                clearSignalControllerCommands = true;

            continue;
        }

        Req_List.Next();
    }
}

void updateETAofRequestsInList(LinkedList<ReqEntry> &Req_List, int &ReqListUpdateFlag)
{
    char temp_log[256];

    Req_List.Reset();

    // sequentially examine each list entry
    while (!Req_List.EndOfList())
    {
        // MZ if the received time of the last SRM is (iObsoleteTimeOfRemainingReq second) ago and the
        // MZ SRM has not been updated during this interval, this request is a residual request and should be deleted!
        // MZ Does the SRM get sent on some interval or only once??????????

        // If the difference between the current time and the time the SRM was received is greater than the obsolescence time
        // we will delete this entry
        if (dTime - Req_List.Data().dSetRequestTime > OBSOLETE_TIME_OF_REMAINED_REQ)
        {
            Req_List.Reset(Req_List.CurrentPosition());

            ReqListUpdateFlag = DELETE_OBSOLETE_REQUEST;

            // delete the current entry
            Req_List.DeleteAt();

            #ifdef LOGGING
                sprintf(temp_log,
                        " ************ Residual request with ID %ld DELETED from the requests list  at time %.2f ************\n",
                        Req_List.Data().VehID, dTime);
                outputlog(temp_log);
            #endif    
        }
        else // update the ETA
        {
            if (Req_List.Data().ETA > 0)
            {
                // if the difference between the current time and the previous time the ETA was updated
                if (dTime - Req_List.Data().dUpdateTimeOfETA >= COUNT_DOWN_INTERVAL_FOR_ETA)
                {
                    // ETA becomes current time minus last update time
                    Req_List.Data().ETA = max(0.0, Req_List.Data().ETA - (dTime - Req_List.Data().dUpdateTimeOfETA));
                    // saved update time becomes current time
                    Req_List.Data().dUpdateTimeOfETA = dTime;
                }
            }
            else
                Req_List.Data().ETA = 0;

            // MZP added 10/30/17
            //if the vehicle is leaving intersection iRequestedPhase
            //MZ if ((Req_List.Data().iVehState == 2) || (Req_List.Data().iVehState == 4))
            if (Req_List.Data().iRequestType == PRIORITY_CANCELLATION)
                Req_List.Data().iLeavingCounter++;
        }

        Req_List.Next();
    }
}

void writeRequestData(const ReqEntry &Req_Entry)
{


} 