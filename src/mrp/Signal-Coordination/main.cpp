#include "SignalCoordination.h"


int main()
{
    SignalCoordination coordination;
    coordination.readIntersectionConfig();
    // coordination.getCurrentTime();
    coordination.readCurrentSignalTimingPlan();
    // while(coordination.checkCoordinationTimeOfTheDay() == true)
    // {
        coordination.getCurrentSignalStatus();
        coordination.generateVirtualCoordinationPriorityRequest();
        coordination.generateModFile();
    // }

}