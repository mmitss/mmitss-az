#pragma once

    const double second_MinLimit = 0.0;
    const double second_MaxLimit = 59.99;              //This value comes frrom J2735 standard
    
class ETA
{
    private:
        int expectedTimeOfArrival_Minute{};
        double expectedTimeOfArrival_Second{0.0};
        double expectedTimeOfArrival_Duration{0.0};      //provide a short interval that extends the ETA

        
    public:
        //Constructor:
        ETA();

        //Setters:
        
        void setETA_Minute(int vehExpectedTimeOfArrival_Minute);
        bool setETA_Second(double vehExpectedTimeOfArrival_Second);
        bool setETA_Duration(double vehDuration); 
        

        //Getters:
        int getETA_Minute();
        double getETA_Second();
        double getETA_Duration();
        

        //Destructor:
        ~ETA();
};


