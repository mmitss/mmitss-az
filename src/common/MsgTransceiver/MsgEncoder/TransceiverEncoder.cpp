/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  TransceiverEncoder.cpp
  Created by: Debashis Das & Niraj Altekar
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. 
*/
#include "TransceiverEncoder.h"
#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"
#include "json/json.h"
#include "BasicVehicle.h"
#include "SignalRequest.h"
#include "SignalStatus.h"
#include "Timestamp.h"

const double MPS_TO_KPH_CONVERSION = 3.6;

const int RED = 3;
const int YELLOW = 8;
const int GREEN = 6;
const int PERMISSIVE = 7;
const int DONOTWALK = 3;
const int PEDCLEAR = 8;
const int WALK = 6;

TransceiverEncoder::TransceiverEncoder()
{
    ofstream outputfile;
    Json::Value jsonObject;
    ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);        
    delete reader;

    applicationPlatform = (jsonObject["ApplicationPlatform"]).asString();
    intersectionName = jsonObject["IntersectionName"].asString();
    // set the time interval for logging the system performance data
    timeInterval = (jsonObject["SystemPerformanceTimeInterval"]).asDouble();
    auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    msgSentTime = static_cast<int>(currentTime);
}

int TransceiverEncoder::getMessageType(string jsonString)
{
    Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;

    if ((jsonObject["MsgType"]).asString() == "MAP")
        messageType = MsgEnum::DSRCmsgID_map;

    else if ((jsonObject["MsgType"]).asString() == "BSM")
        messageType = MsgEnum::DSRCmsgID_bsm;

    else if ((jsonObject["MsgType"]).asString() == "SRM")
        messageType = MsgEnum::DSRCmsgID_srm;

    else if ((jsonObject["MsgType"]).asString() == "SPaT")
        messageType = MsgEnum::DSRCmsgID_spat;

    else if ((jsonObject["MsgType"]).asString() == "SSM")
        messageType = MsgEnum::DSRCmsgID_ssm;

    else
        messageType = MsgEnum::DSRCmsgID_unknown;

    return messageType;
}

string TransceiverEncoder::TransceiverEncoder::BSMEncoder(string jsonString)
{
    BasicVehicle basicVehicle;
    stringstream payloadstream{};
    string bsmMessagePayload;
    /// buffer to hold message payload
    size_t bufSize = DsrcConstants::maxMsgSize;
    vector<uint8_t> buf(bufSize, 0);
    basicVehicle.json2BasicVehicle(jsonString);
    /// dsrcFrameIn to store input to UPER encoding function
    Frame_element_t dsrcFrameIn;
    dsrcFrameIn.reset();

    /// manual input bsmIn
    dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_bsm;
    BSM_element_t &bsmIn = dsrcFrameIn.bsm;
    bsmIn.msgCnt = 1;
    bsmIn.id = basicVehicle.getTemporaryID();
    bsmIn.timeStampSec = static_cast<int16_t>(basicVehicle.getSecMark_Second());
    bsmIn.latitude = DsrcConstants::unit2damega<int32_t>(basicVehicle.getLatitude_DecimalDegree());
    bsmIn.longitude = DsrcConstants::unit2damega<int32_t>(basicVehicle.getLongitude_DecimalDegree());
    bsmIn.elevation = DsrcConstants::unit2deca<int32_t>(basicVehicle.getElevation_Meter());
    bsmIn.yawRate = 0;
    bsmIn.vehLen = 1200;
    bsmIn.vehWidth = 300;
    bsmIn.speed = DsrcConstants::kph2unit<uint16_t>(basicVehicle.getSpeed_MeterPerSecond() * MPS_TO_KPH_CONVERSION);
    bsmIn.heading = DsrcConstants::heading2unit<uint16_t>(basicVehicle.getHeading_Degree());

    /// encode BSM payload
    size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);
    if (payload_size > 0)
    {
        for (size_t i = 0; i < payload_size; i++)
            payloadstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(buf[i]);
    }

    bsmMessagePayload = payloadstream.str();
    bsmMsgCount = bsmMsgCount + 1;

    return bsmMessagePayload;
}

string TransceiverEncoder::TransceiverEncoder::SRMEncoder(string jsonString)
{
    SignalRequest signalRequest;
    stringstream payloadstream{};
    string srmMessagePayload{};
    size_t bufSize = DsrcConstants::maxMsgSize;
    vector<uint8_t> buf(bufSize, 0);
    signalRequest.json2SignalRequest(jsonString);

    /// dsrcFrameIn to store input to UPER encoding function
    Frame_element_t dsrcFrameIn;
    dsrcFrameIn.reset();

    /// manual input srmIn
    dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_srm;
    SRM_element_t &srmIn = dsrcFrameIn.srm;
    srmIn.timeStampMinute = signalRequest.getMinuteOfYear();
    srmIn.timeStampSec = static_cast<int16_t>(signalRequest.getMsOfMinute());
    srmIn.msgCnt = static_cast<int8_t>(signalRequest.getMsgCount());
    srmIn.regionalId = static_cast<int16_t>(signalRequest.getRegionalID());
    srmIn.intId = static_cast<int16_t>(signalRequest.getIntersectionID());
    srmIn.reqId = static_cast<int8_t>(signalRequest.getRegionalID());
    srmIn.inApprochId = static_cast<int8_t>(signalRequest.getInBoundApproachID());
    srmIn.inLaneId = static_cast<int8_t>(signalRequest.getInBoundLaneID());
    srmIn.ETAsec = static_cast<int16_t>(signalRequest.getETA_Second());
    srmIn.ETAminute = static_cast<uint32_t>(signalRequest.getETA_Minute());
    srmIn.duration = static_cast<int16_t>(signalRequest.getETA_Duration());
    srmIn.vehId = signalRequest.getTemporaryVehicleID();
    srmIn.latitude = DsrcConstants::unit2damega<int32_t>(signalRequest.getLatitude_DecimalDegree());
    srmIn.longitude = DsrcConstants::unit2damega<int32_t>(signalRequest.getLongitude_DecimalDegree());
    srmIn.elevation = DsrcConstants::unit2deca<int32_t>(signalRequest.getElevation_Meter());
    srmIn.heading = DsrcConstants::heading2unit<uint16_t>(signalRequest.getHeading_Degree());
    srmIn.speed = DsrcConstants::kph2unit<uint16_t>(signalRequest.getSpeed_MeterPerSecond() * MPS_TO_KPH_CONVERSION);
    srmIn.reqType = static_cast<MsgEnum::requestType>(signalRequest.getPriorityRequestType());
    srmIn.vehRole = static_cast<MsgEnum::basicRole>(signalRequest.getBasicVehicleRole());
    // srmIn.vehType = static_cast<MsgEnum::vehicleType>(signalRequest.getVehicleType());
    /// encode SRM payload
    size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);
    if (payload_size > 0)
    {
        for (size_t i = 0; i < payload_size; i++)
            payloadstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(buf[i]);
    }

    srmMessagePayload = payloadstream.str();
    srmMsgCount = srmMsgCount + 1;

    return srmMessagePayload;
}

string TransceiverEncoder::SPaTEncoder(string jsonString)
{
    Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;

    size_t bufSize = DsrcConstants::maxMsgSize;
    vector<uint8_t> buf(bufSize, 0);
    Frame_element_t dsrcFrameIn;
    dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_spat;
    SPAT_element_t &spatIn = dsrcFrameIn.spat;
    stringstream payloadstream{};
    string spatMessagePayload{};

    string phaseState{};
    string pedPhaseState{};

    spatIn.regionalId = static_cast<uint16_t>(jsonObject["Spat"]["IntersectionState"]["regionalID"].asInt());
    spatIn.id = static_cast<uint16_t>(jsonObject["Spat"]["IntersectionState"]["intersectionID"].asInt());
    spatIn.msgCnt = static_cast<uint8_t>(jsonObject["Spat"]["msgCnt"].asInt());
    spatIn.timeStampMinute = jsonObject["Spat"]["minuteOfYear"].asInt();
    spatIn.timeStampSec = static_cast<uint16_t>(jsonObject["Spat"]["msOfMinute"].asInt());
    std::bitset<16> intersectionStatus(jsonObject["Spat"]["status"].asString());
    spatIn.status = intersectionStatus;

    // Develop vehicle phases
    for(unsigned int i=0; i<jsonObject["Spat"]["phaseState"].size(); i++)
    {
        int phaseNo = jsonObject["Spat"]["phaseState"][i]["phaseNo"].asInt();
        int phaseIndex = phaseNo-1;
        spatIn.permittedPhases.set(phaseIndex); // enable the current phase
        
        spatIn.phaseState[phaseIndex].startTime = static_cast<uint16_t>(jsonObject["Spat"]["phaseState"][i]["startTime"].asInt());
        spatIn.phaseState[phaseIndex].minEndTime = static_cast<uint16_t>(jsonObject["Spat"]["phaseState"][i]["minEndTime"].asInt());
        spatIn.phaseState[phaseIndex].maxEndTime = static_cast<uint16_t>(jsonObject["Spat"]["phaseState"][i]["maxEndTime"].asInt());
        phaseState = jsonObject["Spat"]["phaseState"][i]["currState"].asString();
        if (phaseState == "red")
            spatIn.phaseState[phaseIndex].currState = static_cast<MsgEnum::phaseState>(RED);
        else if (phaseState == "yellow")
            spatIn.phaseState[phaseIndex].currState = static_cast<MsgEnum::phaseState>(YELLOW);
        else if (phaseState == "green")
            spatIn.phaseState[phaseIndex].currState = static_cast<MsgEnum::phaseState>(GREEN);
        else if (phaseState == "permissive_yellow")
            spatIn.phaseState[phaseIndex].currState = static_cast<MsgEnum::phaseState>(PERMISSIVE);
        
    }
    // Develop pedestrian phases
    for(unsigned int i=0; i<jsonObject["Spat"]["pedPhaseState"].size(); i++)
    {
        int pedPhaseNo = jsonObject["Spat"]["pedPhaseState"][i]["phaseNo"].asInt();
        int pedPhaseIndex = 0;
        if (pedPhaseNo == 2)
        {
            pedPhaseIndex = 4;
        }
        else if (pedPhaseNo == 4)
        {
            pedPhaseIndex = 5;
        }
        else if (pedPhaseNo == 6)
        {
            pedPhaseIndex = 6;
        }
        else if (pedPhaseNo == 8)
        {
            pedPhaseIndex = 7;
        }
        spatIn.permittedPedPhases.set(pedPhaseIndex); // enable the current phase
        
        spatIn.pedPhaseState[pedPhaseIndex].startTime = static_cast<uint16_t>(jsonObject["Spat"]["pedPhaseState"][i]["startTime"].asInt());
        spatIn.pedPhaseState[pedPhaseIndex].minEndTime = static_cast<uint16_t>(jsonObject["Spat"]["pedPhaseState"][i]["minEndTime"].asInt());
        spatIn.pedPhaseState[pedPhaseIndex].maxEndTime = static_cast<uint16_t>(jsonObject["Spat"]["pedPhaseState"][i]["maxEndTime"].asInt());
        pedPhaseState = jsonObject["Spat"]["pedPhaseState"][i]["currState"].asString();
        if (pedPhaseState == "do_not_walk")
            spatIn.pedPhaseState[pedPhaseIndex].currState = static_cast<MsgEnum::phaseState>(DONOTWALK);
        else if (pedPhaseState == "ped_clear")
            spatIn.pedPhaseState[pedPhaseIndex].currState = static_cast<MsgEnum::phaseState>(PEDCLEAR);
        else if (pedPhaseState == "walk")
            spatIn.pedPhaseState[pedPhaseIndex].currState = static_cast<MsgEnum::phaseState>(WALK);        
    }


    size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);

    if (payload_size > 0)
    {
        for (size_t i = 0; i < payload_size; i++)
            payloadstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(buf[i]);
    }
    spatMessagePayload = payloadstream.str();

    spatMsgCount = spatMsgCount + 1;

    return spatMessagePayload;
}

string TransceiverEncoder::SSMEncoder(string jsonString)
{
    SignalStatus signalStatus;
    stringstream payloadstream{};
    string ssmMessagePayload{};
    size_t bufSize = DsrcConstants::maxMsgSize;
    vector<uint8_t> buf(bufSize, 0);
    signalStatus.json2SignalStatus(jsonString);

    vector<int> vehicleID = signalStatus.getTemporaryVehicleID();
    vector<int> requestID = signalStatus.getRequestID();
    vector<int> msgCount = signalStatus.getMsgCount();
    vector<int> inBoundLaneID = signalStatus.getInBoundLaneID();
    vector<int> inBoundApproachID = signalStatus.getInBoundApproachID();
    vector<int> basicVehicleRole = signalStatus.getBasicVehicleRole();
    vector<int> expectedTimeOfArrival_Minute = signalStatus.getETA_Minute();
    vector<int> expectedTimeOfArrival_Second = signalStatus.getETA_Second();
    vector<int> expectedTimeOfArrival_Duration = signalStatus.getETA_Duration();
    vector<int> priorityRequestStatus = signalStatus.getPriorityRequestStatus();

    /// dsrcFrameIn to store input to UPER encoding function
    Frame_element_t dsrcFrameIn;
    dsrcFrameIn.reset();

    //Get the values of the variables
    dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_ssm;
    SSM_element_t &ssmIn = dsrcFrameIn.ssm;
    ssmIn.timeStampMinute = signalStatus.getMinuteOfYear();
    ssmIn.timeStampSec = static_cast<int16_t>(signalStatus.getMsOfMinute());
    ssmIn.msgCnt = static_cast<int8_t>(signalStatus.getPRSSequenceNumber());
    ssmIn.updateCnt = static_cast<int8_t>(signalStatus.getPRSUpdateCount());
    ssmIn.regionalId = static_cast<int16_t>(signalStatus.getRegionalID());
    ssmIn.id = static_cast<int16_t>(signalStatus.getIntersectionID());

    for (int i = 0; i < signalStatus.getNoOfRequest(); i++)
    {
        SignalRequetStatus_t requestStatus;
        requestStatus.reset();
        requestStatus.vehId = vehicleID[i];
        requestStatus.reqId = static_cast<int8_t>(requestID[i]);
        requestStatus.sequenceNumber = static_cast<int8_t>(msgCount[i]);
        requestStatus.vehRole = static_cast<MsgEnum::basicRole>((basicVehicleRole[i]));
        requestStatus.inLaneId = static_cast<int8_t>(inBoundLaneID[i]);
        requestStatus.inApprochId = static_cast<int8_t>(inBoundApproachID[i]); //It is not present in TestDecoder
        requestStatus.ETAminute = static_cast<uint32_t>(expectedTimeOfArrival_Minute[i]);
        requestStatus.ETAsec = static_cast<int16_t>(expectedTimeOfArrival_Second[i]);
        requestStatus.duration = static_cast<int16_t>(expectedTimeOfArrival_Duration[i]);
        requestStatus.status = static_cast<MsgEnum::requestStatus>(priorityRequestStatus[i]);

        ssmIn.mpSignalRequetStatus.push_back(requestStatus);
    }
    /// encode SSM payload
    size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);

    if (payload_size > 0)
    {
        for (size_t i = 0; i < payload_size; i++)
            payloadstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(buf[i]);
    }
    ssmMessagePayload = payloadstream.str();
    ssmMsgCount = ssmMsgCount + 1;

    return ssmMessagePayload;
}

bool TransceiverEncoder::sendSystemPerformanceDataLog()
{
    bool sendData{false};
    double currentTime{};
    currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

    if (currentTime - msgSentTime >= timeInterval)
        sendData = true;

    return sendData;
}

string TransceiverEncoder::createJsonStringForSystemPerformanceDataLog(string msgCountType)
{
    string systemPerformanceDataLogJsonString{};
    Json::Value jsonObject;
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "";
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

    jsonObject["MsgType"] = "MsgCount";
    jsonObject["MsgInformation"]["TimeInterval"] = timeInterval;
    jsonObject["MsgInformation"]["Timestamp_posix"] = getPosixTimestamp();
    jsonObject["MsgInformation"]["Timestamp_verbose"] = getVerboseTimestamp();
    jsonObject["MsgInformation"]["MsgCountType"] = msgCountType;
    
    if (applicationPlatform == "roadside")
    {

        jsonObject["MsgInformation"]["MsgSource"] = intersectionName;
        jsonObject["MsgInformation"]["MsgServed"] = "NA";
        jsonObject["MsgInformation"]["MsgRejected"] = "NA";
    }

    else if (applicationPlatform == "vehicle")
        jsonObject["MsgInformation"]["MsgSource"] = "vehicle";

    if (msgCountType == "HostBSM")
    {
        jsonObject["MsgInformation"]["MsgCount"] = bsmMsgCount;
        bsmMsgCount = 0;
    }

    else if (msgCountType == "SRM")
    {
        jsonObject["MsgInformation"]["MsgCount"] = srmMsgCount;
        srmMsgCount = 0;
    }

    else if (msgCountType == "SSM")
    {
        jsonObject["MsgInformation"]["MsgCount"] = ssmMsgCount;
        ssmMsgCount = 0;
    }

    else if (msgCountType == "MAP")
    {
        jsonObject["MsgInformation"]["MsgCount"] = mapMsgCount;
        mapMsgCount = 0;
    }

    else if (msgCountType == "SPaT")
    {
        jsonObject["MsgInformation"]["MsgCount"] = spatMsgCount;
        spatMsgCount = 0;
    }



    systemPerformanceDataLogJsonString = Json::writeString(builder, jsonObject);

    msgSentTime = static_cast<int>(currentTime);

    return systemPerformanceDataLogJsonString;
}

string TransceiverEncoder::getApplicationPlatform()
{
    return applicationPlatform;
}

void TransceiverEncoder::setMapMsgCount(int msgCount)
{
    mapMsgCount = msgCount;
}


TransceiverEncoder::~TransceiverEncoder()
{
}
