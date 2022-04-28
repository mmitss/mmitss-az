#include <iostream>
#include "TrajectoryPhaseStatus.h"
#include "BasicVehicle.h"

using std::cout;
using std::endl;
// using std::fixed;
// using std::setprecision;
// using std::showpoint;
using std::string;
using std::vector;
using std::ofstream;
using std::ifstream;


class TrajectoryPhaseStatusManager
{
private:

    vector<TrajectoryPhaseStatus>TrajectoryPhaseStatusList{};
    
public:
    TrajectoryPhaseStatusManager();
    ~TrajectoryPhaseStatusManager();

    int getMessageType(string jsonString);
    void processBSM(BasicVehicle basicVehicle);
};


