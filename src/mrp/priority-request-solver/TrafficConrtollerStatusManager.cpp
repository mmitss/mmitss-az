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

TrafficConrtollerStatusManager::TrafficConrtollerStatusManager(bool coordination_Request_Status, double cycle_Length, double offset_Value, double coordination_StartTime, double elapsed_Time_In_Cycle,
                                                               int coordinated_Phase1, int coordinated_Phase2, bool logging_Status, bool console_Output_Status, vector<int> listOfDummyPhases,
                                                               vector<TrafficControllerData::TrafficSignalPlan> traffic_Signal_Timing_Plan, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalCoordinationPlan)
{
    coordinationRequestStatus = coordination_Request_Status;
    cycleLength = cycle_Length;
    offset = offset_Value;
    coordinationStartTime = coordination_StartTime;
    elapsedTimeInCycle = elapsed_Time_In_Cycle;
    coordinatedPhase1 = coordinated_Phase1;
    coordinatedPhase2 = coordinated_Phase2;
    logging = logging_Status;
    consoleOutput = console_Output_Status;

    if (!listOfDummyPhases.empty())
        dummyPhasesList = listOfDummyPhases;

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
    int noOfVehicleCall{};
    int noOfPedCall{};
    string temporaryPhaseState{};
    string temporaryPedState{};
    double temporaryElaspedTime{};
    double temporaryElapsedTimeInGmax{}; //It starts when gmax starts timing (vehicle calls on the conflicting phase)
    double temporaryRemainingGmax{};
    double timeStamp = getPosixTimestamp();
    TrafficControllerData::TrafficConrtollerStatus tcStatus;
    trafficControllerStatus.clear();

    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    noOfVehicleCall = (jsonObject["totalVehicleCalls"]).asInt();
    noOfPedCall = (jsonObject["totalPedestrianCalls"]).asInt();

    for (int i = 0; i < noOfVehicleCall; i++)
        vehicleCallList.push_back((jsonObject["vehicleCalls"][i]).asInt());

    for (int i = 0; i < noOfPedCall; i++)
        pedCallList.push_back((jsonObject["pedestrianCalls"][i]).asInt());

    setPhaseCallList();

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

            else if (values[i].getMemberNames()[j] == "ElapsedTimeInGMax")
                temporaryElapsedTimeInGmax = (values[i][values[i].getMemberNames()[j]].asDouble()) / 10.0;

            else if (values[i].getMemberNames()[j] == "RemainingGMax")
                temporaryRemainingGmax = (values[i][values[i].getMemberNames()[j]].asDouble()) / 10.0;

            else if (values[i].getMemberNames()[j] == "PedState")
                temporaryPedState = (values[i][values[i].getMemberNames()[j]].asString());
        }

        if (temporaryCurrentPhase < FirstPhaseOfRing2 && temporaryPhaseState == "green")
        {
            tcStatus.startingPhase1 = temporaryCurrentPhase;
            tcStatus.initPhase1 = Initialize;
            tcStatus.elapsedGreen1 = temporaryElaspedTime;
            tcStatus.elapsedGreenInGmax1 = temporaryElapsedTimeInGmax;
            tcStatus.remainingGMax1 = temporaryRemainingGmax;
            tcStatus.pedState1 = temporaryPedState;
        }

        else if (temporaryCurrentPhase > LastPhaseOfRing1 && temporaryPhaseState == "green")
        {
            tcStatus.startingPhase2 = temporaryCurrentPhase;
            tcStatus.initPhase2 = Initialize;
            tcStatus.elapsedGreen2 = temporaryElaspedTime;
            tcStatus.elapsedGreenInGmax2 = temporaryElapsedTimeInGmax;
            tcStatus.remainingGMax2 = temporaryRemainingGmax;
            tcStatus.pedState2 = temporaryPedState;
        }

        else if (temporaryPhaseState == "yellow")
        {
            for (int k = 0; k < NumberOfStartingPhase; k++)
            {
                temporaryNextPhase = (jsonObject["nextPhases"][k]).asInt();
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCurrentPhase; });
                if ((temporaryCurrentPhase < FirstPhaseOfRing2) && (temporaryNextPhase > 0) &&
                    (temporaryNextPhase < FirstPhaseOfRing2))
                {
                    tcStatus.startingPhase1 = temporaryNextPhase;
                    tcStatus.initPhase1 = findSignalGroup->yellowChange + findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen1 = Initialize;
                }

                else if ((temporaryCurrentPhase > LastPhaseOfRing1) && (temporaryNextPhase > LastPhaseOfRing1) &&
                         (temporaryNextPhase <= LastPhaseOfRing2))
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
                    if (consoleOutput)
                        cout << "[" << fixed << showpoint << setprecision(4) << timeStamp << "] Current Phase and next phase is same" << endl;
                    break;
                }

                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCurrentPhase; });

                if ((temporaryCurrentPhase < FirstPhaseOfRing2) && (temporaryNextPhase > 0) &&
                    (temporaryNextPhase < FirstPhaseOfRing2))
                {
                    tcStatus.startingPhase1 = temporaryNextPhase;
                    //If red clearance time for both phases are not same, One phase will be in red rest. In that case we will get negative init time.
                    if ((findSignalGroup->redClear - temporaryElaspedTime) < 0.0)
                        tcStatus.initPhase1 = 0.5;

                    else
                        tcStatus.initPhase1 = findSignalGroup->redClear - temporaryElaspedTime;

                    tcStatus.elapsedGreen1 = Initialize;
                }

                else if ((temporaryCurrentPhase > LastPhaseOfRing1) && (temporaryNextPhase > LastPhaseOfRing1) &&
                         (temporaryNextPhase <= LastPhaseOfRing2))
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
    setCurrentPedCallStatus();
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
    double earlyReturnedValue{};
    double upperLimitOfGreenTimeForCoordinatedPhase{};
    int temporaryPhase{};

    if (coordinationRequestStatus)
    {
        for (size_t i = 0; i < trafficControllerStatus.size(); i++)
        {
            // For coordinated phase in ring 1
            if (trafficControllerStatus[i].startingPhase1 == coordinatedPhase1 && trafficControllerStatus[i].initPhase1 == 0)
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

                //If elapsed green time is less than gmin, continue
                if (trafficControllerStatus[i].elapsedGreen1 < findSignalGroup1->minGreen)
                    trafficControllerStatus[i].elapsedGreen1 =  trafficControllerStatus[i].elapsedGreen1;

                //If elapsed green time is greater than gmin, and (elapsedTimeInCycle - Tolerance) value is greater than the upperLimitOfGreenTimeForCoordinatedPhase,
                // elapsed green time will be set as (gmax-tolerance)
                else if ((elapsedTimeInCycle - Tolerance) >= upperLimitOfGreenTimeForCoordinatedPhase)
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->maxGreen - Tolerance;

                //If elapsed green time is greater than gmin, the early return value is negative (phase was after the offset) and (elapsedTimeInCycle - Tolerance) value is greater than the upperLimitOfGreenTimeForCoordinatedPhase, then continue
                else if (earlyReturnedValue < 0 && ((elapsedTimeInCycle - Tolerance) < upperLimitOfGreenTimeForCoordinatedPhase))
                    trafficControllerStatus[i].elapsedGreen1 = trafficControllerStatus[i].elapsedGreen1 ;
                
                // If elapsed green time is greater than the gmin  and early return value is positive, early return value will be deducted from elapsed green time.
                //If early return value is greater than the gmin, elapsed green time value may become negative. The elapsed green time will be set as min green
                else if (earlyReturnedValue > 0 && ((elapsedTimeInCycle - Tolerance) < upperLimitOfGreenTimeForCoordinatedPhase))
                {
                    trafficControllerStatus[i].elapsedGreen1 = trafficControllerStatus[i].elapsedGreen1 - earlyReturnedValue;
                    
                    if (trafficControllerStatus[i].elapsedGreen1 < 0)
                        trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;
                }              
            }

            // For non-coordinated phases in ring 1
            else if (trafficControllerStatus[i].startingPhase1 != coordinatedPhase1 && trafficControllerStatus[i].initPhase1 == 0)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase1;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

                //If elapsed green time is greater than the (maxgreen - Tolerance), elaseped green will be set as (maxgreen - Tolerance)
                if (trafficControllerStatus[i].elapsedGreen1 >= findSignalGroup1->maxGreen - Tolerance)
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->maxGreen - Tolerance;
            }

            // For coordinated phase in ring 2
            if (trafficControllerStatus[i].startingPhase2 == coordinatedPhase2 && trafficControllerStatus[i].initPhase2 == 0)
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

                //If elapsed green time is less than gmin, continue
                if (trafficControllerStatus[i].elapsedGreen2 < findSignalGroup2->minGreen)
                    trafficControllerStatus[i].elapsedGreen2 = trafficControllerStatus[i].elapsedGreen2;

                //If elapsed green time is greater than gmin, and (elapsedTimeInCycle - Tolerance) value is greater than the upperLimitOfGreenTimeForCoordinatedPhase,
                // elapsed green time will be set as (gmax-tolerance)
                else if ((elapsedTimeInCycle - Tolerance) >= upperLimitOfGreenTimeForCoordinatedPhase)
                    trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->maxGreen - Tolerance;

                //If elapsed green time is greater than gmin, the early return value is negative (phase was after the offset) and (elapsedTimeInCycle - Tolerance) value is greater than the upperLimitOfGreenTimeForCoordinatedPhase, then continue
                else if (earlyReturnedValue < 0 && ((elapsedTimeInCycle - Tolerance) < upperLimitOfGreenTimeForCoordinatedPhase))
                    trafficControllerStatus[i].elapsedGreen2 = trafficControllerStatus[i].elapsedGreen2;
                
                // If elapsed green time is greater than the gmin  and early return value is positive, early return value will be deducted from elapsed green time.
                //If early return value is greater than the gmin, elapsed green time value may become negative. The elapsed green time will be set as min green
                else if (earlyReturnedValue > 0 && ((elapsedTimeInCycle - Tolerance) < upperLimitOfGreenTimeForCoordinatedPhase))
                {
                    trafficControllerStatus[i].elapsedGreen2 = trafficControllerStatus[i].elapsedGreen2 - earlyReturnedValue;
                    
                    if (trafficControllerStatus[i].elapsedGreen2 < 0)
                        trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
                } 
            }

            // For non-coordinated phases in ring 2
            else if (trafficControllerStatus[i].startingPhase2 != coordinatedPhase2 && trafficControllerStatus[i].initPhase2 == 0)
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

    /*
        - If elapsed green time is less then Gmin, elapsed green time will be as it is
        - If elapsed time is greater than Gmin and there is conflicting phase call,
            - If  and remaining Gmax is greater than zero (Gmax is timing) and 
                elaspedGmax (Gmax - remainingGmax) less than  Gmin, than elapsed green time will be Gmin
            - If  and remaining Gmax is greater than zero (Gmax is timing) and 
                elaspedGmax (Gmax - remainingGmax) greater than  Gmin, than elapsed green time will be elaspedGmax
            - If  and remaining Gmax is zero (Gmax is timed already) and 
                elaspedGmax (Gmax - remainingGmax) greater than  zero (it will be more than Gmax), 
                than elapsed green time will be elaspedGmax
            
    */
    else
    {
        setConflictingPhaseCallStatus();

        for (size_t i = 0; i < trafficControllerStatus.size(); i++)
        {
            temporaryPhase = trafficControllerStatus[i].startingPhase1;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 =
                std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                             [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

            if (trafficControllerStatus[i].elapsedGreen1 < findSignalGroup1->minGreen)
                continue;

            else if (conflictingPhaseCallStatus && (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->minGreen) &&
                     (trafficControllerStatus[i].elapsedGreenInGmax1 > 0))
            {
                if (trafficControllerStatus[i].elapsedGreenInGmax1 < findSignalGroup1->minGreen)
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;

                else
                    trafficControllerStatus[i].elapsedGreen1 = trafficControllerStatus[i].elapsedGreenInGmax1;
            }

            else if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->minGreen)
                trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;

            temporaryPhase = trafficControllerStatus[i].startingPhase2;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 =
                std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                             [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

            if (trafficControllerStatus[i].elapsedGreen2 < findSignalGroup2->minGreen)
                continue;

            else if (conflictingPhaseCallStatus && (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->minGreen) &&
                     (trafficControllerStatus[i].elapsedGreenInGmax2 > 0))
            {
                if (trafficControllerStatus[i].elapsedGreenInGmax2 < findSignalGroup2->minGreen)
                    trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;

                else
                    trafficControllerStatus[i].elapsedGreen2 = trafficControllerStatus[i].elapsedGreenInGmax2;
            }

            else if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->minGreen)
                trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
        }

        validateElapsedGreenTime();
    }
}

/*
    - If elapased time for any of the starting phase is less than zero, the following method will set the elapsed time value as zero.
    - For the dummy phase, the elapsed time will set as the elapsed time of the starting phase which is in opposite ring
*/
void TrafficConrtollerStatusManager::validateElapsedGreenTime()
{
    if (trafficControllerStatus[0].elapsedGreen1 < 0 && trafficControllerStatus[0].elapsedGreen2 < 0)
    {
        trafficControllerStatus[0].elapsedGreen1 = 0.0;
        trafficControllerStatus[0].elapsedGreen2 = 0.0;
    }

    if (trafficControllerStatus[0].elapsedGreen1 < 0)
        trafficControllerStatus[0].elapsedGreen1 = trafficControllerStatus[0].elapsedGreen2;

    if (trafficControllerStatus[0].elapsedGreen2 < 0)
        trafficControllerStatus[0].elapsedGreen2 = trafficControllerStatus[0].elapsedGreen1;

    for (size_t i = 0; i < dummyPhasesList.size(); i++)
    {
        if (trafficControllerStatus[0].startingPhase1 == dummyPhasesList.at(i))
            trafficControllerStatus[0].elapsedGreen1 = trafficControllerStatus[0].elapsedGreen2;

        else if (trafficControllerStatus[0].startingPhase2 == dummyPhasesList.at(i))
            trafficControllerStatus[0].elapsedGreen2 = trafficControllerStatus[0].elapsedGreen1;
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
        trafficControllerStatus[0].remainingGMax1 = trafficControllerStatus[0].remainingGMax2;
    }

    else if (trafficControllerStatus[0].startingPhase2 == 0)
    {
        trafficControllerStatus[0].startingPhase2 = trafficControllerStatus[0].startingPhase1 + NumberOfPhasePerRing;
        trafficControllerStatus[0].elapsedGreen2 = trafficControllerStatus[0].elapsedGreen1;
        trafficControllerStatus[0].initPhase2 = trafficControllerStatus[0].initPhase1;
        trafficControllerStatus[0].remainingGMax2 = trafficControllerStatus[0].remainingGMax1;
    }
}

void TrafficConrtollerStatusManager::setConflictingPhaseCallStatus()
{
    int temporaryPhase{};
    vector<int>::iterator it;
    vector<int> phasesInRingBarrierGroup{};
    vector<int> phasesInRingBarrierGroup1{1, 2, 5, 6};
    vector<int> phasesInRingBarrierGroup2{3, 4, 7, 8};

    if (!phaseCallList.empty())
    {
        if (trafficControllerStatus[0].startingPhase1 <= 2)
            phasesInRingBarrierGroup = phasesInRingBarrierGroup2;

        else if (trafficControllerStatus[0].startingPhase1 > 2 && trafficControllerStatus[0].startingPhase1 <= 4)
            phasesInRingBarrierGroup = phasesInRingBarrierGroup1;

        else if (trafficControllerStatus[0].startingPhase2 <= 6)
            phasesInRingBarrierGroup = phasesInRingBarrierGroup2;

        else if (trafficControllerStatus[0].startingPhase2 > 6 && trafficControllerStatus[0].startingPhase2 <= 8)
            phasesInRingBarrierGroup = phasesInRingBarrierGroup1;

        for (size_t i = 0; i < phasesInRingBarrierGroup.size(); i++)
        {
            temporaryPhase = phasesInRingBarrierGroup.at(i);
            it = std::find(phaseCallList.begin(), phaseCallList.end(), temporaryPhase);

            if (it != phaseCallList.end())
            {
                conflictingPhaseCallStatus = true;
                break;
            }
        }
    }
}

void TrafficConrtollerStatusManager::setConflictingPedCallStatus()
{
    int temporaryPhase{};
    vector<int>::iterator it;
    vector<int> phasesInRingBarrierGroup{};
    vector<int> phasesInRingBarrierGroup1{1, 2, 5, 6};
    vector<int> phasesInRingBarrierGroup2{3, 4, 7, 8};

    if (!pedCallList.empty())
    {
        if (trafficControllerStatus[0].startingPhase1 <= 2)
            phasesInRingBarrierGroup = phasesInRingBarrierGroup2;

        else if (trafficControllerStatus[0].startingPhase1 > 2 && trafficControllerStatus[0].startingPhase1 <= 4)
            phasesInRingBarrierGroup = phasesInRingBarrierGroup1;

        else if (trafficControllerStatus[0].startingPhase2 <= 6)
            phasesInRingBarrierGroup = phasesInRingBarrierGroup2;

        else if (trafficControllerStatus[0].startingPhase2 > 6 && trafficControllerStatus[0].startingPhase2 <= 8)
            phasesInRingBarrierGroup = phasesInRingBarrierGroup1;

        for (size_t i = 0; i < phasesInRingBarrierGroup.size(); i++)
        {
            temporaryPhase = phasesInRingBarrierGroup.at(i);
            it = std::find(pedCallList.begin(), pedCallList.end(), temporaryPhase);

            if (it != pedCallList.end())
            {
                conflictingPedCallStatus = true;
                break;
            }
        }
    }
}

/*
    - The following method merge pedCallList and vehicleCallList elements (phases) into phaseCallList
    - All the phases of pedCallList will insert into the phaseCallList
    - If phaseCallList is empty (pedCallList is empty), vehicleCallList phases will be inserted into the phaseCallList
    - If phaseCallList is not empty, ehicleCallList phases will be inserted into the phaseCallList only if it is not present in the phaseCallList
*/
void TrafficConrtollerStatusManager::setPhaseCallList()
{
    int temporaryPhase{};
    vector<int>::iterator it;
    phaseCallList.clear();
    phaseCallList.insert(phaseCallList.end(), pedCallList.begin(), pedCallList.end());

    if (phaseCallList.empty())
        phaseCallList.insert(phaseCallList.end(), vehicleCallList.begin(), vehicleCallList.end());

    else
    {
        for (size_t i = 0; i < vehicleCallList.size(); i++)
        {
            temporaryPhase = vehicleCallList.at(i);
            it = std::find(phaseCallList.begin(), phaseCallList.end(), temporaryPhase);
            if (it == phaseCallList.end())
                phaseCallList.push_back(temporaryPhase);
        }
    }
}

vector<TrafficControllerData::TrafficConrtollerStatus> TrafficConrtollerStatusManager::getTrafficControllerStatus(string jsonString)
{
    manageCurrentSignalStatus(jsonString);

    return trafficControllerStatus;
}

void TrafficConrtollerStatusManager::setCurrentPedCallStatus()
{
    if ((trafficControllerStatus[0].pedState1 == "walk") || (trafficControllerStatus[0].pedState1 == "ped_clear"))
    {
        currentPedCallStatus = true;
        trafficControllerStatus[0].currentPedCallStatus1 = true;
        trafficControllerStatus[0].pedServiceElapsedTime1 = trafficControllerStatus[0].elapsedGreen1;
    }

    if ((trafficControllerStatus[0].pedState2 == "walk") || (trafficControllerStatus[0].pedState2 == "ped_clear"))
    {
        currentPedCallStatus = true;
        trafficControllerStatus[0].currentPedCallStatus2 = true;
        trafficControllerStatus[0].pedServiceElapsedTime2 = trafficControllerStatus[0].elapsedGreen2;
    }
}

bool TrafficConrtollerStatusManager::getConflictingPedCallStatus()
{
    setConflictingPedCallStatus();
    return conflictingPedCallStatus;
}

/*
    - Method to find the list of conflicting ped call
*/
vector<int> TrafficConrtollerStatusManager::getConflictingPedCallList()
{
    vector<int> conflictingPedCallList{};
    int temporaryPhase{};

    for (size_t i = 0; i < pedCallList.size(); i++)
    {
        temporaryPhase = pedCallList.at(i);
        if ((temporaryPhase != trafficControllerStatus[0].startingPhase1) && (temporaryPhase != trafficControllerStatus[0].startingPhase2))
            conflictingPedCallList.push_back(temporaryPhase);
    }

    return conflictingPedCallList;
}

TrafficConrtollerStatusManager::~TrafficConrtollerStatusManager()
{
}