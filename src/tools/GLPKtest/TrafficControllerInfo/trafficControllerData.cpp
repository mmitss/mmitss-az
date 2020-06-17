#include <iostream>
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>

using std::endl;
using std::cout;
using std::string;
using std::ifstream;
using std::ofstream;
using std::ios;

int main()
{
    ofstream fs;
    Json::Value jsonObject;
    Json::Reader reader;
    ifstream jsonData("TC_Data.json");
    string jsonString((std::istreambuf_iterator<char>(jsonData)), std::istreambuf_iterator<char>());
    reader.parse(jsonString.c_str(), jsonObject);
    //const Json::Value values = jsonObject["TrafficControllerData"]["requestorInfo"];
    fs.open("NewModelData.dat", ios::out);
    fs << "data;\n";
    fs << "param SP1:=" << (jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["SP1"]).asInt() << ";" << endl;
    fs << "param SP2:=" << (jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["SP2"]).asInt()  << ";" << endl;
    fs << "param init1:=" << (jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["init1"]).asDouble() << ";" << endl;
	fs << "param init2:=" << (jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["init2"]).asDouble() << ";" << endl;
	fs << "param Grn1 :=" << (jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["Grn1"]).asDouble() << ";" << endl;
	fs << "param Grn2 :=" << (jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["Grn2"]).asDouble() << ";" << endl;
    
    fs << "param y          \t:="; 
    fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase1"]).asDouble();
    fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase2"]).asDouble();
    fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase3"]).asDouble();
    fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase4"]).asDouble();
    fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase5"]).asDouble();
    fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase6"]).asDouble();
    fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase7"]).asDouble();
    fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase8"]).asDouble() << endl;

    fs << "param red          \t:="; 
    fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase1"]).asDouble();
    fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase2"]).asDouble();
    fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase3"]).asDouble();
    fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase4"]).asDouble();
    fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase5"]).asDouble();
    fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase6"]).asDouble();
    fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase7"]).asDouble();
    fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase8"]).asDouble() << endl;

    fs << "param gmin      \t:=";
    fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase1"]).asDouble();
    fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase2"]).asDouble();
    fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase3"]).asDouble();
    fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase4"]).asDouble();
    fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase5"]).asDouble();
    fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase6"]).asDouble();
    fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase7"]).asDouble();
    fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase8"]).asDouble() << endl;

    fs << "param gmax      \t:=";
    fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble();
    fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble();
    fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble();
    fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble();
    fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble();
    fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble();
    fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble();
    fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble() << endl;

}