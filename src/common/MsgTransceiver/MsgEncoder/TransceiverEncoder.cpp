#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>
#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"
#include "json/json.h"
#include "BasicVehicle.h"
#include "SignalRequest.h"
#include "SignalStatus.h"
#include "TransceiverEncoder.h"

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
    std::ofstream outputfile;
    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

    std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);
    applicationPlatform = (jsonObject["ApplicationPlatform"]).asString();
    intersectionName = jsonObject["IntersectionName"].asString();
    // set the time interval for logging the system performance data
    timeInterval = (jsonObject["SystemPerformanceTimeInterval"]).asDouble();
    auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    msgSentTime = static_cast<int>(currentTime);
}

int TransceiverEncoder::getMessageType(std::string jsonString)
{
    Json::Value jsonObject;
    Json::Reader reader;
    reader.parse(jsonString.c_str(), jsonObject);

    if ((jsonObject["MsgType"]).asString() == "MAP")
    {
        messageType = MsgEnum::DSRCmsgID_map;
    }

    else if ((jsonObject["MsgType"]).asString() == "BSM")
    {
        messageType = MsgEnum::DSRCmsgID_bsm;
    }

    else if ((jsonObject["MsgType"]).asString() == "SRM")
    {
        messageType = MsgEnum::DSRCmsgID_srm;
    }

    else if ((jsonObject["MsgType"]).asString() == "SPaT")
    {
        messageType = MsgEnum::DSRCmsgID_spat;
    }

    else if ((jsonObject["MsgType"]).asString() == "SSM")
    {
        messageType = MsgEnum::DSRCmsgID_ssm;
    }

    else
    {
        messageType = MsgEnum::DSRCmsgID_unknown;
    }

    return messageType;
}

std::string TransceiverEncoder::TransceiverEncoder::BSMEncoder(std::string jsonString)
{
    BasicVehicle basicVehicle;
    std::stringstream payloadstream;
    std::string bsmMessagePayload;
    /// buffer to hold message payload
    size_t bufSize = DsrcConstants::maxMsgSize;
    std::vector<uint8_t> buf(bufSize, 0);
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
        {
            payloadstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(buf[i]);
        }
    }

    bsmMessagePayload = payloadstream.str();
    bsmMsgCount = bsmMsgCount + 1;

    return bsmMessagePayload;
}

std::string TransceiverEncoder::TransceiverEncoder::SRMEncoder(std::string jsonString)
{
    SignalRequest signalRequest;
    std::stringstream payloadstream;
    std::string srmMessagePayload;
    size_t bufSize = DsrcConstants::maxMsgSize;
    std::vector<uint8_t> buf(bufSize, 0);
    signalRequest.json2SignalRequest(jsonString);

    /// dsrcFrameIn to store input to UPER encoding function
    Frame_element_t dsrcFrameIn;
    dsrcFrameIn.reset();

    /// manual input ssmIn
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
    srmIn.ETAminute = signalRequest.getETA_Minute();
    srmIn.duration = static_cast<int16_t>(signalRequest.getETA_Duration());
    srmIn.vehId = signalRequest.getTemporaryVehicleID();
    srmIn.latitude = DsrcConstants::unit2damega<int32_t>(signalRequest.getLatitude_DecimalDegree());
    srmIn.longitude = DsrcConstants::unit2damega<int32_t>(signalRequest.getLongitude_DecimalDegree());
    srmIn.elevation = DsrcConstants::unit2deca<int32_t>(signalRequest.getElevation_Meter());
    srmIn.heading = DsrcConstants::heading2unit<uint16_t>(signalRequest.getHeading_Degree());
    srmIn.speed = DsrcConstants::kph2unit<uint16_t>(signalRequest.getSpeed_MeterPerSecond() * MPS_TO_KPH_CONVERSION);
    srmIn.reqType = static_cast<MsgEnum::requestType>(signalRequest.getPriorityRequestType());
    srmIn.vehRole = static_cast<MsgEnum::basicRole>(signalRequest.getBasicVehicleRole());
    srmIn.vehType = static_cast<MsgEnum::vehicleType>(signalRequest.getVehicleType());
    /// encode SSM payload
    size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);
    if (payload_size > 0)
    {
        for (size_t i = 0; i < payload_size; i++)
        {
            payloadstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(buf[i]);
        }
    }

    srmMessagePayload = payloadstream.str();
    srmMsgCount = srmMsgCount + 1;

    return srmMessagePayload;
}

std::string TransceiverEncoder::SPaTEncoder(std::string jsonString)
{
    Json::Value jsonObject;
    Json::Reader reader;

    reader.parse(jsonString.c_str(), jsonObject);

    size_t bufSize = DsrcConstants::maxMsgSize;
    std::vector<uint8_t> buf(bufSize, 0);
    Frame_element_t dsrcFrameIn;
    dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_spat;
    SPAT_element_t &spatIn = dsrcFrameIn.spat;
    std::stringstream payloadstream;
    std::string spatMessagePayload;

    std::string phaseState{};
    std::string pedPhaseState{};

    spatIn.regionalId = jsonObject["Spat"]["IntersectionState"]["regionalID"].asInt();
    spatIn.id = jsonObject["Spat"]["IntersectionState"]["intersectionID"].asInt();
    spatIn.msgCnt = jsonObject["Spat"]["msgCnt"].asInt();
    spatIn.timeStampMinute = jsonObject["Spat"]["minuteOfYear"].asInt();
    spatIn.timeStampSec = jsonObject["Spat"]["msOfMinute"].asInt();
    std::bitset<16> intersectionStatus(jsonObject["Spat"]["status"].asString());
    spatIn.status = intersectionStatus;
    spatIn.permittedPhases.set(); // all 8 phases permitted
    spatIn.permittedPedPhases.set();

    for (int i = 0; i < 8; i++)
    {
        spatIn.phaseState[i].startTime = jsonObject["Spat"]["phaseState"][i]["startTime"].asInt();
        spatIn.phaseState[i].minEndTime = jsonObject["Spat"]["phaseState"][i]["minEndTime"].asInt();
        spatIn.phaseState[i].maxEndTime = jsonObject["Spat"]["phaseState"][i]["maxEndTime"].asInt();
        phaseState = jsonObject["Spat"]["phaseState"][i]["currState"].asString();
        if (phaseState == "red")
            spatIn.phaseState[i].currState = static_cast<MsgEnum::phaseState>(RED);
        else if (phaseState == "yellow")
            spatIn.phaseState[i].currState = static_cast<MsgEnum::phaseState>(YELLOW);
        else if (phaseState == "green")
            spatIn.phaseState[i].currState = static_cast<MsgEnum::phaseState>(GREEN);
        else if (phaseState == "permissive_yellow")
            spatIn.phaseState[i].currState = static_cast<MsgEnum::phaseState>(PERMISSIVE);

        spatIn.pedPhaseState[i].startTime = jsonObject["Spat"]["pedPhaseState"][i]["startTime"].asInt();
        spatIn.pedPhaseState[i].minEndTime = jsonObject["Spat"]["pedPhaseState"][i]["minEndTime"].asInt();
        spatIn.pedPhaseState[i].maxEndTime = jsonObject["Spat"]["pedPhaseState"][i]["maxEndTime"].asInt();
        pedPhaseState = jsonObject["Spat"]["pedPhaseState"][i]["currState"].asString();
        if (pedPhaseState == "do_not_walk")
            spatIn.pedPhaseState[i].currState = static_cast<MsgEnum::phaseState>(DONOTWALK);
        else if (pedPhaseState == "ped_clear")
            spatIn.pedPhaseState[i].currState = static_cast<MsgEnum::phaseState>(PEDCLEAR);
        else if (pedPhaseState == "walk")
            spatIn.pedPhaseState[i].currState = static_cast<MsgEnum::phaseState>(WALK);
    }
    size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);

    if (payload_size > 0)
    {
        for (size_t i = 0; i < payload_size; i++)
        {
            payloadstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(buf[i]);
        }
    }
    spatMessagePayload = payloadstream.str();

    spatMsgCount = spatMsgCount + 1;

    return spatMessagePayload;
}

std::string TransceiverEncoder::SSMEncoder(std::string jsonString)
{
    SignalStatus signalStatus;
    std::stringstream payloadstream;
    std::string ssmMessagePayload;
    size_t bufSize = DsrcConstants::maxMsgSize;
    std::vector<uint8_t> buf(bufSize, 0);
    signalStatus.json2SignalStatus(jsonString);

    std::vector<int> vehicleID = signalStatus.getTemporaryVehicleID();
    std::vector<int> requestID = signalStatus.getRequestID();
    std::vector<int> msgCount = signalStatus.getMsgCount();
    std::vector<int> inBoundLaneID = signalStatus.getInBoundLaneID();
    std::vector<int> inBoundApproachID = signalStatus.getInBoundApproachID();
    std::vector<int> basicVehicleRole = signalStatus.getBasicVehicleRole();
    std::vector<int> expectedTimeOfArrival_Minute = signalStatus.getETA_Minute();
    std::vector<double> expectedTimeOfArrival_Second = signalStatus.getETA_Second();
    std::vector<double> expectedTimeOfArrival_Duration = signalStatus.getETA_Duration();
    std::vector<int> priorityRequestStatus = signalStatus.getPriorityRequestStatus();

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
        {
            payloadstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(buf[i]);
        }
    }
    ssmMessagePayload = payloadstream.str();
    ssmMsgCount = ssmMsgCount + 1;

    return ssmMessagePayload;
}

bool TransceiverEncoder::sendSystemPerformanceDataLog()
{
    bool sendData{false};
    auto currenTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    if (currenTime - msgSentTime >= timeInterval)
        sendData = true;

    return sendData;
}

std::string TransceiverEncoder::createJsonStringForSystemPerformanceDataLog(std::string msgCountType)
{
    std::string systemPerformanceDataLogJsonString{};
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    Json::StyledStreamWriter styledStreamWriter;
    std::ofstream outputter("systemPerformanceDataLog.json");
    auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    if (applicationPlatform == "roadside")
    {
        jsonObject["MsgType"] = "IntersectionDataLog";
        jsonObject["MsgInformation"]["MsgSource"] = intersectionName;
        jsonObject["MsgInformation"]["MsgServed"] = "NA";
        jsonObject["MsgInformation"]["MsgRejected"] = "NA";
    }

    else if (applicationPlatform == "vehicle")
    {
        jsonObject["MsgType"] = "VehicleDataLog";
        jsonObject["MsgInformation"]["MsgSource"] = "vehicle";
    }

    jsonObject["MsgInformation"]["MsgCountType"] = msgCountType;

    if (msgCountType == "HostBSM")
        jsonObject["MsgInformation"]["MsgCount"] = bsmMsgCount;

    else if (msgCountType == "SRM")
        jsonObject["MsgInformation"]["MsgCount"] = srmMsgCount;

    else if (msgCountType == "SSM")
        jsonObject["MsgInformation"]["MsgCount"] = ssmMsgCount;

    else if (msgCountType == "MAP")
        jsonObject["MsgInformation"]["MsgCount"] = mapMsgCount;

    else if (msgCountType == "SPaT")
        jsonObject["MsgInformation"]["MsgCount"] = spatMsgCount;

    jsonObject["MsgInformation"]["TimeInterval"] = timeInterval;
    jsonObject["MsgInformation"]["MsgSentTime"] = static_cast<int>(currentTime);

    systemPerformanceDataLogJsonString = fastWriter.write(jsonObject);
    styledStreamWriter.write(outputter, jsonObject);

    msgSentTime = static_cast<int>(currentTime);

    return systemPerformanceDataLogJsonString;
}

std::string TransceiverEncoder::getApplicationPlatform()
{
    return applicationPlatform;
}

TransceiverEncoder::~TransceiverEncoder()
{
}
