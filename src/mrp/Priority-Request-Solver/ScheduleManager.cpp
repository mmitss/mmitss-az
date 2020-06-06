#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "json/json.h"
#include "ScheduleManager.h"

#define OMIT_VEH_PHASES 5
#define OMIT_PED_PHASES 6
#define HOLD_PHASES 4
#define FORCEOFF_PHASES 3
#define CALL_VEH_PHASES 1
#define CALL_PED_PHASES 2

ScheduleManager::ScheduleManager()
{
}

ScheduleManager::ScheduleManager(vector<RequestList> requestList, vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus, vector<TrafficControllerData::TrafficSignalPlan> signalPlan, bool EVStatus)
{
    if (!requestList.empty())
        priorityRequestList = requestList;

    if (!signalStatus.empty())
        trafficControllerStatus = signalStatus;

    if (!signalPlan.empty())
        trafficSignalPlan = signalPlan;

    bEVStatus = EVStatus;
}

//void ScheduleManager::obtainRequiredSignalGroup(vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan)
void ScheduleManager::obtainRequiredSignalGroup()
{
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        plannedSignalGroupInRing1.push_back(trafficControllerStatus[i].startingPhase1); //Storing the SP1
        plannedSignalGroupInRing2.push_back(trafficControllerStatus[i].startingPhase2); //Storing the SP2
    }

    for (size_t i = 0; i < trafficSignalPlan.size(); i++) //Obtaining all the required phases for cycle1, ring1
    {
        if (trafficSignalPlan[i].phaseNumber > plannedSignalGroupInRing1.front() && trafficSignalPlan[i].phaseRing == 1)
            plannedSignalGroupInRing1.push_back(trafficSignalPlan[i].phaseNumber);
    }

    for (size_t i = 0; i < trafficSignalPlan.size(); i++) //Obtaining all the required phases for cycle2, ring1
    {
        if (trafficSignalPlan[i].phaseNumber < 5 && trafficSignalPlan[i].phaseRing == 1)
            plannedSignalGroupInRing1.push_back(trafficSignalPlan[i].phaseNumber);
    }

    for (size_t i = 0; i < trafficSignalPlan.size(); i++) //Obtaining all the required phases for cycle3, ring1
    {
        if (trafficSignalPlan[i].phaseNumber < plannedSignalGroupInRing1.front() && trafficSignalPlan[i].phaseRing == 1)
            plannedSignalGroupInRing1.push_back(trafficSignalPlan[i].phaseNumber);
    }

    for (size_t i = 0; i < trafficSignalPlan.size(); i++) //Obtaining all the required phases for cycle1, ring2
    {
        if (trafficSignalPlan[i].phaseNumber > plannedSignalGroupInRing2.front() && trafficSignalPlan[i].phaseRing == 2)
            plannedSignalGroupInRing2.push_back(trafficSignalPlan[i].phaseNumber);
    }

    for (size_t i = 0; i < trafficSignalPlan.size(); i++) //Obtaining all the required phases for cycle2, ring2
    {
        if (trafficSignalPlan[i].phaseNumber < 9 && trafficSignalPlan[i].phaseRing == 2)
            plannedSignalGroupInRing2.push_back(trafficSignalPlan[i].phaseNumber);
    }

    for (size_t i = 0; i < trafficSignalPlan.size(); i++) //Obtaining all the required phases for cycle3, ring2
    {
        if (trafficSignalPlan[i].phaseNumber < plannedSignalGroupInRing2.front() && trafficSignalPlan[i].phaseRing == 2)
            plannedSignalGroupInRing2.push_back(trafficSignalPlan[i].phaseNumber);
    }
}

void ScheduleManager::getOmitPhases()
{
    omitPhases.clear();
    int temporaryPhase{};

    for (int i = 1; i < 9; i++)
    {
        temporaryPhase = i;
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

        if (findSignalGroupOnList == trafficSignalPlan.end())
            omitPhases.push_back(temporaryPhase);
    }
}

void ScheduleManager::readOptimalSignalPlan()
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

            else if (lineNo > 7 && lineNo < 11)
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
    - We have 24 phase duration (3cycles). 
    - Holding ring wise total phase duration for each phase.
    - To store the data ring wise we have to skip the phases of opposite ring.
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
    /*
        - If starting phase is already served for min green time, green time for the starting phase may be zero(based on the solution)
        - In this case size of vector containing green time (like- leftCriticalPoints_GreenTime_Ring1) may not equal to the size of vector containing planned signal group (like- plannedSignalGroupInRing1)
        - Then a value 0.0 will insert at the beginning to match the size of the vectors.
    */

    if (leftCriticalPoints_GreenTime_Ring1.size() != plannedSignalGroupInRing1.size())
        leftCriticalPoints_GreenTime_Ring1.insert(leftCriticalPoints_GreenTime_Ring1.begin(), 0.01);

    if (rightCriticalPoints_GreenTime_Ring1.size() != plannedSignalGroupInRing1.size())
        rightCriticalPoints_GreenTime_Ring1.insert(rightCriticalPoints_GreenTime_Ring1.begin(), 0.01);

    if (leftCriticalPoints_GreenTime_Ring2.size() != plannedSignalGroupInRing2.size())
        leftCriticalPoints_GreenTime_Ring2.insert(leftCriticalPoints_GreenTime_Ring2.begin(), 0.01);

    if (rightCriticalPoints_GreenTime_Ring2.size() != plannedSignalGroupInRing2.size())
        rightCriticalPoints_GreenTime_Ring2.insert(rightCriticalPoints_GreenTime_Ring2.begin(), 0.01);
}

/*
    - Following Method will compute ring wise cummilative sum of phase duration for left and right cricital points
*/
// void ScheduleManager::createEventList(vector<RequestList> priorityRequestList, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan)
void ScheduleManager::createEventList()
{
    Schedule ring1Schedule;
    Schedule ring2Schedule;
    int vehicleSignalGroup{};
    int vehicleSignalGroupRing{};
    int temporaryPhase{};
    vector<int>::iterator it;

    ring1_TCISchedule.clear();
    ring2_TCISchedule.clear();

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

    if(trafficControllerStatus[0].initPhase1>0)
    {
        for (size_t i = 0; i < leftCriticalPoints_PhaseDuration_Ring1.size(); i++)
            leftCriticalPoints_PhaseDuration_Ring1[i] = leftCriticalPoints_PhaseDuration_Ring1[i]+trafficControllerStatus[0].initPhase1;
    
        for (size_t i = 0; i < rightCriticalPoints_PhaseDuration_Ring1.size(); i++)
            rightCriticalPoints_PhaseDuration_Ring1[i] = rightCriticalPoints_PhaseDuration_Ring1[i]+trafficControllerStatus[0].initPhase1;
    }

    if(trafficControllerStatus[0].initPhase2>0)
    {
        for (size_t i = 0; i < leftCriticalPoints_PhaseDuration_Ring2.size(); i++)
            leftCriticalPoints_PhaseDuration_Ring2[i] = leftCriticalPoints_PhaseDuration_Ring2[i]+trafficControllerStatus[0].initPhase2;
    
        for (size_t i = 0; i < rightCriticalPoints_PhaseDuration_Ring2.size(); i++)
            rightCriticalPoints_PhaseDuration_Ring2[i] = rightCriticalPoints_PhaseDuration_Ring2[i]+trafficControllerStatus[0].initPhase2;
    }

    /*Hold Ring1 */
    for (size_t i = 0; i < plannedSignalGroupInRing1.size(); i++)
    {

        ring1Schedule.commandPhase = plannedSignalGroupInRing1[i];
        ring1Schedule.commandType = "hold";

        /*
            -Hold upto the sum of green time and intPhase time for starting phase
        */
        if (i == 0)
        {
            ring1Schedule.commandStartTime = 0.0 + trafficControllerStatus[0].initPhase1;
            ring1Schedule.commandEndTime = leftCriticalPoints_GreenTime_Ring1[i] + trafficControllerStatus[0].initPhase1;
        }
        /*
            - For rest of the phase hold will start at the end of previous phase
            - Hold will continue upto the end of the green time for the corresponding phase
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
        ring2Schedule.commandType = "hold";

        /*
            -Hold upto the sum of green time and initPhase1 for starting phase
        */
        if (i == 0)
        {
            ring2Schedule.commandStartTime = 0.0 + trafficControllerStatus[0].initPhase2;
            ring2Schedule.commandEndTime = leftCriticalPoints_GreenTime_Ring2[i] + trafficControllerStatus[0].initPhase2;
        }

        /*
            - For rest of the phase hold will start at the end of previous phase
            - Hold will continue upto the end of the green time for the corresponding phase
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
        ring1Schedule.commandType = "forceoff";

        /*
            -Force off for starting phase (if first phase having green time greater than zero) is sum of green time and initPhase1
            -End time for force off will be zero
        */
        if (i == 0)
            ring1Schedule.commandStartTime = rightCriticalPoints_GreenTime_Ring1[i] + trafficControllerStatus[0].initPhase1;

        /*
            - For rest of the phase force off at the end of green time of corresponding phase
            - Logic is: Cumulative Phase duration of the previous phase plus green time of the current phase
            - End time for force off will be zero
        */
        else
            ring1Schedule.commandStartTime = rightCriticalPoints_PhaseDuration_Ring1[i - 1] + rightCriticalPoints_GreenTime_Ring1[i];

        ring1Schedule.commandEndTime = ring1Schedule.commandStartTime + 1.0;

        ring1_TCISchedule.push_back(ring1Schedule);
    }

    /*ForceOff Ring2 */
    for (size_t i = 0; i < plannedSignalGroupInRing2.size(); i++)
    {
        ring2Schedule.commandPhase = plannedSignalGroupInRing2[i];
        ring2Schedule.commandType = "forceoff";

        if (i == 0)
            ring2Schedule.commandStartTime = rightCriticalPoints_GreenTime_Ring2[i] + trafficControllerStatus[0].initPhase2;

        else
            ring2Schedule.commandStartTime = rightCriticalPoints_PhaseDuration_Ring2[i - 1] + rightCriticalPoints_GreenTime_Ring2[i];

        ring2Schedule.commandEndTime = ring2Schedule.commandStartTime + 1.0;

        ring2_TCISchedule.push_back(ring2Schedule);
    }

    /*vehicle Call */
    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        vehicleSignalGroup = priorityRequestList[i].requestedPhase;

        for (size_t j = 0; j < trafficSignalPlan.size(); j++)
        {
            if (trafficSignalPlan[j].phaseNumber == vehicleSignalGroup)
                vehicleSignalGroupRing = trafficSignalPlan[j].phaseRing;
        }

        if (vehicleSignalGroupRing == 1)
        {
            ring1Schedule.commandPhase = vehicleSignalGroup;
            ring1Schedule.commandType = "call_veh";
            ring1Schedule.commandStartTime = 0.0;
            ring1Schedule.commandEndTime = priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration;

            ring1_TCISchedule.push_back(ring1Schedule);
        }

        else if (vehicleSignalGroupRing == 2)
        {
            ring2Schedule.commandPhase = vehicleSignalGroup;
            ring2Schedule.commandType = "call_veh";
            ring2Schedule.commandStartTime = 0.0;
            ring2Schedule.commandEndTime = priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration;

            ring2_TCISchedule.push_back(ring2Schedule);
        }
    }

    if (bEVStatus == true)
    {
        getOmitPhases();

        for (size_t i = 0; i < omitPhases.size(); i++)
        {
            temporaryPhase = omitPhases[i];

            if (temporaryPhase < 5)
            {
                ring1Schedule.commandPhase = temporaryPhase;
                ring1Schedule.commandType = "omit_veh";
                ring1Schedule.commandStartTime = 0.0;
                ring1Schedule.commandEndTime = rightCriticalPoints_PhaseDuration_Ring1[rightCriticalPoints_PhaseDuration_Ring1.size()-1];

                ring1_TCISchedule.push_back(ring1Schedule);
            }

            else if (temporaryPhase > 4)
            {
                ring2Schedule.commandPhase = temporaryPhase;
                ring2Schedule.commandType = "omit_veh";
                ring2Schedule.commandStartTime = 0.0;
                ring2Schedule.commandEndTime = rightCriticalPoints_PhaseDuration_Ring2[rightCriticalPoints_PhaseDuration_Ring2.size()-1];

                ring2_TCISchedule.push_back(ring2Schedule);
            }
        }
    }
}

string ScheduleManager::createScheduleJsonString()
{
    vector<Schedule> completeSchedule;
    // ring1_TCISchedule.insert(ring1_TCISchedule.end(), ring2_TCISchedule.begin(), ring2_TCISchedule.end());
    completeSchedule.insert(completeSchedule.end(), ring1_TCISchedule.begin(), ring1_TCISchedule.end());
    completeSchedule.insert(completeSchedule.end(), ring2_TCISchedule.begin(), ring2_TCISchedule.end());
    string scheduleJsonString{};
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
        // for (unsigned int i = 0; i < ring1_TCISchedule.size(); i++)
        // {
        //     jsonObject["Schedule"]["Ring1"][i]["commandPhase"] = ring1_TCISchedule[i].commandPhase;
        //     jsonObject["Schedule"]["Ring1"][i]["commandStartTime"] = ring1_TCISchedule[i].commandStartTime;
        //     jsonObject["Schedule"]["Ring1"][i]["commandEndTime"] = ring1_TCISchedule[i].commandEndTime;
        //     jsonObject["Schedule"]["Ring1"][i]["commandType"] = ring1_TCISchedule[i].commandType;
        // }
        // for (unsigned int i = 0; i < ring2_TCISchedule.size(); i++)
        // {
        //     jsonObject["Schedule"]["Ring2"][i]["commandPhase"] = ring2_TCISchedule[i].commandPhase;
        //     jsonObject["Schedule"]["Ring2"][i]["commandStartTime"] = ring2_TCISchedule[i].commandStartTime;
        //     jsonObject["Schedule"]["Ring2"][i]["commandEndTime"] = ring2_TCISchedule[i].commandEndTime;
        //     jsonObject["Schedule"]["Ring2"][i]["commandType"] = ring2_TCISchedule[i].commandType;
        // }

        for (unsigned int i = 0; i < completeSchedule.size(); i++)
        {
            jsonObject["Schedule"][i]["commandPhase"] = completeSchedule[i].commandPhase;
            jsonObject["Schedule"][i]["commandStartTime"] = completeSchedule[i].commandStartTime;
            jsonObject["Schedule"][i]["commandEndTime"] = completeSchedule[i].commandEndTime;
            jsonObject["Schedule"][i]["commandType"] = completeSchedule[i].commandType;
        }
    }

    scheduleJsonString = fastWriter.write(jsonObject);
    styledStreamWriter.write(outputter, jsonObject);

    return scheduleJsonString;
}


ScheduleManager::~ScheduleManager()
{
}