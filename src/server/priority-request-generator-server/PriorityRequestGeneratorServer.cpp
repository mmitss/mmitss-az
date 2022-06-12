/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorrityRequestGenerator.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. The script is responsible for maintaining Priority Request Generator list of multiple vehicles in simulation environment
  2. The script is developed for receiving bsm data from simulation and map, ssm data from the transceiver
  3. This script use MapManager class to obtain vehicle status based on active map. 
  4. This script has an API to generate srm JSON string for each vehicle using PriorityRequestGenerator class. The JSON string is compatible with asn1 j2735 standard
  5. The generated srm JSON string will send to Message Distributor over a UDP socket.
  6. This script has an API to manage Active Request Table for each vehicle based on the received ssm.
  7. The script has method to delete a vehicle information from the list if BSM is not received from a vehicle for more than predifined time(10sec).
*/

#include "PriorityRequestGeneratorServer.h"
#include <algorithm>
#include <cmath>
#include <chrono>

const double TIME_GAP_BETWEEN_RECEIVING_BSM = 10;
const double DSRC_RANGE = 800.0;

PriorityRequestGeneratorServer::PriorityRequestGeneratorServer()
{
}

/*
    - The following method will check whether the received vehicle information is required to add or update  in the PRGServer List
    - If the received BSM is from a new vehicle, the method will create a vehicle inforamtion object for the vehicle.
    - If vehicle id of the received BSM is already present in the PRGServer List, the method will update the vehicle position and time.
*/
void PriorityRequestGeneratorServer::managingPRGServerList(BasicVehicle basicVehicle)
{
    int vehid{};
    // ServerList vehicleinfo;
    vehicleinfo.reset();

    if (checkAddVehicleIDToPRGServerList(basicVehicle))
    {
        vehicleinfo.vehicleID = basicVehicle.getTemporaryID();
        vehicleinfo.vehicleType = basicVehicle.getType();
        vehicleinfo.updateTime = getCurrentTimeInSeconds();
        vehicleinfo.vehicleLatitude = basicVehicle.getLatitude_DecimalDegree();
        vehicleinfo.vehicleLongitude = basicVehicle.getLongitude_DecimalDegree();
        vehicleinfo.vehicleElevation = basicVehicle.getElevation_Meter();
        PRGServerList.push_back(vehicleinfo);
    }

    else if (checkUpdateVehicleIDInPRGServerList(basicVehicle))
    {
        vehid = basicVehicle.getTemporaryID();
        vector<ServerList>::iterator findVehicleIDInList = std::find_if(std::begin(PRGServerList), std::end(PRGServerList),
                                                                        [&](ServerList const &p)
                                                                        { return p.vehicleID == vehid; });

        findVehicleIDInList->updateTime = getCurrentTimeInSeconds();
        findVehicleIDInList->vehicleLatitude = basicVehicle.getLatitude_DecimalDegree();
        findVehicleIDInList->vehicleLongitude = basicVehicle.getLongitude_DecimalDegree();
        findVehicleIDInList->vehicleElevation = basicVehicle.getElevation_Meter();
    }
}
/*
    - The following method is responsible for processing the received BSM
        - At first, the method calls managingPRGServerList(BasicVehicle basicVehicle) to add or update the vehicle infromation
        - Then the method obtains vehicle information based on the active map and determine whether it is required to send SRM or not.
        - If it is required to send the SRM, the method formulates the srm json string
        - Finally method update map age, delete timed out map and formulate the PRG status json string for the HMI controller.
*/
void PriorityRequestGeneratorServer::processBSM(BasicVehicle basicVehicle)
{
    int veheicleID{};
    sendSRM = false;

    if (basicVehicle.getType() != "car")
    {
        veheicleID = basicVehicle.getTemporaryID();

        managingPRGServerList(basicVehicle);
        vector<ServerList>::iterator findVehicleIDInList = std::find_if(std::begin(PRGServerList), std::end(PRGServerList),
                                                                        [&](ServerList const &p)
                                                                        { return p.vehicleID == veheicleID; });

        findVehicleIDInList->PRG.setSimulationVehicleType(findVehicleIDInList->vehicleType);
        findVehicleIDInList->PRG.setBasicVehicleRole(findVehicleIDInList->PRG.getVehicleType());

        if (findVehicleIDInList->PRG.getVehicleType() != static_cast<int>(MsgEnum::vehicleType::car))
        {
            findVehicleIDInList->PRG.getVehicleInformationFromMAP(findVehicleIDInList->mapManager, basicVehicle);

            if (findVehicleIDInList->PRG.checkPriorityRequestSendingRequirementStatus())
            {
                srmSendingJsonString = findVehicleIDInList->PRG.createSRMJsonString(basicVehicle, findVehicleIDInList->signalRequest, findVehicleIDInList->mapManager);
                sendSRM = true;
            }

            findVehicleIDInList->mapManager.updateMapAge();
            findVehicleIDInList->mapManager.deleteMap();
            findVehicleIDInList->PRG.manageMapStatusInAvailableMapList(findVehicleIDInList->mapManager);
        }
    }
}

/*
    - The following is responsible for processing the received MAP message
    - The method uses MapManager class to maintain available map list
*/
void PriorityRequestGeneratorServer::processMap(string jsonString, MapManager mapManager)
{
    double mapReferenceLatitude{};
    double mapReferenceLongitude{};

    mapManager.json2MapPayload(jsonString);
    mapManager.writeMAPPayloadInFile();
    mapManager.getReferencePoint();
    mapReferenceLatitude = mapManager.getMapReferenceLatitude();
    mapReferenceLongitude = mapManager.getMapReferenceLongitude();

    for (size_t i = 0; i < PRGServerList.size(); i++)
    {
        if (haversineDistance(mapReferenceLatitude, mapReferenceLongitude, PRGServerList[i].vehicleLatitude, PRGServerList[i].vehicleLongitude) <= DSRC_RANGE)
        {
            PRGServerList[i].mapManager.json2MapPayload(jsonString);
            PRGServerList[i].mapManager.maintainAvailableMapList();
        }
    }
}

/*
    - The following is responsible for processing the received SSM
    - The method uses Signal status class to maintain Active Request Table
*/
void PriorityRequestGeneratorServer::processSSM(string jsonString)
{
    for (size_t i = 0; i < PRGServerList.size(); i++)
    {
        PRGServerList[i].signalStatus.json2SignalStatus(jsonString);
        PRGServerList[i].PRG.creatingSignalRequestTable(PRGServerList[i].signalStatus);
        PRGServerList[i].signalStatus.reset();
    }
}

/*
    - Method for deleting the timed out vehicle information.
    - The method will find the vehicle information object in PRGServer list for timed out vehicle ID and delete that object.
*/
void PriorityRequestGeneratorServer::deleteTimedOutVehicleInformationFromPRGServerList()
{
    int veheicleID{};
    if (checkDeleteTimedOutVehicleIDFromList())
    {
        veheicleID = getTimedOutVehicleID();
        vector<ServerList>::iterator findVehicleIDInList = std::find_if(std::begin(PRGServerList), std::end(PRGServerList),
                                                                        [&](ServerList const &p)
                                                                        { return p.vehicleID == veheicleID; });

        if (findVehicleIDInList != PRGServerList.end())
            PRGServerList.erase(findVehicleIDInList);
    }
}

/*
    - Method for computing haversine distance between two gps coordinates
*/
double PriorityRequestGeneratorServer::haversineDistance(double lat1, double lon1, double lat2, double lon2)
{
    double lattitudeDifference{};
    double longitudeDifference{};
    double rad{6371};
    double distance{};
    double intermediateCalculation{};

    lattitudeDifference = (lat2 - lat1) * M_PI / 180.0;
    longitudeDifference = (lon2 - lon1) * M_PI / 180.0;

    // convert to radians
    lat1 = (lat1)*M_PI / 180.0;
    lat2 = (lat2)*M_PI / 180.0;

    // apply formula
    intermediateCalculation = pow(sin(lattitudeDifference / 2), 2) + pow(sin(longitudeDifference / 2), 2) * cos(lat1) * cos(lat2);

    distance = 2 * rad * asin(sqrt(intermediateCalculation)) * 1000.0;

    return distance;
}

/*
    - The following boolean method will determine whether the received vehicle information is required to add in the PRGServer List
    - If vehicle ID is not present in the PRGServer list the method will return true. 
*/
bool PriorityRequestGeneratorServer::checkAddVehicleIDToPRGServerList(BasicVehicle basicVehicle)
{
    bool addVehicleID{false};
    int vehicleID = basicVehicle.getTemporaryID();

    vector<ServerList>::iterator findVehicleIDInList = std::find_if(std::begin(PRGServerList), std::end(PRGServerList),
                                                                    [&](ServerList const &p)
                                                                    { return p.vehicleID == vehicleID; });

    if (PRGServerList.empty())
        addVehicleID = true;

    else if (!PRGServerList.empty() && findVehicleIDInList == PRGServerList.end())
        addVehicleID = true;

    return addVehicleID;
}

/*
    - The following boolean method will determine whether the received vehicle information is required to update in the PRGServer List
    - If vehicle ID is present in the PRGServer list the method will return true.
*/
bool PriorityRequestGeneratorServer::checkUpdateVehicleIDInPRGServerList(BasicVehicle basicVehicle)
{
    bool updateVehicleID{false};
    int veheicleID = basicVehicle.getTemporaryID();

    vector<ServerList>::iterator findVehicleIDInList = std::find_if(std::begin(PRGServerList), std::end(PRGServerList),
                                                                    [&](ServerList const &p)
                                                                    { return p.vehicleID == veheicleID; });

    if (PRGServerList.empty())
        updateVehicleID = false;

    else if (!PRGServerList.empty() && findVehicleIDInList != PRGServerList.end())
        updateVehicleID = true;

    else if (!PRGServerList.empty() && findVehicleIDInList == PRGServerList.end())
        updateVehicleID = false;

    return updateVehicleID;
}

/*
    - The following boolean method will determine whether vehicle information is required to delete from the PRGServer List
    - If there BSM is not received from a vehicle for more than predifined time(10sec),the method will return true.
    - The method will set the timed out vehicle ID
*/
bool PriorityRequestGeneratorServer::checkDeleteTimedOutVehicleIDFromList()
{
    bool deleteVehicleInfo{false};

    if (!PRGServerList.empty())
    {
        for (size_t i = 0; i < PRGServerList.size(); i++)
        {
            if (getCurrentTimeInSeconds() - PRGServerList[i].updateTime > TIME_GAP_BETWEEN_RECEIVING_BSM)
            {
                deleteVehicleInfo = true;
                setTimedOutVehicleID(PRGServerList[i].vehicleID);
                break;
            }
        }
    }

    return deleteVehicleInfo;
}

/*
    - The following is responsible for obtaining SRM sending flag.
*/
bool PriorityRequestGeneratorServer::checkSrmSendingFlag()
{
    return sendSRM;
}

/*
    - Getter for SRM sending json string
*/
string PriorityRequestGeneratorServer::getSRMJsonString()
{
    return srmSendingJsonString;
}

/*
    - Getter for PRG status sending json string
*/
string PriorityRequestGeneratorServer::getPrgStatusJsonString()
{
    return prgStatusSendingJsonString;
}

/*
	- Method for identifying the message type.
*/
int PriorityRequestGeneratorServer::getMessageType(string jsonString)
{
    int messageType{};
    double timeStamp = getPosixTimestamp();
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    std::string errors{};
    bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    if (parsingSuccessful)
    {
        if ((jsonObject["MsgType"]).asString() == "MAP")
            messageType = MsgEnum::DSRCmsgID_map;

        else if ((jsonObject["MsgType"]).asString() == "BSM")
            messageType = MsgEnum::DSRCmsgID_bsm;

        else if ((jsonObject["MsgType"]).asString() == "SSM")
            messageType = MsgEnum::DSRCmsgID_ssm;

        else
            cout << "[" << fixed << showpoint << setprecision(4) << timeStamp << "] Message type is unknown" << std::endl;
    }

    return messageType;
}

/*
    - Setter for timed out vehicle
*/
void PriorityRequestGeneratorServer::setTimedOutVehicleID(int vehicleId)
{
    timedOutVehicleID = vehicleId;
}

/*
    - Getter for timed out vehicle
*/
int PriorityRequestGeneratorServer::getTimedOutVehicleID()
{
    return timedOutVehicleID;
}

/*
    - Method to obtain current time
*/
double PriorityRequestGeneratorServer::getCurrentTimeInSeconds()
{
    double currentTime = getPosixTimestamp();
    ;

    return currentTime;
}

/*
    - Method for printing the PRG server list
*/
void PriorityRequestGeneratorServer::printPRGServerList()
{
    double timeStamp = getPosixTimestamp();
    if (!PRGServerList.empty())
    {
        for (size_t i = 0; i < PRGServerList.size(); i++)
            cout << PRGServerList[i].vehicleID << " " << PRGServerList[i].vehicleType << " " << PRGServerList[i].updateTime << endl;
    }
    else
        cout << "[" << fixed << showpoint << setprecision(2) << timeStamp << "] Active Request Table is empty" << endl;
}

PriorityRequestGeneratorServer::~PriorityRequestGeneratorServer()
{
}