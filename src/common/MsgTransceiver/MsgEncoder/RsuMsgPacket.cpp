#include <string>
#include <sstream>
#include "RsuMsgPacket.h"

void RsuMsgPacket::setMsgType(std::string msgId)
{
    if(msgId == "0012")
        type = "MAP";
    if(msgId == "0014")
        type = "BSM";
    if(msgId == "001D" || msgId == "001d")
        type = "SRM";
    if(msgId == "0013")
        type = "SPAT";
    if(msgId == "001E" || msgId == "001e")
        type = "SSM";
}

void RsuMsgPacket::setPsid(std::string msgType)
{
    if(msgType == "MAP")
        psid = "0xE0000017";
    if(msgType == "SPAT")
        psid = "0x8002";
    if(msgType == "SSM")
        psid = "0xE0000020";
    if(msgType == "BSM")
        psid = "0x20";
    if(msgType == "SRM")
        psid = "0xE0000019";
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