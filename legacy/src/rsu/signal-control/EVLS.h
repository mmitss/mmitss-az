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
#include "ListHandle.h"

using namespace std;

//Define Parameters for car following model
float jam_length=6.56;  //minmum headway of a vehicle (AX)
float acc_diff_thre=1.96;  //Acceleration difference threshold
float desired_spd=13.8889;         //vehicle desired speed (ffs)   m/s = 50km/h
float lamda=0.162;            //parameter for estimating speed
float veh_length=4.75;       //unified vehicle length

extern char temp_log[512];


extern float penetration;       //penetration rate
extern float dsrc_range;
extern LinkedList <ConnectedVehicle> vehlist_each_phase;  //vehicle list that saves the raw data from trajectory awareness
extern int ArrivalTable[131][8];     //maximum planning time horizon * number of phases

extern int outputlog(char *output);

LinkedList <ConnectedVehicle> sorted_vehlist;  //sorted list by Lane No and distance to stop bar, inserted vehicle will be addd to this list

void sort_list(int totalsize);  //Sort the vehicle in the list by lane then distance to the stop bar

int determine_state(float cur_pos, float cur_spd, float cur_acc, float lead_pos, float lead_spd, float lead_acc); //determine state of the vehicle based on Wiedemann car following model
//State definition:   1: free flow    2: emergency    3: closing    4: following

float cal_dec(float speed);  //Calculate the deceleration rate based on current spd

void EVLS(int phase_est, long current_time, float red_elapse_time, int No_lanes);    //phase_est is the current estimated phase

void EVLS(int phase_est, long current_time, float red_elapse_time, int No_lanes)
{
	int i,j;
	
	ConnectedVehicle temp_veh;

	sorted_vehlist.ClearList();
	
	//other parameters
	//long current_time=1400265597;

	//Note:  always plan at the beginning of the green, so here plan at beginning of 2 and 6, red elapse time of 2 and 6 is the previous red duration
	//int phase_read[8]={1, 3, 1, 1, 1, 3, 1, 1}; 
	//float red_elapse_time[8]={33.870200, 50.370252, 2.100000, 14.330087, 33.870198, 47.150205, 2.099997, 15.550095};

	//All the vehicle data are from phase 2 now!!

	//Currently it is 50% penetration rate!!
	//int No_lanes=1;  //number of lanes on the approach

	sprintf(temp_log,"The sorted Vehicle List is:\n");
	outputlog(temp_log);cout<<temp_log;
	
	sort_list(vehlist_each_phase.ListSize());  //sort list to the correct order

	//print the sorted list

	sorted_vehlist.Reset();
	cout.precision(12);
	while(!sorted_vehlist.EndOfList())  
	{
		cout<<sorted_vehlist.Data().lane<<" "<<sorted_vehlist.Data().stopBarDistance<<" "<<sorted_vehlist.Data().Speed<<" "<<sorted_vehlist.Data().acceleration<<" "<<sorted_vehlist.Data().time_stop<<endl;
		sorted_vehlist.Next();
	}

	//find out how many vehicles in each lane
	int * No_veh_per_lane;   //number of vehicle each lane (both equipped and later inserted)
	No_veh_per_lane= new int[No_lanes];

	//lane No and lane mapping
	int * Lane_No_Mapping;
	Lane_No_Mapping=new int[No_lanes];

	cout<<"I am here!"<<endl;

	int index=0;
	sorted_vehlist.Reset();
	int tmp_lane=sorted_vehlist.Data().lane;
	int count=1;
	No_veh_per_lane[index]=count;
	sorted_vehlist.Next();
	while(!sorted_vehlist.EndOfList())  
	{
		if(sorted_vehlist.Data().lane==tmp_lane)
		{
			count++;
			No_veh_per_lane[index]=count;
			Lane_No_Mapping[index]=sorted_vehlist.Data().lane;			
		}
		else
		{
			count=1;
			index++;
			No_veh_per_lane[index]=count;
			tmp_lane=sorted_vehlist.Data().lane;
			Lane_No_Mapping[index]=sorted_vehlist.Data().lane;
		}
		sorted_vehlist.Next();
	}

	sprintf(temp_log,"The lane index mapping to each lane is: %d %d %d\n",Lane_No_Mapping[0],Lane_No_Mapping[1],Lane_No_Mapping[2]);
	outputlog(temp_log);cout<<temp_log;

	sprintf(temp_log,"The No. of vehicle per lane is: %d %d %d\n",No_veh_per_lane[0],No_veh_per_lane[1],No_veh_per_lane[2]);
	outputlog(temp_log);cout<<temp_log;
	
	
	//reduce No_lanes if no vehicle in one lane
	No_lanes=index+1;
	
	sprintf(temp_log,"Reduced lane to: %d\n",No_lanes);
	outputlog(temp_log);cout<<temp_log;
	
	
	//Record how many vehicles are inserted each lane in the slow down region
	int *inserted_each_lane; 
	inserted_each_lane=new int[10]; //at most 8 lanes, no need to map the lane id!!
	
	//Define maximum number of vehicles can be inserted each lane
	int max_insertion;
	if (penetration>=0.75)
		max_insertion=1;
	if (penetration<0.75 && penetration>=0.5)
		max_insertion=2;
	if (penetration<0.5)
		max_insertion=3;
	
	int max_q_prop_time= (int) (1.0/penetration);
	
	//Step 1: calculate the queuing part for each lane
	float * est_queue_length;
	est_queue_length= new float[No_lanes];

	//number of queued equipped vehicle each lane
	int *No_queue_veh;
	No_queue_veh= new int[No_lanes];

	//Last queued equipped vehicle position
	float *Last_queue_equipped_veh_pos;
	Last_queue_equipped_veh_pos=new float[No_lanes];

	//number of queued vehicle (both equipped and unequipped) each lane
	int *No_queue_veh_all;  //The same
	No_queue_veh_all= new int[No_lanes];

	for (i=0;i<No_lanes;i++)
	{
		No_queue_veh[i]=0;
	}

	sorted_vehlist.Reset();
	for (j=0;j<No_lanes;j++)
	{
		long time1=current_time-(int) red_elapse_time;    //red start time Note: this index is only for phase 6, need to be changed later!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		float queue_length1=0;
		double queue_speed=0;
		float queue_length2=0;
		long time2;
		for(i=0;i<No_veh_per_lane[j];i++)
		{
			if(sorted_vehlist.Data().Speed<0.1)  //stopped vehicle
			{
				queue_length2=sorted_vehlist.Data().stopBarDistance;
				time2=sorted_vehlist.Data().time_stop;
				//Need to deal with the case of over saturation, some vehicle stops before the red beginning at the end of the queue
				if (time2<=time1)
					queue_speed=0;
				else
					queue_speed=(queue_length2-queue_length1)/(time2-time1);
				time1=time2;
				queue_length1=queue_length2;
				No_queue_veh[j]++;
			}
			sorted_vehlist.Next();
		}
		int q_time_after_last_veh;
		if(max_q_prop_time<=current_time-time2)
			q_time_after_last_veh=max_q_prop_time;
		else
			q_time_after_last_veh=current_time-time2;
			
		est_queue_length[j]=queue_length2+queue_speed*q_time_after_last_veh;
		Last_queue_equipped_veh_pos[j]=queue_length2;
	}
	
	sprintf(temp_log,"Current time is: %d\n",current_time);
	outputlog(temp_log);cout<<temp_log;
	
	sprintf(temp_log,"Estimated queue length of each lane is: %f %f %f\n",est_queue_length[0],est_queue_length[1],est_queue_length[2]);
	outputlog(temp_log);cout<<temp_log;


	//For queuing part, all we need is the No. of vehicles because all the ETAs of queued vehicle are 0
	/*  Calculate the No. Vehicles in queue based on queue length and add vehicle to sorted_vehlist */
	
	int flag=0;  //This flag is used to identify the position of the inserted vehicle
	for (i=0;i<No_lanes;i++)
	{
		if(i==0)
		flag=No_queue_veh[i];
		else //i>0
		{
			flag=0;
			for(j=0;j<i;j++)
				flag+=No_veh_per_lane[j];
			flag+=No_queue_veh[i];
		}
		int No_total_veh_queue=ceil(est_queue_length[i]/jam_length)+1;//+1 is because of the vehicle at the stop line
		No_queue_veh_all[i]=No_total_veh_queue;
		//Note here the sum of No_queue_veh_all is not equal to the size of sorted_vehlist, because only add vehicle at the last of the queue, not for other gaps!!!!!!!!!!!!!!!!!!!!!!!
		if(est_queue_length[i]-Last_queue_equipped_veh_pos[i]>jam_length/2) //Insert a vehicle at the last of the queue for calculation in the slow down region
		{			
				temp_veh.Speed=0;
				temp_veh.stopBarDistance=Last_queue_equipped_veh_pos[i]+jam_length;
				temp_veh.lane=Lane_No_Mapping[i];
				temp_veh.acceleration=0;
				sorted_vehlist.Reset(flag);
				sorted_vehlist.InsertAt(temp_veh);
				No_veh_per_lane[i]++;
		}
	}
	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Use this No_queue_veh_all value for the No of vehicle for the queuing part for the arrival table
	sprintf(temp_log,"The number of queued vehicle of each lane are (both equipped and unequipped): %d %d %d\n",No_queue_veh_all[0],No_queue_veh_all[1],No_queue_veh_all[2]);
	outputlog(temp_log);cout<<temp_log;
	
	

	//print the sorted list	
	sprintf(temp_log,"Vehicle list after adding virtual queuing vehicle is:\n");
	outputlog(temp_log);cout<<temp_log;
	
	//~ sorted_vehlist.Reset();
	//~ cout.precision(12);
	//~ while(!sorted_vehlist.EndOfList())  
	//~ {
		//~ sprintf(temp_log,"%d %f %f %f\n",sorted_vehlist.Data().lane,sorted_vehlist.Data().stopBarDistance,sorted_vehlist.Data().Speed,sorted_vehlist.Data().time_stop);
		//~ outputlog(temp_log);cout<<temp_log;
		//~ sorted_vehlist.Next();
	//~ }

	
	//Step2: Calculate the slow-down part of each lane
	//Define the slow down region
	float slow_region=50;    //50m for the slow down region temporary, need to change later!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	float ave_queue_length;  //average queue length of each lane, since in vissim the queue length are almost the same for all lanes
	float total_queue_length=0;
	for(i=0;i<No_lanes;i++)
	{
		total_queue_length+=est_queue_length[i];
	}
	ave_queue_length=total_queue_length/No_lanes;


	sorted_vehlist.Reset();
	//Assign moving dynamics to leading vehicle
	//We can't insert vehicle for the first vehicle because lack of information
	float speed_pre=sorted_vehlist.Data().Speed;
	float acc_pre=sorted_vehlist.Data().acceleration;
	float pos_pre=sorted_vehlist.Data().stopBarDistance;
	float lane_pre=sorted_vehlist.Data().lane;
	sorted_vehlist.Next();



	while (!sorted_vehlist.EndOfList())
	{
		

		if(sorted_vehlist.Data().Speed!=0 && sorted_vehlist.Data().lane==lane_pre && sorted_vehlist.Data().stopBarDistance>ave_queue_length+3 && sorted_vehlist.Data().stopBarDistance<ave_queue_length+slow_region)  //moving vehicle in the same lane
		{
			//Determine car following state
			int veh_state=determine_state(sorted_vehlist.Data().stopBarDistance,sorted_vehlist.Data().Speed,sorted_vehlist.Data().acceleration,pos_pre,speed_pre,acc_pre);
			//calculate max acc and min acc
			float min_spd=sorted_vehlist.Data().Speed<=speed_pre?sorted_vehlist.Data().Speed:speed_pre;
			float max_acc=3.5-3.5/40*min_spd;  //m/s
			float min_acc=-20+1.5/60*min_spd;

			float delta_x=sorted_vehlist.Data().stopBarDistance-pos_pre;  //headway
			float AX=jam_length;  //min headway
			float CX=40;
			float SDV=pow((delta_x-AX)/CX,2);
			float OPDV=-2.25*SDV;
			float BX=2.5*sqrt(min_spd);
			float ABX=AX+2.5*sqrt(min_spd);
			float SDX=AX+3.75*sqrt(min_spd);
			float delta_v=sorted_vehlist.Data().Speed-speed_pre;

			//calculate the desired acceleration based on vehicle state
			float desired_acc;
			if (veh_state==1)  //free flow state
				desired_acc=max_acc<desired_spd-sorted_vehlist.Data().Speed?max_acc:desired_spd-sorted_vehlist.Data().Speed;
			if (veh_state==2)  // Emergency state
				desired_acc=0.5*(delta_v*delta_v/(AX-delta_x))+acc_pre+min_acc*(ABX-delta_x)/BX;
			if (veh_state==3)  //closing state
			{
				float temp=0.5*delta_v*delta_v/(ABX-delta_x)+acc_pre;
				if(temp>=min_acc)
					desired_acc=temp;
				else
					desired_acc=min_acc;
			}
			if (veh_state==4)  //following state
				desired_acc=0;

			//Determin whether to trigger the insertion
			int trigger=0;               //0: not trigger  1: triggered insertion
			//Trigger one: in free flow region, if acc is two small
			if (veh_state==1 && desired_acc-sorted_vehlist.Data().acceleration>acc_diff_thre)
				trigger=1;
			//Trigger two: in closing or following region, and following distance is too long
			if ((veh_state==3 || veh_state==4) && delta_x>2*SDX)   //here 2 is a parameter for calibration!!!!!!!!!!!!!  modify the original model!!!!!!!!!!!!!!!!!!
				trigger=1;

			//If triggered insertion 
			if (trigger==1)
			{
				//Calculate pos, speed and acc of inserted vehicle
				//speed
				float tmp_spd=sorted_vehlist.Data().Speed+lamda*sorted_vehlist.Data().acceleration>0?sorted_vehlist.Data().Speed+lamda*sorted_vehlist.Data().acceleration:0;
				temp_veh.Speed=tmp_spd;

				//acc
				if (acc_pre<=0 && sorted_vehlist.Data().acceleration<=0)  //if both of the leading and following vehicle are decelerating
					temp_veh.acceleration=cal_dec(tmp_spd);
				else
					temp_veh.acceleration=0;

				//position
				float tmp_delta_x;
				if (tmp_spd<sorted_vehlist.Data().Speed)  //the inserted leading vehicle is slower, the current vehicle is in closing region
				{
					if (fabs(sorted_vehlist.Data().acceleration-temp_veh.acceleration)>0.1)  //if the deceleration rate is different
					tmp_delta_x=7.25+3.75*sqrt(tmp_spd)-0.5*pow(min(-lamda*sorted_vehlist.Data().acceleration,sorted_vehlist.Data().Speed),2)/(sorted_vehlist.Data().acceleration-temp_veh.acceleration);
					else
					tmp_delta_x=7.25+3.75*sqrt(tmp_spd);
				}
				else
					tmp_delta_x=7.25+3.75*sqrt(tmp_spd);
				temp_veh.stopBarDistance=sorted_vehlist.Data().stopBarDistance-tmp_delta_x;
				
				//lane    same lane
				temp_veh.lane=sorted_vehlist.Data().lane;

				//check the availablity of the inserted vehicle
				if (temp_veh.stopBarDistance>pos_pre+jam_length && inserted_each_lane[temp_veh.lane]<max_insertion)
				//if (temp_veh.stopBarDistance>pos_pre+jam_length)
				{
					//increase the inserted vehicle number
					inserted_each_lane[temp_veh.lane]++;
					
					//Add the inserted vehicle to the list
					sorted_vehlist.InsertAt(temp_veh);
					cout<<"Current Vehicle "<<sorted_vehlist.Data().lane<<" "<<sorted_vehlist.Data().stopBarDistance<<" "<<sorted_vehlist.Data().Speed<<" "<<sorted_vehlist.Data().acceleration<<" "<<sorted_vehlist.Data().time_stop<<endl;
					int Pos_in_list=sorted_vehlist.CurrentPosition();
					sorted_vehlist.Reset(Pos_in_list-1);
				}
			}

		}

		speed_pre=sorted_vehlist.Data().Speed;
		acc_pre=sorted_vehlist.Data().acceleration;
		pos_pre=sorted_vehlist.Data().stopBarDistance;
		lane_pre=sorted_vehlist.Data().lane;
		sorted_vehlist.Next();

	}

	//Step 3: Insert Vehicles in the free flow region
	//Step 3.1 Count No. of veh in free flow region
	int eqp_veh_in_free_region=0;
	int * eqp_veh_in_fr_lane;
	eqp_veh_in_fr_lane= new int[No_lanes];
	for (i=0;i<No_lanes;i++)
	{
		eqp_veh_in_fr_lane[i]=0;
	}

	float total_speed=0;
	sorted_vehlist.Reset();
	while (!sorted_vehlist.EndOfList())
	{
		if (sorted_vehlist.Data().stopBarDistance>ave_queue_length+slow_region)   //when vehicles are in free flow region
		{
			eqp_veh_in_free_region++;
			total_speed+=sorted_vehlist.Data().Speed;
			for(i=0;i<No_lanes;i++)
			{
				if(sorted_vehlist.Data().lane==Lane_No_Mapping[i])
					eqp_veh_in_fr_lane[i]++;
			}
		}
		sorted_vehlist.Next();
	}

	int total_veh_in_free_region=eqp_veh_in_free_region/penetration;
	int veh_ffr_each_lane=total_veh_in_free_region/No_lanes;
	float ave_ffs=total_speed/eqp_veh_in_free_region;
		
	//cout<<"Equipped vehile in free flow region of each lane is: "<<eqp_veh_in_fr_lane[0]<<" "<<eqp_veh_in_fr_lane[1]<<" "<<eqp_veh_in_fr_lane[2]<<endl;

	for (i=0;i<No_lanes;i++)
	{
		int inserted_No=veh_ffr_each_lane-eqp_veh_in_fr_lane[i];
		for (j=0;j<inserted_No;j++)
		{
			float distance=(dsrc_range-ave_queue_length-slow_region)/inserted_No;
			//here should come to the end of the list
			temp_veh.lane=Lane_No_Mapping[i];
			temp_veh.stopBarDistance=ave_queue_length+slow_region+distance*j;
			temp_veh.acceleration=0;
			temp_veh.Speed=ave_ffs;
			sorted_vehlist.InsertRear(temp_veh);
		}
	}


	//print the final vehicle list
	sprintf(temp_log,"Total No. of Vehicle in the list is %d:\n",sorted_vehlist.ListSize());
	outputlog(temp_log);cout<<temp_log;
	
	sprintf(temp_log," Final Vehicle list size is:\n");
	outputlog(temp_log);cout<<temp_log;


//	cout.precision(12);	
//	fstream fp;
//	fp.open("estimated_vehlist.txt",fstream::out);

	//~ sorted_vehlist.Reset();
//~ 
	//~ while(!sorted_vehlist.EndOfList())  
	//~ {
		//~ sprintf(temp_log,"%d %f %f %f %f\n",sorted_vehlist.Data().lane,sorted_vehlist.Data().stopBarDistance,sorted_vehlist.Data().Speed,sorted_vehlist.Data().acceleration,sorted_vehlist.Data().time_stop);
		//~ outputlog(temp_log);cout<<temp_log;
//~ 
		//~ sorted_vehlist.Next();
	//~ }
//		fp.close();

	//cout<<"I am here1"<<endl;

	//write to the corresponding column of arrival table
	sorted_vehlist.Reset();
	
	int total_veh_queue_all_lane=0;
	
	for(i=0;i<No_lanes;i++)
	{
		total_veh_queue_all_lane+=No_queue_veh_all[i];
	}
	ArrivalTable[0][phase_est-1]=total_veh_queue_all_lane;  //Total number of queued vehicle
	
	cout<<"I am here2"<<endl;
	
	while(!sorted_vehlist.EndOfList())  
	{
		
		if (sorted_vehlist.Data().Speed>2)  //stopped vehicle
		{
			int ETA= ceil(sorted_vehlist.Data().stopBarDistance/sorted_vehlist.Data().Speed);
			if (ETA<=50)
				ArrivalTable[ETA][phase_est-1]++;     //increase the value in ETA's row
		}
		sorted_vehlist.Next();
	}

	cout<<"I am here3"<<endl;

delete(No_veh_per_lane);
delete(Lane_No_Mapping);
delete(inserted_each_lane);
delete(est_queue_length);
delete(No_queue_veh);
delete(No_queue_veh_all);
delete(eqp_veh_in_fr_lane);

cout<<"I am here4"<<endl;



}

void sort_list(int totalsize)
{
	ConnectedVehicle tmpveh;
	int list_size=0;
	vehlist_each_phase.Reset();
	int i;
	while(!vehlist_each_phase.EndOfList())  //match vehicle according to vehicle ID
	{
		if(list_size==0)  //first one always insert to the list
		{
			tmpveh=vehlist_each_phase.Data();
			sorted_vehlist.InsertRear(tmpveh);
			list_size++;
		}
		else
		{
			sorted_vehlist.Reset();
			for(i=0;i<sorted_vehlist.ListSize();i++)  //insert the vehicle to the right place
			{
				if(vehlist_each_phase.Data().lane>sorted_vehlist.Data().lane)  //compare lane first
					sorted_vehlist.Next();
				else if (vehlist_each_phase.Data().lane==sorted_vehlist.Data().lane)
				{
					if(vehlist_each_phase.Data().stopBarDistance>sorted_vehlist.Data().stopBarDistance)  //compare distance to stopbar second
						sorted_vehlist.Next();
					else
					{
						tmpveh=vehlist_each_phase.Data();
						sorted_vehlist.InsertAt(tmpveh);
						break;
					}
				}
				else
				{
					tmpveh=vehlist_each_phase.Data();
					sorted_vehlist.InsertAt(tmpveh);
					break;
				}
			}
			if (i==sorted_vehlist.ListSize())  //means already comes to the last one
			{
				tmpveh=vehlist_each_phase.Data();
				sorted_vehlist.InsertRear(tmpveh);
			}
		}
		if(sorted_vehlist.ListSize()==totalsize)
			break;
		vehlist_each_phase.Next();
	}
}

int determine_state(float cur_pos, float cur_spd, float cur_acc, float lead_pos, float lead_spd, float lead_acc)
{
	int state=0;
	float delta_x=cur_pos-lead_pos;  //headway
	float AX=jam_length;  //min headway
	float CX=40;
	float SDV=pow((delta_x-AX)/CX,2);
	float OPDV=-2.25*SDV;
	float ABX_v=min(cur_spd,lead_spd);   //this speed is used in the calculation of ABX
	float ABX=AX+2.5*sqrt(ABX_v);
	float SDX=AX+3.75*sqrt(ABX_v);
	float delta_v=cur_spd-lead_spd;

	if(delta_v<OPDV)
		state=1;
	else
	{
		if (delta_x<ABX)
			state=2;
		else
		{
			if (delta_v>SDV)
				state=3;
			else
			{
				if(delta_x>SDX)
					state=1;
				else
					state=4;
			}
		}
	}
	return state;
}

float cal_dec(float speed)  //Calculate the deceleration rate based on current spd
{
	float dec=0;
	if(speed<2.78)
		dec=-0.91;
	if (speed>=2.78 && speed<5.56)
		dec=-1.92;
	if (speed>=5.56 && speed<8.33)
		dec=-1.82;
	if (speed>=8.33 && speed<11.11)
		dec=-1.26;
	if (speed>=11.11 && speed<13.89)
		dec=-0.67;
	return dec;
}
