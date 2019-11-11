
#include <string>


class TransceiverDecoder
{
    private:
        int messageType;
        std::string sendingJsonString;
        std::string MAPIdentifier ="0012";
        std::string BSMIdentifier = "0014";
        std::string SRMIdentifier = "001D";
        std::string SPaTIdentifier = "0013";
        std::string SSMIdentifier = "001E";

    public:
        TransceiverDecoder();
        ~TransceiverDecoder();
        
        int getMessageType(std::string payload);
        std::string createJsonStingOfMapPayload(std::string mapPayload);
        std::string bsmDecoder(std::string bsmPayload);
        std::string srmDecoder(std::string srmPayload);
        std::string ssmDecoder(std::string ssmPayload);
        std::string spatDecoder(std::string spatPayload);
};