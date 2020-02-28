#pragma once

namespace Schedule
{
    struct TCISchedule
    {
        int commandPhase{};
        double commandStartTime{};
        double commandEndTime{};        
        int commandType{};

        void reset()
        {
            commandPhase = 0;
            commandStartTime = 0.0;
            commandEndTime = 0.0;
            commandType = 0;
        }
    };
    
    struct GLPKSchedule
    {
        int phaseNo{};
        double time{};
        int action{};
    };
};