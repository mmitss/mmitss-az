#pragma once
#include <string>

class Spat
{
    private:
        int regionalID{};
        int intersectionID{};
        int msgCnt{};

    public:
        //Constructor:
        Spat();
        //Getters:
        //Setters:
        //JsonHandlers:
        void json2Spat(std::string jsonString);
        std::string spat2Json();
        //Destructor:
        ~Spat();
};