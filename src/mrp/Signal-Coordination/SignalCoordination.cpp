#include <fstream>
// #include <chrono>
// #include <ctime> 
#include <time.h> 
#include <algorithm>
#include "SignalCoordination.h"
#include "json/json.h"



SignalCoordination::SignalCoordination()
{
}

void SignalCoordination::generateVirtualCoordinationPriorityRequest()
{

}

void SignalCoordination::getCurrentSignalStatus()
{
    int temporaryPhase{};
    TrafficControllerData::TrafficConrtollerStatus tcStatus;
    Json::Value jsonObject;
    Json::Reader reader;
    ifstream jsonData("trafficControllerStatus.json");
    string jsonString((std::istreambuf_iterator<char>(jsonData)), std::istreambuf_iterator<char>());
    reader.parse(jsonString.c_str(), jsonObject);

    trafficControllerStatus.clear();
    tcStatus.startingPhase1 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["SP1"].asInt();
    tcStatus.startingPhase2 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["SP2"].asInt();
    tcStatus.initPhase1 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["init1"].asDouble();
    tcStatus.initPhase2 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["init2"].asDouble();
    tcStatus.elapsedGreen1 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["Grn1"].asDouble();
    tcStatus.elapsedGreen2 = jsonObject["TrafficControllerData"]["TrafficControllerStatus"]["Grn2"].asDouble();
    trafficControllerStatus.push_back(tcStatus);
    //If signal phase is on rest, elapsed green time will be more than gmax. In that case elapsed green time will be min green time.
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        temporaryPhase = trafficControllerStatus[i].startingPhase1;
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                   [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
        if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->maxGreen)
            trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;

        temporaryPhase = trafficControllerStatus[i].startingPhase2;
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                   [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
        if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->maxGreen)
            trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
    }

    //This is optional. For priniting few attributes of the TCStatus in the console.
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        cout << trafficControllerStatus[i].startingPhase1 << " " << trafficControllerStatus[i].initPhase1 << " " << trafficControllerStatus[i].elapsedGreen2 << endl;
    }
}


/*
    - Method for obtaining static traffic signal plan from TCI
*/
void SignalCoordination::readCurrentSignalTimingPlan()
{
    TrafficControllerData::TrafficSignalPlan signalPlan;

    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream signalPlanJson("signalPlan.json");
    std::string configJsonString((std::istreambuf_iterator<char>(signalPlanJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);
    const Json::Value values = jsonObject["TimingPlan"];
    noOfPhase = (jsonObject["TimingPlan"]["NoOfPhase"]).asInt();
    cout << "Total Phase No: " << noOfPhase << endl;

    for (int i = 0; i < noOfPhase; i++)
    {
        PhaseNumber.push_back((jsonObject["TimingPlan"]["PhaseNumber"][i]).asInt());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PedWalk.push_back((jsonObject["TimingPlan"]["PedWalk"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PedClear.push_back((jsonObject["TimingPlan"]["PedClear"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        MinGreen.push_back((jsonObject["TimingPlan"]["MinGreen"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        Passage.push_back((jsonObject["TimingPlan"]["Passage"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        MaxGreen.push_back((jsonObject["TimingPlan"]["MaxGreen"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        YellowChange.push_back((jsonObject["TimingPlan"]["YellowChange"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        RedClear.push_back((jsonObject["TimingPlan"]["RedClear"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PhaseRing.push_back((jsonObject["TimingPlan"]["PhaseRing"][i]).asInt());
    }

    trafficSignalPlan.clear();
    for (int i = 0; i < noOfPhase; i++)
    {
        signalPlan.phaseNumber = PhaseNumber[i];
        signalPlan.pedWalk = PedWalk[i];
        signalPlan.pedClear = PedClear[i];
        signalPlan.minGreen = MinGreen[i];
        signalPlan.passage = Passage[i];
        signalPlan.maxGreen = MaxGreen[i];
        signalPlan.yellowChange = YellowChange[i];
        signalPlan.redClear = RedClear[i];
        signalPlan.phaseRing = PhaseRing[i];
        trafficSignalPlan.push_back(signalPlan);
    }
    //Obtain the phases in P11, P12, P21, P22
    for (int i = 0; i < noOfPhase; i++)
    {
        if (trafficSignalPlan[i].phaseNumber < 3 && trafficSignalPlan[i].phaseRing == 1)
            P11.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 2 && trafficSignalPlan[i].phaseNumber < 5 && trafficSignalPlan[i].phaseRing == 1)
            P12.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 4 && trafficSignalPlan[i].phaseNumber < 7 && trafficSignalPlan[i].phaseRing == 2)
            P21.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 6 && trafficSignalPlan[i].phaseRing == 2)
            P22.push_back(trafficSignalPlan[i].phaseNumber);
    }

    noOfPhasesInRing1 = unsigned(P11.size() + P12.size());
    noOfPhasesInRing2 = unsigned(P21.size() + P22.size());
}


void SignalCoordination::getCoordinationTime()
{
    Json::Value jsonObject;
    Json::Reader reader;
    ifstream signalPlanJson("IntersectionConfig.json");
    string configJsonString((std::istreambuf_iterator<char>(signalPlanJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);

    coordinationStartTime = jsonObject["CoordinationStartTime"].asDouble();
    coordinationEndTime = jsonObject["CoordinationEndTime"].asDouble();
    offsetTime = jsonObject["Offset"].asDouble();
    cycleLength = jsonObject["CycleLength"].asDouble();
    
}

bool SignalCoordination::checkCoordinationTimeOfTheDay()
{
    double currentHourOfTheDay{};
    time_t s = 1; 
    struct tm* current_time; 
  
    // time in seconds 
    s = time(NULL); 
  
    // to get current time 
    current_time = localtime(&s); 
    currentHourOfTheDay = current_time->tm_hour;

    if (currentHourOfTheDay > coordinationStartTime && currentHourOfTheDay < coordinationEndTime)
        bCoordination = true;
    
    else
        bCoordination = false;

    return bCoordination;
    
}

void SignalCoordination::getFirstCycleLength()
{

}

void SignalCoordination::getCurrentTime()
{
    time_t s = 1; 
    struct tm* current_time; 
  
    // time in seconds 
    s = time(NULL); 
  
    // to get current time 
    current_time = localtime(&s); 
  
    // print time in minutes, 
    // hours and seconds 
    // printf("%02d:%02d:%02d ", 
    //        current_time->tm_hour, 
    //        current_time->tm_min, 
    //        current_time->tm_sec);

    double currentTime = current_time->tm_hour * 3600.00 + current_time->tm_min * 60.00+ current_time->tm_sec;
    cout << "Current Time; " << currentTime << endl;
}


SignalCoordination::~SignalCoordination()
{
}