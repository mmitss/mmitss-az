#pragma once

namespace Schedule
{
    struct TCISchedule
    {
        int phaseNo{};
        double phaseDuration{};
        double phaseGreenTime{};
        double commandTime{};
        int commandType{};
        int commandId{};

        void reset()
        {
            phaseNo = 0;
            phaseDuration = 0.0;
            phaseGreenTime = 0.0;
            commandTime = 0.0;
            commandType = 0;
            commandId = 0;
        }
    };
    struct GLPKSchedule
    {
        int phaseNo{};
        double time{};
        int action{};
    };
};