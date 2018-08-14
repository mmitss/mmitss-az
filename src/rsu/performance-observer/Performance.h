#include <vector>

class Performance
{
public:
	int LQV[8][5]; //Last Queued Vehicle in which we are interested!!
	int LQV_stop_flag[8][5];  //the stop flag of last queued vehicle 
	
	double maximum_estimation[8][5]; //Estimation of the distance to the last queued vehicle
    double maximum_estimation2[8][5]; //Estimation of the distance to the second last queued vehicle
	int queue_counter[8][5]; //Number of Vehicles in the queue
	double LQV_discharging[8][5]; //Last Queued Vehicle Start time of discharging
	
	float App_Ext_Tardity[12]; //Extended Tardity value for each approach
	int App_Ext_counter[12]; //To count the number of vehicles for each approach in Extended Tardity
	float vehicle_tardity[12]; //Tardity value for each vehicle
	float total_distance[12]; //Summation of all distances traveled by vehicles used in tardity function
	int num_observation[12]; //Number of observations of the vehicles that completed their trip within the range
	
	float App_TT[12]; //Approach TT as follows: 0:NB, 1:EB, 2:SB, 3:WB, 4:NBLT, 5:NBRT, 6:EBLT, 7:EBRT, 8:SBLT, 9:SBRT, 10:WBLT, 11:WBRT
	float App_Delay[12]; //Approach Delay as follows: 0:NB, 1:EB, 2:SB, 3:WB
	float App_numstops[12]; //Average number of stops per vehicle per approach
	float App_dsrc[12];  //Approach DSRC Range

				
};
