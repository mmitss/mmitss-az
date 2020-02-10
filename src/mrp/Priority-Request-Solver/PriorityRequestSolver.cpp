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
#include <jsoncpp/json/json.h>
#include <algorithm>

const int transitWeight = 1;
const int truckWeight = 1;

PriorityRequestSolver::PriorityRequestSolver()
{
}

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

    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        cout << priorityRequestList[i].vehicleID << " " << priorityRequestList[i].basicVehicleRole << " " << priorityRequestList[i].vehicleETA << endl;
    }
}

void PriorityRequestSolver::getCurrentSignalStatus()
{
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

    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        cout << trafficControllerStatus[i].startingPhase1 << " " << trafficControllerStatus[i].initPhase1 << " " << trafficControllerStatus[i].elapsedGreen2 << endl;
    }
}

void PriorityRequestSolver::getRequestedSignalGroupFromPriorityRequestList()
{
    //vector<int>requestedSignalGroup;
    for (size_t i = 0; i < priorityRequestList.size(); i++)
        requestedSignalGroup.push_back(priorityRequestList[i].requestedPhase);
}

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

void PriorityRequestSolver::generateDatFile()
{
    int vehicleClass{}; //to match the old PRSolver
    int numberOfRequest{};
    int ReqSeq = 1;
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
                    fs << priorityRequestList[i].vehicleETA+priorityRequestList[i].vehicleETA_Duration << "\t";
                else
                    fs << ".\t";
            }
            ReqSeq++;
            fs << "\n";
        }
    }

    fs << ";\n";
    fs <<"end;";
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

void PriorityRequestSolver::GLPKSolver()
{
    double startOfSolve{};
    double endOfSolve{};

    char modFile[128] = "NewModel.mod";
    glp_prob *mip;
    glp_tran *tran;
    int ret{};
    int success = 1;
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
}
void PriorityRequestSolver::printSignalPlan()
{
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        cout << trafficSignalPlan[i].phaseNumber << " " << trafficSignalPlan[i].phaseRing << " " << trafficSignalPlan[i].minGreen << endl;
    }
}

// void PriorityRequestSolver::GenerateModFile()
// // =================Defining the sets ======================
// 	if (P11.size() == 1)
// 	{
// 		FileMod << "set P11:={" << P11[0] << "}; \n";
// 		PhaseSeqArray[kk] = P11[0];
// 		kk++;
// 	}
// 	else if (P11.size() == 2)
// 	{
// 		FileMod << "set P11:={" << P11[0] << "," << P11[1] << "};  \n";
// 		PhaseSeqArray[kk] = P11[0];
// 		kk++;
// 		PhaseSeqArray[kk] = P11[1];
// 		kk++;
// 	}

// 	if (P12.size() == 1)
// 	{
// 		FileMod << "set P12:={" << P12[0] << "}; \n";
// 		PhaseSeqArray[kk] = P12[0];
// 		kk++;
// 	}
// 	else if (P12.size() == 2)
// 	{
// 		FileMod << "set P12:={" << P12[0] << "," << P12[1] << "};\n";
// 		PhaseSeqArray[kk] = P12[0];
// 		kk++;
// 		PhaseSeqArray[kk] = P12[1];
// 		kk++;
// 	}

// 	if (P21.size() == 1)
// 	{
// 		FileMod << "set P21:={" << P21[0] << "};\n";
// 		PhaseSeqArray[kk] = P21[0];
// 		kk++;
// 	}
// 	else if (P21.size() == 2)
// 	{
// 		FileMod << "set P21:={" << P21[0] << "," << P21[1] << "};\n";
// 		PhaseSeqArray[kk] = P21[0];
// 		kk++;
// 		PhaseSeqArray[kk] = P21[1];
// 		kk++;
// 	}

// 	if (P22.size() == 1)
// 	{
// 		FileMod << "set P22:={" << P22[0] << "};\n";
// 		PhaseSeqArray[kk] = P22[0];
// 		kk++;
// 	}
// 	else if (P22.size() == 2)
// 	{
// 		FileMod << "set P22:={" << P22[0] << "," << P22[1] << "};\n";
// 		PhaseSeqArray[kk] = P22[0];
// 		kk++;
// 		PhaseSeqArray[kk] = P22[1];
// 		kk++;
// 	}

// 	FileMod << "set P:={";
// 	for (int i = 0; i < kk; i++)
// 	{
// 		if (i != kk - 1)
// 			FileMod << " " << PhaseSeqArray[i] << ",";
// 		else
// 			FileMod << " " << PhaseSeqArray[i];
// 	}
// 	FileMod << "};\n";
// 	FileMod << "set K  := {1..3};\n"; // Only two cycles ahead are considered in the model. But we should count the third cycle in the cycle set. Because, assume we are in the midle of cycle one. Therefore, we have cycle 1, 2 and half of cycle 3.
// 	FileMod << "set J  := {1..10};\n";
// 	FileMod << "set P2 := {1..8};\n";
// 	FileMod << "set T  := {1..10};\n"; // at most 10 different types of vehicle may be considered , EV are 1, Transit are 2, Trucks are 3

// 	// at most 4 rcoordination requests may be considered, 2 phase * 2 cycles ahead. For example, if phases 2 and 6 are coordinated,
// 	// then CP={2,6} and CP1={2} and CP2={6}. The values of Cl1 and Cu1 show the lower and upper bound of the arrival time of coordination request for phase p in CP1
// 	// The values of Cl2 and Cu2 show the lower and upper bound of the arrival time of coordination request for phase p in CP2
// 	FileMod << "set E:={1,2};\n";

// 	FileMod << "set C :={1,2};\n";
// 	FileMod << "set CP:={";
// 	if ((dCoordinationSplit[0] > 0) && (dCoordinationSplit[1] > 0))
// 		FileMod << iCoordinatedPhase[0] << "," << iCoordinatedPhase[1] << "};\n";
// 	else if ((dCoordinationSplit[0] > 0) && (dCoordinationSplit[1] <= 0))
// 		FileMod << iCoordinatedPhase[0] << "};\n";
// 	else if ((dCoordinationSplit[0] <= 0) && (dCoordinationSplit[1] > 0))
// 		FileMod << iCoordinatedPhase[1] << "};\n";
// 	else
// 		FileMod << " };\n";

// 	FileMod << "set CP1:={";
// 	if (dCoordinationSplit[0] > 0)
// 		FileMod << iCoordinatedPhase[0] << "};\n";
// 	else
// 		FileMod << " };\n";

// 	FileMod << "set CP2:={";
// 	if (dCoordinationSplit[1] > 0)
// 		FileMod << iCoordinatedPhase[1] << "};\n";
// 	else
// 		FileMod << " };\n";

// 	if (codeUsage == ADAPTIVE_PRIORITY) // If arrival table is considered ( in Integrated Priority and Adaptive Control) at most 130 vehicles and at most 20 lanes
// 	{
// 		FileMod << "set I  :={1..130};\n";
// 		FileMod << "set L  :={1..20};\n";
// 	}
// 	FileMod << "  \n";
// 	//========================Parameters=========================

// 	FileMod << "param y    {p in P}, >=0,default 0;\n";
// 	FileMod << "param red  {p in P}, >=0,default 0;\n";
// 	FileMod << "param gmin {p in P}, >=0,default 0;\n";
// 	FileMod << "param gmax {p in P}, >=0,default 0;\n";
// 	FileMod << "param init1,default 0;\n";
// 	FileMod << "param init2,default 0;\n";
// 	FileMod << "param Grn1, default 0;\n";
// 	FileMod << "param Grn2, default 0;\n";
// 	FileMod << "param SP1,  integer,default 0;\n";
// 	FileMod << "param SP2,  integer,default 0;\n";
// 	FileMod << "param M:=9999,integer;\n";
// 	FileMod << "param alpha:=100,integer;\n";
// 	FileMod << "param Rl{p in P, j in J}, >=0,  default 0;\n";
// 	FileMod << "param Ru{p in P, j in J}, >=0,  default 0;\n";
// 	FileMod << "param Cl1{p in CP, c in C}, >=0,  default 0;\n";
// 	FileMod << "param Cu1{p in CP, c in C}, >=0,  default 0;\n";
// 	FileMod << "param Cl2{p in CP, c in C}, >=0,  default 0;\n";
// 	FileMod << "param Cu2{p in CP, c in C}, >=0,  default 0;\n";

// 	if (dCoordinationWeight > 0)
// 		FileMod << "param coordinationOn,:= 1;\n";
// 	else
// 		FileMod << "param coordinationOn,:= 0;\n";
// 	FileMod << "param cycle, :=" << dCoordinationCycle << ";\n"; //    # if we have coordination, the cycle length
// 	FileMod << "param active_coord1{p in CP, c in C}, integer, :=(if (Cl1[p,c]>0 and coordinationOn=1) then	1 else	0);\n";
// 	FileMod << "param active_coord2{p in CP, c in C}, integer, :=(if (Cl2[p,c]>0 and coordinationOn=1) then	1 else	0);\n";
// 	FileMod << "param PrioType { t in T}, >=0, default 0;  \n";
// 	FileMod << "param PrioWeigth { t in T}, >=0, default 0;  \n";
// 	FileMod << "param priorityType{j in J}, >=0, default 0;  \n";
// 	FileMod << "param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  \n";
// 	FileMod << "param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);\n";
// 	FileMod << "param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);\n";
// 	FileMod << "param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);\n";
// 	FileMod << "param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);\n";
// 	FileMod << "param ReqNo:=sum{p in P,j in J} active_pj[p,j];\n";
// 	FileMod << "param CoordNo:= sum{p in CP1,c in C} active_coord1[p,c]+sum{p in CP2,c in C} active_coord2[p,c];\n";

// 	// the following parameters added in order to consider case when the max green time in one barrier group expired but not in the other barrier group
// 	FileMod << "param sumOfGMax11, := sum{p in P11} (gmax[p]*coef[p,1]);\n";
// 	FileMod << "param sumOfGMax12, := sum{p in P12} (gmax[p]*coef[p,1]);\n";
// 	FileMod << "param sumOfGMax21, := sum{p in P21} (gmax[p]*coef[p,1]);\n";
// 	FileMod << "param sumOfGMax22, := sum{p in P22} (gmax[p]*coef[p,1]);\n";
// 	FileMod << "param barrier1GmaxSlack, := sumOfGMax11 - sumOfGMax21 ;\n";
// 	FileMod << "param barrier2GmaxSlack, := sumOfGMax12 - sumOfGMax22 ;\n";
// 	FileMod << "param gmaxSlack{p in P}, := (if coef[p,1]=0 then 0 else (if (p in P11) then gmax[p]*max(0,-barrier1GmaxSlack)/sumOfGMax11  else ( if (p in P21) then gmax[p]*max(0,+barrier1GmaxSlack)/sumOfGMax21  else ( if (p in P12) then gmax[p]*max(0,-barrier2GmaxSlack)/sumOfGMax12  else ( if (p in P22) then gmax[p]*max(0,barrier2GmaxSlack)/sumOfGMax22  else 0) ) ) )    ); \n";
// 	FileMod << "param gmaxPerRng{p in P,k in K}, := (if (k=1) then gmax[p]+gmaxSlack[p] else	gmax[p]);\n";

// 	if (codeUsage == ADAPTIVE_PRIORITY) // If arrival table is considered ( in Integrated Priority and Adaptive Control)
// 	{
// 		FileMod << "param s ,>=0,default 0;\n";
// 		FileMod << "param qSp ,>=0,default 0;\n";
// 		FileMod << "param Ar{i in I}, >=0,  default 0;\n";
// 		FileMod << "param Tq{i in I}, >=0,  default 0;\n";
// 		FileMod << "param active_arr{i in I}, integer, :=(if Ar[i]>0 then 1 else 0);\n";
// 		FileMod << "param SumOfActiveArr, := (if (sum{i in I} Ar[i])>0 then (sum{i in I} Ar[i]) else 1);\n";
// 		FileMod << "param Ve{i in I}, >=0,  default 0;\n";
// 		FileMod << "param Ln{i in I}, >=0,  default 0;\n";
// 		FileMod << "param Ph{i in I}, >=0,  default 0;\n";
// 		FileMod << "param L0{l in L}, >=0,  default 0;\n";
// 		FileMod << "param LaPhSt{l in L}, integer;\n";
// 		FileMod << "param Ratio{i in I}, >=0,  default 0;\n";
// 		FileMod << "param indic{i in I},:= (if (Ratio[i]-L0[Ln[i]]/(s-qSp))> 0 then 1 else 0);\n";
// 	}
// 	FileMod << "  \n";
// 	// ==================== VARIABLES =======================
// 	FileMod << "var t{p in P,k in K,e in E}, >=0;\n";
// 	FileMod << "var g{p in P,k in K,e in E}, >=0;\n";
// 	FileMod << "var v{p in P,k in K,e in E}, >=0;\n";
// 	FileMod << "var d{p in P,j in J}, >=0;\n";
// 	FileMod << "var theta{p in P,j in J}, binary;\n";
// 	FileMod << "var ttheta{p in P,j in J}, >=0;\n";
// 	FileMod << "var PriorityDelay;\n";
// 	FileMod << "var Flex;\n";
// 	FileMod << "var zeta1{p in CP1,c in C}, binary;\n";
// 	FileMod << "var zetatl{p in CP1,c in C}, >=0;\n";
// 	FileMod << "var zeta2{p in CP1,c in C}, binary;\n";
// 	FileMod << "var zetatu{p in CP1,c in C}, >=0;\n";
// 	FileMod << "var gamma1{p in CP2,c in C}, binary;\n";
// 	FileMod << "var gammatl{p in CP2,c in C}, >=0;\n";
// 	FileMod << "var gamma2{p in CP2,c in C}, binary;\n";
// 	FileMod << "var gammatu{p in CP2,c in C}, >=0;\n";
// 	FileMod << "var coordDelay1{p in CP1,c in C}, >=0;\n";
// 	FileMod << "var coordDelay2{p in CP2,c in C}, >=0;\n";

// 	if (codeUsage == ADAPTIVE_PRIORITY) // If arrival table is considered ( in Integrated Priority and Adaptive Control)
// 	{
// 		FileMod << "var miu{i in I}, binary; \n";
// 		FileMod << "var rd{i in I}, >=0; \n";
// 		FileMod << "var q{i in I}, >=0; \n";
// 		FileMod << "var qq{i in I}; \n";
// 		FileMod << "var qIndic{i in I}, binary; \n";
// 		FileMod << "var qmiu{i in I}, >=0; \n";
// 		FileMod << "var tmiu{ i in I}, >=0; \n";
// 		FileMod << "var gmiu{ i in I}, >=0; \n";
// 		FileMod << "var del{i in I}, binary; \n";
// 		FileMod << "var TotRegVehDel, >=0; \n";
// 		FileMod << "var delT1{i in I}, >=0; \n";
// 		FileMod << "var delT2{i in I}, >=0; \n";
// 		FileMod << "var ze{i in I}, binary; \n";
// 		FileMod << "var delZe{i in I}, binary; \n";
// 		FileMod << "var delZeT1{i in I}, >=0; \n";
// 		FileMod << "var delZeT2{i in I}, >=0; \n";
// 	}
// 	FileMod << "  \n";

// 	// ===================Constraints==============================

// 	FileMod << "s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  \n";
// 	FileMod << "s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  \n";
// 	FileMod << "s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  \n";
// 	// # constraints in the same cycle in same P??
// 	FileMod << "s.t. Prec_11_11_c1{e in E,p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
// 	FileMod << "s.t. Prec_12_12_c1{e in E,p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
// 	FileMod << "s.t. Prec_21_21_c1{e in E,p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
// 	FileMod << "s.t. Prec_22_22_c1{e in E,p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
// 	// # constraints in the same cycle in connecting
// 	FileMod << "s.t. Prec_11_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
// 	FileMod << "s.t. Prec_11_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
// 	FileMod << "s.t. Prec_21_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
// 	FileMod << "s.t. Prec_21_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
// 	// #================ END of cycle 1======================#

// 	// # constraints in the same cycle in same P??
// 	FileMod << "s.t. Prec_11_11_c23{e in E,p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
// 	FileMod << "s.t. Prec_12_12_c23{e in E,p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
// 	FileMod << "s.t. Prec_21_21_c23{e in E,p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
// 	FileMod << "s.t. Prec_22_22_c23{e in E,p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";

// 	// # constraints in the same cycle in connecting
// 	FileMod << "s.t. Prec_11_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
// 	FileMod << "s.t. Prec_11_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
// 	FileMod << "s.t. Prec_21_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";
// 	FileMod << "s.t. Prec_21_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";

// 	// # constraints in connecting in different cycles
// 	FileMod << "s.t. Prec_12_11_c23{e in E,p in P11, k in K: (card(P11)+p+1)=4 and k>1 }:    t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";
// 	FileMod << "s.t. Prec_22_11_c23{e in E,p in P11, k in K: (card(P11)+p+1+4)=8 and k>1 }:  t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";
// 	FileMod << "s.t. Prec_12_21_c23{e in E,p in P21, k in K: (card(P21)+p+1-4)=4 and k>1 }:  t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";
// 	FileMod << "s.t. Prec_22_21_c23{e in E,p in P21, k in K: (card(P21)+p+1)=8 and k>1 }:    t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";

// 	FileMod << "s.t. PhaseLen{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];\n";
// 	FileMod << "s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";
// 	FileMod << "s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";

// 	FileMod << "s.t. PrioDelay1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; \n";
// 	FileMod << "s.t. PrioDelay2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n";
// 	FileMod << "s.t. PrioDelay3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];\n";
// 	FileMod << "s.t. PrioDelay4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);\n";
// 	FileMod << "s.t. PrioDelay5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];\n";
// 	FileMod << "s.t. PrioDelay6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];\n";
// 	FileMod << "s.t. PrioDelay7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];\n";
// 	FileMod << "s.t. PrioDelay8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; \n";
// 	FileMod << "s.t. PrioDelay9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";
// 	if (dCoordinationWeight > 0)
// 	{
// 		FileMod << "s.t. c0{p in CP1,c in C: active_coord1[p,c]>0}: coordDelay1[p,c] = ( zetatl[p,c] - Cl1[p,c]*zeta1[p,c] ) + (-zetatu[p,c] + Cu1[p,c]*zeta2[p,c]);\n";
// 		FileMod << "s.t. cc0{p in CP2,c in C: active_coord2[p,c]>0}: coordDelay2[p,c] = ( gammatl[p,c] - Cl2[p,c]*gamma1[p,c] ) + (-gammatu[p,c] + Cu2[p,c]*gamma2[p,c]);\n";

// 		FileMod << "s.t. c1{p in CP1: active_coord1[p,1]>0}: M*zeta1[p,1] >= (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) - Cl1[p,1]; \n";
// 		FileMod << "s.t. c2{p in CP1: active_coord1[p,1]>0}: Cl1[p,1] - (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) <= M*(1-zeta1[p,1]);\n";
// 		FileMod << "s.t. c3{p in CP1: active_coord1[p,1]>0}: (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) - M*(1-zeta1[p,1]) <= zetatl[p,1]; \n";
// 		FileMod << "s.t. c4{p in CP1: active_coord1[p,1]>0}: zetatl[p,1] <= (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) + M*(1-zeta1[p,1]) ; \n";
// 		FileMod << "s.t. c5{p in CP1: active_coord1[p,1]>0}: zetatl[p,1] <= M*zeta1[p,1]; \n";

// 		FileMod << "s.t. c6{p in CP1: active_coord1[p,2]>0}: M*zeta1[p,2] >= (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) - Cl1[p,2]; \n";
// 		FileMod << "s.t. c7{p in CP1: active_coord1[p,2]>0}: Cl1[p,2] - (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) <= M*(1-zeta1[p,2]);\n";
// 		FileMod << "s.t. c8{p in CP1: active_coord1[p,2]>0}: (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) - M*(1-zeta1[p,2]) <= zetatl[p,2]; \n";
// 		FileMod << "s.t. c9{p in CP1: active_coord1[p,2]>0}: zetatl[p,2] <= (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) + M*(1-zeta1[p,2]) ; \n";
// 		FileMod << "s.t. c10{p in CP1: active_coord1[p,2]>0}: zetatl[p,2] <= M*zeta1[p,2]; \n";

// 		FileMod << "s.t. c11{p in CP1: active_coord1[p,1]>0}: M*zeta2[p,1] >= Cu1[p,1] - coef[p,1]*(g[p,1,1]+t[p,1,1]) - (1-coef[p,1])*(g[p,2,1]+t[p,2,1]); \n";
// 		FileMod << "s.t. c12{p in CP1: active_coord1[p,1]>0}: coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) - Cu1[p,1] <= M*(1-zeta2[p,1]); \n";
// 		FileMod << "s.t. c13{p in CP1: active_coord1[p,1]>0}: coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) - M*(1-zeta2[p,1]) <= zetatu[p,1]; \n";
// 		FileMod << "s.t. c14{p in CP1: active_coord1[p,1]>0}: zetatu[p,1] <= coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) + M*(1-zeta2[p,1]) ; \n";
// 		FileMod << "s.t. c15{p in CP1: active_coord1[p,1]>0}: zetatu[p,1] <= M*zeta2[p,1]; \n";

// 		FileMod << "s.t. c16{p in CP1: active_coord1[p,2]>0}: M*zeta2[p,2] >= Cu1[p,2]- coef[p,1]*(g[p,2,1]+t[p,2,1]) - (1-coef[p,1])*(g[p,3,1]+t[p,3,1]); \n";
// 		FileMod << "s.t. c17{p in CP1: active_coord1[p,2]>0}: (coef[p,1])*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1])-Cu1[p,2] <= M*(1-zeta2[p,2]); \n";
// 		FileMod << "s.t. c18{p in CP1: active_coord1[p,2]>0}: coef[p,1]*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1]) - M*(1-zeta2[p,2])  <= zetatu[p,2]; \n";
// 		FileMod << "s.t. c19{p in CP1: active_coord1[p,2]>0}: zetatu[p,2] <=  coef[p,1]*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1])+ M*(1-zeta2[p,2]) ;\n";
// 		FileMod << "s.t. c20{p in CP1: active_coord1[p,2]>0}: zetatu[p,2] <= M*zeta2[p,2];\n";

// 		FileMod << "s.t. cc1{p in CP2: active_coord2[p,1]>0}: M*gamma1[p,1] >= (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) - Cl2[p,1];\n";
// 		FileMod << "s.t. cc2{p in CP2: active_coord2[p,1]>0}: Cl2[p,1] - (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) <= M*(1-gamma1[p,1]);\n";
// 		FileMod << "s.t. cc3{p in CP2: active_coord2[p,1]>0}: (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) - M*(1-gamma1[p,1]) <= gammatl[p,1]; \n";
// 		FileMod << "s.t. cc4{p in CP2: active_coord2[p,1]>0}: gammatl[p,1] <= (coef[p,1]*t[p,1,1]+(1-coef[p,1])*t[p,2,1]) + M*(1-gamma1[p,1]);\n";
// 		FileMod << "s.t. cc5{p in CP2: active_coord2[p,1]>0}: gammatl[p,1] <= M*gamma1[p,1]; \n";

// 		FileMod << "s.t. cc6{p in CP2: active_coord2[p,2]>0}: M*gamma1[p,2] >= (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) - Cl2[p,2];\n";
// 		FileMod << "s.t. cc7{p in CP2: active_coord2[p,2]>0}: Cl2[p,2] - (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) <= M*(1-gamma1[p,2]);\n";
// 		FileMod << "s.t. cc8{p in CP2: active_coord2[p,2]>0}: (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) - M*(1-gamma1[p,2]) <= gammatl[p,2]; \n";
// 		FileMod << "s.t. cc9{p in CP2: active_coord2[p,2]>0}: gammatl[p,2] <= (coef[p,1]*t[p,2,1]+(1-coef[p,1])*t[p,3,1]) + M*(1-gamma1[p,2]);\n";
// 		FileMod << "s.t. cc10{p in CP2: active_coord2[p,2]>0}: gammatl[p,2] <= M*gamma1[p,2]; \n";

// 		FileMod << "s.t. cc11{p in CP2: active_coord2[p,1]>0}: M*gamma2[p,1] >= Cu2[p,1]-coef[p,1]*(g[p,1,1]+t[p,1,1]) - (1-coef[p,1])*(g[p,2,1]+t[p,2,1]); \n";
// 		FileMod << "s.t. cc12{p in CP2: active_coord2[p,1]>0}: coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) - Cu2[p,1] <= M*(1-gamma2[p,1]); \n";
// 		FileMod << "s.t. cc13{p in CP2: active_coord2[p,1]>0}: coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) - M*(1-gamma2[p,1]) <= gammatu[p,1]; \n";
// 		FileMod << "s.t. cc14{p in CP2: active_coord2[p,1]>0}: gammatu[p,1] <= coef[p,1]*(g[p,1,1]+t[p,1,1]) + (1-coef[p,1])*(g[p,2,1]+t[p,2,1]) + M*(1-gamma2[p,1]) ;\n";
// 		FileMod << "s.t. cc15{p in CP2: active_coord2[p,1]>0}: gammatu[p,1] <= M*gamma2[p,1]; \n";

// 		FileMod << "s.t. cc16{p in CP2: active_coord2[p,2]>0}: M*gamma2[p,2] >= Cu2[p,2]-coef[p,1]*(g[p,2,1]+t[p,2,1]) - (1-coef[p,1])*(g[p,3,1]+t[p,3,1]); \n";
// 		FileMod << "s.t. cc17{p in CP2: active_coord2[p,2]>0}: (coef[p,1])*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1])-Cu2[p,2] <= M*(1-gamma2[p,2]);\n";
// 		FileMod << "s.t. cc18{p in CP2: active_coord2[p,2]>0}: coef[p,1]*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1]) - M*(1-gamma2[p,2])  <= gammatu[p,2]; \n";
// 		FileMod << "s.t. cc19{p in CP2: active_coord2[p,2]>0}: gammatu[p,2] <=  coef[p,1]*(g[p,2,1]+t[p,2,1]) + (1-coef[p,1])*(g[p,3,1]+t[p,3,1])+ M*(1-gamma2[p,2]) ;\n";
// 		FileMod << "s.t. cc20{p in CP2: active_coord2[p,2]>0}: gammatu[p,2] <= M*gamma2[p,2]; \n";
// 	}

// 	if (codeUsage == ADAPTIVE_PRIORITY)
// 	{
// 		FileMod << "s.t. RVehD1{i in I: (active_arr[i]>0)}: rd[i] >= t[Ph[i],1]+q[i]/s-(Ar[i]-q[i])/Ve[i]+Tq[i];  \n ";
// 		FileMod << "s.t. RVehD2{i in I: (active_arr[i]>0)} : M*miu[i] >= q[i]/s + Ar[i]/Ve[i] -g[Ph[i],1];  \n ";
// 		FileMod << "s.t. RVehD3{i in I: (active_arr[i]>0 )} : rd[i] >= tmiu[i] + qmiu[i]/s - gmiu[i] - Ar[i]*miu[i]/Ve[i] + qmiu[i]/Ve[i] + Tq[i]; \n ";
// 		FileMod << "s.t. RVehD5{i in I,ii in I: (active_arr[i]>0 and active_arr[ii]>0  and Ln[i]=Ln[ii] and Ar[i]<Ar[ii])} : miu[i] <= miu[ii] ; \n ";
// 		FileMod << "s.t. RVehD6{i in I: (active_arr[i]>0)}: tmiu[i]<=M*miu[i];   \n ";
// 		FileMod << "s.t. RVehD7{p in P,i in I: (active_arr[i]>0 and p=Ph[i])}: t[p,2]-M*(1-miu[i])<=tmiu[i];   \n ";
// 		FileMod << "s.t. RVehD8{p in P,i in I: (active_arr[i]>0 and p=Ph[i])}: t[p,2]+M*(1-miu[i])>=tmiu[i]; \n ";
// 		FileMod << "s.t. RVehD9{i in I: active_arr[i]>0 }: gmiu[i]<=M*miu[i];   \n ";
// 		FileMod << "s.t. RVehD10{p in P,i in I: (active_arr[i]>0 and p=Ph[i])}: g[p,1]-M*(1-miu[i])<=gmiu[i];   \n ";
// 		FileMod << "s.t. RVehD11{p in P,i in I: (active_arr[i]>0 and p=Ph[i])}: g[p,1]+M*(1-miu[i])>=gmiu[i]; \n ";
// 		FileMod << "s.t. RVehD12{i in I: (active_arr[i]>0)}: qmiu[i]<=M*miu[i];   \n ";
// 		FileMod << "s.t. RVehD13{i in I: (active_arr[i]>0)}: q[i]-M*(1-miu[i])<=qmiu[i];   \n ";
// 		FileMod << "s.t. RVehD14{i in I: (active_arr[i]>0)}: q[i]+M*(1-miu[i])>=qmiu[i]; \n ";
// 		FileMod << "s.t. RVehD15{i in I,k in K: (active_arr[i]>0)}: qq[i]=((LaPhSt[Ln[i]])*(L0[Ln[i]]+Ratio[i]*qSp-g[Ph[i],1]*s)*indic[i])+((1-LaPhSt[Ln[i]])*(L0[Ln[i]]*del[i]+Ratio[i]*qSp*del[i]-s*Ratio[i]*del[i]+s*((1-coef[Ph[i],1])*delT2[i]+coef[Ph[i],1]*delT1[i])-s*Ratio[i]*delZe[i]-s*((1-coef[Ph[i],1])*delZeT2[i]+coef[Ph[i],1]*delZeT1[i]))); \n ";
// 		FileMod << "s.t. RVehD16{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*(t[Ph[i],1]-M*(1-del[i]))<=coef[Ph[i],1]*delT1[i];  \n ";
// 		FileMod << "s.t. RVehD17{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*(t[Ph[i],1]+M*(1-del[i]))>=coef[Ph[i],1]*delT1[i]; \n ";
// 		FileMod << "s.t. RVehD18{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*delT1[i]<=M*del[i]; \n ";
// 		FileMod << "s.t. RVehD19{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*t[Ph[i],2]-M*(1-del[i])<=(1-coef[Ph[i],1])*delT2[i];  \n ";
// 		FileMod << "s.t. RVehD20{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*t[Ph[i],2]+M*(1-del[i])>=(1-coef[Ph[i],1])*delT2[i];  \n ";
// 		FileMod << "s.t. RVehD21{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*delT2[i]<=M*del[i];  \n ";
// 		FileMod << "s.t. RVehD22{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*(t[Ph[i],1]-M*(1-delZe[i]))<=delZeT1[i];  \n ";
// 		FileMod << "s.t. RVehD23{i in I: (active_arr[i]>0)}: t[Ph[i],1]+M*(1-delZe[i])>=coef[Ph[i],1]*delZeT1[i];  \n ";
// 		FileMod << "s.t. RVehD24{i in I: (active_arr[i]>0)}: coef[Ph[i],1]*delZeT1[i]<=M*delZe[i];  \n ";
// 		FileMod << "s.t. RVehD25{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*(t[Ph[i],2]-M*(1-delZe[i]))<=delZeT2[i];  \n ";
// 		FileMod << "s.t. RVehD26{i in I: (active_arr[i]>0)}: t[Ph[i],2]+M*(1-delZe[i])>=(1-coef[Ph[i],1])*delZeT2[i];  \n ";
// 		FileMod << "s.t. RVehD27{i in I: (active_arr[i]>0)}: (1-coef[Ph[i],1])*delZeT2[i]<=M*delZe[i];  \n ";
// 		FileMod << "s.t. RVehD28{i in I: (active_arr[i]>0)}: delZe[i]<=del[i];  \n ";
// 		FileMod << "s.t. RVehD29{i in I: (active_arr[i]>0)}: delZe[i]<=ze[i];  \n ";
// 		FileMod << "s.t. RVehD30{i in I: (active_arr[i]>0)}: delZe[i]>=del[i]+ze[i]-1;  \n ";
// 		FileMod << "s.t. RVehD31{i in I: (active_arr[i]>0)}: ((1-coef[Ph[i],1])*t[Ph[i],2] + coef[Ph[i],1]*t[Ph[i],1] - Ratio[i])<=M*ze[i];  \n ";
// 		FileMod << "s.t. RVehD32{i in I: (active_arr[i]>0)}: ((1-coef[Ph[i],1])*t[Ph[i],2] + coef[Ph[i],1]*t[Ph[i],1] - Ratio[i])>=M*(ze[i]-1);  \n ";
// 		FileMod << "s.t. RVehD33{i in I: (active_arr[i]>0)}: s*(((1-coef[Ph[i],1])*t[Ph[i],2] + coef[Ph[i],1]*t[Ph[i],1]) + L0[Ln[i]])/(s-qSp)- Ratio[i]<=M*del[i];  \n ";
// 		FileMod << "s.t. RVehD34{i in I: (active_arr[i]>0)}: s*(((1-coef[Ph[i],1])*t[Ph[i],2] + coef[Ph[i],1]*t[Ph[i],1]) + L0[Ln[i]])/(s-qSp)- Ratio[i]>=M*(del[i]-1);  \n ";
// 		FileMod << "s.t. RVehD35{i in I: (active_arr[i]>0)}: qq[i]<=M*qIndic[i];  \n ";
// 		FileMod << "s.t. RVehD36{i in I: (active_arr[i]>0)}: qq[i]>=M*(qIndic[i]-1);  \n ";
// 		FileMod << "s.t. RVehD37{i in I: (active_arr[i]>0)}: qq[i]-M*(1-qIndic[i])<=q[i];  \n ";
// 		FileMod << "s.t. RVehD38{i in I: (active_arr[i]>0)}: qq[i]+M*(1-qIndic[i])>=q[i];  \n ";
// 		FileMod << "s.t. RVehD39{i in I: (active_arr[i]>0)}: q[i]<=M*qIndic[i];  \n ";
// 		FileMod << "s.t. TotRegVehDelay: TotRegVehDel=(sum{i in I} active_arr[i]*rd[i])/SumOfActiveArr;  \n";
// 	}

// 	FileMod << "s.t. Flexib: Flex= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];\n ";
// 	if (haveEVinList != 1)
// 		FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) )  + PrioWeigth[6]*(sum{p in CP1,c in C: active_coord1[p,c]>0} coordDelay1[p,c] + sum{p in CP2,c in C: active_coord2[p,c]>0} coordDelay2[p,c])  - 0.01*Flex; \n "; // The coeficient to Flex should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points
// 	if (haveEVinList == 1)																																																																	  // incase there is EV in the list, there is not need to provide felexibility for signal actuation and also no need for considering coorination
// 		FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) ) ; \n ";

// 	if (codeUsage != ADAPTIVE_PRIORITY) // in case we do not consider egular vehicles (no arrival table)
// 		FileMod << "minimize delay: PriorityDelay  ;\n";
// 	else
// 		FileMod << "  minimize delay:  TotRegVehDel+  PriorityDelay;     \n";
// 	//=============================Writing the Optimal Output into the Results.txt file ==================================
// 	FileMod << "  \n";
// 	FileMod << "solve;  \n";
// 	FileMod << "  \n";
// 	FileMod << "printf \" \" > \"/nojournal/bin/Results.txt\";  \n";
// 	FileMod << "printf \"%3d  %3d \\n \",SP1, SP2 >>\"/nojournal/bin/Results.txt\";  \n";
// 	FileMod << "printf \"%5.2f  %5.2f %5.2f  %5.2f \\n \",init1, init2,Grn1,Grn2 >>\"/nojournal/bin/Results.txt\";  \n";
// 	FileMod << "for {k in K}   \n";
// 	FileMod << " { \n";
// 	FileMod << "     for {p in P2} \n";
// 	FileMod << "        { \n";
// 	FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,1] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
// 	FileMod << "        } \n";
// 	FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
// 	FileMod << " } \n";
// 	FileMod << "  \n";
// 	FileMod << "for {k in K}   \n";
// 	FileMod << " { \n";
// 	FileMod << "     for {p in P2} \n";
// 	FileMod << "        { \n";
// 	FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,2] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
// 	FileMod << "        } \n";
// 	FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
// 	FileMod << " } \n";
// 	FileMod << "for {k in K}   \n";
// 	FileMod << " { \n";
// 	FileMod << "     for {p in P2} \n";
// 	FileMod << "        { \n";
// 	FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,1] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
// 	FileMod << "        } \n";
// 	FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
// 	FileMod << " } \n";
// 	FileMod << "  \n";
// 	FileMod << "for {k in K}   \n";
// 	FileMod << " { \n";
// 	FileMod << "     for {p in P2} \n";
// 	FileMod << "        { \n";
// 	FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,2] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
// 	FileMod << "        } \n";
// 	FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
// 	FileMod << " } \n";
// 	FileMod << "  \n";
// 	FileMod << "printf \"%3d \\n \", ReqNo >>\"/nojournal/bin/Results.txt\";  \n";
// 	FileMod << "  \n";
// 	FileMod << "for {p in P,j in J : Rl[p,j]>0}  \n";
// 	FileMod << " {  \n";
// 	FileMod << "   printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1)), Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>\"/nojournal/bin/Results.txt\";\n"; // the  term " coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1))" is used to know the request is served in which cycle. For example, aasume there is a request for phase 4. If the request is served in firsr cycle, the term will be 4, the second cycle, the term will be 14 and the third cycle, the term will be 24
// 	FileMod << " } \n";
// 	FileMod << "printf \"%3d \\n \", CoordNo >>\"/nojournal/bin/Results.txt\";  \n";
// 	if (dCoordinationWeight > 0)
// 	{
// 		if (dCoordinationSplit[0] > 0.0)
// 		{
// 			FileMod << "for {c in C,p in CP: active_coord1[p,c]>0} \n";
// 			FileMod << "        { \n";
// 			FileMod << "		printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", p, Cl1[p,c],Cu1[p,c], coordDelay1[p,c] , 6 >>\"/nojournal/bin/Results.txt\";  \n";
// 			FileMod << "        }\n";
// 		}
// 		if (dCoordinationSplit[1] > 0.0)
// 		{
// 			FileMod << "for {c in C,p in CP: active_coord2[p,c]>0} "
// 					<< "\n";
// 			FileMod << "        { \n";
// 			FileMod << "		printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", p, Cl2[p,c],Cu2[p,c], coordDelay2[p,c] , 6 >>\"/nojournal/bin/Results.txt\";  \n";
// 			FileMod << "         }\n";
// 		}
// 	}

// 	FileMod << "printf \"%5.2f \\n \", PriorityDelay + 0.01*Flex>>\"/nojournal/bin/Results.txt\"; \n";
// 	if (codeUsage == ADAPTIVE_PRIORITY) // in case we do not consider egular vehicles (no arrival table)
// 		FileMod << "printf \"%5.2f \\n \", TotRegVehDel >>\"/nojournal/bin/Results.txt\";  	\n ";
// 	FileMod << "printf \"%5.2f \\n \", sum{p in CP1,c in C: active_coord1[p,c]>0} coordDelay1[p,c] + sum{p in CP2,c in C: active_coord2[p,c]>0} coordDelay2[p,c]  >>\"/nojournal/bin/Results.txt\"; \n";
// 	FileMod << "printf \"%5.2f \\n \", Flex >>\"/nojournal/bin/Results.txt\"; \n";
// 	FileMod << "printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
// 	//------------- End of Print the Main body of mode----------------
// 	FileMod << "end;\n";
// 	FileMod.close();
// }

PriorityRequestSolver::~PriorityRequestSolver()
{
}