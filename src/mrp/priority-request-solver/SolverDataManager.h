/***********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  SolverDataManager.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. Header file for SolverDataManager class
*/
#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "RequestList.h"
#include "TrafficSignalPlan.h"

using std::vector;
using std::ios;
using std::ofstream;
using std::cout;
using std::endl;

#define VehicleClass_EmergencyVehicle 1
#define VehicleClass_Transit 2
#define VehicleClass_Truck 3
#define VehicleClass_EmergencyVehicleSplitRequest 4
#define VehicleClass_Coordination 5

class SolverDataManager
{
private:
    int numberOfTransitInList{};
    int numberOfTruckInList{};
    int numberOfEVInList{};
    int numberOfEVSplitRequestInList{};
    int numberOfCoordinationRequestInCycle1{};
    int numberOfCoordinationRequestInCycle2{};
    double maxEV_ETA{};
    double maxEV_ETA_Duration{};
    double EmergencyVehicleWeight{1.0};
    double EmergencyVehicleSplitPhaseWeight{0.1};        
    double TransitWeight{1.0};
    double TruckWeight{1.0};
    double DilemmaZoneRequestWeight{20.0};
    double CoordinationWeight{1.0};
    vector<int> requestedSignalGroup;
    vector<RequestList> priorityRequestList;
    vector<RequestList> dilemmaZoneRequestList;
    vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus;
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan;

public:
    SolverDataManager();
    SolverDataManager(vector<RequestList> requestList);
    SolverDataManager(vector<RequestList> dilemmaZoneList, vector<RequestList> requestList, vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus, vector<TrafficControllerData::TrafficSignalPlan> signalPlan, double EV_Weight, double EV_SplitPhase_Weight, double Transit_Weight, double Truck_Weight, double DZ_Request_Weight, double Coordination_Weight);
    
    ~SolverDataManager();

    vector<int> getRequestedSignalGroupFromPriorityRequestList();
    void removeDuplicateSignalGroup();
    void addAssociatedSignalGroup();
    void modifyGreenMax();
    void modifyCurrentSignalStatus();
    void findMaximumETAofEV();
    void generateDatFile(bool emergencyVehicleStatus);
    bool findSignalGroupInList(int signalGroup);
};
