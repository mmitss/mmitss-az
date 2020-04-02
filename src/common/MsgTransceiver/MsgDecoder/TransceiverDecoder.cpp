#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cstring>
#include <string.h>

#include "AsnJ2735Lib.h"
#include "locAware.h"
#include "dsrcConsts.h"
#include "json/json.h"
#include "BasicVehicle.h"
#include "SignalRequest.h"
#include "SignalStatus.h"
#include "ActiveRequest.h"
#include "TransceiverDecoder.h"
#include "Timestamp.h"

const double KPH_TO_MPS_CONVERSION = 0.277778;
const int RED = 3;
const int YELLOW = 8;
const int GREEN = 6;
const int PERMISSIVE = 7;
const int DONOTWALK = 3;
const int PEDCLEAR = 8;
const int WALK = 6;

TransceiverDecoder::TransceiverDecoder()
{
}

int TransceiverDecoder::getMessageType(std::string payload)
{
    std::string subPayload = payload.substr(0, 4);

    std::vector<std::string> MessageIdentifier;

    MessageIdentifier = {MAPIdentifier, BSMIdentifier, SRMIdentifier_UpperCase, SRMIdentifier_LowerCase, SPaTIdentifier, SSMIdentifier_UpperCase, SSMIdentifier_LowerCase};

    if (MessageIdentifier.at(0).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_map;
    }

    else if (MessageIdentifier.at(1).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_bsm;
    }

    else if (MessageIdentifier.at(2).compare(subPayload) == 0 || MessageIdentifier.at(3).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_srm;
    }

    else if (MessageIdentifier.at(4).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_spat;
    }
    else if (MessageIdentifier.at(5).compare(subPayload) == 0 || MessageIdentifier.at(6).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_ssm;
    }
    return messageType;
}

std::string TransceiverDecoder::createJsonStingOfMapPayload(std::string mapPayload)
{
    std::ofstream outputfile;
    std::string fmap{};
    std::string intersectionName{};
    std::string mapName{};
    int intersectionID{};
    bool singleFrame = false;
    std::string deleteFileName = "Map.map.payload";
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    std::string jsonString{};
    Json::Value jsonObject_config;
    Json::Reader reader;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());

    reader.parse(configJsonString.c_str(), jsonObject_config);
    outputfile.open("Map.map.payload");
    outputfile << "payload"
               << " "
               << "Map"
               << " " << mapPayload << std::endl;
    outputfile.close();

    fmap = "Map.map.payload";
    intersectionName = "Map";

    /// instance class LocAware (Map Engine)
    LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
    intersectionID = plocAwareLib->getIntersectionIdByName(intersectionName);
    mapName = "Map" + std::to_string(intersectionID);
    jsonObject["MsgType"] = "MAP";
    jsonObject["IntersectionName"] = mapName;
    jsonObject["MapPayload"] = mapPayload;
    jsonObject["IntersectionID"] = intersectionID;
    jsonString = fastWriter.write(jsonObject);

    remove(deleteFileName.c_str());
    delete plocAwareLib;

    return jsonString;
}

std::string TransceiverDecoder::bsmDecoder(std::string bsmPayload)
{
    BasicVehicle basicVehicle;
    std::string jsonString{};

    /// buffer to hold message payload
    size_t bufSize = DsrcConstants::maxMsgSize;
    std::vector<uint8_t> buf(bufSize, 0);
    /// dsrcFrameOut to store UPER decoding result
    Frame_element_t dsrcFrameOut;

    std::string output;
    size_t cnt = bsmPayload.length() / 2;

    for (size_t i = 0; cnt > i; ++i)
    {
        uint32_t s = 0;
        std::stringstream ss;
        ss << std::hex << bsmPayload.substr(i * 2, 2);
        ss >> s;
        output.push_back(static_cast<unsigned char>(s));
    }

    size_t index = 0;
    for (std::vector<uint8_t>::iterator it = buf.begin(); it != buf.end() && index < output.size(); ++it)
    {
        *it = output[index];
        index++;
    }
    size_t payload_size = output.size();
    if (payload_size > 0 && (AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0) && (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_bsm))
    {
        BSM_element_t &bsmOut = dsrcFrameOut.bsm;
        basicVehicle.setTemporaryID(bsmOut.id);
        basicVehicle.setSecMark_Second((bsmOut.timeStampSec) / 1000.0);
        basicVehicle.setPosition(DsrcConstants::damega2unit<int32_t>(bsmOut.latitude), DsrcConstants::damega2unit<int32_t>(bsmOut.longitude), DsrcConstants::deca2unit<int32_t>(bsmOut.elevation));
        basicVehicle.setSpeed_MeterPerSecond(round(DsrcConstants::unit2kph<uint16_t>(bsmOut.speed) * KPH_TO_MPS_CONVERSION));
        basicVehicle.setHeading_Degree(round(DsrcConstants::unit2heading<uint16_t>(bsmOut.heading)));
        basicVehicle.setType(0);
        basicVehicle.setLength_cm(bsmOut.vehLen);
        basicVehicle.setWidth_cm(bsmOut.vehWidth);
        jsonString = basicVehicle.basicVehicle2Json();
    }

    return jsonString;
}

std::string TransceiverDecoder::srmDecoder(std::string srmPayload)
{
    SignalRequest signalRequest;

    std::string jsonString{};

    /// buffer to hold message payload
    size_t bufSize = DsrcConstants::maxMsgSize;
    std::vector<uint8_t> buf(bufSize, 0);
    /// dsrcFrameOut to store UPER decoding result
    Frame_element_t dsrcFrameOut;

    std::string output;
    size_t cnt = srmPayload.length() / 2;

    for (size_t i = 0; cnt > i; ++i)
    {
        uint32_t s = 0;
        std::stringstream ss;
        ss << std::hex << srmPayload.substr(i * 2, 2);
        ss >> s;

        output.push_back(static_cast<unsigned char>(s));
    }

    size_t index = 0;
    for (std::vector<uint8_t>::iterator it = buf.begin(); it != buf.end() && index < output.size(); ++it)
    {
        *it = output[index];
        index++;
    }
    size_t payload_size = output.size();

    if (payload_size > 0 && (AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0) && (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_srm))
    {
        SRM_element_t &srmOut = dsrcFrameOut.srm;

        signalRequest.setMsgCount(static_cast<unsigned int>(srmOut.msgCnt));
        signalRequest.setMinuteOfYear(srmOut.timeStampMinute);
        signalRequest.setMsOfMinute(srmOut.timeStampSec);
        signalRequest.setRegionalID(srmOut.regionalId);
        signalRequest.setIntersectionID(srmOut.intId);
        signalRequest.setRequestID(static_cast<unsigned int>(srmOut.reqId));
        signalRequest.setPriorityRequestType(static_cast<unsigned int>(srmOut.reqType));
        signalRequest.setInBoundLaneIntersectionAccessPoint(static_cast<unsigned int>(srmOut.inLaneId), static_cast<unsigned int>(srmOut.inApprochId));
        signalRequest.setETA(srmOut.ETAminute, srmOut.ETAsec, srmOut.duration);
        signalRequest.setTemporaryVechileID(srmOut.vehId);
        signalRequest.setBasicVehicleRole(static_cast<unsigned int>(srmOut.vehRole));
        signalRequest.setPosition(DsrcConstants::damega2unit<int32_t>(srmOut.latitude), DsrcConstants::damega2unit<int32_t>(srmOut.longitude), DsrcConstants::deca2unit<int32_t>(srmOut.elevation));
        signalRequest.setHeading_Degree(round(DsrcConstants::unit2heading<uint16_t>(srmOut.heading)));
        signalRequest.setSpeed_MeterPerSecond(round(DsrcConstants::unit2kph<uint16_t>(srmOut.speed) * KPH_TO_MPS_CONVERSION));
        signalRequest.setVehicleType(static_cast<unsigned int>(srmOut.vehType));

        jsonString = signalRequest.signalRequest2Json();
    }

    return jsonString;
}

std::string TransceiverDecoder::ssmDecoder(std::string ssmPayload)
{
    SignalStatus signalStatus;
    std::vector<ActiveRequest> ActiveRequestTable;
    ActiveRequest activeRequest;
    std::string jsonString{};

    /// buffer to hold message payload
    size_t bufSize = DsrcConstants::maxMsgSize;
    std::vector<uint8_t> buf(bufSize, 0);
    /// dsrcFrameOut to store UPER decoding result
    Frame_element_t dsrcFrameOut;

    std::string output;
    size_t cnt = ssmPayload.length() / 2;

    for (size_t i = 0; cnt > i; ++i)
    {
        uint32_t s = 0;
        std::stringstream ss;
        ss << std::hex << ssmPayload.substr(i * 2, 2);
        ss >> s;

        output.push_back(static_cast<unsigned char>(s));
    }

    size_t index = 0;
    for (std::vector<uint8_t>::iterator it = buf.begin(); it != buf.end() && index < output.size(); ++it)
    {
        *it = output[index];
        index++;
    }
    size_t payload_size = output.size();

    if (payload_size > 0 && (AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0) && (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_ssm))
    {
        SSM_element_t &ssmOut = dsrcFrameOut.ssm;

        signalStatus.setMinuteOfYear(ssmOut.timeStampMinute);
        signalStatus.setMsOfMinute(ssmOut.timeStampSec);
        signalStatus.setSequenceNumber(static_cast<unsigned int>(ssmOut.msgCnt));
        signalStatus.setUpdateCount(static_cast<unsigned int>(ssmOut.updateCnt));
        signalStatus.setRegionalID(ssmOut.regionalId);
        signalStatus.setIntersectionID(ssmOut.id);
        signalStatus.setNoOfRequest(unsigned(ssmOut.mpSignalRequetStatus.size()));

        // for (const auto &RequetStatus : ssmOut.mpSignalRequetStatus)
        // {
        //     activeRequest.vehicleID = RequetStatus.vehId;
        //     activeRequest.requestID = static_cast<unsigned int>(RequetStatus.reqId);
        //     activeRequest.msgCount = static_cast<unsigned int>(RequetStatus.sequenceNumber);
        //     activeRequest.basicVehicleRole = static_cast<unsigned int>(RequetStatus.vehRole);
        //     activeRequest.vehicleLaneID = static_cast<unsigned int>(RequetStatus.inLaneId);
        //     activeRequest.vehicleETA = RequetStatus.ETAminute * 60.0 + RequetStatus.ETAsec;
        //     activeRequest.prsStatus = static_cast<unsigned int>(RequetStatus.status);
        //     ActiveRequestTable.push_back(activeRequest);
        //     activeRequest.reset();
        // }
        for (int i=0; i < ssmOut.mpSignalRequetStatus.size(); i++)
        {
            activeRequest.vehicleID = ssmOut.mpSignalRequetStatus[i].vehId;
            activeRequest.requestID = static_cast<unsigned int>(ssmOut.mpSignalRequetStatus[i].reqId);
            activeRequest.msgCount = static_cast<unsigned int>(ssmOut.mpSignalRequetStatus[i].sequenceNumber);
            activeRequest.basicVehicleRole = static_cast<unsigned int>(ssmOut.mpSignalRequetStatus[i].vehRole);
            activeRequest.vehicleLaneID = static_cast<unsigned int>(ssmOut.mpSignalRequetStatus[i].inLaneId);
            activeRequest.vehicleETA = ssmOut.mpSignalRequetStatus[i].ETAminute * 60.0 + ssmOut.mpSignalRequetStatus[i].ETAsec;
            activeRequest.prsStatus = static_cast<unsigned int>(ssmOut.mpSignalRequetStatus[i].status);
            ActiveRequestTable.push_back(activeRequest);
        }

        signalStatus.setTemporaryVechileID(ActiveRequestTable);
        signalStatus.setRequestID(ActiveRequestTable);
        signalStatus.setMsgCount(ActiveRequestTable);
        signalStatus.setBasicVehicleRole(ActiveRequestTable);
        signalStatus.setInBoundLaneIntersectionAccessPoint(ActiveRequestTable);
        signalStatus.setETA(ActiveRequestTable);
        signalStatus.setPriorityRequestStatus(ActiveRequestTable);

        jsonString = signalStatus.signalStatus2Json(ActiveRequestTable);
    }
    return jsonString;
}

std::string TransceiverDecoder::spatDecoder(std::string spatPayload)
{
    std::string jsonString;

    /// buffer to hold message payload
    size_t bufSize = DsrcConstants::maxMsgSize;
    std::vector<uint8_t> buf(bufSize, 0);
    /// dsrcFrameOut to store UPER decoding result
    Frame_element_t dsrcFrameOut;

    std::string output;
    size_t cnt = spatPayload.length() / 2;

    for (size_t i = 0; cnt > i; ++i)
    {
        uint32_t s = 0;
        std::stringstream ss;
        ss << std::hex << spatPayload.substr(i * 2, 2);
        ss >> s;

        output.push_back(static_cast<unsigned char>(s));
    }

    size_t index = 0;
    for (std::vector<uint8_t>::iterator it = buf.begin(); it != buf.end() && index < output.size(); ++it)
    {
        *it = output[index];
        index++;
    }
    size_t payload_size = output.size();

    if (payload_size > 0 && (AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0) && (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_spat))
    {
        SPAT_element_t &spatOut = dsrcFrameOut.spat;

        Json::Value jsonObject;
        Json::FastWriter fastWriter;
        int currVehPhaseState{};
        int currPedPhaseState{}; 

        jsonObject["MsgType"] = "SPaT";
        jsonObject["Timestamp_verbose"] = getVerboseTimestamp();
        jsonObject["Timestamp_posix"] = getVerboseTimestamp();
        jsonObject["Spat"]["IntersectionState"]["regionalID"] = spatOut.regionalId;
        jsonObject["Spat"]["IntersectionState"]["intersectionID"] = spatOut.id;
        jsonObject["Spat"]["msgCnt"] = static_cast<unsigned int>(spatOut.msgCnt);
        jsonObject["Spat"]["minuteOfYear"] = spatOut.timeStampMinute;
        jsonObject["Spat"]["msOfMinute"] = spatOut.timeStampSec;
        jsonObject["Spat"]["status"] = spatOut.status.to_string();

        for (int i = 0; i < 8; i++)
        {
            if (spatOut.permittedPhases.test(i))
            {
                const auto &phaseState = spatOut.phaseState[i];
                jsonObject["Spat"]["phaseState"][i]["phaseNo"] = (i + 1);
                jsonObject["Spat"]["phaseState"][i]["startTime"] = phaseState.startTime;
                jsonObject["Spat"]["phaseState"][i]["minEndTime"] = phaseState.minEndTime;
                jsonObject["Spat"]["phaseState"][i]["maxEndTime"] = phaseState.maxEndTime;
                jsonObject["Spat"]["phaseState"][i]["elapsedTime"] = 0;
                currVehPhaseState = static_cast<unsigned int>(phaseState.currState);
                if(currVehPhaseState == RED)
                    jsonObject["Spat"]["phaseState"][i]["currState"] = "red";
                else if(currVehPhaseState == YELLOW)
                    jsonObject["Spat"]["phaseState"][i]["currState"] = "yellow";
                else if(currVehPhaseState == GREEN)
                    jsonObject["Spat"]["phaseState"][i]["currState"] = "green";
                else if(currVehPhaseState == PERMISSIVE)
                    jsonObject["Spat"]["phaseState"][i]["currState"] = "permissive_yellow";
            }
        }

        for (int i = 0; i < 8; i++)
        {
            if (spatOut.permittedPedPhases.test(i))
            {
                const auto &phaseState = spatOut.pedPhaseState[i];
                jsonObject["Spat"]["pedPhaseState"][i]["phaseNo"] = (i + 1);
                jsonObject["Spat"]["pedPhaseState"][i]["currState"] = static_cast<unsigned int>(phaseState.currState);
                jsonObject["Spat"]["pedPhaseState"][i]["startTime"] = phaseState.startTime;
                jsonObject["Spat"]["pedPhaseState"][i]["minEndTime"] = phaseState.minEndTime;
                jsonObject["Spat"]["pedPhaseState"][i]["maxEndTime"] = phaseState.maxEndTime;
                jsonObject["Spat"]["pedPhaseState"][i]["elapsedTime"] = 0;
                if(currPedPhaseState == DONOTWALK)
                    jsonObject["Spat"]["pedPhaseState"][i]["currState"] = "do_not_walk";
                else if(currPedPhaseState == PEDCLEAR)
                    jsonObject["Spat"]["pedPhaseState"][i]["currState"] = "ped_clear";
                else if(currPedPhaseState == WALK)
                    jsonObject["Spat"]["pedPhaseState"][i]["currState"] = "walk";
            }
        }

        jsonString = fastWriter.write(jsonObject);
        return jsonString;
    }
}

TransceiverDecoder::~TransceiverDecoder()
{
}
