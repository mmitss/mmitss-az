#include "SignalCoordination.h"


int main()
{
    SignalCoordination coordination;
    coordination.getCoordinationTime();
    coordination.getCurrentTime();
    coordination.readCurrentSignalTimingPlan();
    while(coordination.checkCoordinationTimeOfTheDay() == true)
    {
        coordination.getCurrentSignalStatus();
    }

}