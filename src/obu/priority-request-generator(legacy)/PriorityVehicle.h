/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  NMAP.cpp  
 *  Created by Yiheng Feng on 9/27/14. Revised by MZP 10/20/17
 *  University of Arizona
 *  ATLAS Research Center
 *  College of Engineering
 *
 *  This code was develop under the supervision of Professor Larry Head
 *  in the ATLAS Research Center.
 *
 *  Revision History:
 * 
 *  
 */
 
#include "PositionLocal3D.h"
#include <vector>
class PriorityVehicle
{
	public:
		long lVehID; 
		double dSpeed;  //Vehicle current Speed
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
		
	    bool bIntersectionInRange[10];	// Assume we can get at most 10 intersection maps at any time // This variable shows the maps that the vehicle is in the geofence disrance of it
		int	iNoOfIntersectionsInRange;  // Assume this variable is at most 10
		long lRxIntersectionID[10];     // This variable shows the intersection ID of the received maps 
		double dDistanceToStopBar[10];
		double dETAtoStopBar[10];
		int	iIntersectionApproach[10];
		int	iIntersectionLane[10];
		double	dQueueClearTime[10];
		int	iVehStatus[10];    		    // whether the vehicle is approaching inthequeue leaving the intersection
		long lApproachingIntersectionID ;
		long lLeavingIntersectionID;
		int iInLane;
		int iOutLane;
		short sMsgCount;
		bool bCanceled;
		int iMsgCnt;
		int iVehicleType;
		
};
