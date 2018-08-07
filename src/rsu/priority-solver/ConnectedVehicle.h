#include "PositionLocal3D.h"
#include <vector>

class ConnectedVehicle
{
public:
	int TempID; //ID, temp, since it is changing every 5 mins
	double Speed;  //Vehicle current Speed
	int desiredPhase ; //traffic signal phase associated with a lane (MAP + phase association)
	double stopBarDistance ; //distance to the stop bar from current location (MAP)
	double estArrivalTime ; // estimated arrival time at the stop bar given current speed
	PositionLocal3D traj[5000]; //store vehicle trajectory, totally store 500s trajectory
	int nFrame;  //store how many frames are recorded.
	double heading; //vehicle heading
	double N_Offset[5000];  //offset to the center of the intersection
	double E_Offset[5000];  //in cm
	int Dsecond;
	int pre_requested_phase;
	int Phase_Request_Counter;
	int req_phase;  //current requested phase
	double ETA;        //estimated travel time
	double time[5000];       //Absolute time that the trajectory information is received
	int active_flag;   //a flag to indicate whether this vehicle is still in the network:
	                   //every time the vehicle send BSM to RSE, the flag is reset to 2
					   //every time COP starts the flag is reduced by 1
					   //if the flag is <0, delete the vehicle from the list
	double receive_timer;  //this is the time to set the frequency of receiving to reduce computational effort
	int approach;  //the approach no
	int lane;      //the lane number
	double acceleration;  
	double time_stop;    //time point the vehicle first stopped
	int stop_flag;      //flag that indicates whether the vehicle has stopped or not;  0: not stop;  1: stopped
	//	-----------------Methods-------------------------


};
