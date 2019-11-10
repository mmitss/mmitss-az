#pragma once


class IntersectionAccessPoint
{
    private:
        int laneID{};
        int approachID{};
        int laneConnectionID{};

    public:
        //Constructor:
        IntersectionAccessPoint();

        //Setters:
        void setLaneID(int vehLaneID);
        void setApproachID(int vehApproachID);
        void setLaneConnectionID(int vehLaneConnectionID); 

        //Getters:
        int getLaneID();
        int getApproachID();
        int getLaneConnectionID();

        //Destructor:
        ~IntersectionAccessPoint();
};

