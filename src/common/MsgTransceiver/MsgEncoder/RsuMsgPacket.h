/***********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  RsuMsgPacket.h
  Created by: Niraj Altekar
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. Header file for RsuMsgPacket class
*/
#ifndef _RSU_MSG_PACKET_
#define _RSU_MSG_PACKET_

class RsuMsgPacket
{
    private:
        double version = 0.7;
        std::string type{};
        std::string psid{};
        int priority = 7;
        std::string txMode{};
        std::string txChannel{};
        std::string txInterval = "0";
        std::string deliveryStart = "";
        std::string deliveryStop = "";
        std::string signature = "False";
        std::string encryption = "False";
        std::string payload{};

        std::string bsmMsgId{};
        std::string srmMsgId_lower{};
        std::string srmMsgId_upper{};
        std::string spatMsgId{};
        std::string mapMsgId{};
        std::string ssmMsgId_lower{};
        std::string ssmMsgId_upper{};

        std::string bsmPsid{};
        std::string srmPsid{};
        std::string spatPsid{};
        std::string mapPsid{};
        std::string ssmPsid{};

        std::string bsmTxChannel{};
        std::string srmTxChannel{};
        std::string mapTxChannel{};
        std::string spatTxChannel{};
        std::string ssmTxChannel{};

        std::string bsmTxMode{};
        std::string srmTxMode{};
        std::string spatTxMode{};
        std::string mapTxMode{};
        std::string ssmTxMode{};

        void setMsgType(std::string msgPayload);
        void setPsid(std::string msgType);
        void setTxMode(std::string msgType);
        void setTxChannel(std::string msgType);
                
    public:
        RsuMsgPacket();
        std::string getMsgPacket(std::string msgPayload);
};

#endif //_RSU_MSG_PACKET_
