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
	//PositionLocal3D traj[500]; //store vehicle trajectory, totally store 500s trajectory
	int nFrame;  //store how many frames are recorded.
	double heading; //vehicle heading
	double N_Offset[5];  //offset to the center of the intersection
	double E_Offset[5];  //in cm
	int Dsecond;
	int pre_requested_phase;
	int Phase_Request_Counter;
	int req_phase;  //current requested phase
	double ETA;        //estimated travel time
	//double time[500];       //Absolute time that the trajectory information is received
	int active_flag;   //a flag to indicate whether this vehicle is still in the network:
	                   //every time the vehicle send BSM to RSE, the flag is reset to 2
					   //every time COP starts the flag is reduced by 1
					   //if the flag is <0, delete the vehicle from the list
	double receive_timer;  //this is the time to set the frequency of receiving to reduce computational effort
	int approach;  //the current approach no
	int previous_approach; //stores the previous approach no
	int lane;      //the lane number
	int previous_lane; //stores the previous lane no
	double acceleration;  
	double time_stop;    //time point the vehicle first stopped
	int stop_flag;      //flag that indicates whether the vehicle has stopped or not;  0: not stop;  1: stopped
	int queue_flag;		//flag that indicates whether the vehicle is in the queue or not; 0:not in queue; 1:in queue -------> mainly for Performance Observer
	int processed;      //flag indicating that if the vehicle already go through FindVehInMAP function, only vehicle be processed can be sent to other components
	double entry_time;     //entry time of the vehicle first in DSRC range
	double leaving_time;   //the last time point before the vehicle leaves the DSRC range, or the current time if the vehicle is still in range
	int TT; //Travel Time of the vehicle traversing the DSRC range
	double delay; //Delay of the vehicle traversing the DSRC range
	double Distance; //Distance that the vehicle traversed up to now since it gets into the range
	int update_flag; //flag to decide whehter to update, add, or remove a vehcile from the current list
	int passed_flag; //flag to decide whether the vehicle passed the stop bar for the first time or not. 0:not passed, 1: passed
	double dsrc_range;    //The value of the range of DSRC Radio in meters
	double dsrc_range_rsu;    //The value of the range of DSRC Radio in meters in accordance to the location of the rsu
//	-----------------Methods-------------------------


};
