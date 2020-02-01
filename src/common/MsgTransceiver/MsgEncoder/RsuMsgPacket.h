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
        int txChannel{};
        std::string txInterval = "";
        std::string deliveryStart = "";
        std::string deliveryStop = "";
        std::string signature = "False";
        std::string encryption = "False";
        std::string payload{};
        void setMsgType(std::string msgPayload);
        void setPsid(std::string msgType);
        void setTxMode(std::string msgType);
        void setTxChannel(std::string msgType);
                
    public:
        std::string getMsgPacket(std::string msgPayload);
};

#endif //_RSU_MSG_PACKET_