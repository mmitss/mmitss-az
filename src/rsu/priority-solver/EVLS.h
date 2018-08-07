#include <stdio.h>
#include <vector>
#include <fstream>
#include <assert.h>
#include "math.h"
#include "time.h"
#include <string.h>
#include <iostream>
#include <string>
#include "stdlib.h"
#include <list>
#include "ConnectedVehicle.h"
#include "LinkedList.h"
//#include "ListHandle.h"

//using namespace std;


extern char temp_log[512];
extern float penetration;       //penetration rate
extern float dsrc_range;
extern LinkedList <ConnectedVehicle> vehlist_each_phase;  //vehicle list that saves the raw data from trajectory awareness
extern LinkedList <ConnectedVehicle> trackedveh; 
extern int outputlog(char *output);

//Define Parameters for car following model
extern float jam_length;  //minmum headway of a vehicle (AX)
extern float acc_diff_thre;  //Acceleration difference threshold
extern float desired_spd;         //vehicle desired speed (ffs)   m/s = 50km/h
extern float lamda;            //parameter for estimating speed
extern float veh_length;       //unified vehicle length

extern LinkedList <ConnectedVehicle> sorted_vehlist;  //sorted list by Lane No and distance to stop bar, inserted vehicle will be addd to this list

void sort_list(int totalsize);  //Sort the vehicle in the list by lane then distance to the stop bar

int determine_state(float cur_pos, float cur_spd, float cur_acc, float lead_pos, float lead_spd, float lead_acc); //determine state of the vehicle based on Wiedemann car following model
//State definition:   1: free flow    2: emergency    3: closing    4: following

float cal_dec(float speed);  //Calculate the deceleration rate based on current spd

void EVLS(int phase_est, long current_time, float red_elapse_time, int No_lanes);    //phase_est is the current estimated phase
