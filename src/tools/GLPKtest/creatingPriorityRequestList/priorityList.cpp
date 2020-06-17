#include <iostream>
#include <list>
#include <fstream>
#include <jsoncpp/json/json.h>
#include "requestEntry.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::list;
using std::string;

int main()
{
    int transitWeight = 1;
    int truckWeight = 1;
    int ReqSeq = 1;
    int numberOfRequest{};
    int numberOfTransitInList{};
    int numberOfTruckInList{};
    int vehicleClass{}; //to match the old PRSolver
    int noOfRequest{};
    int *vehicleID = new int[noOfRequest];
    int *basicVehicleRole = new int[noOfRequest];
    int *inBoundLaneID = new int[noOfRequest];
    double *expectedTimeOfArrival = new double[noOfRequest];
    double *expectedTimeOfArrival_Duration  = new double[noOfRequest];
    int *vehicleRequestedPhase = new int[noOfRequest];
    int *priorityRequestStatus = new int[noOfRequest];

    list<requestEntry> ActiveRequestList;

    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream configJson("requestList.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);
    const Json::Value values = jsonObject["PriorityRequestList"]["requestorInfo"];
    noOfRequest = (jsonObject["PriorityRequestList"]["noOfRequest"]).asInt();
    requestEntry requestList;

    for (int i = 0; i < noOfRequest; i++)
    {
        for (size_t j = 0; j < values[i].getMemberNames().size(); j++)
        {
            if (values[i].getMemberNames()[j] == "vehicleID")
                vehicleID[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "basicVehicleRole")
                basicVehicleRole[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "laneID")
                inBoundLaneID[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "vehicleETA")
                expectedTimeOfArrival[i] = values[i][values[i].getMemberNames()[j]].asDouble();
            
            if (values[i].getMemberNames()[j] == "vehicleETA_Duration")
                expectedTimeOfArrival_Duration[i] = values[i][values[i].getMemberNames()[j]].asDouble();
            
            if (values[i].getMemberNames()[j] == "requestedPhase")
                vehicleRequestedPhase[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "prioritystatus")
                priorityRequestStatus[i] = values[i][values[i].getMemberNames()[j]].asInt();
        }
    }

    ActiveRequestList.clear();
    for (int i = 0; i < noOfRequest; i++)
    {
        requestList.vehicleID = vehicleID[i];
        requestList.basicVehicleRole = basicVehicleRole[i];
        requestList.vehicleLaneID = inBoundLaneID[i];
        requestList.vehicleETA = expectedTimeOfArrival[i];
        requestList.vehicleETADuration = expectedTimeOfArrival_Duration[i];
        requestList.requestedPhase = vehicleRequestedPhase[i];
        requestList.prsStatus = priorityRequestStatus[i];
        ActiveRequestList.push_front(requestList);
    }

    std::list<requestEntry>::iterator it;
    for (it = ActiveRequestList.begin(); it != ActiveRequestList.end(); ++it)
        std::cout << it->vehicleID << '\t' << it->basicVehicleRole << '\t' << it->requestedPhase << '\t' << it->vehicleETA << std::endl;


    delete vehicleID;
    delete basicVehicleRole;
    delete inBoundLaneID;
    delete expectedTimeOfArrival;
    delete priorityRequestStatus;
    delete expectedTimeOfArrival_Duration;
    delete vehicleRequestedPhase;


    ofstream fs;
    ifstream jsonData("TC_Data.json");
    string jsonString((std::istreambuf_iterator<char>(jsonData)), std::istreambuf_iterator<char>());
    reader.parse(jsonString.c_str(), jsonObject);
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
    fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["yellow"]["phase8"]).asDouble() << ";" << endl;

    fs << "param red          \t:="; 
    fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase1"]).asDouble();
    fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase2"]).asDouble();
    fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase3"]).asDouble();
    fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase4"]).asDouble();
    fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase5"]).asDouble();
    fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase6"]).asDouble();
    fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase7"]).asDouble();
    fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["red"]["phase8"]).asDouble() << ";" << endl;

    fs << "param gmin      \t:=";
    fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase1"]).asDouble();
    fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase2"]).asDouble();
    fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase3"]).asDouble();
    fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase4"]).asDouble();
    fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase5"]).asDouble();
    fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase6"]).asDouble();
    fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase7"]).asDouble();
    fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmin"]["phase8"]).asDouble() << ";" << endl;

    fs << "param gmax      \t:=";

    if(!ActiveRequestList.empty())
    {
        std::list<requestEntry>::iterator it1;
        for (it1 = ActiveRequestList.begin(); it1 != ActiveRequestList.end(); ++it1)
        {   
            if(it1->requestedPhase == 1)
            {
                fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble() *1.25;
                fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble();
                fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble();
                fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble();
                fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble();
                fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble() *1.25;
                fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble();
                fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble();
            }
                
            if(it1->requestedPhase == 2)
            {
                fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble();
                fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble()*1.25;
                fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble();
                fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble();
                fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble()*1.25;
                fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble();
                fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble();
                fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble();

            }

            if(it1->requestedPhase == 3)
            {
                fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble();
                fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble();
                fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble()*1.25;
                fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble();
                fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble();
                fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble();
                fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble();
                fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble()*1.25;

            }

            if(it1->requestedPhase == 4)
            {
                fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble();
                fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble();
                fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble();
                fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble()*1.25;
                fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble();
                fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble();
                fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble()*1.25;
                fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble();

            }

            if(it1->requestedPhase == 5)
            {
                fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble();
                fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble()*1.25;
                fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble();
                fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble();
                fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble()*1.25;
                fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble();
                fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble();
                fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble();

            }

            if(it1->requestedPhase == 6)
            {
                fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble()*1.25;
                fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble();
                fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble();
                fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble();
                fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble();
                fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble()*1.25;
                fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble();
                fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble();

            }

            if(it1->requestedPhase == 7)
            {
                fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble();
                fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble();
                fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble();
                fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble()*1.25;
                fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble();
                fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble();
                fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble()*1.25;
                fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble();

            }

            if(it1->requestedPhase == 8)
            {
                fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble();
                fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble();
                fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble()*1.25;
                fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble();
                fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble();
                fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble();
                fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble();
                fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble()*1.25;

            }
        }          
    }

    else
    {
        fs << "\t" << "1" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase1"]).asDouble();
        fs << "\t" << "2" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase2"]).asDouble();
        fs << "\t" << "3" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase3"]).asDouble();
        fs << "\t" << "4" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase4"]).asDouble();
        fs << "\t" << "5" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase5"]).asDouble();
        fs << "\t" << "6" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase6"]).asDouble();
        fs << "\t" << "7" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase7"]).asDouble();
        fs << "\t" << "8" << "\t" <<(jsonObject["TrafficControllerData"]["TtafficControllerConfiguration"]["gmax"]["phase8"]).asDouble();
    }

    fs << ";\n\n";
    fs << "param priorityType:= ";
    
    if(!ActiveRequestList.empty())
    {
        std::list<requestEntry>::iterator it2;
        for (it2 = ActiveRequestList.begin(); it2 != ActiveRequestList.end(); ++it2)
        {   
            vehicleClass = 0;
            numberOfRequest++;
            if(it2->basicVehicleRole == 16)
            {
                numberOfTransitInList++;
                vehicleClass = 2;
            }
                
            else if (it2->basicVehicleRole == 9)
            {
                numberOfTruckInList++;
                vehicleClass = 3;
            }
                
            fs << numberOfRequest;
            fs << " " << vehicleClass << " ";
        }
        while (numberOfRequest < 10)
		{
			numberOfRequest++;
			fs << numberOfRequest;
			fs << " ";
			fs << 0;
			fs << " ";
		}
		fs << " ;  \n";
        
    }

    else
	{
		fs << " 1 0 2 0 3 5 4 0 5 0 6 0 7 0 8 0 9 0 10 0 ; \n";
	}

    fs << "param PrioWeigth:=  1 1 2 ";
    if(numberOfTransitInList > 0)
        fs << transitWeight / numberOfTransitInList;
    else
    {
        fs << 0;
    }
    fs << " 3 ";
    if(numberOfTruckInList > 0)
    	fs << truckWeight / numberOfTruckInList;
    else
    {
        fs << 0;
    }
	fs << " 4 0 5 0 6 0 7 0 8 0 9 0 10 0 ; \n";

    fs << "param Rl (tr): 1 2 3 4 5 6 7 8:=\n";
    if(!ActiveRequestList.empty())
    {
        std::list<requestEntry>::iterator it3;
        for (it3 = ActiveRequestList.begin(); it3 != ActiveRequestList.end(); ++it3)
        {
            fs << ReqSeq << "  ";
            for (int j = 1; j <= 8; j++)
            {
                if(it3->requestedPhase == j)
                    fs << it3->vehicleETA << "  ";
                else
                    fs << ".\t";
            }
            ReqSeq++;
            fs << "\n";
        }
    ReqSeq = 1;
    }

    fs << ";\n";

    fs << "param Ru (tr): 1 2 3 4 5 6 7 8:=\n";
    if(!ActiveRequestList.empty())
    {
        std::list<requestEntry>::iterator it3;
        for (it3 = ActiveRequestList.begin(); it3 != ActiveRequestList.end(); ++it3)
        {
            fs << ReqSeq << "  ";
            for (int j = 1; j <= 8; j++)
            {
                if(it3->requestedPhase == j)
                    fs << it3->vehicleETA + it3->vehicleETADuration << "  ";
                else
                    fs << ".\t";
            }
            ReqSeq++;
            fs << "\n";
        }
        ReqSeq = 1;
    }

    fs << ";\n";
    fs <<"end;";

}