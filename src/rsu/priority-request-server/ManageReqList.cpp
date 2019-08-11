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
#include "PriorityConfig.h"
#include "ReqEntryListHandle.h"

//----- ReqListUpdateFlag=1: ADD a new request
//----- ReqListUpdateFlag=2: UPDATED request (changing the speed, joining the queue, leaving the queue)
//----- ReqListUpdateFlag=3: DELETE an obsolete request
//----- ReqListUpdateFlag=4: CANCEL a request due to leaving intersection
//----- ReqListUpdateFlag=5:
//----- ReqListUpdateFlag=6: RESOLVE the problem every 10 seconds if there is coordination in the request table
//----- ReqListUpdateFlag=7: Update the coordination request

using namespace std;



int FindTimesInList(LinkedList <ReqEntry> Req_List, int Veh_Class) {
    Req_List.Reset();
    int times = 0;

    while (!Req_List.EndOfList()) {
        if (Req_List.Data().VehClass == Veh_Class) {
            times++;
        }

        Req_List.Next();
    }

    return times;
}

void UpdateList(LinkedList <ReqEntry> &Req_List, char *RcvMsg, int phaseStatus[8],
                 int &ReqListUpdateFlag, int CombinedPhase[], int &flagForClearingInterfaceCmd) {

    char temp_log[256];
    int iNewReqDiviser = 0;
    int iRecvReqListDiviser = 0;
    ReqEntry NewReq; // default NewReq.Split_Phase=-10;
    char RSU_ID[16], msg[16];
    sscanf(RcvMsg, "%s %s %ld %d %f %d %f %lf %d %d %d %d %d %d %d %d %d %d %lf ", msg, RSU_ID, &NewReq.VehID,
           &NewReq.VehClass,
           &NewReq.ETA, &NewReq.Phase, &NewReq.MinGreen, &NewReq.dSetRequestTime,
           &NewReq.iInLane, &NewReq.iOutLane, &NewReq.iStrHour, &NewReq.iStrMinute, &NewReq.iStrSecond,
           &NewReq.iEndHour, &NewReq.iEndMinute, &NewReq.iEndSecond, &NewReq.iVehState, &NewReq.iMsgCnt,
           &NewReq.dTimeInCycle);
    // MZP
    if (NewReq.MinGreen > 0)  // means vehicle is in the queue, we need queue clearance to be considered in Solver
        NewReq.ETA = 0;

    NewReq.dUpdateTimeOfETA = dTime;
    int iNumberOfEVinList = numberOfEVs(Req_List);
    int SplitPhase = 0;

    if ((NewReq.VehClass == EV) && (iNumberOfEVinList == 0))  // if this is the first EV in the list
    {
        SplitPhase = FindSplitPhase(NewReq.Phase, phaseStatus , CombinedPhase);
        NewReq.Split_Phase = SplitPhase;
    } else
        NewReq.Split_Phase = 0;

    //---------------Beginning of Handling the Received Requests.----------------//

    int pos = FindInReqList(Req_List, NewReq);
    if ((strcmp(msg, "request") == 0 || strcmp(msg, "coord_request") == 0) && NewReq.Phase >
                                                                              0)  // the vehicle is approaching the intersection or in it is in the queue and requesting a phase or if it is a coordination request
    {
        if (pos <
            0)  // Request is NOT in the List, problem should be solved by Solver, therefore ReqListUpdateFlag becomes positive
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
            ReqListUpdateFlag = 1;
            sprintf(temp_log, "*** Add New Request **** { %s }\t \t FLAG  %d at time (%.2f).\n", RcvMsg,
                    ReqListUpdateFlag, dTime);
            outputlog(temp_log);
        } else  // The request is already in the list.
        {
            Req_List.Reset(pos);
            iRecvReqListDiviser = (int) Req_List.Data().iMsgCnt / 10;
            iNewReqDiviser = (int) NewReq.iMsgCnt / 10;
            // MZP      outputlog("All the data in the request list is:\n");
            // MZP
            /*
              if (iRecvReqListDiviser==iNewReqDiviser) // the recived SRM with identical MsgCnt type is already in the list, so we should not add the new req to the list. We should just update the ETA of the request in the list
            {
                ReqListUpdateFlag=2;
                Req_List.Data().ETA=Req_List.Data().ETA-dRollingHorInterval;
                if (Req_List.Data().ETA<0)
                    Req_List.Data().ETA=0;
                sprintf(temp_log,"%ld %d %f %d %f %f %d %d %d %d %d %d %d %d %d %d\n",Req_List.Data().VehID,Req_List.Data().VehClass,Req_List.Data().ETA,Req_List.Data().Phase,Req_List.Data().MinGreen,
                Req_List.Data().fSetRequestTime, Req_List.Data().iInLane, Req_List.Data().iOutLane, Req_List.Data().iStrHour,Req_List.Data().iStrMinute,Req_List.Data().iStrSecond,
                Req_List.Data().iEndHour,Req_List.Data().iEndMinute,Req_List.Data().iEndSecond,Req_List.Data().iVehState,Req_List.Data().iMsgCnt);

            }
            else
            */
            if (iRecvReqListDiviser !=
                iNewReqDiviser) // the recived SRM with identical MsgCnt type is not already in the list, so we should  add/update the new req to the list.
            {
                // MZP
                /*
                   if(NewReq.VehClass==EV)  //Modified by YF: Solve the problem that if duing the first request the split phase is -1, it will never change!!!
                   {
                       NewReq.Split_Phase=SplitPhase;
                   }
                   else
                   {
                       NewReq.Split_Phase=Req_List.Data().Split_Phase;
                   }
               */
                if ((NewReq.VehClass == EV) || (iNumberOfEVinList ==
                                                0)) // We resolve the problem for the new updated request if the new updated request is EV, or there is not EV in the list.
                {
                    ReqListUpdateFlag = 2;
                }
                Req_List.Data() = NewReq; // Update the existed entry.
                Req_List.Data().dUpdateTimeOfETA = dTime;
            }
        }
        //----------------End Update the Requests list according to the received time.--------//
    } else if (strcmp(msg, "request_clear") == 0) {
        if (pos >= 0) // this is the first time we receive the clear request
        {
            Req_List.Reset(pos);
            if (Req_List.ListSize() > 1) // if there is another request in the table we should solve the problem again
                ReqListUpdateFlag = 4;
            else
                flagForClearingInterfaceCmd = 1;
            Req_List.Data() = NewReq;
            sprintf(temp_log, "Set the clear request in the list %s at time (%.2f).\n", RcvMsg, dTime);
            outputlog(temp_log);
        } else {
            sprintf(temp_log,
                    "A new cancel request is received but the request is not in the list. The request is ignored.\n");
            outputlog(temp_log);
        }
        /*  MZP 12/2/17
         iRecvReqListDiviser = (int) Req_List.Data().iMsgCnt /10;
         iNewReqDiviser = (int) NewReq.iMsgCnt /10;
         cout<<"iRecvReqListDiviser"<<iRecvReqListDiviser<<endl;
         cout<<"iNewReqDiviser"<<iNewReqDiviser<<endl;
         if (iRecvReqListDiviser != iNewReqDiviser) // if the recived SRM with identical MsgCnt type is not already in the list, so we should  add/update the new req to the list.
         {
             Req_List.Data() = NewReq;
             sprintf(temp_log,"Set the clear request in the list %s at time (%.2f).\n",RcvMsg,dTime);
             outputlog(temp_log);
         }else
         {
             Req_List.Reset(pos);
             Req_List.DeleteAt();
             sprintf(temp_log,"CLEAR the request form list %s at time (%.2f).\n",RcvMsg,dTime);
             outputlog(temp_log);
         }
     }

     if( Req_List.ListSize()>0 ) // if there is another request in the table we should colve the problem again
         ReqListUpdateFlag=4;
     else
         flagForClearingInterfaceCmd=1;
         cout<<"HERE Req_List.ListSize()"<<Req_List.ListSize()<<endl;
          */
    }
    //---------------End of Handling the Received Requests.----------------//
    if (Req_List.ListSize() == 0 && ReqListUpdateFlag != 4) {
        sprintf(temp_log, "*************Empty List at time (%.2f).\n", dTime);
        outputlog(temp_log);
        ReqListUpdateFlag = 0;
    }
}

int numberOfEVs(LinkedList <ReqEntry> ReqList) {
    int iNumber = 0;
    ReqList.Reset();
    while (!ReqList.EndOfList()) {
        if (ReqList.Data().VehClass == EV) {
            iNumber++;
        }
        ReqList.Next();
    }
    return iNumber;
}

int FindSplitPhase(int phase, int phaseStatus[8], int CombinedPhase[]) {
    //*** From global array CombinedPhase[] to get the combined phase :get_configfile() generates CombinedPhase[]
    //*** If the phase exsits, the value is not 0; if not exists, the default value is '0'.; "-1"  means will not change later
    //*** The argument phase should be among {1..8}
    //int Phase_Seq[8]={1,2,3,4,5,6,7,8};
    //*** NOW also consider the case that if phase 6 is current or next, will only call 2, and 6 will be in the solver as well

    int combined_phase = 0;

    switch (phase) {
        case 1:
            combined_phase = CombinedPhase[6 - 1];
            break;
        case 2: // if current phase or next phase is 6: we should not call phase 5, because cannot reverse from 6 to 5;
        {
            if (phaseStatus[6 - 1] == 2 ||
                phaseStatus[6 - 1] == 7) // do not consider not enable case: phaseStatus[6-1]==3
                combined_phase = -1;
            else
                combined_phase = CombinedPhase[5 - 1];
            break;
        }
        case 3:
            combined_phase = CombinedPhase[8 - 1];
            break;
        case 4: {
            if (phaseStatus[8 - 1] == 2 || phaseStatus[8 - 1] == 7)
                combined_phase = -1;
            else
                combined_phase = CombinedPhase[7 - 1];
            break;
        }
        case 5:
            combined_phase = CombinedPhase[2 - 1];
            break;
        case 6: {
            if (phaseStatus[2 - 1] == 2 || phaseStatus[2 - 1] == 7)
                combined_phase = -1;
            else
                combined_phase = CombinedPhase[1 - 1];
            break;
        }
        case 7:
            combined_phase = CombinedPhase[4 - 1];
            break;
        case 8: {
            if (phaseStatus[4 - 1] == 2 || phaseStatus[4 - 1] == 7)
                combined_phase = -1;
            else
                combined_phase = CombinedPhase[3 - 1];
            break;
        }
        default:
            cout << "*** Wrong Phase Information***" << endl;
            system("pause");
            break;

    }

    return combined_phase;
}

// Find if the TestEntry is in the list, return value is the position.
int FindInReqList(LinkedList <ReqEntry> ReqList, ReqEntry TestEntry) {
    ReqList.Reset();
    int temp = -1;

    if (ReqList.ListEmpty()) {
        return temp;
    } else {
        while (!ReqList.EndOfList()) {
            if (ReqList.Data().VehID == TestEntry.VehID) {

                return ReqList.CurrentPosition();
            }
            ReqList.Next();
        }
    }
    return temp;
}

int currentFlagInRequestFile(char *filename) {
    char temp_log[256];
    int iCurrentFlag = 0;
    int iRequestNumber = 0;
    char cTemp[256];
    fstream fss;
    fss.open(filename, fstream::in);
    string lineread;

    getline(fss, lineread); // Read first line: should be [Num_req] [No] [UpdateFlag]
    if (lineread.size() != 0) {
        sscanf(lineread.c_str(), "%*s %d %d", &iRequestNumber, &iCurrentFlag);
    }
    sprintf(cTemp, " Number of Requests in the requests_combined.txt is:  %d", iRequestNumber);
    outputlog(temp_log);
    fss.close();
    return iCurrentFlag;
}


int getCurrentFlagInReqFile(const char *filename) {
    fstream fss;
    fss.open(filename, fstream::in);
    int iTempFlag = 0;
    int iTempReqNum = 0;
    string lineread;
    getline(fss, lineread); // Read first line: should be [Num_req] [No] [UpdateFlag]
    if (lineread.size() != 0)
        sscanf(lineread.c_str(), "%*s %d %d", &iTempReqNum, &iTempFlag);
    fss.close();
    return iTempFlag;
}

/*
int ReqListFromFile(char *filename,LinkedList<ReqEntry>& Req_List)
{
    fstream fss;
    fss.open(filename,fstream::in);
    ReqEntry req_temp;

    int ReqNo=-1;
    char RSU_ID[20];
    long vehicle_ID;
    int Veh_Class,Req_Phase,Req_SplitPhase;
    float fSetRequestTime;
    float ETA,MinGrn;
	int iinlane,ioutlane,istrhour,istrminute,istrsecond,iendhour,iendminute,iendsecond,ivehstate,imsgcnt; 
	string lineread;
    Req_List.ClearList();

    getline(fss,lineread); // Read first line: should be [Num_req] [No] [UpdateFlag]
    sscanf(lineread.c_str(),"%*s %d %d",&ReqNo,&ReqListUpdateFlag);
    cout<<"The total Requests is:"<<ReqNo<<endl;

    while(!fss.eof() && ReqNo>0)
    {
        getline(fss,lineread);
        if(lineread.size()!=0)
        {
			sscanf(lineread.c_str(),"%s %ld %d %f %d %f %f %d %d %d %d %d %d %d %d %d %d %d",RSU_ID,&vehicle_ID,&Veh_Class,&ETA, 
			&Req_Phase,&MinGrn,&fSetRequestTime,&Req_SplitPhase,&iinlane,&ioutlane,&istrhour,&istrminute,&istrsecond,&iendhour,&iendminute,&iendsecond,&ivehstate,&imsgcnt); 
            ReqEntry req_temp(vehicle_ID,Veh_Class,ETA,Req_Phase,MinGrn,fSetRequestTime,Req_SplitPhase,iinlane,ioutlane,istrhour,istrminute,istrsecond,iendhour,iendminute,iendsecond,ivehstate,imsgcnt);  
            Req_List.InsertAfter(req_temp);
            cout<<lineread<<endl;
        }
        else
        {
            cout<<"Blank Line in requests.txt file!\n";
        }
    }
    fss.close();
    return ReqNo;

}
*/
void PrintList(LinkedList <ReqEntry> &ReqList) {
    if (ReqList.ListEmpty()) {
        cout << "NO entry in the list!\n";
    } else {
        ReqList.Reset();
        while (!ReqList.EndOfList()) {
            cout << ReqList.Data();
            ReqList.Next();
        }
        cout << "!!!!List print OVER!!!" << endl;
    }
}


void PrintList2File(const char *Filename, const string& rsu_id, LinkedList <ReqEntry> &ReqList, int ReqListUpdateFlag, int IsCombined) {
    // If IsCombined=1 (There is no EV) print combined phase information into "requests.txt". // BY DJ 2012.3.27
    // The argument of IsCombined is optional, default value is 0, means no EV
    // phase_status is from the ASC controller

    FILE *pFile = fopen(Filename, "w");
    int TotalReqNum = 0;
    int CurPhase;
    int SplitPhase;
    int times;
    if (!ReqList.ListEmpty() && pFile != NULL) {
        if (IsCombined == 0)  // output to "requests_combined.txt"
        {
            times = FindTimesInList(ReqList, EV);
            ReqList.Reset();
            if (times == 1)  //ONLY have one EV will possiblly call split phase. JD 2012.3.27
            {
                while (!ReqList.EndOfList()) {
                    if (ReqList.Data().VehClass == EV && ReqList.Data().Split_Phase > 0)
                        TotalReqNum += 2;   // We need send two requests.
                    else
                        TotalReqNum += 1;
                    ReqList.Next();
                }
                fprintf(pFile, "Num_req %d %d\n", TotalReqNum, ReqListUpdateFlag);
                ReqList.Reset();
                while (!ReqList.EndOfList()) {
                    CurPhase = ReqList.Data().Phase;              // Current request phase
                    SplitPhase = ReqList.Data().Split_Phase;
                    if (SplitPhase <= 0 || ReqList.Data().VehClass != EV) {
                        fprintf(pFile, "%s %ld %d %.2f %d %.2f %lf %d %d %d %d %d %d %d %d %d %d %.2lf\n",
                                rsu_id.c_str(), ReqList.Data().VehID, ReqList.Data().VehClass,
                                ReqList.Data().ETA, CurPhase, ReqList.Data().MinGreen, ReqList.Data().dSetRequestTime,
                                ReqList.Data().iInLane, ReqList.Data().iOutLane, ReqList.Data().iStrHour,
                                ReqList.Data().iStrMinute, ReqList.Data().iStrSecond,
                                ReqList.Data().iEndHour, ReqList.Data().iEndMinute, ReqList.Data().iEndSecond,
                                ReqList.Data().iVehState, ReqList.Data().iMsgCnt, ReqList.Data().dTimeInCycle);
                    } else  // write both the requeste phase (CurPhase) and the split phase (SplitPhase).
                    {
                        fprintf(pFile, "%s %ld %d %.2f %d %.2f %lf %d %d %d %d %d %d %d %d %d %d %d %.2lf\n",
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
            } else  // when we have multiple EV at the intersection, there is no split phase for them
            {
                TotalReqNum = ReqList.ListSize();
                fprintf(pFile, "Num_req %d %d\n", TotalReqNum, ReqListUpdateFlag);
                ReqList.Reset();
                while (!ReqList.EndOfList()) {
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
        } else if (IsCombined ==
                   1)  //----- EV will have split phase requests: output "requests.txt": will add split_phase in the sequence of data for each EV request
        {
            TotalReqNum = ReqList.ListSize();
            fprintf(pFile, "Num_req %d %d\n", TotalReqNum, 0);
            if (!ReqList.ListEmpty()) {
                ReqList.Reset();
                while (!ReqList.EndOfList()) {
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


void deleteThePassedVehicle(LinkedList <ReqEntry> &Req_List, int &ReqListUpdateFlag, int &flagForClearingInterfaceCmd) {
    char temp_log[256];
    Req_List.Reset();
    while (!Req_List.EndOfList()) {
        if (Req_List.Data().iLeavingCounter >=
            3) // three time steps after the vehicle leaving request is received, the vehicle is deleted from the list.
        {
            Req_List.DeleteAt();
            sprintf(temp_log, "CLEAR the request form list\n");
            outputlog(temp_log);
            if (Req_List.ListSize() > 0) // if there is another request in the table we should colve the problem again
                ReqListUpdateFlag = 4;
            else
                flagForClearingInterfaceCmd = 1;

            continue;
        }
        Req_List.Next();
    }
}

void updateETAofRequestsInList(LinkedList <ReqEntry> &Req_List, int &ReqListUpdateFlag, const double dCountDownIntervalForETA ) {
    char temp_log[256];
    int icoordphase1 = priorityConfig.iCoordinatedPhase[0];
    int icoordphase2 = priorityConfig.iCoordinatedPhase[1];
    double dcoordphase1split = priorityConfig.dCoordinationPhaseSplit[priorityConfig.iCoordinatedPhase[0] - 1];
    double dcoordphase2split = priorityConfig.dCoordinationPhaseSplit[priorityConfig.iCoordinatedPhase[1] - 1];
    double dCoordinationCycle = priorityConfig.dCoordCycle;
    double dLargerSplit = priorityConfig.dLargerCoordinationSplit;
    //double doffset = priorityConfig.dCoordOffset;
    Req_List.Reset();
    while (!Req_List.EndOfList()) {
        //cout<<"Req_List.Data().dSetRequestTime"<<Req_List.Data().dSetRequestTime<<endl;
        //cout<<"Dtime"<<dTime<<endl;
        if ((dTime - Req_List.Data().dSetRequestTime > OBSOLETE_TIME_OF_REMAINED_REQ) && (Req_List.Data().VehClass !=
                                                                                       COORDINATION))// if the received time of the last SRM is (iObsoleteTimeOfRemainingReq second) ago and the SRM has not been updated during this interval, this request is a residual request and should be deleted!
        {
            Req_List.Reset(Req_List.CurrentPosition());
            sprintf(temp_log,
                    " ************ Residual request with ID %ld DELETED from the requests list  at time %.2f ************\n",
                    Req_List.Data().VehID, dTime);
            ReqListUpdateFlag = 3;
            Req_List.DeleteAt();
            outputlog(temp_log);
            // MZP       continue;
        } else {
            if ((Req_List.Data().VehClass == COORDINATION)) {
                if (dcoordphase1split == dcoordphase2split) // if the two coordinated phse have the same split time
                {
                    if (Req_List.Data().ETA == 0 && Req_List.Data().MinGreen > 1)
                        Req_List.Data().MinGreen = dcoordphase1split - dCurrentTimeInCycle;
                    if (Req_List.Data().ETA > 0 && Req_List.Data().MinGreen == 0) {
                        if (dCurrentTimeInCycle > 0)
                            Req_List.Data().ETA = dCoordinationCycle - dCurrentTimeInCycle;
                        else
                            Req_List.Data().ETA = -dCurrentTimeInCycle;
                    }
                } else {
                    if (icoordphase1 == Req_List.Data().Phase) {
                        if (Req_List.Data().ETA == 0 && Req_List.Data().MinGreen > 1)
                            Req_List.Data().MinGreen = dLargerSplit - dCurrentTimeInCycle;
                        if (Req_List.Data().ETA > 0 && Req_List.Data().MinGreen == 0) {
                            // if signal is on the coordinated split time , one coordinated phase has ETA and the other has MinGreen
                            if ((dCurrentTimeInCycle <= max(dcoordphase2split - dcoordphase1split ,0.0))) 
                                Req_List.Data().ETA =
                                        max(dcoordphase2split - dcoordphase1split, 0.0) - dCurrentTimeInCycle;
                            else
                                Req_List.Data().ETA = dCoordinationCycle - dCurrentTimeInCycle +
                                                      max(dcoordphase2split - dcoordphase1split, 0.0);
                        }
                    }
                    if (icoordphase2 == Req_List.Data().Phase) {
                        if (Req_List.Data().ETA == 0 && Req_List.Data().MinGreen > 1)
                            Req_List.Data().MinGreen = dLargerSplit - dCurrentTimeInCycle;
                        if (Req_List.Data().ETA > 0 && Req_List.Data().MinGreen == 0) {
                            if (dCurrentTimeInCycle <= max(dcoordphase1split - dcoordphase2split, 0.0))
                                Req_List.Data().ETA =
                                        max(dcoordphase1split - dcoordphase2split, 0.0) - dCurrentTimeInCycle;
                            else
                                Req_List.Data().ETA = dCoordinationCycle - dCurrentTimeInCycle +
                                                      max(dcoordphase1split - dcoordphase2split, 0.0);
                        }
                    }
                }
                Req_List.Data().dTimeInCycle = dCurrentTimeInCycle;
            } else {
                if (Req_List.Data().ETA > 0) {
                    if (dTime - Req_List.Data().dUpdateTimeOfETA >= dCountDownIntervalForETA) {
                        Req_List.Data().ETA = max(0.0,
                                                  Req_List.Data().ETA - (dTime - Req_List.Data().dUpdateTimeOfETA));
                        Req_List.Data().dUpdateTimeOfETA = dTime;
                    }
                } else
                    Req_List.Data().ETA = 0;
                // MZP added 10/30/17
                if ((Req_List.Data().iVehState == 2) ||
                    (Req_List.Data().iVehState == 4)) //if the vehicle is leaving intersection
                    Req_List.Data().iLeavingCounter++;
            }
        }
        Req_List.Next();
    }
}	
