/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  TrafficConrtollerStatusManager.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script contains method to manage the receive current signal status message from the TCI.
*/
#include "TrafficConrtollerStatusManager.h"
#include <fstream>
#include <cmath>
#include <algorithm>

TrafficConrtollerStatusManager::TrafficConrtollerStatusManager(bool coordination_Request_Status, double cycle_Length, double offset_Value, double coordination_StartTime,
                                                               int coordinated_Phase1, int coordinated_Phase2, bool logging_Status, string file_Name,
                                                               vector<TrafficControllerData::TrafficSignalPlan> traffic_Signal_Timing_Plan, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalCoordinationPlan)
{
    coordinationRequestStatus = coordination_Request_Status;
    cycleLength = cycle_Length;
    offset = offset_Value;
    coordinationStartTime = coordination_StartTime;
    coordinatedPhase1 = coordinated_Phase1;
    coordinatedPhase2 = coordinated_Phase2;
    loggingStatus = logging_Status;
    fileName = file_Name;

    if (!traffic_Signal_Timing_Plan.empty())
        trafficSignalPlan = traffic_Signal_Timing_Plan;

    if (!trafficSignalCoordinationPlan.empty())
        trafficSignalPlan_SignalCoordination = trafficSignalCoordinationPlan;
}

/*
    - If new priority request is received this method will obtain the current traffic signal Status.
    - If the current phase is in yellow or red state in the json string, the method set next phase as starting phase.
    - If the current phase is in yellow or red state, the method calculates the init (time to start starting phase) value.
    - If red clearance time for both phases are not same, one phase can be in red rest. In that case init time can be negative. The method sets init time same as the init time of other starting phase..
    - If starting phase is on rest or elapsed green time is more than gmax, the method sets the elapsed green time min green time.
*/
void TrafficConrtollerStatusManager::manageCurrentSignalStatus(string jsonString)
{
    int temporaryCurrentPhase{};
    int temporaryNextPhase{};
    string temporaryPhaseState{};
    double temporaryElaspedTime{};
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    TrafficControllerData::TrafficConrtollerStatus tcStatus;
    trafficControllerStatus.clear();

    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    cout << "[" << fixed << showpoint << setprecision(2) << currentTime << "] Received Current Signal Status" << endl;

    const Json::Value values = jsonObject["currentPhases"];

    for (int i = 0; i < NumberOfStartingPhase; i++)
    {
        for (size_t j = 0; j < values[i].getMemberNames().size(); j++)
        {
            if (values[i].getMemberNames()[j] == "Phase")
                temporaryCurrentPhase = values[i][values[i].getMemberNames()[j]].asInt();

            else if (values[i].getMemberNames()[j] == "State")
                temporaryPhaseState = values[i][values[i].getMemberNames()[j]].asString();

            else if (values[i].getMemberNames()[j] == "ElapsedTime")
                temporaryElaspedTime = (values[i][values[i].getMemberNames()[j]].asDouble()) / 10.0;
        }

        if (temporaryCurrentPhase < FirstPhaseOfRing2 && temporaryPhaseState == "green")
        {
            tcStatus.startingPhase1 = temporaryCurrentPhase;
            tcStatus.initPhase1 = Initialize;
            tcStatus.elapsedGreen1 = temporaryElaspedTime;
        }

        else if (temporaryCurrentPhase > LastPhaseOfRing1 && temporaryPhaseState == "green")
        {
            tcStatus.startingPhase2 = temporaryCurrentPhase;
            tcStatus.initPhase2 = Initialize;
            tcStatus.elapsedGreen2 = temporaryElaspedTime;
        }

        else if (temporaryPhaseState == "yellow")
        {
            for (int k = 0; k < NumberOfStartingPhase; k++)
            {
                temporaryNextPhase = (jsonObject["nextPhases"][k]).asInt();
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCurrentPhase; });
                if (temporaryNextPhase > 0 && temporaryNextPhase < FirstPhaseOfRing2)
                {
                    tcStatus.startingPhase1 = temporaryNextPhase;
                    tcStatus.initPhase1 = findSignalGroup->yellowChange + findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen1 = Initialize;
                }
                else if (temporaryNextPhase > LastPhaseOfRing1 && temporaryNextPhase <= LastPhaseOfRing2)
                {
                    tcStatus.startingPhase2 = temporaryNextPhase;
                    tcStatus.initPhase2 = findSignalGroup->yellowChange + findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen2 = Initialize;
                }
            }
        }

        else if (temporaryPhaseState == "red")
        {
            for (int k = 0; k < NumberOfStartingPhase; k++)
            {
                temporaryNextPhase = (jsonObject["nextPhases"][k]).asInt();
                /*
                    - Current phase and next phase can be same in case of T intersection.
                    - For example, the scenario when phase 4 is only yellow/red since phase 8 is missing. phase 2 and 6 was green before phase 4.
                    - In this case current signal status message can be following: 
                        {"currentPhases": [{"Phase": 4, "State": "yellow", "ElapsedTime": 5}, {"Phase": 6, "State": "red", "ElapsedTime": 136}], 
                        "MsgType": "CurrNextPhaseStatus", "nextPhases": [6]}
                */
                if (temporaryCurrentPhase == temporaryNextPhase)
                {
                    cout << "[" << fixed << showpoint << setprecision(2) << currentTime << "] Current Phase and next phase is same" << endl;
                    break;
                }

                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCurrentPhase; });

                if (temporaryNextPhase > 0 && temporaryNextPhase < FirstPhaseOfRing2)
                {
                    tcStatus.startingPhase1 = temporaryNextPhase;
                    //If red clearance time for both phases are not same, One phase will be in red rest. In that case we will get negative init time.
                    if ((findSignalGroup->redClear - temporaryElaspedTime) < 0.0)
                        tcStatus.initPhase1 = 0.5;

                    else
                        tcStatus.initPhase1 = findSignalGroup->redClear - temporaryElaspedTime;

                    tcStatus.elapsedGreen1 = Initialize;
                }

                else if (temporaryNextPhase > LastPhaseOfRing1 && temporaryNextPhase <= LastPhaseOfRing2)
                {
                    tcStatus.startingPhase2 = temporaryNextPhase;

                    if ((findSignalGroup->redClear - temporaryElaspedTime) < 0.0)
                        tcStatus.initPhase2 = 0.5;

                    else
                        tcStatus.initPhase2 = findSignalGroup->redClear - temporaryElaspedTime;

                    tcStatus.elapsedGreen2 = Initialize;
                }
            }
        }
    }

    trafficControllerStatus.push_back(tcStatus);
    validateTrafficControllerStatus();
    modifyTrafficControllerStatus();
}

/*
- If there is coordination request for coordinated phases:
    - phase elapsed time will be gmin if elapsed time in cycle is less than the coordinated phases upper limit of Green Time
    - for coordinated phases amount early retun time is deducted from the elapsed time after passing the gmin
    - otherwise phase elapsed time will be gmax - tolerace(say 1 second)
- If there is no coordination request then- 
    If signal phase is on rest or elapsed green time is more than gmax, then elapsed green time will be set as min green time.
*/
void TrafficConrtollerStatusManager::modifyTrafficControllerStatus()
{
    double currentTimeOfToday{};
    double earlyReturnedValue{};
    double upperLimitOfGreenTimeForCoordinatedPhase{};
    double elapsedTimeInCycle{};
    int temporaryPhase{};

    if (coordinationRequestStatus)
    {
        currentTimeOfToday = getCurrentTime();
        elapsedTimeInCycle = fmod((currentTimeOfToday - coordinationStartTime), cycleLength);

        //Temporary logging code for debugging coordination
        ofstream outputfile;
        ifstream infile;

        if (loggingStatus)
        {
            outputfile.open(fileName, std::ios_base::app);
            auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            outputfile << "\nThe elapsed time in a cycle at time " << currentTime << " is " << elapsedTimeInCycle << endl;
            outputfile.close();
        }

        for (size_t i = 0; i < trafficControllerStatus.size(); i++)
        {
            // For coordinated phase in ring 1
            if (trafficControllerStatus[i].startingPhase1 == coordinatedPhase1)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase1;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

                //compute the early returned value and upper limit of green time
                if (elapsedTimeInCycle > offset)
                    earlyReturnedValue = trafficControllerStatus[i].elapsedGreen1 + offset - elapsedTimeInCycle;

                else
                    earlyReturnedValue = 0.0;

                upperLimitOfGreenTimeForCoordinatedPhase = offset + findSignalGroup1->maxGreen;

                //If (elapsedTimeInCycle -Tolerance) value is in the range of upperLimitOfGreenTimeForCoordinatedPhase and (upperLimitOfGreenTimeForCoordinatedPhase + PRS_Timed_Out_Value),
                // elasped green time will be set as max green time - Tolerance
                if ((elapsedTimeInCycle - Tolerance) >= upperLimitOfGreenTimeForCoordinatedPhase &&
                    (elapsedTimeInCycle - Tolerance) < (upperLimitOfGreenTimeForCoordinatedPhase + PRS_Timed_Out_Value))
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->maxGreen - Tolerance;

                //If elapsed green time is greater than the min green time  and early return value is positive, 
                //early return value will be deducted from elapsed green time.
                else if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->minGreen && earlyReturnedValue > 0)
                    trafficControllerStatus[i].elapsedGreen1 = trafficControllerStatus[i].elapsedGreen1 - earlyReturnedValue;

                // If early return value is greater than the gmin, elapsed green time value may become negative.
                // The elapsed green time will be set as min green
                if (trafficControllerStatus[i].elapsedGreen1 < 0)
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;
            }

            // For non-coordinated phases in ring 1
            else if (trafficControllerStatus[i].startingPhase1 != coordinatedPhase1)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase1;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

                //If elapsed green time is greater than the (maxgreen - Tolerance), elaseped green will be set as (maxgreen - Tolerance)
                if (trafficControllerStatus[i].elapsedGreen1 >= findSignalGroup1->maxGreen - Tolerance)
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->maxGreen - Tolerance;
            }

            // For coordinated phase in ring 2
            if (trafficControllerStatus[i].startingPhase2 == coordinatedPhase2)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase2;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
                //compute the early returned value and upper limit of green time
                if (elapsedTimeInCycle > offset)
                    earlyReturnedValue = trafficControllerStatus[i].elapsedGreen2 + offset - elapsedTimeInCycle;

                else
                    earlyReturnedValue = 0.0;

                upperLimitOfGreenTimeForCoordinatedPhase = offset + findSignalGroup2->maxGreen;

                //If (elapsedTimeInCycle -Tolerance) value is in the range of upperLimitOfGreenTimeForCoordinatedPhase and (upperLimitOfGreenTimeForCoordinatedPhase + PRS_Timed_Out_Value),
                // elasped green time will be set as max green time - Tolerance
                if (elapsedTimeInCycle - Tolerance > upperLimitOfGreenTimeForCoordinatedPhase &&
                    (elapsedTimeInCycle - Tolerance) < (upperLimitOfGreenTimeForCoordinatedPhase + PRS_Timed_Out_Value))
                    trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->maxGreen - Tolerance;

                //If elapsed green time is greater than the min green time  and early return value is positive, 
                //early return value will be deducted from elapsed green time.
                else if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->minGreen && earlyReturnedValue > 0)
                    trafficControllerStatus[i].elapsedGreen2 = trafficControllerStatus[i].elapsedGreen2 - earlyReturnedValue;

                // If early return value is greater than the gmin, elapsed green time value may become negative.
                // The elapsed green time will be set as min green
                if (trafficControllerStatus[i].elapsedGreen2 < 0)
                    trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
            }

            // For non-coordinated phases in ring 2
            else if (trafficControllerStatus[i].startingPhase2 != coordinatedPhase2)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase2;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
                
                //If elapsed green time is greater than the (maxgreen - Tolerance), elaseped green will be set as (maxgreen - Tolerance)
                if (trafficControllerStatus[i].elapsedGreen2 >= findSignalGroup2->maxGreen - Tolerance)
                    trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->maxGreen - Tolerance;
            }
        }
    }

    else
    {
        for (size_t i = 0; i < trafficControllerStatus.size(); i++)
        {
            temporaryPhase = trafficControllerStatus[i].startingPhase1;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                       [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
            if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->minGreen)
                trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;

            temporaryPhase = trafficControllerStatus[i].startingPhase2;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                       [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
            if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->minGreen)
                trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
        }
    }
}

/*
    - The method is responsible for avoiding edge condition. It checks whether starting phase1 is in ring 1 and starting phase2 is in ring 2
    - For T-intersection starting phase can be zero. In that case, the method fills the the elapsed green time and the init time values with the elapsed green time and the init time values of other starting phase.
*/
void TrafficConrtollerStatusManager::validateTrafficControllerStatus()
{
    if (trafficControllerStatus[0].startingPhase1 == 0)
    {
        trafficControllerStatus[0].startingPhase1 = trafficControllerStatus[0].startingPhase2 - NumberOfPhasePerRing;
        trafficControllerStatus[0].elapsedGreen1 = trafficControllerStatus[0].elapsedGreen2;
        trafficControllerStatus[0].initPhase1 = trafficControllerStatus[0].initPhase2;
    }

    else if (trafficControllerStatus[0].startingPhase2 == 0)
    {
        trafficControllerStatus[0].startingPhase2 = trafficControllerStatus[0].startingPhase1 + NumberOfPhasePerRing;
        trafficControllerStatus[0].elapsedGreen2 = trafficControllerStatus[0].elapsedGreen1;
        trafficControllerStatus[0].initPhase2 = trafficControllerStatus[0].initPhase1;
    }
}

vector<TrafficControllerData::TrafficConrtollerStatus> TrafficConrtollerStatusManager::getTrafficControllerStatus(string jsonString)
{
    manageCurrentSignalStatus(jsonString);

    return trafficControllerStatus;
}

double TrafficConrtollerStatusManager::getCurrentTime()
{
    double currentTime{};
    time_t s = 1;
    struct tm *current_time;

    // time in seconds
    s = time(NULL);

    // to get current time
    current_time = localtime(&s);

    currentTime = current_time->tm_hour * 3600.00 + current_time->tm_min * 60.00 + current_time->tm_sec;

    return currentTime;
}

TrafficConrtollerStatusManager::~TrafficConrtollerStatusManager()
{
}