#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cstring>
#include <string.h>

#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"
#include "json.h"
#include "BasicVehicle.h"
#include "SignalRequest.h"
#include "SignalStatus.h"
#include "ActiveRequest.h"
#include "TransceiverDecoder.h"

const double KPH_TO_MPS_CONVERSION = 0.277778;

TransceiverDecoder::TransceiverDecoder()
{
}

int TransceiverDecoder::getMessageType(std::string payload)
{

    std::string subPayload = payload.substr(0, 4);

    std::vector<std::string> MessageIdentifier;

    MessageIdentifier = {MAPIdentifier, BSMIdentifier, SRMIdentifier, SPaTIdentifier, SSMIdentifier};

    if (MessageIdentifier.at(0).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_map;
    }

    else if (MessageIdentifier.at(1).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_bsm;
    }

    else if (MessageIdentifier.at(2).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_srm;
    }

    else if (MessageIdentifier.at(3).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_spat;
    }
    else if (MessageIdentifier.at(4).compare(subPayload) == 0)
    {
        messageType = MsgEnum::DSRCmsgID_ssm;
    }
    return messageType;
}

// std::string TransceiverDecoder::decodeMessagePayload(std::string payload)
// {
//     std::string decodedJsonString{};

//     if (getMessageType(payload) == MsgEnum::DSRCmsgID_map)
//     {
//         createJsonStingOfMapPayload(payload);
//     }

//     else if (getMessageType(payload) == MsgEnum::DSRCmsgID_bsm)
//     {
//         bsmDecoder(payload);
//     }

//     else if (getMessageType(payload) == MsgEnum::DSRCmsgID_srm)
//     {
//         srmDecoder(payload);
//     }

//     else if (getMessageType(payload) == MsgEnum::DSRCmsgID_spat)
//     {
//     }

//     else if (getMessageType(payload) == MsgEnum::DSRCmsgID_ssm)
//     {
//         ssmDecoder(payload);
//     }

//     else if (getMessageType(payload) = MsgEnum::DSRCmsgID_unknown)
//     {
//       std::cout<<"Message Type is unknown"<<std::endl;
//     }

//     return decodedJsonString;
// }

std::string TransceiverDecoder::createJsonStingOfMapPayload(std::string mapPayload)
{
    Json::Value jsonObject;
    // Json::StyledStreamWriter styledStreamWriter;
    // std::ofstream outputter("output.json");
    Json::FastWriter fastWriter;
    std::string jsonString;

    Json::Value jsonObject_config;
	Json::Reader reader;
	std::ifstream configJson("ConfigurationInfo.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject_config);
    std::string intersectionName = (jsonObject_config["IntersectionName"]).asString();
    const int mapReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();

    jsonObject["MsgType"] = "MAP";
    jsonObject["IntersectionName"] = intersectionName;
    jsonObject["MapPayload"] = mapPayload;
    // styledStreamWriter.write(outputter, jsonObject);
    jsonString = fastWriter.write(jsonObject);

    std::cout << jsonString << std::endl;
    return jsonString;
}

std::string TransceiverDecoder::bsmDecoder(std::string bsmPayload)
{
    BasicVehicle basicVehicle;
    std::string jsonString;

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
        basicVehicle.setSecMark_Second((bsmOut.timeStampSec)/1000.0);
        basicVehicle.setPosition(DsrcConstants::damega2unit<int32_t>(bsmOut.latitude), DsrcConstants::damega2unit<int32_t>(bsmOut.longitude), DsrcConstants::deca2unit<int32_t>(bsmOut.elevation));
        basicVehicle.setSpeed_MeterPerSecond(round(DsrcConstants::unit2kph<uint16_t>(bsmOut.speed)*KPH_TO_MPS_CONVERSION));
        basicVehicle.setHeading_Degree(round(DsrcConstants::unit2heading<uint16_t>(bsmOut.heading)));
        basicVehicle.setType(0);

        jsonString = basicVehicle.basicVehicle2Json();
        std::cout << jsonString << std::endl;
    }

    return jsonString;
}

std::string TransceiverDecoder::srmDecoder(std::string srmPayload)
{
    SignalRequest signalRequest;

    std::string jsonString;

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
        std::cout << "decode_msgFrame for SRM succeed" << std::endl;

        signalRequest.setMsgCount(static_cast<unsigned int>(srmOut.msgCnt));
        signalRequest.setMinuteOfYear(srmOut.timeStampMinute);
        signalRequest.setMsOfMinute(srmOut.timeStampSec);
        signalRequest.setRegionalID(srmOut.regionalId);
        signalRequest.setIntersectionID(srmOut.intId);
        signalRequest.setRequestID(static_cast<unsigned int>(srmOut.reqId));
        signalRequest.setPriorityRequestType(static_cast<unsigned int>(srmOut.reqType));
        signalRequest.setInBoundLaneIntersectionAccessPoint(static_cast<unsigned int>(srmOut.inLaneId),static_cast<unsigned int>(srmOut.inApprochId));
        // signalRequest.setETA((srmOut.ETAminute - srmOut.timeStampMinute), srmOut.ETAsec / 1000.0, srmOut.duration / 1000.0);
        signalRequest.setETA(srmOut.ETAminute, srmOut.ETAsec, srmOut.duration);
        signalRequest.setTemporaryVechileID(srmOut.vehId);
        signalRequest.setBasicVehicleRole(static_cast<unsigned int>(srmOut.vehRole));
        signalRequest.setPosition(DsrcConstants::damega2unit<int32_t>(srmOut.latitude), DsrcConstants::damega2unit<int32_t>(srmOut.longitude), DsrcConstants::deca2unit<int32_t>(srmOut.elevation));
        signalRequest.setHeading_Degree(round(DsrcConstants::unit2heading<uint16_t>(srmOut.heading)));
        signalRequest.setSpeed_MeterPerSecond(round(DsrcConstants::unit2kph<uint16_t>(srmOut.speed)*KPH_TO_MPS_CONVERSION));
        signalRequest.setVehicleType(static_cast<unsigned int>(srmOut.vehType));

        jsonString = signalRequest.signalRequest2Json();

        std::cout << jsonString << std::endl;
    }

    return jsonString;
}

std::string TransceiverDecoder::ssmDecoder(std::string ssmPayload)
{
    SignalStatus signalStatus;
    std::vector<ActiveRequest> ActiveRequestTable;
    ActiveRequest activeRequest;
    std::string jsonString;

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
        std::cout << "decode_msgFrame for SSM succeed" << std::endl;

        signalStatus.setMinuteOfYear(ssmOut.timeStampMinute);
        signalStatus.setMsOfMinute(ssmOut.timeStampSec);
        signalStatus.setSequenceNumber(static_cast<unsigned int>(ssmOut.msgCnt));
        signalStatus.setUpdateCount(static_cast<unsigned int>(ssmOut.updateCnt));
        signalStatus.setRegionalID(ssmOut.regionalId);
        signalStatus.setIntersectionID(ssmOut.id);
        signalStatus.setNoOfRequest(unsigned(ssmOut.mpSignalRequetStatus.size()));
        // size_t reqNo = 0;
        for (const auto &RequetStatus : ssmOut.mpSignalRequetStatus)
        {
            activeRequest.vehicleID = RequetStatus.vehId;
            activeRequest.requestID = static_cast<unsigned int>(RequetStatus.reqId);
            activeRequest.msgCount = static_cast<unsigned int>(RequetStatus.sequenceNumber);
            activeRequest.basicVehicleRole = static_cast<unsigned int>(RequetStatus.vehRole);
            activeRequest.vehicleLaneID = static_cast<unsigned int>(RequetStatus.inLaneId);
            activeRequest.vehicleETA = RequetStatus.ETAminute * 60.0 + RequetStatus.ETAsec;
            activeRequest.prsStatus = static_cast<unsigned int>(RequetStatus.status);
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

    std::cout << jsonString << std::endl;

    return jsonString;
}

TransceiverDecoder::~TransceiverDecoder()
{
}