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

void PriorityRequestSolver::readOptimalPlan()
{
    ifstream infile;
    string lineread{};
    int tempLine{};
    vector<int> SP;
    vector<int> init;
    vector<int> elapsedGrn;

    infile.open("Results.txt");
    // getline(infile, lineread);

    if (infile.fail())
        std::cout << "Fail to open file" << std::endl;

    else
    {
        for (std::string line; getline(infile, line);)
        {
            stringstream strToSplit(line.c_str());
            while (strToSplit >> tempLine)
                SP.push_back(tempLine);
        }
        // getline(infile, lineread);
        // stringstream strToSplit(lineread.c_str());
        // while(strToSplit >> tempLine)
        //     SP.push_back(tempLine);
        // split(lineread.c_str());
    }
}

void PriorityRequestSolver::split(string strToSplit)
{
    std::stringstream in(strToSplit);
    // vector<int> a;
    int temp;
    while (in >> temp)
    {
        a.push_back(temp);
    }
}

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
}
void PriorityRequestSolver::printSignalPlan()
{
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        cout << trafficSignalPlan[i].phaseNumber << " " << trafficSignalPlan[i].phaseRing << " " << trafficSignalPlan[i].minGreen << endl;
    }
}

void PriorityRequestSolver::GenerateModFile()
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

PriorityRequestSolver::~PriorityRequestSolver()
{
}