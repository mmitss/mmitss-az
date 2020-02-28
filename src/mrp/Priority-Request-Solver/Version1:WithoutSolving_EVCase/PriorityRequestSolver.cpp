/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorityRequestSolver.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. 
*/

#include "PriorityRequestSolver.h"
#include "glpk.h"
//#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <sys/time.h>
#include "json/json.h"
#include <algorithm>
#include <cmath>

const int transitWeight = 1;
const int truckWeight = 1;
const double MAXGREEN = 50.0;

#define OMIT_VEH_PHASES 2
#define OMIT_PED_PHASES 3
#define HOLD_PHASES 4
#define FORCEOFF_PHASES 5
#define CALL_VEH_PHASES 6
#define CALL_PED_PHASES 7

PriorityRequestSolver::PriorityRequestSolver()
{
}

/*
    - This method is responsible for creating priority request list received from the PRS as Json String.
*/
void PriorityRequestSolver::createPriorityRequestList(string jsonString)
{
    int noOfRequest{};
    RequestList requestList;
    Json::Value jsonObject;
    Json::Reader reader;
    reader.parse(jsonString.c_str(), jsonObject);

    if ((jsonObject["MsgType"]).asString() == "PriorityRequest")
    {
        priorityRequestList.clear();
        noOfRequest = (jsonObject["PriorityRequestList"]["noOfRequest"]).asInt();
        for (int i = 0; i < noOfRequest; i++)
        {
            requestList.vehicleID = jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleID"].asInt();
            requestList.vehicleType = jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleType"].asInt();
            requestList.basicVehicleRole = jsonObject["PriorityRequestList"]["requestorInfo"][i]["basicVehicleRole"].asInt();
            requestList.laneID = jsonObject["PriorityRequestList"]["requestorInfo"][i]["inBoundLaneID"].asInt();
            requestList.vehicleETA = jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"].asDouble();
            requestList.vehicleETA_Duration = jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"].asDouble();
            requestList.requestedPhase = jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"].asInt();
            requestList.prioritystatus = jsonObject["PriorityRequestList"]["requestorInfo"][i]["priorityRequestStatus"].asInt();
            priorityRequestList.push_back(requestList);
        }
    }
    // setPhaseCallForRequestedSignalGroup();
    //This is optional. For priniting few attributes of the priority request list in the console
    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        cout << priorityRequestList[i].vehicleID << " " << priorityRequestList[i].basicVehicleRole << " " << priorityRequestList[i].vehicleETA << endl;
    }
}

bool PriorityRequestSolver::findEVInList()
{
    if (priorityRequestList.empty())
        bEVStatus = false;
    else
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            if (priorityRequestList[i].vehicleType == 2)
            {
                bEVStatus = true;
                break;
            }
        }
    }

    return bEVStatus;
}

/*
    - If EV is priority request list, delete all the priority request from the list apart from EV
*/
void PriorityRequestSolver::modifyPriorityRequestList()
{
    int temporaryVehicleID{};

    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        temporaryVehicleID = priorityRequestList[i].vehicleID;
        if (priorityRequestList[i].vehicleType != 2)
        {
            vector<RequestList>::iterator findVehicleIDOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                             [&](RequestList const &p) { return p.vehicleID == temporaryVehicleID; });

            priorityRequestList.erase(findVehicleIDOnList);
            i--;
        }
        else
            noOfEVInList++;
    }
}

/*
    - If here is EV in priority request list Dat file will have information about EV requested phases and current phases.
*/
void PriorityRequestSolver::getEVPhases()
{
    int tempSignalGroup{};
    vector<int>::iterator it;
    plannedEVPhases = requestedSignalGroup;
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        tempSignalGroup = trafficControllerStatus[i].startingPhase1;
        it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), tempSignalGroup);
        if (it == requestedSignalGroup.end())
            plannedEVPhases.push_back(tempSignalGroup);

        tempSignalGroup = trafficControllerStatus[i].startingPhase2;
        it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), tempSignalGroup);
        if (it == requestedSignalGroup.end())
            plannedEVPhases.push_back(tempSignalGroup);
    }
    sort(plannedEVPhases.begin(), plannedEVPhases.end());
}
/*
    - If new priority request  is received this method will obtain the current traffic signal Status.
*/
void PriorityRequestSolver::getCurrentSignalStatus()
{
    int temporaryPhase{};
    TrafficControllerData::TrafficConrtollerStatus tcStatus;
    Json::Value jsonObject;
    Json::Reader reader;
    ifstream jsonData("trafficControllerStatus.json");
    string jsonString((std::istreambuf_iterator<char>(jsonData)), std::istreambuf_iterator<char>());
    reader.parse(jsonString.c_str(), jsonObject);

    trafficControllerStatus.clear();
    tcStatus.startingPhase1 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["SP1"].asInt();
    tcStatus.startingPhase2 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["SP2"].asInt();
    tcStatus.initPhase1 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["init1"].asDouble();
    tcStatus.initPhase2 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["init2"].asDouble();
    tcStatus.elapsedGreen1 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["Grn1"].asDouble();
    tcStatus.elapsedGreen2 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["Grn2"].asDouble();
    trafficControllerStatus.push_back(tcStatus);
    //If signal phase is on rest, elapsed green time will be more than gmax. In that case elapsed green time will be min green time.
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        temporaryPhase = trafficControllerStatus[i].startingPhase1;
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                   [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
        if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->maxGreen)
            trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;

        temporaryPhase = trafficControllerStatus[i].startingPhase2;
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                   [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
        if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->maxGreen)
            trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
    }

    //This is optional. For priniting few attributes of the TCStatus in the console.
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        cout << trafficControllerStatus[i].startingPhase1 << " " << trafficControllerStatus[i].initPhase1 << " " << trafficControllerStatus[i].elapsedGreen2 << endl;
    }
}

/*
    - This method will obtain requested signal group information from the priority request list and store them in requestedSignalGroup list.
*/
void PriorityRequestSolver::getRequestedSignalGroupFromPriorityRequestList()
{
    //vector<int>requestedSignalGroup;
    for (size_t i = 0; i < priorityRequestList.size(); i++)
        requestedSignalGroup.push_back(priorityRequestList[i].requestedPhase);
}

/*
    - This method is responsible for removing the duplicate signal group number from requestedSignalGroup list.
        -If there is multiple priority request for signal group then there will be duplicate signal group in requestedSignalGroup list.
*/
void PriorityRequestSolver::removeDuplicateSignalGroup()
{
    auto end = requestedSignalGroup.end();
    for (auto it = requestedSignalGroup.begin(); it != end; ++it)
        end = std::remove(it + 1, end, *it);

    requestedSignalGroup.erase(end, requestedSignalGroup.end());
}

/*
- This function is responsible for finding associated signal group from another ring for requested signal group
- At first all requested signal group information are stored in another temporary vector
- Associated signal group is obtained by +/- 4. If requested phase is in ring 1 add 4. If equested phase is in ring 2 substract 4.
- Check if the associated signal group is enabled for the intersection
- Append associated signal group information in the orignal signal group list.
- Remove the duplicate phase number
*/
void PriorityRequestSolver::addAssociatedSignalGroup()
{
    vector<int> tempListOfRequestedSignalGroup = requestedSignalGroup;
    int associatedSignalGroup{};
    int tempRequestedSignalGroup{};

    for (auto i = requestedSignalGroup.begin(); i != requestedSignalGroup.end(); ++i)
    {
        tempRequestedSignalGroup = *i;
        if (tempRequestedSignalGroup < 5)
        {
            associatedSignalGroup = tempRequestedSignalGroup + 4;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                      [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == associatedSignalGroup; });
            if (findSignalGroup != trafficSignalPlan.end())
                tempListOfRequestedSignalGroup.push_back(associatedSignalGroup);
        }
        else
        {
            associatedSignalGroup = tempRequestedSignalGroup - 4;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                      [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == associatedSignalGroup; });
            if (findSignalGroup != trafficSignalPlan.end())
                tempListOfRequestedSignalGroup.push_back(associatedSignalGroup);
        }
    }
    requestedSignalGroup = tempListOfRequestedSignalGroup;
    removeDuplicateSignalGroup();
}

/*
    - This function will increase the  value of green max by 15% if there is Transit or Truck in the priority request list.
*/
void PriorityRequestSolver::modifyGreenMax()
{
    for (auto i = requestedSignalGroup.begin(); i != requestedSignalGroup.end(); ++i)
    {
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                  [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == *i; });
        if (findSignalGroup != trafficSignalPlan.end())
            findSignalGroup->maxGreen = findSignalGroup->maxGreen * 1.15;
    }
}

/*
    - This function is responsible for creating Data file for glpk Solver based on priority request list and TCI data.
*/
void PriorityRequestSolver::generateDatFile()
{
    int temporaryPhase{};
    vector<int> temporaryPhaseNumber{};
    temporaryPhaseNumber = PhaseNumber;
    vector<int>::iterator it;
    int vehicleClass{}; //to match the old PRSolver
    int numberOfRequest{};
    int ReqSeq = 1;
    double tempVehicleETA{};
    double tempVehicleETA_Duration{};
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan_EV;
    vector<double> temporaryVehicleETA;
    vector<double> temporaryVehicleETA_Duration;

    ofstream fs;
    fs.open("NewModelData.dat", ios::out);
    fs << "data;\n";
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        fs << "param SP1:=" << trafficControllerStatus[i].startingPhase1 << ";" << endl;
        fs << "param SP2:=" << trafficControllerStatus[i].startingPhase2 << ";" << endl;
        fs << "param init1:=" << trafficControllerStatus[i].initPhase1 << ";" << endl;
        fs << "param init2:=" << trafficControllerStatus[i].initPhase2 << ";" << endl;
        fs << "param Grn1 :=" << trafficControllerStatus[i].elapsedGreen1 << ";" << endl;
        fs << "param Grn2 :=" << trafficControllerStatus[i].elapsedGreen2 << ";" << endl;
    }
    if (bEVStatus == true)
    {
        trafficSignalPlan_EV.insert(trafficSignalPlan_EV.end(), trafficSignalPlan.begin(), trafficSignalPlan.end());
        sort(plannedEVPhases.begin(), plannedEVPhases.end()); //arrange the numbers in ascending order

        for (size_t j = 0; j < plannedEVPhases.size(); j++)
        {
            temporaryPhase = plannedEVPhases.at(j);
            it = std::find(temporaryPhaseNumber.begin(), temporaryPhaseNumber.end(), temporaryPhase);
            if (it != temporaryPhaseNumber.end())
            {
                temporaryPhaseNumber.erase(it);
            }
        }

        for (size_t i = 0; i < temporaryPhaseNumber.size(); i++)
        {
            temporaryPhase = temporaryPhaseNumber.at(i);

            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan_EV), std::end(trafficSignalPlan_EV),
                                                                                                            [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

            if (findSignalGroupOnList != trafficSignalPlan_EV.end())
                trafficSignalPlan_EV.erase(findSignalGroupOnList);
        }

        //Find the maximum ETA and ETA duration from among all the EV
        for (size_t k = 0; k < priorityRequestList.size(); k++)
        {
            temporaryVehicleETA.push_back(priorityRequestList[k].vehicleETA);
            temporaryVehicleETA_Duration.push_back(priorityRequestList[k].vehicleETA_Duration);
        }

        tempVehicleETA = *max_element(temporaryVehicleETA.begin(), temporaryVehicleETA.end());
        tempVehicleETA_Duration = *max_element(temporaryVehicleETA_Duration.begin(), temporaryVehicleETA_Duration.end());

        fs << "param y          \t:=";
        for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
            fs << "\t" << trafficSignalPlan_EV[i].phaseNumber << "\t" << trafficSignalPlan_EV[i].yellowChange;
        fs << ";\n";

        fs << "param red          \t:=";
        for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
            fs << "\t" << trafficSignalPlan_EV[i].phaseNumber << "\t" << trafficSignalPlan_EV[i].redClear;
        fs << ";\n";

        fs << "param gmin      \t:=";
        for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
            fs << "\t" << trafficSignalPlan_EV[i].phaseNumber << "\t" << trafficSignalPlan_EV[i].minGreen;
        fs << ";\n";

        fs << "param gmax      \t:=";

        if (MAXGREEN > tempVehicleETA + tempVehicleETA_Duration)
        {
            for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
                fs << "\t" << trafficSignalPlan_EV[i].phaseNumber << "\t" << MAXGREEN;
        }

        else
        {
            for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
                fs << "\t" << trafficSignalPlan_EV[i].phaseNumber << "\t" << tempVehicleETA + tempVehicleETA_Duration;
        }

        fs << ";\n";
    }

    else
    {
        fs << "param y          \t:=";
        for (size_t i = 0; i < trafficSignalPlan.size(); i++)
            fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].yellowChange;
        fs << ";\n";

        fs << "param red          \t:=";
        for (size_t i = 0; i < trafficSignalPlan.size(); i++)
            fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].redClear;
        fs << ";\n";

        fs << "param gmin      \t:=";
        for (size_t i = 0; i < trafficSignalPlan.size(); i++)
            fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].minGreen;
        fs << ";\n";

        fs << "param gmax      \t:=";
        for (size_t i = 0; i < trafficSignalPlan.size(); i++)
            fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].maxGreen;
        fs << ";\n";
    }

    fs << "param priorityType:= ";

    if (!priorityRequestList.empty())
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            vehicleClass = 0;
            numberOfRequest++;
            if (priorityRequestList[i].basicVehicleRole == 16)
            {
                numberOfTransitInList++;
                vehicleClass = 2;
            }

            else if (priorityRequestList[i].basicVehicleRole == 9)
            {
                numberOfTruckInList++;
                vehicleClass = 3;
            }

            fs << numberOfRequest;
            fs << " " << vehicleClass << " ";
        }
        while (numberOfRequest < 10)
        {
            numberOfRequest++;
            fs << numberOfRequest;
            fs << " ";
            fs << 0;
            fs << " ";
        }
        fs << " ;  \n";
    }

    else
    {
        fs << " 1 0 2 0 3 5 4 0 5 0 6 0 7 0 8 0 9 0 10 0 ; \n";
    }

    fs << "param PrioWeigth:=  1 1 2 ";
    if (numberOfTransitInList > 0)
        fs << transitWeight / numberOfTransitInList;
    else
    {
        fs << 0;
    }
    fs << " 3 ";
    if (numberOfTruckInList > 0)
        fs << truckWeight / numberOfTruckInList;
    else
    {
        fs << 0;
    }
    fs << " 4 0 5 0 6 0 7 0 8 0 9 0 10 0 ; \n";

    fs << "param Rl (tr): 1 2 3 4 5 6 7 8:=\n";

    if (!priorityRequestList.empty())
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            fs << ReqSeq << "  ";
            for (int j = 0; j < noOfPhase; j++)
            {
                if (priorityRequestList[i].requestedPhase == trafficSignalPlan[j].phaseNumber)
                    fs << priorityRequestList[i].vehicleETA << "\t";
                else
                    fs << ".\t";
            }
            ReqSeq++;
            fs << "\n";
        }
    }

    fs << ";\n";
    ReqSeq = 1;

    fs << "param Ru (tr): 1 2 3 4 5 6 7 8:=\n";

    if (!priorityRequestList.empty())
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            fs << ReqSeq << "  ";
            for (int j = 0; j < noOfPhase; j++)
            {
                if (priorityRequestList[i].requestedPhase == trafficSignalPlan[j].phaseNumber)
                    fs << priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration << "\t";
                else
                    fs << ".\t";
            }
            ReqSeq++;
            fs << "\n";
        }
    }

    fs << ";\n";
    fs << "end;";
    fs.close();
}

void PriorityRequestSolver::printvector()
{
    for (auto i = requestedSignalGroup.begin(); i != requestedSignalGroup.end(); ++i)
        cout << "phases: " << *i << " " << endl;
}

double PriorityRequestSolver::GetSeconds()
{
    struct timeval tv_tt;
    gettimeofday(&tv_tt, NULL);
    return (static_cast<double>(tv_tt.tv_sec) + static_cast<double>(tv_tt.tv_usec) / 1.e6);
}

/*
    - Method of solving the request in the priority request list  based on mod and dat files
    - Solution will be written in the Results.txt file
*/
void PriorityRequestSolver::GLPKSolver()
{
    double startOfSolve{};
    double endOfSolve{};

    char modFile[128] = "NewModel.mod";
    glp_prob *mip;
    glp_tran *tran;
    int ret{};
    int success = 1;
    if (bEVStatus == true)
    {
        strcpy(modFile, "NewModel_EV.mod");
    }
    mip = glp_create_prob();
    tran = glp_mpl_alloc_wksp();

    ret = glp_mpl_read_model(tran, modFile, 1);

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating model\n");
        goto skip;
    }

    ret = glp_mpl_read_data(tran, "NewModelData.dat");

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating data\n");
        goto skip;
    }

    ret = glp_mpl_generate(tran, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Error on generating model\n");
        goto skip;
    }

    glp_mpl_build_prob(tran, mip);
    glp_simplex(mip, NULL);
    success = glp_intopt(mip, NULL);
    endOfSolve = GetSeconds();
    cout << "Success=" << success << endl;
    cout << "Time of Solve" << endOfSolve - startOfSolve << endl;
    ret = glp_mpl_postsolve(tran, mip, GLP_MIP);
    if (ret != 0)
        fprintf(stderr, "Error on postsolving model\n");
skip:
    glp_mpl_free_wksp(tran);
    glp_delete_prob(mip);
}

void PriorityRequestSolver::readOptimalSignalPlan()
{
    ifstream infile;
    string lineread{};
    int tempLine{};
    double temporaryLine{};
    vector<int> SP;
    vector<double> init;
    vector<double> elapsedGrn;
    vector<double> leftCriticalPoints;
    vector<double> rightCriticalPoints;
    vector<double> leftCriticalPoints_GreenTime;
    vector<double> rightCriticalPoints_GreenTime;

    infile.open("Results.txt");
    // getline(infile, lineread);

    if (infile.fail())
        std::cout << "Fail to open file" << std::endl;

    else
    {
        for (int lineNo = 0; getline(infile, lineread) && lineNo < 20; lineNo++)
        {
            if (lineNo == 0)
            {
                stringstream strToSplit(lineread.c_str());
                while (strToSplit >> tempLine)
                    SP.push_back(tempLine);
            }
            else if (lineNo == 1)
            {
                stringstream strToSplit(lineread.c_str());
                double init1{}, init2{}, grn1{}, grn2{};
                strToSplit >> init1 >> init2 >> grn1 >> grn2;
                init.push_back(init1);
                init.push_back(init2);
                elapsedGrn.push_back(grn1);
                elapsedGrn.push_back(grn2);
            }

            else if (lineNo > 1 && lineNo < 5)
            {
                stringstream strToSplit(lineread.c_str());
                while (strToSplit >> temporaryLine)
                    leftCriticalPoints.push_back(temporaryLine);

                // double lcp1{}, lcp2{}, lcp3{}, lcp4{}, lcp5{}, lcp6{}, lcp7{}, lcp8{};
                // strToSplit>> lcp1 >> lcp2 >> lcp3 >> lcp4 >> lcp5 >> lcp6 >> lcp7 >> lcp8;
            }

            else if (lineNo > 4 && lineNo < 8)
            {
                stringstream strToSplit(lineread.c_str());
                while (strToSplit >> temporaryLine)
                    rightCriticalPoints.push_back(temporaryLine);
            }

            else if (lineNo > 7 && lineNo <= 10)
            {
                stringstream strToSplit(lineread.c_str());
                while (strToSplit >> temporaryLine)
                    leftCriticalPoints_GreenTime.push_back(temporaryLine);
            }

            else if (lineNo > 10 && lineNo < 14)
            {
                stringstream strToSplit(lineread.c_str());
                while (strToSplit >> temporaryLine)
                    rightCriticalPoints_GreenTime.push_back(temporaryLine);
            }
        }
    }
    infile.close();
    leftCriticalPoints_PhaseDuration_Ring1.clear();
    rightCriticalPoints_PhaseDuration_Ring1.clear();
    leftCriticalPoints_PhaseDuration_Ring2.clear();
    rightCriticalPoints_PhaseDuration_Ring2.clear();

    /*
    - Holding the total phase duration for ring wise each phase. 
    - We have 24 phase duration (3cycles). 
    - To store the data ring wise we have skip the phases of opposite ring.
    */
    for (size_t i = 0; i < leftCriticalPoints.size(); i++)
    {
        leftCriticalPoints_PhaseDuration_Ring1.push_back(leftCriticalPoints.at(i));
        //Intotal we have 24 phase information. Ring1{0,1,2,3,8,9,10,11,16,17,18,19}
        if (i == 3)
            i = 7;
        else if (i == 11)
            i = 15;
        else if (i == 19)
            break;
    }

    for (size_t i = 0; i < rightCriticalPoints.size(); i++)
    {
        rightCriticalPoints_PhaseDuration_Ring1.push_back(rightCriticalPoints.at(i));
        if (i == 3)
            i = 7;
        else if (i == 11)
            i = 15;
        else if (i == 19)
            break;
    }

    for (size_t i = 4; i < leftCriticalPoints.size(); i++)
    {
        leftCriticalPoints_PhaseDuration_Ring2.push_back(leftCriticalPoints.at(i));
        if (i == 7)
            i = 11;
        else if (i == 15)
            i = 19;
    }

    for (size_t i = 4; i < rightCriticalPoints.size(); i++)
    {
        rightCriticalPoints_PhaseDuration_Ring2.push_back(rightCriticalPoints.at(i));
        if (i == 7)
            i = 11;
        else if (i == 15)
            i = 19;
    }

    /*
    - Holding the total greeen time for ring wise each phase. 
    - We have 24 phase duration (3cycles). 
    - To store the data ring wise we have skip the phases of opposite ring.
    */
    for (size_t i = 0; i < leftCriticalPoints_GreenTime.size(); i++)
    {
        leftCriticalPoints_GreenTime_Ring1.push_back(leftCriticalPoints_GreenTime.at(i));
        if (i == 3)
            i = 7;
        else if (i == 11)
            i = 15;
        else if (i == 19)
            break;
    }

    for (size_t i = 0; i < rightCriticalPoints_GreenTime.size(); i++)
    {
        rightCriticalPoints_GreenTime_Ring1.push_back(rightCriticalPoints_GreenTime.at(i));
        if (i == 3)
            i = 7;
        else if (i == 11)
            i = 15;
        else if (i == 19)
            break;
    }

    for (size_t i = 4; i < leftCriticalPoints_GreenTime.size(); i++)
    {
        leftCriticalPoints_GreenTime_Ring2.push_back(leftCriticalPoints_GreenTime.at(i));
        if (i == 7)
            i = 11;
        else if (i == 15)
            i = 19;
    }

    for (size_t i = 4; i < rightCriticalPoints_GreenTime.size(); i++)
    {
        rightCriticalPoints_GreenTime_Ring2.push_back(rightCriticalPoints_GreenTime.at(i));
        if (i == 7)
            i = 11;
        else if (i == 15)
            i = 19;
    }

    //Removing all the elements having 0 value.
    for (auto i = leftCriticalPoints_PhaseDuration_Ring1.begin(); i != leftCriticalPoints_PhaseDuration_Ring1.end(); ++i)
    {
        if (*i == 0)
        {
            leftCriticalPoints_PhaseDuration_Ring1.erase(i);
            i--;
        }
    }

    for (auto i = rightCriticalPoints_PhaseDuration_Ring1.begin(); i != rightCriticalPoints_PhaseDuration_Ring1.end(); ++i)
    {
        if (*i == 0)
        {
            rightCriticalPoints_PhaseDuration_Ring1.erase(i);
            i--;
        }
    }

    for (auto i = leftCriticalPoints_PhaseDuration_Ring2.begin(); i != leftCriticalPoints_PhaseDuration_Ring2.end(); ++i)
    {
        if (*i == 0)
        {
            leftCriticalPoints_PhaseDuration_Ring2.erase(i);
            i--;
        }
    }

    for (auto i = rightCriticalPoints_PhaseDuration_Ring2.begin(); i != rightCriticalPoints_PhaseDuration_Ring2.end(); ++i)
    {
        if (*i == 0)
        {
            rightCriticalPoints_PhaseDuration_Ring2.erase(i);
            i--;
        }
    }

    for (auto i = leftCriticalPoints_GreenTime_Ring1.begin(); i != leftCriticalPoints_GreenTime_Ring1.end(); ++i)
    {
        if (*i == 0)
        {
            leftCriticalPoints_GreenTime_Ring1.erase(i);
            i--;
        }
    }

    for (auto i = rightCriticalPoints_GreenTime_Ring1.begin(); i != rightCriticalPoints_GreenTime_Ring1.end(); ++i)
    {
        if (*i == 0)
        {
            rightCriticalPoints_GreenTime_Ring1.erase(i);
            i--;
        }
    }

    for (auto i = leftCriticalPoints_GreenTime_Ring2.begin(); i != leftCriticalPoints_GreenTime_Ring2.end(); ++i)
    {
        if (*i == 0)
        {
            leftCriticalPoints_GreenTime_Ring2.erase(i);
            i--;
        }
    }

    for (auto i = rightCriticalPoints_GreenTime_Ring2.begin(); i != rightCriticalPoints_GreenTime_Ring2.end(); ++i)
    {
        if (*i == 0)
        {
            rightCriticalPoints_GreenTime_Ring2.erase(i);
            i--;
        }
    }

    if (leftCriticalPoints_GreenTime_Ring1.size() != plannedSignalGroupInRing1.size())
        leftCriticalPoints_GreenTime_Ring1.insert(leftCriticalPoints_GreenTime_Ring1.begin(), 0.0);

    if (rightCriticalPoints_GreenTime_Ring1.size() != plannedSignalGroupInRing1.size())
        rightCriticalPoints_GreenTime_Ring1.insert(rightCriticalPoints_GreenTime_Ring1.begin(), 0.0);

    if (leftCriticalPoints_GreenTime_Ring2.size() != plannedSignalGroupInRing2.size())
        leftCriticalPoints_GreenTime_Ring2.insert(leftCriticalPoints_GreenTime_Ring2.begin(), 0.0);

    if (rightCriticalPoints_GreenTime_Ring2.size() != plannedSignalGroupInRing2.size())
        rightCriticalPoints_GreenTime_Ring2.insert(rightCriticalPoints_GreenTime_Ring2.begin(), 0.0);
}

void PriorityRequestSolver::createEventList()
{
    Schedule::TCISchedule ring1Schedule;
    Schedule::TCISchedule ring2Schedule;
    int vehicleSignalGroup{};
    int vehicleSignalGroupRing{};
    int temporaryPhase{};
    double tempVehicleETA{};
    double tempVehicleETA_Duration{};
    vector<double> temporaryVehicleETA;
    vector<double> temporaryVehicleETA_Duration;
    vector<int>::iterator it;

    //Only One EV is the list
    if (bEVStatus == true && noOfEVInList <= 2)
    {
        ring1_TCISchedule.clear();
        ring2_TCISchedule.clear();

        // if(EV_P12.size()==0 && EV_P22.size())

        for (size_t i = 0; i < trafficControllerStatus.size(); i++)
        {
            temporaryPhase = trafficControllerStatus[i].startingPhase1;
            it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), temporaryPhase);

            //If requested phases and starting phases are different
            if (it == requestedSignalGroup.end()) //Check whether starting phase 1 is in requested Signal group or not. If it is not in requested signal group forceoff that phase.
            {
                ring1Schedule.commandPhase = temporaryPhase;
                ring1Schedule.commandType = FORCEOFF_PHASES;
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = 0.0;
                ring1_TCISchedule.push_back(ring1Schedule);

                for (size_t j = 0; j < requestedSignalGroup.size(); j++)
                {
                    vehicleSignalGroup = requestedSignalGroup.at(j);

                    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1RingNO = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                     [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == vehicleSignalGroup; });

                    if (findSignalGroup1RingNO->phaseRing == 1) //Find the requested phase which is on ring1. Need to hold that phase. Starting time will be sum of yellow and red time of starting phase1
                    {
                        vector<TrafficControllerData::TrafficSignalPlan>::iterator findStartingPhase1_OnSignalPlan = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                                  [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

                        vector<RequestList>::iterator findSignalGroup1_OnPriorityList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                                     [&](RequestList const &p) { return p.requestedPhase == vehicleSignalGroup; });

                        ring1Schedule.commandPhase = vehicleSignalGroup;
                        ring1Schedule.commandType = HOLD_PHASES;
                        ring1Schedule.commandStartTime = findStartingPhase1_OnSignalPlan->yellowChange + findStartingPhase1_OnSignalPlan->redClear;
                        ring1Schedule.commandEndTime = findSignalGroup1_OnPriorityList->vehicleETA + findSignalGroup1_OnPriorityList->vehicleETA_Duration;
                        ring1_TCISchedule.push_back(ring1Schedule);
                    }
                }
            }
            //If starting phases are in requested signal group
            else
            {
                for (size_t j = 0; j < requestedSignalGroup.size(); j++)
                {
                    vehicleSignalGroup = requestedSignalGroup.at(j);

                    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1_OnSignalPlan = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                            [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == vehicleSignalGroup; });

                    if (temporaryPhase == vehicleSignalGroup && findSignalGroup1_OnSignalPlan->phaseRing == 1) //Find the requested phase which is on ring1. Need to hold that phase. Starting time will be 0.0 and end time will be eta+eta_duration
                    {
                        vector<RequestList>::iterator findETAOfSignalGroup1 = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                           [&](RequestList const &p) { return p.requestedPhase == vehicleSignalGroup; });

                        ring1Schedule.commandPhase = vehicleSignalGroup;
                        ring1Schedule.commandType = HOLD_PHASES;
                        ring1Schedule.commandStartTime = 0.0;
                        ring1Schedule.commandEndTime = findETAOfSignalGroup1->vehicleETA + findETAOfSignalGroup1->vehicleETA_Duration;
                        ring1_TCISchedule.push_back(ring1Schedule);
                    }
                }
            }

            temporaryPhase = trafficControllerStatus[i].startingPhase2;
            it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), temporaryPhase);
            //If requested phases and starting phases are different
            if (it == requestedSignalGroup.end()) //Check whether starting phase 2 is in requested Signal group or not. If it is not in requested signal group forceoff that phase.
            {
                ring2Schedule.commandPhase = temporaryPhase;
                ring2Schedule.commandType = FORCEOFF_PHASES;
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = 0.0;
                ring2_TCISchedule.push_back(ring2Schedule);

                for (size_t j = 0; j < requestedSignalGroup.size(); j++)
                {
                    vehicleSignalGroup = requestedSignalGroup.at(j);

                    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2RingNO = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                     [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == vehicleSignalGroup; });

                    if (findSignalGroup2RingNO->phaseRing == 2) //Find the requested phase which is on ring1. Need to hold that phase. Starting time will be sum of yellow and red time of starting phase1
                    {
                        vector<TrafficControllerData::TrafficSignalPlan>::iterator findStartingPhase2_OnSignalPlan = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                                  [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

                        vector<RequestList>::iterator findSignalGroup2_OnPriorityList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                                     [&](RequestList const &p) { return p.requestedPhase == vehicleSignalGroup; });

                        ring2Schedule.commandPhase = vehicleSignalGroup;
                        ring2Schedule.commandType = HOLD_PHASES;
                        ring2Schedule.commandStartTime = findStartingPhase2_OnSignalPlan->yellowChange + findStartingPhase2_OnSignalPlan->redClear;
                        ring2Schedule.commandEndTime = findSignalGroup2_OnPriorityList->vehicleETA + findSignalGroup2_OnPriorityList->vehicleETA_Duration;
                        ring2_TCISchedule.push_back(ring2Schedule);
                    }
                }
            }
            //If starting phases are in requested signal group
            else
            {
                for (size_t j = 0; j < requestedSignalGroup.size(); j++)
                {
                    vehicleSignalGroup = requestedSignalGroup.at(j);

                    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2_OnSignalPlan = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                            [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == vehicleSignalGroup; });

                    if (temporaryPhase == vehicleSignalGroup && findSignalGroup2_OnSignalPlan->phaseRing == 2) //Find the requested phase which is on ring1. Need to hold that phase. Starting time will be 0.0 and end time will be eta+eta_duration
                    {
                        vector<RequestList>::iterator findSignalGroupOnPriorityList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                                   [&](RequestList const &p) { return p.requestedPhase == vehicleSignalGroup; });

                        ring2Schedule.commandPhase = vehicleSignalGroup;
                        ring2Schedule.commandType = HOLD_PHASES;
                        ring2Schedule.commandStartTime = 0.0;
                        ring2Schedule.commandEndTime = findSignalGroupOnPriorityList->vehicleETA + findSignalGroupOnPriorityList->vehicleETA_Duration;
                        ring2_TCISchedule.push_back(ring2Schedule);
                    }
                }
            }
        }

        /*vehicle Call */
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            vehicleSignalGroup = priorityRequestList[i].requestedPhase;

            for (int k = 0; k < noOfPhase; k++)
            {
                if (trafficSignalPlan[k].phaseNumber == vehicleSignalGroup)
                    vehicleSignalGroupRing = trafficSignalPlan[k].phaseRing;
            }

            if (vehicleSignalGroupRing == 1)
            {
                ring1Schedule.commandPhase = vehicleSignalGroup;
                ring1Schedule.commandType = CALL_VEH_PHASES;
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration;

                ring1_TCISchedule.push_back(ring1Schedule);
            }

            if (vehicleSignalGroupRing == 2)
            {
                ring2Schedule.commandPhase = vehicleSignalGroup;
                ring2Schedule.commandType = CALL_VEH_PHASES;
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration;

                ring2_TCISchedule.push_back(ring2Schedule);
            }
        }
    }
    //If there is multiple EV comming from the same direction
    else if (bEVStatus == true && noOfEVInList > 2 && requestedSignalGroup.size() <= 2)
    {
        ring1_TCISchedule.clear();
        ring2_TCISchedule.clear();

        //Find the maximum ETA and ETA duration from among all the EV
        for (size_t k = 0; k < priorityRequestList.size(); k++)
        {
            temporaryVehicleETA.push_back(priorityRequestList[k].vehicleETA);
            temporaryVehicleETA_Duration.push_back(priorityRequestList[k].vehicleETA_Duration);
        }

        tempVehicleETA = *max_element(temporaryVehicleETA.begin(), temporaryVehicleETA.end());
        tempVehicleETA_Duration = *max_element(temporaryVehicleETA_Duration.begin(), temporaryVehicleETA_Duration.end());

        for (size_t i = 0; i < trafficControllerStatus.size(); i++)
        {
            temporaryPhase = trafficControllerStatus[i].startingPhase1;
            it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), temporaryPhase);
            //If requested phases and starting phases are different
            if (it == requestedSignalGroup.end()) //Check whether starting phase 1 is in requested Signal group or not. If it is not in requested signal group forceoff that phase.
            {
                ring1Schedule.commandPhase = temporaryPhase;
                ring1Schedule.commandType = FORCEOFF_PHASES;
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = 0.0;
                ring1_TCISchedule.push_back(ring1Schedule);

                for (size_t j = 0; j < requestedSignalGroup.size(); j++)
                {
                    vehicleSignalGroup = requestedSignalGroup.at(j);

                    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1RingNO = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                     [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == vehicleSignalGroup; });

                    if (findSignalGroup1RingNO->phaseRing == 1) //Find the requested phase which is on ring1. Need to hold that phase. Starting time will be sum of yellow and red time of starting phase1
                    {
                        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnSignalPlan = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                              [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

                        ring1Schedule.commandPhase = vehicleSignalGroup;
                        ring1Schedule.commandType = HOLD_PHASES;
                        ring1Schedule.commandStartTime = findSignalGroupOnSignalPlan->yellowChange + findSignalGroupOnSignalPlan->redClear;
                        ring1Schedule.commandEndTime = tempVehicleETA + tempVehicleETA_Duration;
                        ring1_TCISchedule.push_back(ring1Schedule);
                    }
                }
            }
            //If starting phases are in requested signal group
            else
            {
                for (size_t j = 0; j < requestedSignalGroup.size(); j++)
                {
                    vehicleSignalGroup = requestedSignalGroup.at(j);

                    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnSignalPlan = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == vehicleSignalGroup; });

                    if (temporaryPhase == vehicleSignalGroup && findSignalGroupOnSignalPlan->phaseRing == 1) //Find the requested phase which is on ring1. Need to hold that phase. Starting time will be 0.0 and end time will be eta+eta_duration
                    {
                        // vector<RequestList>::iterator findSignalGroupOnPriorityList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                        //                                                                            [&](RequestList const &p) { return p.requestedPhase == vehicleSignalGroup; });

                        ring1Schedule.commandPhase = vehicleSignalGroup;
                        ring1Schedule.commandType = HOLD_PHASES;
                        ring1Schedule.commandStartTime = 0.0;
                        ring1Schedule.commandEndTime = tempVehicleETA + tempVehicleETA_Duration;
                        ring1_TCISchedule.push_back(ring1Schedule);
                    }
                }
            }

            temporaryPhase = trafficControllerStatus[i].startingPhase2;
            it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), temporaryPhase);
            //If requested phases and starting phases are different
            if (it == requestedSignalGroup.end()) //Check whether starting phase 2 is in requested Signal group or not. If it is not in requested signal group forceoff that phase.
            {
                ring2Schedule.commandPhase = temporaryPhase;
                ring2Schedule.commandType = FORCEOFF_PHASES;
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = 0.0;
                ring2_TCISchedule.push_back(ring2Schedule);

                for (size_t j = 0; j < requestedSignalGroup.size(); j++)
                {
                    vehicleSignalGroup = requestedSignalGroup.at(j);

                    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2RingNO = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                     [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == vehicleSignalGroup; });

                    if (findSignalGroup2RingNO->phaseRing == 2) //Find the requested phase which is on ring1. Need to hold that phase. Starting time will be sum of yellow and red time of starting phase1
                    {
                        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnSignalPlan = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                              [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

                        ring2Schedule.commandPhase = vehicleSignalGroup;
                        ring2Schedule.commandType = HOLD_PHASES;
                        ring2Schedule.commandStartTime = findSignalGroupOnSignalPlan->yellowChange + findSignalGroupOnSignalPlan->redClear;
                        ring2Schedule.commandEndTime = tempVehicleETA + tempVehicleETA_Duration;
                        ring2_TCISchedule.push_back(ring2Schedule);
                    }
                }
            }
            //If starting phases are in requested signal group
            else
            {
                for (size_t j = 0; j < requestedSignalGroup.size(); j++)
                {
                    vehicleSignalGroup = requestedSignalGroup.at(j);

                    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnSignalPlan = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == vehicleSignalGroup; });

                    if (temporaryPhase == vehicleSignalGroup && findSignalGroupOnSignalPlan->phaseRing == 2) //Find the requested phase which is on ring1. Need to hold that phase. Starting time will be 0.0 and end time will be eta+eta_duration
                    {
                        // vector<RequestList>::iterator findSignalGroupOnPriorityList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                        //                                                                            [&](RequestList const &p) { return p.requestedPhase == vehicleSignalGroup; });

                        ring2Schedule.commandPhase = vehicleSignalGroup;
                        ring2Schedule.commandType = HOLD_PHASES;
                        ring2Schedule.commandStartTime = 0.0;
                        ring2Schedule.commandEndTime = tempVehicleETA + tempVehicleETA_Duration;
                        ring2_TCISchedule.push_back(ring2Schedule);
                    }
                }
            }
        }
        /*vehicle Call */
        for (size_t i = 0; i < requestedSignalGroup.size(); i++)
        {
            vehicleSignalGroup = requestedSignalGroup.at(i);

            for (int k = 0; k < noOfPhase; k++)
            {
                if (trafficSignalPlan[k].phaseNumber == vehicleSignalGroup)
                    vehicleSignalGroupRing = trafficSignalPlan[k].phaseRing;
            }

            if (vehicleSignalGroupRing == 1)
            {
                ring1Schedule.commandPhase = vehicleSignalGroup;
                ring1Schedule.commandType = CALL_VEH_PHASES;
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = tempVehicleETA + tempVehicleETA_Duration;

                ring1_TCISchedule.push_back(ring1Schedule);
            }

            if (vehicleSignalGroupRing == 2)
            {
                ring2Schedule.commandPhase = vehicleSignalGroup;
                ring2Schedule.commandType = CALL_VEH_PHASES;
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = tempVehicleETA + tempVehicleETA_Duration;

                ring2_TCISchedule.push_back(ring2Schedule);
            }
        }
    }
    //Multiple EV coming from different direction
    else if (bEVStatus == true && noOfEVInList > 2 && requestedSignalGroup.size() > 2)
    {
        //Computing ring wise cummilative sum for phase duration of left and right cricital points
        // For example leftCriticalPoints PhaseDuration for any phase is the sum of leftCriticalPoints PhaseDuration of previous phase and leftCriticalPoints PhaseDuration of that phase.
        for (size_t i = 1; i < leftCriticalPoints_PhaseDuration_Ring1.size(); i++)
        {
            leftCriticalPoints_PhaseDuration_Ring1[i] = leftCriticalPoints_PhaseDuration_Ring1[i - 1] + leftCriticalPoints_PhaseDuration_Ring1[i];
        }

        for (size_t i = 1; i < rightCriticalPoints_PhaseDuration_Ring1.size(); i++)
        {
            rightCriticalPoints_PhaseDuration_Ring1[i] = rightCriticalPoints_PhaseDuration_Ring1[i - 1] + rightCriticalPoints_PhaseDuration_Ring1[i];
        }

        for (size_t i = 1; i < leftCriticalPoints_PhaseDuration_Ring2.size(); i++)
        {
            leftCriticalPoints_PhaseDuration_Ring2[i] = leftCriticalPoints_PhaseDuration_Ring2[i - 1] + leftCriticalPoints_PhaseDuration_Ring2[i];
        }

        for (size_t i = 1; i < rightCriticalPoints_PhaseDuration_Ring2.size(); i++)
        {
            rightCriticalPoints_PhaseDuration_Ring2[i] = rightCriticalPoints_PhaseDuration_Ring2[i - 1] + rightCriticalPoints_PhaseDuration_Ring2[i];
        }

        //Computing ring wise cummilative sum for grren time of left and right cricital points
        // for (size_t i = 1; i < leftCriticalPoints_GreenTime_Ring1.size(); i++)
        // {
        //     leftCriticalPoints_GreenTime_Ring1[i] = leftCriticalPoints_GreenTime_Ring1[i - 1] + leftCriticalPoints_GreenTime_Ring1[i];
        // }

        // for (size_t i = 1; i < rightCriticalPoints_GreenTime_Ring1.size(); i++)
        // {
        //     rightCriticalPoints_GreenTime_Ring1[i] = rightCriticalPoints_GreenTime_Ring1[i - 1] + rightCriticalPoints_GreenTime_Ring1[i];
        // }

        // for (size_t i = 1; i < leftCriticalPoints_GreenTime_Ring2.size(); i++)
        // {
        //     leftCriticalPoints_GreenTime_Ring2[i] = leftCriticalPoints_GreenTime_Ring2[i - 1] + leftCriticalPoints_GreenTime_Ring2[i];
        // }

        // for (size_t i = 1; i < rightCriticalPoints_GreenTime_Ring1.size(); i++)
        // {
        //     rightCriticalPoints_GreenTime_Ring2[i] = rightCriticalPoints_GreenTime_Ring2[i - 1] + rightCriticalPoints_GreenTime_Ring2[i];
        // }

        ring1_TCISchedule.clear();
        ring2_TCISchedule.clear();

        /*Hold Ring1 */
        for (size_t i = 0; i < plannedSignalGroupInRing1.size(); i++)
        {

            ring1Schedule.commandPhase = plannedSignalGroupInRing1[i];
            ring1Schedule.commandType = HOLD_PHASES;

            /*
            -Hold upto green time for starting phase(first phase having green time greater than zero)
        */
            if (i == 0)
            {
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = leftCriticalPoints_GreenTime_Ring1[i];
            }
            /*
            - For rest of the phase hold will start at the end of previous phase
            - For rest of the phase hold will continue upto the end the green time
            - Logic is: Cumulative Phase duration of the previous phase plus green time of the current phase
        */
            else
            {
                ring1Schedule.commandStartTime = leftCriticalPoints_PhaseDuration_Ring1[i - 1];
                ring1Schedule.commandEndTime = leftCriticalPoints_PhaseDuration_Ring1[i - 1] + leftCriticalPoints_GreenTime_Ring1[i];
            }

            ring1_TCISchedule.push_back(ring1Schedule);
        }

        /*Hold Ring2 */
        for (size_t i = 0; i < plannedSignalGroupInRing2.size(); i++)
        {
            ring2Schedule.commandPhase = plannedSignalGroupInRing2[i];
            ring2Schedule.commandType = HOLD_PHASES;

            /*
            -Hold upto green time for starting phase(first phase having green time greater than zero)
        */
            if (i == 0)
            {
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = leftCriticalPoints_GreenTime_Ring2[i];
            }

            /*
            - For rest of the phase hold will start at the end of previous phase
            - For rest of the phase hold will continue upto the end the green time
            - Logic is: Cumulative Phase duration of the previous phase plus green time of the current phase
        */
            else
            {
                ring2Schedule.commandStartTime = leftCriticalPoints_PhaseDuration_Ring2[i - 1];
                ring2Schedule.commandEndTime = leftCriticalPoints_PhaseDuration_Ring2[i - 1] + leftCriticalPoints_GreenTime_Ring2[i];
            }

            ring2_TCISchedule.push_back(ring2Schedule);
        }

        /*ForceOff Ring1 */
        for (size_t i = 0; i < plannedSignalGroupInRing1.size(); i++)
        {
            ring1Schedule.commandPhase = plannedSignalGroupInRing1[i];
            ring1Schedule.commandType = FORCEOFF_PHASES;

            /*
            -Force off for starting phase (first phase having green time greater than zero) at the end of green time
            -End time for force off will be zero
        */
            if (i == 0)
                ring1Schedule.commandStartTime = rightCriticalPoints_GreenTime_Ring1[i];

            /*
            - For rest of the phase force off at the end of green time of corresponding phase
            - Logic is: Cumulative Phase duration of the previous phase plus green time of the current phase
            - End time for force off will be zero
        */
            else
                ring1Schedule.commandStartTime = rightCriticalPoints_PhaseDuration_Ring1[i - 1] + rightCriticalPoints_GreenTime_Ring1[i];

            ring1Schedule.commandEndTime = 0.0;

            ring1_TCISchedule.push_back(ring1Schedule);
        }

        /*ForceOff Ring2 */
        for (size_t i = 0; i < plannedSignalGroupInRing2.size(); i++)
        {
            ring2Schedule.commandPhase = plannedSignalGroupInRing2[i];
            ring2Schedule.commandType = FORCEOFF_PHASES;

            if (i == 0)
                ring2Schedule.commandStartTime = rightCriticalPoints_GreenTime_Ring2[i];

            else
                ring2Schedule.commandStartTime = rightCriticalPoints_PhaseDuration_Ring2[i - 1] + rightCriticalPoints_GreenTime_Ring2[i];

            ring2Schedule.commandEndTime = 0.0;

            ring2_TCISchedule.push_back(ring2Schedule);
        }

        /*vehicle Call */
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            vehicleSignalGroup = priorityRequestList[i].requestedPhase;

            for (int k = 0; k < noOfPhase; k++)
            {
                if (trafficSignalPlan[k].phaseNumber == vehicleSignalGroup)
                    vehicleSignalGroupRing = trafficSignalPlan[k].phaseRing;
            }

            if (vehicleSignalGroupRing == 1)
            {
                ring1Schedule.commandPhase = vehicleSignalGroup;
                ring1Schedule.commandType = CALL_VEH_PHASES;
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration;

                ring1_TCISchedule.push_back(ring1Schedule);
            }

            if (vehicleSignalGroupRing == 2)
            {
                ring2Schedule.commandPhase = vehicleSignalGroup;
                ring2Schedule.commandType = CALL_VEH_PHASES;
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration;

                ring2_TCISchedule.push_back(ring2Schedule);
            }
        }
    }

    else
    {
        //Computing ring wise cummilative sum for phase duration of left and right cricital points
        // For example leftCriticalPoints PhaseDuration for any phase is the sum of leftCriticalPoints PhaseDuration of previous phase and leftCriticalPoints PhaseDuration of that phase.
        for (size_t i = 1; i < leftCriticalPoints_PhaseDuration_Ring1.size(); i++)
        {
            leftCriticalPoints_PhaseDuration_Ring1[i] = leftCriticalPoints_PhaseDuration_Ring1[i - 1] + leftCriticalPoints_PhaseDuration_Ring1[i];
        }

        for (size_t i = 1; i < rightCriticalPoints_PhaseDuration_Ring1.size(); i++)
        {
            rightCriticalPoints_PhaseDuration_Ring1[i] = rightCriticalPoints_PhaseDuration_Ring1[i - 1] + rightCriticalPoints_PhaseDuration_Ring1[i];
        }

        for (size_t i = 1; i < leftCriticalPoints_PhaseDuration_Ring2.size(); i++)
        {
            leftCriticalPoints_PhaseDuration_Ring2[i] = leftCriticalPoints_PhaseDuration_Ring2[i - 1] + leftCriticalPoints_PhaseDuration_Ring2[i];
        }

        for (size_t i = 1; i < rightCriticalPoints_PhaseDuration_Ring2.size(); i++)
        {
            rightCriticalPoints_PhaseDuration_Ring2[i] = rightCriticalPoints_PhaseDuration_Ring2[i - 1] + rightCriticalPoints_PhaseDuration_Ring2[i];
        }

        //Computing ring wise cummilative sum for grren time of left and right cricital points
        // for (size_t i = 1; i < leftCriticalPoints_GreenTime_Ring1.size(); i++)
        // {
        //     leftCriticalPoints_GreenTime_Ring1[i] = leftCriticalPoints_GreenTime_Ring1[i - 1] + leftCriticalPoints_GreenTime_Ring1[i];
        // }

        // for (size_t i = 1; i < rightCriticalPoints_GreenTime_Ring1.size(); i++)
        // {
        //     rightCriticalPoints_GreenTime_Ring1[i] = rightCriticalPoints_GreenTime_Ring1[i - 1] + rightCriticalPoints_GreenTime_Ring1[i];
        // }

        // for (size_t i = 1; i < leftCriticalPoints_GreenTime_Ring2.size(); i++)
        // {
        //     leftCriticalPoints_GreenTime_Ring2[i] = leftCriticalPoints_GreenTime_Ring2[i - 1] + leftCriticalPoints_GreenTime_Ring2[i];
        // }

        // for (size_t i = 1; i < rightCriticalPoints_GreenTime_Ring1.size(); i++)
        // {
        //     rightCriticalPoints_GreenTime_Ring2[i] = rightCriticalPoints_GreenTime_Ring2[i - 1] + rightCriticalPoints_GreenTime_Ring2[i];
        // }

        ring1_TCISchedule.clear();
        ring2_TCISchedule.clear();

        /*Hold Ring1 */
        for (int i = 0; i < 8; i++)
        {

            ring1Schedule.commandPhase = plannedSignalGroupInRing1[i];
            ring1Schedule.commandType = HOLD_PHASES;

            /*
            -Hold upto green time for starting phase(first phase having green time greater than zero)
        */
            if (i == 0)
            {
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = leftCriticalPoints_GreenTime_Ring1[i];
            }
            /*
            - For rest of the phase hold will start at the end of previous phase
            - For rest of the phase hold will continue upto the end the green time
            - Logic is: Cumulative Phase duration of the previous phase plus green time of the current phase
        */
            else
            {
                ring1Schedule.commandStartTime = leftCriticalPoints_PhaseDuration_Ring1[i - 1];
                ring1Schedule.commandEndTime = leftCriticalPoints_PhaseDuration_Ring1[i - 1] + leftCriticalPoints_GreenTime_Ring1[i];
            }

            ring1_TCISchedule.push_back(ring1Schedule);
        }

        /*Hold Ring2 */
        for (int i = 0; i < 8; i++)
        {
            ring2Schedule.commandPhase = plannedSignalGroupInRing2[i];
            ring2Schedule.commandType = HOLD_PHASES;

            /*
            -Hold upto green time for starting phase(first phase having green time greater than zero)
        */
            if (i == 0)
            {
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = leftCriticalPoints_GreenTime_Ring2[i];
            }

            /*
            - For rest of the phase hold will start at the end of previous phase
            - For rest of the phase hold will continue upto the end the green time
            - Logic is: Cumulative Phase duration of the previous phase plus green time of the current phase
        */
            else
            {
                ring2Schedule.commandStartTime = leftCriticalPoints_PhaseDuration_Ring2[i - 1];
                ring2Schedule.commandEndTime = leftCriticalPoints_PhaseDuration_Ring2[i - 1] + leftCriticalPoints_GreenTime_Ring2[i];
            }

            ring2_TCISchedule.push_back(ring2Schedule);
        }

        /*ForceOff Ring1 */
        for (int i = 0; i < 8; i++)
        {
            ring1Schedule.commandPhase = plannedSignalGroupInRing1[i];
            ring1Schedule.commandType = FORCEOFF_PHASES;

            /*
            -Force off for starting phase (first phase having green time greater than zero) at the end of green time
            -End time for force off will be zero
        */
            if (i == 0)
                ring1Schedule.commandStartTime = rightCriticalPoints_GreenTime_Ring1[i];

            /*
            - For rest of the phase force off at the end of green time of corresponding phase
            - Logic is: Cumulative Phase duration of the previous phase plus green time of the current phase
            - End time for force off will be zero
        */
            else
                ring1Schedule.commandStartTime = rightCriticalPoints_PhaseDuration_Ring1[i - 1] + rightCriticalPoints_GreenTime_Ring1[i];

            ring1Schedule.commandEndTime = 0.0;

            ring1_TCISchedule.push_back(ring1Schedule);
        }

        /*ForceOff Ring2 */
        for (int i = 0; i < 8; i++)
        {
            ring2Schedule.commandPhase = plannedSignalGroupInRing2[i];
            ring2Schedule.commandType = FORCEOFF_PHASES;

            if (i == 0)
                ring2Schedule.commandStartTime = rightCriticalPoints_GreenTime_Ring2[i];

            else
                ring2Schedule.commandStartTime = rightCriticalPoints_PhaseDuration_Ring2[i - 1] + rightCriticalPoints_GreenTime_Ring2[i];

            ring2Schedule.commandEndTime = 0.0;

            ring2_TCISchedule.push_back(ring2Schedule);
        }

        /*vehicle Call */
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            vehicleSignalGroup = priorityRequestList[i].requestedPhase;

            for (int k = 0; k < noOfPhase; k++)
            {
                if (trafficSignalPlan[k].phaseNumber == vehicleSignalGroup)
                    vehicleSignalGroupRing = trafficSignalPlan[k].phaseRing;
            }

            if (vehicleSignalGroupRing == 1)
            {
                ring1Schedule.commandPhase = vehicleSignalGroup;
                ring1Schedule.commandType = CALL_VEH_PHASES;
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration;

                ring1_TCISchedule.push_back(ring1Schedule);
            }

            if (vehicleSignalGroupRing == 2)
            {
                ring2Schedule.commandPhase = vehicleSignalGroup;
                ring2Schedule.commandType = CALL_VEH_PHASES;
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration;

                ring2_TCISchedule.push_back(ring2Schedule);
            }
        }
    }
}

/*
    -
*/
string PriorityRequestSolver::createScheduleJsonString()
{
    // vector<Schedule::TCISchedule> completeSchedule;
    // ring1_TCISchedule.insert(ring1_TCISchedule.end(), ring2_TCISchedule.begin(), ring2_TCISchedule.end());
    // completeSchedule.insert(completeSchedule.end(), ring1_TCISchedule.begin(), ring1_TCISchedule.end());
    // completeSchedule.insert(completeSchedule.end(), ring2_TCISchedule.begin(), ring2_TCISchedule.end());

    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    Json::StyledStreamWriter styledStreamWriter;
    ofstream outputter("schedule.json");
    jsonObject["MsgType"] = "Schedule";
    // jsonObject["Schedule"]["Type"] = "clear";
    // jsonObject["Schedule"]["CommandType"] = "Event";

    if (ring1_TCISchedule.empty() && ring2_TCISchedule.empty())
    {
        jsonObject["Schedule"] = "Clear";
    }

    else
    {
        for (unsigned int i = 0; i < ring1_TCISchedule.size(); i++)
        {
            jsonObject["Schedule"]["Ring1"][i]["commandPhase"] = ring1_TCISchedule[i].commandPhase;
            jsonObject["Schedule"]["Ring1"][i]["commandStartTime"] = ring1_TCISchedule[i].commandStartTime;
            jsonObject["Schedule"]["Ring1"][i]["commandEndTime"] = ring1_TCISchedule[i].commandEndTime;
            jsonObject["Schedule"]["Ring1"][i]["commandType"] = ring1_TCISchedule[i].commandType;
        }
        for (unsigned int i = 0; i < ring2_TCISchedule.size(); i++)
        {
            jsonObject["Schedule"]["Ring2"][i]["commandPhase"] = ring2_TCISchedule[i].commandPhase;
            jsonObject["Schedule"]["Ring2"][i]["commandStartTime"] = ring2_TCISchedule[i].commandStartTime;
            jsonObject["Schedule"]["Ring2"][i]["commandEndTime"] = ring2_TCISchedule[i].commandEndTime;
            jsonObject["Schedule"]["Ring2"][i]["commandType"] = ring2_TCISchedule[i].commandType;
        }

        // for (unsigned int i = 0; i < completeSchedule.size(); i++)
        // {
        //     jsonObject["Schedule"][i]["commandPhase"] = completeSchedule[i].commandPhase;
        //     jsonObject["Schedule"][i]["commandStartTime"] = completeSchedule[i].commandStartTime;
        //     jsonObject["Schedule"][i]["commandEndTime"] = completeSchedule[i].commandEndTime;
        //     jsonObject["Schedule"][i]["commandType"] = completeSchedule[i].commandType;
        // }
    }

    scheduleJsonString = fastWriter.write(jsonObject);
    styledStreamWriter.write(outputter, jsonObject);

    return scheduleJsonString;
}

/*
    - Based on the starting phase we have to identify in total 16phases from 3 cycles. 
        -Like if our starting phase is 3,7 then plannedSignalGroupInRing1{3,4,1,2,3,4,1,2}- second phase 1,2 is from cycle 3. 
*/
void PriorityRequestSolver::obtainRequiredSignalGroup()
{

    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        plannedSignalGroupInRing1.push_back(trafficControllerStatus[i].startingPhase1); //Storing the SP1
        plannedSignalGroupInRing2.push_back(trafficControllerStatus[i].startingPhase2); //Storing the SP2
    }

    if (bEVStatus == true)
    {
        for (size_t i = 0; i < plannedEVPhases.size(); i++) //Obtaining all the required phases for cycle1, ring1
        {
            if (trafficSignalPlan_EV[i].phaseNumber > plannedSignalGroupInRing1.front() && trafficSignalPlan_EV[i].phaseRing == 1)
                plannedSignalGroupInRing1.push_back(trafficSignalPlan_EV[i].phaseNumber);
        }

        for (size_t i = 0; i < plannedEVPhases.size(); i++) //Obtaining all the required phases for cycle2, ring1
        {
            if (trafficSignalPlan_EV[i].phaseNumber < 5 && trafficSignalPlan_EV[i].phaseRing == 1)
                plannedSignalGroupInRing1.push_back(trafficSignalPlan_EV[i].phaseNumber);
        }

        for (size_t i = 0; i < plannedEVPhases.size(); i++) //Obtaining all the required phases for cycle3, ring1
        {
            if (trafficSignalPlan_EV[i].phaseNumber < plannedSignalGroupInRing1.front() && trafficSignalPlan_EV[i].phaseRing == 1)
                plannedSignalGroupInRing1.push_back(trafficSignalPlan_EV[i].phaseNumber);
        }

        for (size_t i = 0; i < plannedEVPhases.size(); i++) //Obtaining all the required phases for cycle1, ring2
        {
            if (trafficSignalPlan_EV[i].phaseNumber > plannedSignalGroupInRing2.front() && trafficSignalPlan_EV[i].phaseRing == 2)
                plannedSignalGroupInRing2.push_back(trafficSignalPlan_EV[i].phaseNumber);
        }

        for (size_t i = 0; i < plannedEVPhases.size(); i++) //Obtaining all the required phases for cycle2, ring2
        {
            if (trafficSignalPlan_EV[i].phaseNumber < 9 && trafficSignalPlan_EV[i].phaseRing == 2)
                plannedSignalGroupInRing2.push_back(trafficSignalPlan_EV[i].phaseNumber);
        }

        for (size_t i = 0; i < plannedEVPhases.size(); i++) //Obtaining all the required phases for cycle3, ring2
        {
            if (trafficSignalPlan_EV[i].phaseNumber < plannedSignalGroupInRing2.front() && trafficSignalPlan_EV[i].phaseRing == 2)
                plannedSignalGroupInRing2.push_back(trafficSignalPlan_EV[i].phaseNumber);
        }
    }

    else
    {
        for (int i = 0; i < noOfPhase; i++) //Obtaining all the required phases for cycle1, ring1
        {
            if (trafficSignalPlan[i].phaseNumber > plannedSignalGroupInRing1.front() && trafficSignalPlan[i].phaseRing == 1)
                plannedSignalGroupInRing1.push_back(trafficSignalPlan[i].phaseNumber);
        }

        for (int i = 0; i < noOfPhase; i++) //Obtaining all the required phases for cycle2, ring1
        {
            if (trafficSignalPlan[i].phaseNumber < 5 && trafficSignalPlan[i].phaseRing == 1)
                plannedSignalGroupInRing1.push_back(trafficSignalPlan[i].phaseNumber);
        }

        for (int i = 0; i < noOfPhase; i++) //Obtaining all the required phases for cycle3, ring1
        {
            if (trafficSignalPlan[i].phaseNumber < plannedSignalGroupInRing1.front() && trafficSignalPlan[i].phaseRing == 1)
                plannedSignalGroupInRing1.push_back(trafficSignalPlan[i].phaseNumber);
        }

        for (int i = 0; i < noOfPhase; i++) //Obtaining all the required phases for cycle1, ring2
        {
            if (trafficSignalPlan[i].phaseNumber > plannedSignalGroupInRing2.front() && trafficSignalPlan[i].phaseRing == 2)
                plannedSignalGroupInRing2.push_back(trafficSignalPlan[i].phaseNumber);
        }

        for (int i = 0; i < noOfPhase; i++) //Obtaining all the required phases for cycle2, ring2
        {
            if (trafficSignalPlan[i].phaseNumber < 9 && trafficSignalPlan[i].phaseRing == 2)
                plannedSignalGroupInRing2.push_back(trafficSignalPlan[i].phaseNumber);
        }

        for (int i = 0; i < noOfPhase; i++) //Obtaining all the required phases for cycle3, ring2
        {
            if (trafficSignalPlan[i].phaseNumber < plannedSignalGroupInRing2.front() && trafficSignalPlan[i].phaseRing == 2)
                plannedSignalGroupInRing2.push_back(trafficSignalPlan[i].phaseNumber);
        }
    }
}

// void PriorityRequestSolver::split(string strToSplit)
// {
//     std::stringstream in(strToSplit);
//     // vector<int> a;
//     int temp;
//     while (in >> temp)
//     {
//         a.push_back(temp);
//     }
// }

/*
    -
*/
bool PriorityRequestSolver::GLPKSolutionValidation()
{
    bool bValid{false};
    ifstream infile;

    infile.open("Results.txt");

    if (infile.fail())
        std::cout << "Fail to open file" << std::endl;

    else
    {
    }

    // void Menu::readFile() {
    // string line;
    // vector <string> lines;
    // ifstream myfile ("file.txt");

    // if (myfile.is_open()) {
    //     while (getline (myfile, line)) {
    //       cout << line << "\n";
    //       lines.push_back(line);
    //     }

    //     myfile.close();
    // } else {
    //    cout << "Unable to open file";
    // }

    return bValid;
}

/*
    -
*/
void PriorityRequestSolver::readCurrentSignalTimingPlan()
{
    TrafficControllerData::TrafficSignalPlan signalPlan;

    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream signalPlanJson("signalPlan.json");
    std::string configJsonString((std::istreambuf_iterator<char>(signalPlanJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);
    const Json::Value values = jsonObject["TimingPlan"];
    noOfPhase = (jsonObject["TimingPlan"]["NoOfPhase"]).asInt();
    cout << "Total Phase No: " << noOfPhase << endl;

    for (int i = 0; i < noOfPhase; i++)
    {
        PhaseNumber.push_back((jsonObject["TimingPlan"]["PhaseNumber"][i]).asInt());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PedWalk.push_back((jsonObject["TimingPlan"]["PedWalk"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PedClear.push_back((jsonObject["TimingPlan"]["PedClear"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        MinGreen.push_back((jsonObject["TimingPlan"]["MinGreen"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        Passage.push_back((jsonObject["TimingPlan"]["Passage"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        MaxGreen.push_back((jsonObject["TimingPlan"]["MaxGreen"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        YellowChange.push_back((jsonObject["TimingPlan"]["YellowChange"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        RedClear.push_back((jsonObject["TimingPlan"]["RedClear"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PhaseRing.push_back((jsonObject["TimingPlan"]["PhaseRing"][i]).asInt());
    }

    trafficSignalPlan.clear();
    for (int i = 0; i < noOfPhase; i++)
    {
        signalPlan.phaseNumber = PhaseNumber[i];
        signalPlan.pedWalk = PedWalk[i];
        signalPlan.pedClear = PedClear[i];
        signalPlan.minGreen = MinGreen[i];
        signalPlan.passage = Passage[i];
        signalPlan.maxGreen = MaxGreen[i];
        signalPlan.yellowChange = YellowChange[i];
        signalPlan.redClear = RedClear[i];
        signalPlan.phaseRing = PhaseRing[i];
        trafficSignalPlan.push_back(signalPlan);
    }
    //Obtain the phases in P11, P12, P21, P22
    for (int i = 0; i < noOfPhase; i++)
    {
        if (trafficSignalPlan[i].phaseNumber < 3 && trafficSignalPlan[i].phaseRing == 1)
            P11.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 2 && trafficSignalPlan[i].phaseNumber < 5 && trafficSignalPlan[i].phaseRing == 1)
            P12.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 4 && trafficSignalPlan[i].phaseNumber < 7 && trafficSignalPlan[i].phaseRing == 2)
            P21.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 6 && trafficSignalPlan[i].phaseRing == 2)
            P22.push_back(trafficSignalPlan[i].phaseNumber);
    }

    noOfPhasesInRing1 = unsigned(P11.size() + P12.size());
    noOfPhasesInRing2 = unsigned(P21.size() + P22.size());
}

/*
    -
*/
void PriorityRequestSolver::printSignalPlan()
{
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        cout << trafficSignalPlan[i].phaseNumber << " " << trafficSignalPlan[i].phaseRing << " " << trafficSignalPlan[i].minGreen << endl;
    }
}

/*
- This method is responsible for obtaining dynamic EV Traffic Signal Plan
*/
void PriorityRequestSolver::getEVTrafficSignalPlan()
{
    int temporaryPhase{};
    vector<int> temporaryPhaseNumber{};
    temporaryPhaseNumber = PhaseNumber;
    vector<int>::iterator it;
    EV_P11.clear();
    EV_P12.clear();
    EV_P21.clear();
    EV_P22.clear();
    trafficSignalPlan_EV.clear();
    trafficSignalPlan_EV.insert(trafficSignalPlan_EV.end(), trafficSignalPlan.begin(), trafficSignalPlan.end());
    // sort(plannedEVPhases.begin(), plannedEVPhases.end()); //arrange the numbers in ascending order

    for (size_t j = 0; j < plannedEVPhases.size(); j++)
    {
        temporaryPhase = plannedEVPhases.at(j);
        it = std::find(temporaryPhaseNumber.begin(), temporaryPhaseNumber.end(), temporaryPhase);
        if (it != temporaryPhaseNumber.end())
        {
            temporaryPhaseNumber.erase(it);
        }
    }

    for (size_t i = 0; i < temporaryPhaseNumber.size(); i++)
    {
        temporaryPhase = temporaryPhaseNumber.at(i);

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan_EV), std::end(trafficSignalPlan_EV),
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

        if (findSignalGroupOnList != trafficSignalPlan_EV.end())
            trafficSignalPlan_EV.erase(findSignalGroupOnList);
    }

    //Obtain the phases in P11, P12, P21, P22
    for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
    {
        temporaryPhase = trafficSignalPlan_EV[i].phaseNumber;
        if (trafficSignalPlan_EV[i].phaseNumber < 3 && trafficSignalPlan_EV[i].phaseRing == 1)
            EV_P11.push_back(trafficSignalPlan_EV[i].phaseNumber);

        else if (trafficSignalPlan_EV[i].phaseNumber > 2 && trafficSignalPlan_EV[i].phaseNumber < 5 && trafficSignalPlan_EV[i].phaseRing == 1)
            EV_P12.push_back(trafficSignalPlan_EV[i].phaseNumber);

        else if (trafficSignalPlan_EV[i].phaseNumber > 4 && trafficSignalPlan_EV[i].phaseNumber < 7 && trafficSignalPlan_EV[i].phaseRing == 2)
            EV_P21.push_back(trafficSignalPlan_EV[i].phaseNumber);

        else if (trafficSignalPlan_EV[i].phaseNumber > 6 && trafficSignalPlan_EV[i].phaseRing == 2)
            EV_P22.push_back(trafficSignalPlan_EV[i].phaseNumber);
    }
}

/*
    -Dynamic mod file for EV. If there is multiple EV and they are requesting for different phase 
*/
void PriorityRequestSolver::generateEVModFile()
{
    int phasePosition{};

    ofstream FileMod;
    FileMod.open("NewModel_EV.mod", ios::out);
    // =================Defining the sets ======================

    if (EV_P11.size() == 1)
        FileMod << "set P11:={" << EV_P11[0] << "}; \n";

    else if (EV_P11.size() == 2)
        FileMod << "set P11:={" << EV_P11[0] << "," << EV_P11[1] << "};  \n";

    if (EV_P12.size() == 1)
        FileMod << "set P12:={" << EV_P12[0] << "}; \n";

    else if (EV_P12.size() == 2)
        FileMod << "set P12:={" << EV_P12[0] << "," << EV_P12[1] << "};  \n";

    if (EV_P21.size() == 1)
        FileMod << "set P21:={" << EV_P21[0] << "}; \n";

    else if (EV_P21.size() == 2)
        FileMod << "set P21:={" << EV_P21[0] << "," << EV_P21[1] << "};  \n";

    if (EV_P22.size() == 1)
        FileMod << "set P22:={" << EV_P22[0] << "}; \n";

    else if (EV_P22.size() == 2)
        FileMod << "set P22:={" << EV_P22[0] << "," << EV_P22[1] << "};  \n";

    FileMod << "set P:={";
    for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
    {
        if (i == trafficSignalPlan_EV.size() - 1)
            FileMod << trafficSignalPlan_EV[i].phaseNumber;
        else
            FileMod << trafficSignalPlan_EV[i].phaseNumber << ","
                    << " ";
    }

    FileMod << "};\n";

    FileMod << "set K  := {1..3};\n"; // Only two cycles ahead are considered in the model. But we should count the third cycle in the cycle set. Because, assume we are in the midle of cycle one. Therefore, we have cycle 1, 2 and half of cycle 3.
    FileMod << "set J  := {1..10};\n";
    FileMod << "set P2 := {1..8};\n";
    FileMod << "set T  := {1..10};\n"; // at most 10 different types of vehicle may be considered , EV are 1, Transit are 2, Trucks are 3

    // at most 4 rcoordination requests may be considered, 2 phase * 2 cycles ahead. For example, if phases 2 and 6 are coordinated,
    // then CP={2,6} and CP1={2} and CP2={6}. The values of Cl1 and Cu1 show the lower and upper bound of the arrival time of coordination request for phase p in CP1
    // The values of Cl2 and Cu2 show the lower and upper bound of the arrival time of coordination request for phase p in CP2
    FileMod << "set E:={1,2};\n";
    FileMod << "\n";
    // //========================Parameters=========================

    FileMod << "param y    {p in P}, >=0,default 0;\n";
    FileMod << "param red  {p in P}, >=0,default 0;\n";
    FileMod << "param gmin {p in P}, >=0,default 0;\n";
    FileMod << "param gmax {p in P}, >=0,default 0;\n";
    FileMod << "param init1,default 0;\n";
    FileMod << "param init2,default 0;\n";
    FileMod << "param Grn1, default 0;\n";
    FileMod << "param Grn2, default 0;\n";
    FileMod << "param SP1,  integer,default 0;\n";
    FileMod << "param SP2,  integer,default 0;\n";
    FileMod << "param M:=9999,integer;\n";
    FileMod << "param alpha:=100,integer;\n";
    FileMod << "param Rl{p in P, j in J}, >=0,  default 0;\n";
    FileMod << "param Ru{p in P, j in J}, >=0,  default 0;\n";

    // FileMod << "param cycle, :=" << dCoordinationCycle << ";\n"; //    # if we have coordination, the cycle length
    FileMod << "param cycle, :=" << 100 << ";\n";
    FileMod << "param PrioType { t in T}, >=0, default 0;  \n";
    FileMod << "param PrioWeigth { t in T}, >=0, default 0;  \n";
    FileMod << "param priorityType{j in J}, >=0, default 0;  \n";
    FileMod << "param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  \n";
    FileMod << "param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);\n";
    FileMod << "param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);\n";
    FileMod << "param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);\n";
    FileMod << "param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);\n";
    FileMod << "param ReqNo:=sum{p in P,j in J} active_pj[p,j];\n";

    // // the following parameters added in order to consider case when the max green time in one barrier group expired but not in the other barrier group
    // FileMod << "param sumOfGMax11, := sum{p in P11} (gmax[p]*coef[p,1]);\n";
    // FileMod << "param sumOfGMax12, := sum{p in P12} (gmax[p]*coef[p,1]);\n";
    // FileMod << "param sumOfGMax21, := sum{p in P21} (gmax[p]*coef[p,1]);\n";
    // FileMod << "param sumOfGMax22, := sum{p in P22} (gmax[p]*coef[p,1]);\n";
    // FileMod << "param barrier1GmaxSlack, := sumOfGMax11 - sumOfGMax21 ;\n";
    // FileMod << "param barrier2GmaxSlack, := sumOfGMax12 - sumOfGMax22 ;\n";
    // FileMod << "param gmaxSlack{p in P}, := (if coef[p,1]=0 then 0 else (if (p in P11) then gmax[p]*max(0,-barrier1GmaxSlack)/sumOfGMax11  else ( if (p in P21) then gmax[p]*max(0,+barrier1GmaxSlack)/sumOfGMax21  else ( if (p in P12) then gmax[p]*max(0,-barrier2GmaxSlack)/sumOfGMax12  else ( if (p in P22) then gmax[p]*max(0,barrier2GmaxSlack)/sumOfGMax22  else 0) ) ) )    ); \n";
    FileMod << "param gmaxPerRng{p in P,k in K}, := gmax[p];\n";
    FileMod << "\n";
    // // ==================== VARIABLES =======================
    FileMod << "var t{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var g{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var v{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var d{p in P,j in J}, >=0;\n";
    FileMod << "var theta{p in P,j in J}, binary;\n";
    FileMod << "var ttheta{p in P,j in J}, >=0;\n";
    FileMod << "var PriorityDelay;\n";
    FileMod << "var Flex;\n";

    FileMod << "\n";

    // // ===================Constraints==============================

    FileMod << "s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  \n";
    FileMod << "s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  \n";
    FileMod << "s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  \n";
    // # constraints in the same cycle in same ring and barrier group
    if (EV_P11.size() > 1)
        FileMod << "s.t. Prec_11_11_c1{e in E,p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P12.size() > 1)
        FileMod << "s.t. Prec_12_12_c1{e in E,p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P21.size() > 1)
        FileMod << "s.t. Prec_21_21_c1{e in E,p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P22.size() > 1)
        FileMod << "s.t. Prec_22_22_c1{e in E,p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    // # constraints in the same cycle in connecting
    if (EV_P11.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. Prec_11_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[" << EV_P11[phasePosition] << ",1,e]+v[" << EV_P11[phasePosition] << ",1,e];\n";
    }

    if (EV_P11.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. Prec_11_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[" << EV_P11[phasePosition] << ",1,e]+v[" << EV_P11[phasePosition] << ",1,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. Prec_21_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[" << EV_P21[phasePosition] << ",1,e]+v[" << EV_P21[phasePosition] << ",1,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. Prec_21_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[" << EV_P21[phasePosition] << ",1,e]+v[" << EV_P21[phasePosition] << ",1,e];\n";
    }

    // #================ END of cycle 1======================#

    // # constraints in the same cycle in same ring and barrier group
    if (EV_P11.size() > 1)
        FileMod << "s.t. Prec_11_11_c23{e in E,p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P12.size() > 1)
        FileMod << "s.t. Prec_12_12_c23{e in E,p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P21.size() > 1)
        FileMod << "s.t. Prec_21_21_c23{e in E,p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P22.size() > 1)
        FileMod << "s.t. Prec_22_22_c23{e in E,p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";

    // # constraints in the same cycle in connecting

    if (EV_P11.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. Prec_11_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=" << static_cast<int>(EV_P12.size()) + EV_P12[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P11[phasePosition] << ",k,e]+v[" << EV_P11[phasePosition] << ",k,e];\n";
    }

    if (EV_P11.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. Prec_11_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=" << static_cast<int>(EV_P22.size()) + EV_P22[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P11[phasePosition] << ",k,e]+v[" << EV_P11[phasePosition] << ",k,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. Prec_21_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=" << static_cast<int>(EV_P12.size()) + EV_P12[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P21[phasePosition] << ",k,e]+v[" << EV_P21[phasePosition] << ",k,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. Prec_21_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=" << static_cast<int>(EV_P22.size()) + EV_P22[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P21[phasePosition] << ",k,e]+v[" << EV_P21[phasePosition] << ",k,e];\n";
    }

    // # constraints in connecting in different cycles
    if (EV_P12.size() > 0 && EV_P11.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P12.size()) - 1;
        FileMod << "s.t. Prec_12_11_c23{e in E,p in P11, k in K: (card(P11)+p+1)=" << static_cast<int>(EV_P11.size()) + EV_P11[0] + 1 << " and k>1 }:    t[p,k,e]=t[" << EV_P12[phasePosition] << ",k-1,e]+v[" << EV_P12[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P22.size() > 0 && EV_P11.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P22.size()) - 1;
        FileMod << "s.t. Prec_22_11_c23{e in E,p in P11, k in K: (card(P11)+p+1+4)=" << static_cast<int>(EV_P11.size()) + EV_P11[0] + 1 + 4 << " and k>1 }:  t[p,k,e]=t[" << EV_P22[phasePosition] << ",k-1,e]+v[" << EV_P22[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P12.size() > 0 && EV_P21.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P12.size()) - 1;
        FileMod << "s.t. Prec_12_21_c23{e in E,p in P21, k in K: (card(P21)+p+1-4)=" << static_cast<int>(EV_P21.size()) + EV_P21[0] + 1 - 4 << " and k>1 }:  t[p,k,e]=t[" << EV_P12[phasePosition] << ",k-1,e]+v[" << EV_P12[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P22.size() > 0 && EV_P21.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P22.size()) - 1;
        FileMod << "s.t. Prec_22_21_c23{e in E,p in P21, k in K: (card(P21)+p+1)=" << static_cast<int>(EV_P21.size()) + EV_P21[0] + 1 << " and k>1 }:    t[p,k,e]=t[" << EV_P22[phasePosition] << ",k-1,e]+v[" << EV_P22[phasePosition] << ",k-1,e];\n";
    }

    FileMod << "s.t. PhaseLen{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];\n";
    FileMod << "s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";
    FileMod << "s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";

    FileMod << "s.t. PrioDelay1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; \n";
    FileMod << "s.t. PrioDelay2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n";
    FileMod << "s.t. PrioDelay3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];\n";
    FileMod << "s.t. PrioDelay4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);\n";
    FileMod << "s.t. PrioDelay5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];\n";
    FileMod << "s.t. PrioDelay6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];\n";
    FileMod << "s.t. PrioDelay7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];\n";
    FileMod << "s.t. PrioDelay8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; \n";
    FileMod << "s.t. PrioDelay9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";

    FileMod << "s.t. Flexib: Flex= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];\n ";
    FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) )  - 0.01*Flex; \n "; // The coeficient to Flex should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points
    // FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) ); \n ";
    FileMod << "  minimize delay: PriorityDelay;     \n";

    //=============================Writing the Optimal Output into the Results.txt file ==================================
    FileMod << "  \n";
    FileMod << "solve;  \n";
    FileMod << "  \n";
    FileMod << "printf \" \" > \"Results.txt\";  \n";
    FileMod << "printf \"%3d  %3d \\n \",SP1, SP2 >>\"Results.txt\";  \n";
    FileMod << "printf \"%5.2f  %5.2f %5.2f  %5.2f \\n \",init1, init2,Grn1,Grn2 >>\"Results.txt\";  \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,1] else 0  >>\"Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,2] else 0  >>\"Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"Results.txt\";\n";
    FileMod << " } \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,1] else 0  >>\"Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,2] else 0  >>\"Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "printf \"%3d \\n \", ReqNo >>\"Results.txt\";  \n";
    FileMod << "  \n";
    FileMod << "for {p in P,j in J : Rl[p,j]>0}  \n";
    FileMod << " {  \n";
    FileMod << "   printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1)), Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>\"Results.txt\";\n"; // the  term " coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1))" is used to know the request is served in which cycle. For example, aasume there is a request for phase 4. If the request is served in firsr cycle, the term will be 4, the second cycle, the term will be 14 and the third cycle, the term will be 24
    FileMod << " } \n";

    FileMod << "printf \"%5.2f \\n \", PriorityDelay + 0.01*Flex>>\"Results.txt\"; \n";

    FileMod << "printf \"%5.2f \\n \", Flex >>\"Results.txt\"; \n";
    FileMod << "printf \" \\n \">>\"Results.txt\";\n";
    //------------- End of Print the Main body of mode----------------
    FileMod << "end;\n";
    FileMod.close();
}
/*
    -
*/
void PriorityRequestSolver::generateModFile()
{
    ofstream FileMod;
    FileMod.open("NewModel.mod", ios::out);
    // =================Defining the sets ======================

    if (P11.size() == 1)
        FileMod << "set P11:={" << P11[0] << "}; \n";

    else if (P11.size() == 2)
        FileMod << "set P11:={" << P11[0] << "," << P11[1] << "};  \n";

    if (P12.size() == 1)
        FileMod << "set P12:={" << P12[0] << "}; \n";

    else if (P12.size() == 2)
        FileMod << "set P12:={" << P12[0] << "," << P12[1] << "};  \n";

    if (P21.size() == 1)
        FileMod << "set P21:={" << P21[0] << "}; \n";

    else if (P21.size() == 2)
        FileMod << "set P21:={" << P21[0] << "," << P21[1] << "};  \n";

    if (P22.size() == 1)
        FileMod << "set P22:={" << P22[0] << "}; \n";

    else if (P22.size() == 2)
        FileMod << "set P22:={" << P22[0] << "," << P22[1] << "};  \n";

    FileMod << "set P:={";
    for (int i = 0; i < noOfPhase; i++)
    {
        if (i != noOfPhase - 1)
            FileMod << " " << PhaseNumber[i] << ",";
        else
            FileMod << " " << PhaseNumber[i];
    }
    FileMod << "};\n";

    FileMod << "set K  := {1..3};\n"; // Only two cycles ahead are considered in the model. But we should count the third cycle in the cycle set. Because, assume we are in the midle of cycle one. Therefore, we have cycle 1, 2 and half of cycle 3.
    FileMod << "set J  := {1..10};\n";
    FileMod << "set P2 := {1..8};\n";
    FileMod << "set T  := {1..10};\n"; // at most 10 different types of vehicle may be considered , EV are 1, Transit are 2, Trucks are 3

    // at most 4 rcoordination requests may be considered, 2 phase * 2 cycles ahead. For example, if phases 2 and 6 are coordinated,
    // then CP={2,6} and CP1={2} and CP2={6}. The values of Cl1 and Cu1 show the lower and upper bound of the arrival time of coordination request for phase p in CP1
    // The values of Cl2 and Cu2 show the lower and upper bound of the arrival time of coordination request for phase p in CP2
    FileMod << "set E:={1,2};\n";
    FileMod << "\n";
    // //========================Parameters=========================

    FileMod << "param y    {p in P}, >=0,default 0;\n";
    FileMod << "param red  {p in P}, >=0,default 0;\n";
    FileMod << "param gmin {p in P}, >=0,default 0;\n";
    FileMod << "param gmax {p in P}, >=0,default 0;\n";
    FileMod << "param init1,default 0;\n";
    FileMod << "param init2,default 0;\n";
    FileMod << "param Grn1, default 0;\n";
    FileMod << "param Grn2, default 0;\n";
    FileMod << "param SP1,  integer,default 0;\n";
    FileMod << "param SP2,  integer,default 0;\n";
    FileMod << "param M:=9999,integer;\n";
    FileMod << "param alpha:=100,integer;\n";
    FileMod << "param Rl{p in P, j in J}, >=0,  default 0;\n";
    FileMod << "param Ru{p in P, j in J}, >=0,  default 0;\n";

    // FileMod << "param cycle, :=" << dCoordinationCycle << ";\n"; //    # if we have coordination, the cycle length
    FileMod << "param cycle, :=" << 100 << ";\n";
    FileMod << "param PrioType { t in T}, >=0, default 0;  \n";
    FileMod << "param PrioWeigth { t in T}, >=0, default 0;  \n";
    FileMod << "param priorityType{j in J}, >=0, default 0;  \n";
    FileMod << "param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  \n";
    FileMod << "param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);\n";
    FileMod << "param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);\n";
    FileMod << "param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);\n";
    FileMod << "param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);\n";
    FileMod << "param ReqNo:=sum{p in P,j in J} active_pj[p,j];\n";

    // // the following parameters added in order to consider case when the max green time in one barrier group expired but not in the other barrier group
    FileMod << "param sumOfGMax11, := sum{p in P11} (gmax[p]*coef[p,1]);\n";
    FileMod << "param sumOfGMax12, := sum{p in P12} (gmax[p]*coef[p,1]);\n";
    FileMod << "param sumOfGMax21, := sum{p in P21} (gmax[p]*coef[p,1]);\n";
    FileMod << "param sumOfGMax22, := sum{p in P22} (gmax[p]*coef[p,1]);\n";
    FileMod << "param barrier1GmaxSlack, := sumOfGMax11 - sumOfGMax21 ;\n";
    FileMod << "param barrier2GmaxSlack, := sumOfGMax12 - sumOfGMax22 ;\n";
    FileMod << "param gmaxSlack{p in P}, := (if coef[p,1]=0 then 0 else (if (p in P11) then gmax[p]*max(0,-barrier1GmaxSlack)/sumOfGMax11  else ( if (p in P21) then gmax[p]*max(0,+barrier1GmaxSlack)/sumOfGMax21  else ( if (p in P12) then gmax[p]*max(0,-barrier2GmaxSlack)/sumOfGMax12  else ( if (p in P22) then gmax[p]*max(0,barrier2GmaxSlack)/sumOfGMax22  else 0) ) ) )    ); \n";
    FileMod << "param gmaxPerRng{p in P,k in K}, := (if (k=1) then gmax[p]+gmaxSlack[p] else	gmax[p]);\n";

    FileMod << "\n";
    // // ==================== VARIABLES =======================
    FileMod << "var t{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var g{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var v{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var d{p in P,j in J}, >=0;\n";
    FileMod << "var theta{p in P,j in J}, binary;\n";
    FileMod << "var ttheta{p in P,j in J}, >=0;\n";
    FileMod << "var PriorityDelay;\n";
    FileMod << "var Flex;\n";

    FileMod << "\n";

    // // ===================Constraints==============================

    FileMod << "s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  \n";
    FileMod << "s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  \n";
    FileMod << "s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  \n";
    // # constraints in the same cycle in same P??
    FileMod << "s.t. Prec_11_11_c1{e in E,p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. Prec_12_12_c1{e in E,p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. Prec_21_21_c1{e in E,p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. Prec_22_22_c1{e in E,p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    // # constraints in the same cycle in connecting
    FileMod << "s.t. Prec_11_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
    FileMod << "s.t. Prec_11_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
    FileMod << "s.t. Prec_21_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
    FileMod << "s.t. Prec_21_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
    // // #================ END of cycle 1======================#

    // // # constraints in the same cycle in same P??
    FileMod << "s.t. Prec_11_11_c23{e in E,p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. Prec_12_12_c23{e in E,p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. Prec_21_21_c23{e in E,p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. Prec_22_22_c23{e in E,p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";

    // # constraints in the same cycle in connecting
    FileMod << "s.t. Prec_11_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
    FileMod << "s.t. Prec_11_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
    FileMod << "s.t. Prec_21_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";
    FileMod << "s.t. Prec_21_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";

    // # constraints in connecting in different cycles
    FileMod << "s.t. Prec_12_11_c23{e in E,p in P11, k in K: (card(P11)+p+1)=4 and k>1 }:    t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";
    FileMod << "s.t. Prec_22_11_c23{e in E,p in P11, k in K: (card(P11)+p+1+4)=8 and k>1 }:  t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";
    FileMod << "s.t. Prec_12_21_c23{e in E,p in P21, k in K: (card(P21)+p+1-4)=4 and k>1 }:  t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";
    FileMod << "s.t. Prec_22_21_c23{e in E,p in P21, k in K: (card(P21)+p+1)=8 and k>1 }:    t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";

    FileMod << "s.t. PhaseLen{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];\n";
    FileMod << "s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";
    FileMod << "s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";

    FileMod << "s.t. PrioDelay1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; \n";
    FileMod << "s.t. PrioDelay2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n";
    FileMod << "s.t. PrioDelay3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];\n";
    FileMod << "s.t. PrioDelay4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);\n";
    FileMod << "s.t. PrioDelay5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];\n";
    FileMod << "s.t. PrioDelay6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];\n";
    FileMod << "s.t. PrioDelay7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];\n";
    FileMod << "s.t. PrioDelay8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; \n";
    FileMod << "s.t. PrioDelay9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";

    FileMod << "s.t. Flexib: Flex= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];\n ";
    FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) )  - 0.01*Flex; \n "; // The coeficient to Flex should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points

    FileMod << "  minimize delay: PriorityDelay;     \n";
    //=============================Writing the Optimal Output into the Results.txt file ==================================
    FileMod << "  \n";
    FileMod << "solve;  \n";
    FileMod << "  \n";
    FileMod << "printf \" \" > \"Results.txt\";  \n";
    FileMod << "printf \"%3d  %3d \\n \",SP1, SP2 >>\"Results.txt\";  \n";
    FileMod << "printf \"%5.2f  %5.2f %5.2f  %5.2f \\n \",init1, init2,Grn1,Grn2 >>\"Results.txt\";  \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,1] else 0  >>\"Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,2] else 0  >>\"Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"Results.txt\";\n";
    FileMod << " } \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,1] else 0  >>\"Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,2] else 0  >>\"Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "printf \"%3d \\n \", ReqNo >>\"Results.txt\";  \n";
    FileMod << "  \n";
    FileMod << "for {p in P,j in J : Rl[p,j]>0}  \n";
    FileMod << " {  \n";
    FileMod << "   printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1)), Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>\"Results.txt\";\n"; // the  term " coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1))" is used to know the request is served in which cycle. For example, aasume there is a request for phase 4. If the request is served in firsr cycle, the term will be 4, the second cycle, the term will be 14 and the third cycle, the term will be 24
    FileMod << " } \n";

    FileMod << "printf \"%5.2f \\n \", PriorityDelay + 0.01*Flex>>\"Results.txt\"; \n";

    FileMod << "printf \"%5.2f \\n \", Flex >>\"Results.txt\"; \n";
    FileMod << "printf \" \\n \">>\"Results.txt\";\n";
    //------------- End of Print the Main body of mode----------------
    FileMod << "end;\n";
    FileMod.close();
}

int PriorityRequestSolver::getNoOfEVInList()
{
    return noOfEVInList;
}

int PriorityRequestSolver::getRequestedSignalGroupSize()
{
    int signalGroupSize{};
    signalGroupSize = static_cast<int>(requestedSignalGroup.size());
    return signalGroupSize;
}

PriorityRequestSolver::~PriorityRequestSolver()
{
}