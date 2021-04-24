/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  TrafficSignalPlan.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is contains the data structure for static information of traffic controller
*/

#pragma once
#include <string>

using std::string;

namespace TrafficControllerData
{
    struct TrafficSignalPlan
    {
        int phaseNumber{};
        double pedWalk{};
        double pedClear{};
        double minGreen{};
        double passage{};
        double maxGreen{};
        double yellowChange{};
        double redClear{};
        int phaseRing{};

        void reset()
        {
            phaseNumber = 0;
            pedWalk = 0.0;
            pedClear = 0.0;
            minGreen = 0.0;
            passage = 0.0;
            maxGreen = 0.0;
            yellowChange = 0.0;
            redClear = 0.0;
            phaseRing = 0;
        }
    };
    struct TrafficConrtollerStatus
    {
        int startingPhase1{};
        int startingPhase2{};
        double initPhase1{};
        double initPhase2{};
        double elapsedGreen1{};
        double elapsedGreen2{};
        double elapsedGreenInGmax1{};
        double elapsedGreenInGmax2{};
        double remainingGMax1{};
        double remainingGMax2{};
        double pedServiceElapsedTime1{};
        double pedServiceElapsedTime2{};
        bool currentPedCallStatus1{};
        bool currentPedCallStatus2{};
        string pedState1{};
        string pedState2{};

        void reset()
        {
            startingPhase1 = 0;
            startingPhase2 = 0;
            initPhase1 = 0.0;
            initPhase2 = 0.0;
            elapsedGreen1 = 0.0;
            elapsedGreen2 = 0.0;
            elapsedGreenInGmax1 = 0.0;
            elapsedGreenInGmax2 = 0.0;
            remainingGMax1 = 0.0;
            remainingGMax2 = 0.0;
            pedServiceElapsedTime1 = 0.0;
            pedServiceElapsedTime2 = 0.0;
            currentPedCallStatus1 = false;
            currentPedCallStatus2 = false;
            pedState1 = "";
            pedState2 = "";
        }
    };

}; // namespace TrafficControllerData