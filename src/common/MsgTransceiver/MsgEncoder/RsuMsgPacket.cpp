#include <string>
#include <sstream>
#include <fstream>
#include "RsuMsgPacket.h"
#include "json/json.h"


RsuMsgPacket::RsuMsgPacket()
{
    Json::Value jsonObject_config;
    Json::Reader reader;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject_config);

    bsmMsgId = jsonObject_config["msgId"]["bsm"].asString();
    srmMsgId_lower = jsonObject_config["msgId"]["srm_lower"].asString();
    srmMsgId_upper = jsonObject_config["msgId"]["srm_upper"].asString();
    spatMsgId = jsonObject_config["msgId"]["spat"].asString();
    mapMsgId = jsonObject_config["msgId"]["map"].asString();
    ssmMsgId_lower = jsonObject_config["msgId"]["ssm_lower"].asString();
    ssmMsgId_upper = jsonObject_config["msgId"]["ssm_upper"].asString();

    bsmPsid = jsonObject_config["psid"]["bsm"].asString();
    srmPsid = jsonObject_config["psid"]["srm"].asString();
    spatPsid = jsonObject_config["psid"]["spat"].asString();
    mapPsid = jsonObject_config["psid"]["map"].asString();
    ssmPsid = jsonObject_config["psid"]["ssm"].asString();
}

void RsuMsgPacket::setMsgType(std::string msgId)
{
    if(msgId == mapMsgId)
        type = "MAP";
    if(msgId == bsmMsgId)
        type = "BSM";
    if(msgId == srmMsgId_lower || msgId == srmMsgId_upper)
        type = "SRM";
    if(msgId == spatMsgId)
        type = "SPAT";
    if(msgId == ssmMsgId_lower || msgId == ssmMsgId_upper)
        type = "SSM";
}

void RsuMsgPacket::setPsid(std::string msgType)
{
    if(msgType == "MAP")
        psid = "0x" + mapPsid;
    if(msgType == "SPAT")
        psid = "0x" + spatPsid;
    if(msgType == "SSM")
        psid = "0x" + ssmPsid;
    if(msgType == "BSM")
        psid ="0x" + bsmPsid;
    if(msgType == "SRM")
        psid = "0x" + srmPsid;
}

void RsuMsgPacket::setTxMode(std::string msgType)
{
    if(msgType == "MAP")
        txMode = "CONT";
    if(msgType == "SPAT")
        txMode = "CONT";
    if(msgType == "SSM")
        txMode = "ALT";
    if(msgType == "BSM")
        txMode = "CONT";
    if(msgType == "SRM")
        txMode = "ALT";
}

void RsuMsgPacket::setTxChannel(std::string msgType)
{
    if(msgType == "MAP")
        txChannel = 172;
    if(msgType == "SPAT")
        txChannel = 172;
    if(msgType == "SSM")
        txChannel = 182;
    if(msgType == "BSM")
        txChannel = 172;
    if(msgType == "SRM")
        txChannel = 182;
}

std::string RsuMsgPacket::getMsgPacket(std::string msgPayload)
{
    payload = msgPayload;
    std::string msgId = msgPayload.substr(0,4);
    setMsgType(msgId);
    setTxMode(type);
    setTxChannel(type);
    setPsid(type);


    std::stringstream ss{};
    ss << "Version=" << version << std::endl;
    ss << "Type=" << type << std::endl;
    ss << "PSID=" << psid << std::endl;
    ss << "Priority=" << priority << std::endl;
    ss << "TxMode=" << txMode << std::endl;
    ss << "TxChannel=" << txChannel << std::endl;
    ss << "TxInterval=" << txInterval << std::endl;
    ss << "DeliveryStart=" << deliveryStart << std::endl;
    ss << "DeliveryStop=" << deliveryStop << std::endl;
    ss << "Signature=" << signature << std::endl;
    ss << "Encryption=" << encryption << std::endl;
    ss << "Payload=" << payload << std::endl;

    std::string msgPacket = ss.str();

    return msgPacket;
}