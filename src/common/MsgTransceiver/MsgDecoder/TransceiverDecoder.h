
#include <string>


class TransceiverDecoder
{
    private:
        int messageType;
        std::string sendingJsonString;
        std::string MAPIdentifier ="0012";
        std::string BSMIdentifier = "0014";
        std::string SRMIdentifier_UpperCase = "001D";
        std::string SRMIdentifier_LowerCase = "001d";
        std::string SPaTIdentifier = "0013";
        std::string SSMIdentifier_UpperCase = "001E";
        std::string SSMIdentifier_LowerCase = "001e";

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