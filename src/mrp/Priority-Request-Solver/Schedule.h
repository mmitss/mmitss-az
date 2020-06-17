#pragma once

struct Schedule
{
    int commandPhase{};
    double commandStartTime{};
    double commandEndTime{};
    std::string commandType{};

    void reset()
    {
        commandPhase = 0;
        commandStartTime = 0.0;
        commandEndTime = 0.0;
        commandType = "";
    }
};

