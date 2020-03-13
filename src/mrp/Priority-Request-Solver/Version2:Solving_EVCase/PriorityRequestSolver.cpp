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
#include <fstream>
#include <stdio.h>
#include <sys/time.h>
#include "json/json.h"
#include <algorithm>
#include <cmath>

// const int transitWeight = 1;
// const int truckWeight = 1;
// const double MAXGREEN = 50.0;

// #define OMIT_VEH_PHASES 2
// #define OMIT_PED_PHASES 3
// #define HOLD_PHASES 4
// #define FORCEOFF_PHASES 5
// #define CALL_VEH_PHASES 6
// #define CALL_PED_PHASES 7

PriorityRequestSolver::PriorityRequestSolver()
{
}

/*
	- Method for identifying the message type.
*/
int PriorityRequestSolver::getMessageType(string jsonString)
{
    int messageType{};
    Json::Value jsonObject;
    Json::Reader reader;
    reader.parse(jsonString.c_str(), jsonObject);

    if ((jsonObject["MsgType"]).asString() == "PriorityRequest")
    {
        messageType = static_cast<int>(msgType::priorityRequest);
    }

    else if ((jsonObject["MsgType"]).asString() == "ClearRequest")
    {
        messageType = static_cast<int>(msgType::clearRequest);
    }

    else
        std::cout << "Message type is unknown" << std::endl;

    return messageType;
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

    // setPhaseCallForRequestedSignalGroup();
    //This is optional. For priniting few attributes of the priority request list in the console
    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        cout << priorityRequestList[i].vehicleID << " " << priorityRequestList[i].basicVehicleRole << " " << priorityRequestList[i].vehicleETA << endl;
    }
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

void PriorityRequestSolver::setOptimizationInput()
{

    if (bEVStatus == true)
    {
        modifyPriorityRequestList();
        // SolverDataManager solverDataManager;
        // requestedSignalGroup.clear();
        // requestedSignalGroup = solverDataManager.getRequestedSignalGroupFromPriorityRequestList();
        getRequestedSignalGroup();
        getEVPhases();
        getEVTrafficSignalPlan();
        generateEVModFile();
        SolverDataManager solverDataManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan_EV);
        solverDataManager.generateDatFile(bEVStatus);
    }

    else
    {
        SolverDataManager solverDataManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan);
        solverDataManager.getRequestedSignalGroupFromPriorityRequestList();
        // solverDataManager.removeDuplicateSignalGroup();
        solverDataManager.addAssociatedSignalGroup();
        solverDataManager.modifyGreenMax();
        solverDataManager.generateDatFile(bEVStatus);
    }
}

void PriorityRequestSolver::getRequestedSignalGroup()
{
    SolverDataManager solverDataManager(priorityRequestList);
    requestedSignalGroup.clear();
    requestedSignalGroup = solverDataManager.getRequestedSignalGroupFromPriorityRequestList();
}
/*
    - Method of solving the request in the priority request list  based on mod and dat files
    - Solution will be written in the Results.txt file
*/
// void PriorityRequestSolver::GLPKSolver(SolverDataManager solverDataManager)
void PriorityRequestSolver::GLPKSolver()
{
    double startOfSolve{};
    double endOfSolve{};
    // SolverDataManager solverDataManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan);

    // solverDataManager.generateDatFile(priorityRequestList,trafficControllerStatus,trafficSignalPlan);
    // solverDataManager.generateDatFile();
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

/*
    - 
*/
// string PriorityRequestSolver::getScheduleforTCI(ScheduleManager scheduleManager)
// {
//     string scheduleJsonString{};
//     scheduleManager.obtainRequiredSignalGroup(trafficControllerStatus, trafficSignalPlan);
//     scheduleManager.readOptimalSignalPlan();
//     scheduleManager.createEventList(priorityRequestList, trafficSignalPlan);
//     scheduleJsonString = scheduleManager.createScheduleJsonString();

//     priorityRequestList.clear();
//     trafficControllerStatus.clear();
//     return scheduleJsonString;
// }

string PriorityRequestSolver::getScheduleforTCI()
{
    string scheduleJsonString{};
    if (bEVStatus == true)
    {
        ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan_EV);

        scheduleManager.obtainRequiredSignalGroup();
        scheduleManager.readOptimalSignalPlan();
        scheduleManager.createEventList();
        scheduleJsonString = scheduleManager.createScheduleJsonString();
    }

    else
    {
        ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan);

        scheduleManager.obtainRequiredSignalGroup();
        scheduleManager.readOptimalSignalPlan();
        scheduleManager.createEventList();
        scheduleJsonString = scheduleManager.createScheduleJsonString();
    }

    priorityRequestList.clear();
    trafficControllerStatus.clear();
    return scheduleJsonString;
}

/*
    - If here is EV in priority request list, Dat file will have information about EV requested phases and current phases.
*/
void PriorityRequestSolver::getEVPhases()
{
    int tempSignalGroup{};
    vector<int>::iterator it;
    plannedEVPhases.clear();
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
    - If new priority request is received this method will obtain the current traffic signal Status.
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
    - Method for obtaining static traffic signal plan from TCI
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
    - Method printing the traffic signal plan
*/
void PriorityRequestSolver::printSignalPlan()
{
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        cout << trafficSignalPlan[i].phaseNumber << " " << trafficSignalPlan[i].phaseRing << " " << trafficSignalPlan[i].minGreen << endl;
    }
}

/*
    - Method for generating Mod File for transit and truck
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

double PriorityRequestSolver::GetSeconds()
{
    struct timeval tv_tt;
    gettimeofday(&tv_tt, NULL);
    return (static_cast<double>(tv_tt.tv_sec) + static_cast<double>(tv_tt.tv_usec) / 1.e6);
}

PriorityRequestSolver::~PriorityRequestSolver()
{
}