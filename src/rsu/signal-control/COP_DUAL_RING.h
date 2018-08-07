#include <stdio.h>
#include <iostream>
#include <math.h>


using namespace std;

#define		J       2    /*  number of barriers/stages planned  */
#define		P	8    /*  number of phases  */
#define		T       120   /*  total planned time +1  */

extern int MinGreen[8];
extern int MaxGreen[8];
extern int Yellow[8];
extern int Red[8];
extern int PedWalk[8];
extern int PedClr[8];
extern int Ped_Phase_Considered[8];

extern int ArrivalTable[131][8];
//extern int opt_s[3];
extern int opt_sig_plan[2][2*2];
extern int opt_sig_seq[2][2*2];
extern char temp_log[512];
extern int LaneNo[8];

extern int outputlog(char *output);

extern int delay_type;

extern int queue_warning_P1;
extern int queue_warning_P5;

extern double red_elapse_time[8];   //Red elapse time of each phase


float	v[J][T];	/*table of values for barrier level*/
int	X[J][T];	/*table of decisions for barrier level*/

//int InitPhase[2];  //The green phase of two rings when COP starts
//int GrnElapse[2];  //The green elapse time
//int PassedPhase[2];  //Already passed phase in the same barrier for ring 1 and ring 2, only used for the first stage/barrier

int B_Min[J];   //minimum time of each barrier/stage
int B_Max[J];   //maximum time of each barrier/stage

//int Skip_phase[P];    //if no vehicle is coming in the arrival table, corresponding phase can be skipped
                      //0: can skip the phase; 1: can't skip the phase

int planned_phase[2][2];    //planned phase in one barrier, two rings

int best_phase_schedule[J][T][2][2];    //stage * maximum possible barrier length * ring1 * ring2
int best_phase_sequence[J][T][2][2];    


int Q_temp[T][P];  //temporary standing queue at each stage
int Q_perm[J][T][P];  //permanent standing queue at each stage

int COP_DUAL_RING(int InitPhase[2], int GrnElapse[2], int PassedPhase[2], int Skip_phase[8]);
float f(int ring, int phase1, int phase2, int g1, int g2, int total_length, int Num_Veh[8], float Num_Ratio[8]);  //calculate the delay when planning two phases at the first stage
float f1(int ring, int phase1, int phase2, int g1, int g2, int total_length, int B_L, int stage, int Num_Veh[8],float Num_Ratio[8]); //calculate the delay when planning two phases at the following stages the last parameter is the length of this planning barrier
float f_r(int ring, int phase1, int phase2, int g1, int g2, int total_length,int Num_Veh[8],float Num_Ratio[8]);  //calculate the delay when planning one phase at the first stage
float f1_r(int ring, int phase1, int phase2, int g1, int g2, int total_length, int B_L, int stage, int Num_Veh[8],float Num_Ratio[8]);  //calculate the delay when planning one phase at the following stages

float f_other(int ring, int phase1, int phase2, int total_length, int Num_Veh[8],float Num_Ratio[8]);   //calculate the delay of ring other than phase1 and phase2

float f1_other(int ring, int phase1, int phase2, int total_length, int B_L, int stage, int Num_Veh[8],float Num_Ratio[8]);


//int delay(int phase1, int phase2, int g1, int g2, int total_length); //calculate the delay, the order of the phases are pre-ordered
int add_missing_phase(int barrier);

int COP_DUAL_RING(int InitPhase[2], int GrnElapse[2], int PassedPhase[2], int Skip_phase[8])
{

//int Phases[P]={1,2,3,4,5,6,7,8};      //total 8 phases

int i,j,k;
int stage;
		
int t; //time horizon

//InitPhase[0]=1; InitPhase[1]=5;  //initial phase
//GrnElapse[0]=10; GrnElapse[1]=9;   //green elapse time
//PassedPhase[0]=0; PassedPhase[1]=0;  //no phase in this ring has passed
//Skip_phase[0]=1;Skip_phase[1]=1;Skip_phase[2]=1;Skip_phase[3]=1;Skip_phase[4]=1;Skip_phase[5]=1;Skip_phase[6]=1;Skip_phase[7]=1; //no phase skipped

int start_barrier;


sprintf(temp_log,"The Delay Type is: %d\n",delay_type);
outputlog(temp_log);cout<<temp_log;


sprintf(temp_log,"Ped Phase needs to be considered: %d %d %d %d %d %d %d %d\n",Ped_Phase_Considered[0],Ped_Phase_Considered[1],Ped_Phase_Considered[2],Ped_Phase_Considered[3],Ped_Phase_Considered[4],Ped_Phase_Considered[5],Ped_Phase_Considered[6],Ped_Phase_Considered[7]);
outputlog(temp_log);cout<<temp_log;

//1. Decide which barrier we start from
start_barrier=(InitPhase[0]<3)?0:1; 

//Consider queue warning!!!!!!!
if (start_barrier==0)  //first barrier
{
	if(queue_warning_P1==1)
	MinGreen[0]=18;
	if(queue_warning_P5==1)
	MinGreen[4]=18;
}
//consider the Ped constraint, if the ped phase is on, or called, then when planning, MinGreen should be changed
int MinGreen_Ped[8];   //This is the minimum green time considering the ped !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
for(i=0;i<8;i++)
{
	if (Ped_Phase_Considered[i]!=0)
		MinGreen_Ped[i]= max(PedWalk[i]+PedClr[i],MinGreen[i]);
	else
		MinGreen_Ped[i]=MinGreen[i];
}




//initial best_phase_schedule and best_phase_sequence
for (i=0;i<J;i++)
{
	for(j=0;j<T;j++)
	{
		best_phase_schedule[i][j][0][0]=0;
		best_phase_schedule[i][j][0][1]=0;
		best_phase_schedule[i][j][1][0]=0;
		best_phase_schedule[i][j][1][1]=0;
		best_phase_sequence[i][j][0][0]=0;
		best_phase_sequence[i][j][0][1]=0;
		best_phase_sequence[i][j][1][0]=0;
		best_phase_sequence[i][j][1][1]=0;
	}
}

//initial pernement queue

for(i=0;i<J;i++)
{
	for(j=0;j<T;j++)
	{
		v[i][j]=0;
		X[i][j]=0;
		for(k=0;k<P;k++)
		{
			Q_perm[i][j][k]=0;
		}
	}
}



stage=0;

//2. Construct the first stage barrier

//2.1 decide which phases are in plan of the two rings
for (j=0;j<2;j++)
{
	planned_phase[j][0]=1+start_barrier*2+j*4;
	planned_phase[j][1]=2+start_barrier*2+j*4;
	for(i=0;i<2;i++)
	{
		if(Skip_phase[i+start_barrier*2+j*4]==0)
			planned_phase[j][i]=0;  //skip the phase if no call
	}

	if(PassedPhase[j]!=0)
	{
		planned_phase[j][PassedPhase[j]-start_barrier*2-j*4-1]=0;  //skip the passed phase
	}
	planned_phase[j][InitPhase[j]-start_barrier*2-j*4-1]=InitPhase[j];  //the phase already started can't be skipped even if no call
}


//sprintf(temp_log,"The planned phases are: %d %d %d %d\n",planned_phase[0][0],planned_phase[0][1],planned_phase[1][0],planned_phase[1][1]);
//outputlog(temp_log); cout<<temp_log;


//2.2 Based on the planned pahse, calculate min and max for the barrier
int ring1_min, ring2_min, ring1_max, ring2_max;
ring1_min=0; ring2_min=0; ring1_max=0; ring2_max=0;

for(i=0;i<2;i++)  //ring 1
{
	if(planned_phase[0][i]!=0 && planned_phase[0][i]!=InitPhase[0])  //not the current phase, but in plan
	{
		ring1_min+=MinGreen_Ped[planned_phase[0][i]-1]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
		ring1_max+=MaxGreen[planned_phase[0][i]-1]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
	}
	if (planned_phase[0][i]!=0 && planned_phase[0][i]==InitPhase[0])  //the current phase in plan
	{
		ring1_min+=Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1]+max(0,MinGreen_Ped[planned_phase[0][i]-1]-GrnElapse[0]);
		ring1_max+=MaxGreen[planned_phase[0][i]-1]-GrnElapse[0]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
	}
}
for(i=0;i<2;i++)  //ring 2
{
	if(planned_phase[1][i]!=0 && planned_phase[1][i]!=InitPhase[1])
	{
		ring2_min+=MinGreen_Ped[planned_phase[1][i]-1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
		ring2_max+=MaxGreen[planned_phase[1][i]-1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
	}
	if(planned_phase[1][i]!=0 && planned_phase[1][i]==InitPhase[1])  //the current phase in plan
	{
		ring2_min+=Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1]+max(0,MinGreen_Ped[planned_phase[1][i]-1]-GrnElapse[1]);
		ring2_max+=MaxGreen[planned_phase[1][i]-1]-GrnElapse[1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
	}
}
B_Min[stage]=max(ring1_min,ring2_min);
B_Max[stage]=min(ring1_max,ring2_max);  //if one ring exceeds the maximum green time, just put the last phase to green rest!!!!!

//2.3 for each fixed Barrier time, manipulate the green time between the two phases
int tempG[2][2]; //temp green time for each combination
int effectiveG[2];  //for ring 1 and ring 2;

for(i=1;i<T;i++)
{

	//reset Q_temp
	for (j=1;j<T;j++)
		for(k=0;k<P;k++)
			Q_temp[j][k]=0;
	//assign initial queue to Q-Temp[0][P]
	for (k=0;k<P;k++)
	{
		Q_temp[0][k]=ArrivalTable[0][k];
	}

	if (B_Min[stage]<=i && B_Max[stage]>=i) //feasible
	{
		X[0][i]=i; //decision variable
		//cout<<"Barrier length is: "<<i<<endl;
		//calculate the effective total green time, just length - Y - R of each planned phase
		effectiveG[0]=i;effectiveG[1]=i;
		for (j=0;j<2;j++)
			for(k=0;k<2;k++)
			{
				if (planned_phase[j][k]!=0)
					effectiveG[j]-=Yellow[planned_phase[j][k]-1]+Red[planned_phase[j][k]-1];
			}
		//Add another constraint: if the other ring only have one phase, this the sum of two phases of this ring shouldn't exceed the maximum green of the phase in the other ring
		

		//Calculate Number of Vehicle in each phase until time i;
		int Num_veh[8]={0};   //Number of vehicles each phase
		int total_num_veh=0;
		float Num_Veh_Ratio[8]={0};  //radio of vehicles e.g. ratio=Num_veh[i]/(sum(Num_veh[i]);
		for(int ii=0;ii<P;ii++)
		{
			for (int iii=0;iii<i;iii++)
			{
				Num_veh[ii]+=ArrivalTable[iii][ii];
			}
		total_num_veh+=Num_veh[ii];
		}
		for(int ii=0;ii<P;ii++)
			Num_Veh_Ratio[ii]=Num_veh[ii]/(total_num_veh*1.0);


		//for both of two rings there are three cases
		float temp_v[2];
		temp_v[0]=99999.0; temp_v[1]=99999.0;
		for (k=0;k<2;k++)  //two rings
		{

			//cout<<"ring "<<k+1<<endl;
			//case 1: both phases are not skipped
			if(planned_phase[k][0]!=0 && planned_phase[k][1]!=0)
			{
				//the initial phase can ternimate at any time, so start from 0 to Gmax-GrnElapse
				if(planned_phase[k][0]==InitPhase[k])
				{
					float delay_other_phase=f_other(k, planned_phase[k][0],planned_phase[k][1],i, Num_veh,Num_Veh_Ratio);   //calculate the delay of other phases in the same ring rather than planned phase
					for(j=max(0,MinGreen_Ped[planned_phase[k][0]-1]-GrnElapse[k]);j<=MaxGreen[planned_phase[k][0]-1]-GrnElapse[k];j++)
					{
						tempG[k][0]=j;
						tempG[k][1]=effectiveG[k]-tempG[k][0];
						if (tempG[k][1]>=MinGreen_Ped[planned_phase[k][1]-1] && tempG[k][1]<=MaxGreen[planned_phase[k][1]-1])  //feasible solution
						{
							//calculate delay function produced by this combination!!!!!!!!!!!!!!!!!!!!
							float temp_vv=f(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,Num_veh,Num_Veh_Ratio)+delay_other_phase;
							if (temp_vv<temp_v[k]) //calculate the delay
							{
								temp_v[k]=temp_vv; //save the best delay
								//save the best phase schedule
								best_phase_schedule[stage][i][k][0]=tempG[k][0];
								best_phase_schedule[stage][i][k][1]=tempG[k][1];
								for (int kkk=4*k;kkk<4+4*k;kkk++)
										Q_perm[stage][i][kkk]=Q_temp[i][kkk];
							}
							//cout<<"tempG[0][0] is "<<tempG[k][0]<<endl;
							//cout<<"tempG[0][1] is "<<tempG[k][1]<<endl;
						}
					}
				}
				if(planned_phase[k][1]==InitPhase[k])
				{
					float delay_other_phase=f_other(k, planned_phase[k][1],planned_phase[k][0],i, Num_veh,Num_Veh_Ratio);   //calculate the delay of other phases in the same ring rather than planned phase
					for(j=max(0,MinGreen_Ped[planned_phase[k][1]-1]-GrnElapse[k]);j<=MaxGreen[planned_phase[k][1]-1]-GrnElapse[k];j++)
					{
						tempG[k][1]=j;
						tempG[k][0]=effectiveG[k]-tempG[k][1];
						if (tempG[k][0]>=MinGreen_Ped[planned_phase[k][0]-1] && tempG[k][0]<=MaxGreen[planned_phase[k][0]-1])  //feasible solution
						{
							//calculate delay function produced by this combination!!!!!!!!!!!!!!!!!!!!
							float temp_vv=f(k,planned_phase[k][1],planned_phase[k][0],tempG[k][1],tempG[k][0],i,Num_veh,Num_Veh_Ratio)+delay_other_phase;
							if (temp_vv<temp_v[k]) //calculate the delay
							{
								temp_v[k]=temp_vv;  //save the best delay
								//save the best phase schedule
								best_phase_schedule[stage][i][k][0]=tempG[k][0];
								best_phase_schedule[stage][i][k][1]=tempG[k][1];
									for (int kkk=4*k;kkk<4+4*k;kkk++)
										Q_perm[stage][i][kkk]=Q_temp[i][kkk];
							}
							//cout<<"tempG[0][0] is "<<tempG[k][0]<<endl;
							//cout<<"tempG[0][1] is "<<tempG[k][1]<<endl;
						}
					}
				}
			}
			//case 2: first phase is not skipped, second phase is skipped
			if(planned_phase[k][0]!=0 && planned_phase[k][1]==0)
			{
				tempG[k][0]=effectiveG[k];
				tempG[k][1]=0;
				//cout<<"tempG[0][0] is "<<tempG[k][0]<<endl;
				//cout<<"tempG[0][1] is "<<tempG[k][1]<<endl;
				if (tempG[k][0]<=MaxGreen[planned_phase[k][0]-1])
				{
					temp_v[k]=f_r(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,Num_veh,Num_Veh_Ratio);
					//save the best phase schedule
					best_phase_schedule[stage][i][k][0]=tempG[k][0];
					best_phase_schedule[stage][i][k][1]=tempG[k][1];
					for (int kkk=4*k;kkk<4+4*k;kkk++)
						Q_perm[stage][i][kkk]=Q_temp[i][kkk];
				}
				

			}
			//case 3: first phase is skipped, second phase is not skipped
			if(planned_phase[k][0]==0 && planned_phase[k][1]!=0)
			{
				tempG[k][0]=0;
				tempG[k][1]=effectiveG[k];
				//cout<<"tempG[0][0] is "<<tempG[k][0]<<endl;
				//cout<<"tempG[0][1] is "<<tempG[k][1]<<endl;
				if (tempG[k][1]<=MaxGreen[planned_phase[k][1]-1])
				{
					temp_v[k]=f_r(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,Num_veh,Num_Veh_Ratio);
					//save the best phase schedule
					best_phase_schedule[stage][i][k][0]=tempG[k][0];
					best_phase_schedule[stage][i][k][1]=tempG[k][1];
					for (int kkk=4*k;kkk<4+4*k;kkk++)
						Q_perm[stage][i][kkk]=Q_temp[i][kkk];
				}
			}
		}
	v[stage][i]=temp_v[0]+temp_v[1];
	}
	else  //if (i>=B_min && i<=B_max)
	{
		v[stage][i]=99999;   //if not feasible, the delay are set to the maximum
		X[stage][i]=0;
	}
}

cout<<"start of the later stages"<<endl;

//2. Construct the second and third stage barrier

for(stage=1;stage<J;stage++)
{
	//advance to another barrier
	cout<<"Stage "<<stage<<endl;
	start_barrier=(start_barrier==1)?0:1;

	//2. Construct the first stage barrier

	//2.1 decide which phases are in plan of the two rings
	for (j=0;j<2;j++)
	{
		planned_phase[j][0]=1+start_barrier*2+j*4;
		planned_phase[j][1]=2+start_barrier*2+j*4;
		for(i=0;i<2;i++)
		{
			if(Skip_phase[i+start_barrier*2+j*4]==0)
				planned_phase[j][i]=0;  //skip the phase if no call
		}
	}	
	//Complete the barrier constrain by adding missing phase
	if(planned_phase[0][0]!=0 || planned_phase[0][1]!=0 || planned_phase[1][0]!=0 || planned_phase[1][1]!=0) //either one of the phases is not zero
	add_missing_phase(start_barrier);
	
	//sprintf(temp_log,"The planned phases are: %d %d %d %d\n",planned_phase[0][0],planned_phase[0][1],planned_phase[1][0],planned_phase[1][1]);
	//outputlog(temp_log); cout<<temp_log;

	ring1_min=0; ring2_min=0; ring1_max=0; ring2_max=0;

	for(i=0;i<2;i++)  //ring 1
	{
		if(planned_phase[0][i]!=0)
		{
			ring1_min+=MinGreen_Ped[planned_phase[0][i]-1]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
			ring1_max+=MaxGreen[planned_phase[0][i]-1]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
		}
	}
	for(i=0;i<2;i++)  //ring 2
	{
		if(planned_phase[1][i]!=0)
		{
			ring2_min+=MinGreen_Ped[planned_phase[1][i]-1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
			ring2_max+=MaxGreen[planned_phase[1][i]-1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
		}
	}
	B_Min[stage]=max(ring1_min,ring2_min);
	B_Max[stage]=min(ring1_max,ring2_max);  //if one ring exceeds the maximum green time, just put the last phase to green rest!!!!!

	int min_cons=0;
	int max_cons=B_Max[stage];  //minimum, and maximum constrain of the start and end time of the phase

	for(i=0;i<stage;i++)
	{
		min_cons+=B_Min[i];
		max_cons+=B_Max[i];
		max_cons=min(max_cons,T-1); //max time should be constrained by the total amount to time allocated
	}	

	int s;
	int temp_X;

	//Variable to same value of the performance function, with same residual queue, same green length and same phase sequence
	float *perf_func_value_seq1;  
	perf_func_value_seq1= new float[5120000]; //2*40*40*40*40    ring*residual Q1*residual Q2*g1*g2
	float *perf_func_value_seq2;
	perf_func_value_seq2= new float[5120000];

	for(i=1;i<5120000;i++)
	{
		perf_func_value_seq1[i]=99999.0;   //initialize as a big number
		perf_func_value_seq2[i]=99999.0;
	}
	

	//2.3 for each fixed Barrier time, manipulate the green time between the two phases
	for(i=1;i<T;i++)
	{
		if(i>=min_cons+B_Min[stage] && i<=max_cons)
		{
			v[stage][i]=99999;
			//int temp_total_v;
			int opt_schedule[2][2];   //optimal phase sequence
			int opt_seq[2][2];   //optimal phase sequence
			int temp_Q_perm[8];
			for(s=max(B_Min[stage],i-B_Max[stage-1]);s<=min(i-min_cons,B_Max[stage]);s++)
			{

				//Calculate Number of Vehicle in each phase until time between s and i= arrival + residual queue at s-i

				int Num_veh[8]={0};   //Number of vehicles each phase
				int total_num_veh=0;
				float Num_Veh_Ratio[8]={0};  //radio of vehicles e.g. ratio=Num_veh[i]/(sum(Num_veh[i]);
				for(int ii=0;ii<P;ii++)
				{
					for (int iii=i-s;iii<i;iii++)
					{
						Num_veh[ii]+=ArrivalTable[iii][ii];
					}
					Num_veh[ii]+=Q_perm[stage-1][i-s][ii];
					total_num_veh+=Num_veh[ii];
				}

				for(int ii=0;ii<P;ii++)
					Num_Veh_Ratio[ii]=Num_veh[ii]/(total_num_veh*1.0);
				
				//sprintf(temp_log,"Num Vehicle is: %d %d %d %d %d %d %d %d\n",Num_veh[0],Num_veh[1],Num_veh[2],Num_veh[3],Num_veh[4],Num_veh[5],Num_veh[6],Num_veh[7]);
				//outputlog(temp_log);

				float temp_total_v;
				//reset Q_temp
				for (j=1;j<T;j++)
					for(k=0;k<P;k++)
						Q_temp[j][k]=0;
				//assign initial queue to Q-Temp[0][P]
				for (k=0;k<P;k++)
				{
					Q_temp[i-s][k]=Q_perm[stage-1][i-s][k];
				}
				temp_X=s;

				//cout<<"Barrier length is: "<<i<<endl;
				//calculate the effective total green time, just length - Y - R of each planned phase
				effectiveG[0]=s;effectiveG[1]=s;
				for (j=0;j<2;j++)
					for(k=0;k<2;k++)
					{
						if (planned_phase[j][k]!=0)
							effectiveG[j]-=Yellow[planned_phase[j][k]-1]+Red[planned_phase[j][k]-1];
					}
				//for both of two rings there are three cases
				float temp_v[2];
				temp_v[0]=99999; temp_v[1]=99999;
				for (k=0;k<2;k++)  //two rings
				{
					
					//case 1: both phases are not skipped
					if(planned_phase[k][0]!=0 && planned_phase[k][1]!=0)
					{
						float delay_other_phase=f1_other(k,planned_phase[k][0],planned_phase[k][1],i,s,stage,Num_veh,Num_Veh_Ratio);
						for(j=MinGreen_Ped[planned_phase[k][0]-1];j<=MaxGreen[planned_phase[k][0]-1];j++)   //phase sequence 1
						{
							tempG[k][0]=j;
							tempG[k][1]=effectiveG[k]-tempG[k][0];
							if (tempG[k][1]>=MinGreen_Ped[planned_phase[k][1]-1] && tempG[k][1]<=MaxGreen[planned_phase[k][1]-1])  //feasible solution
							{
								//get the residual queue
								float temp_vv;
								int Q_temp_P1=Q_perm[stage-1][i-s][planned_phase[k][0]-1];
								int Q_temp_P2=Q_perm[stage-1][i-s][planned_phase[k][1]-1];
								int array_position=k*2560000+Q_temp_P1*64000+Q_temp_P2*1600+tempG[k][0]*40+tempG[k][1]-1;
								if (Q_temp_P1<=40 && Q_temp_P2<=40 && tempG[k][0]<=40 && tempG[k][1]<=40 && i-s>30)   //make sure all the values are in the boundary of the matrix definition, and the ARRIVAL TABLE IS THE SAME
								{					
									if (perf_func_value_seq1[array_position]<99999)   //already calculated
									{
										temp_vv=perf_func_value_seq1[array_position]+delay_other_phase;
										//temp_vv=f1(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage)+delay_other_phase;
										//count++;
									}
									else
									{
										int f1_value=f1(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage,Num_veh,Num_Veh_Ratio);
										temp_vv=f1_value+delay_other_phase;
										perf_func_value_seq1[array_position]=f1_value;
									}

								}
								else
								{
									temp_vv=f1(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage,Num_veh,Num_Veh_Ratio)+delay_other_phase;
									//perf_func_value_seq1[array_position]=temp_vv;
								}

								//float temp_vv=f1(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage,Num_veh,Num_Veh_Ratio)+delay_other_phase;

								if (temp_vv<temp_v[k]) //calculate the delay
								{
									temp_v[k]=temp_vv; //save the best delay
									//save the best phase schedule
									opt_schedule[k][0]=tempG[k][0];
									opt_schedule[k][1]=tempG[k][1];
									opt_seq[k][0]=planned_phase[k][0];
									opt_seq[k][1]=planned_phase[k][1];

									for (int kkk=4*k;kkk<4+4*k;kkk++)
										temp_Q_perm[kkk]=Q_temp[i][kkk];
								}
								//calculate delay function produced by this combination!!!!!!!!!!!!!!!!!!!!
								//cout<<"tempG[0][0] is "<<tempG[k][0]<<endl;
								//cout<<"tempG[0][1] is "<<tempG[k][1]<<endl;
							}
						}
						
						for(j=MinGreen_Ped[planned_phase[k][1]-1];j<=MaxGreen[planned_phase[k][1]-1];j++)   //switch phase sequence
						{
							tempG[k][1]=j;
							tempG[k][0]=effectiveG[k]-tempG[k][1];
							if (tempG[k][0]>=MinGreen_Ped[planned_phase[k][0]-1] && tempG[k][0]<=MaxGreen[planned_phase[k][0]-1])  //feasible solution
							{
							    float temp_vv;
								int Q_temp_P1=Q_perm[stage-1][i-s][planned_phase[k][1]];
								int Q_temp_P2=Q_perm[stage-1][i-s][planned_phase[k][0]];
								int array_position=k*2560000+Q_temp_P1*64000+Q_temp_P2*1600+tempG[k][1]*40+tempG[k][0]-1;
								//cout<<perf_func_value_seq2[array_position]<<endl;
								if (Q_temp_P1<=40 && Q_temp_P2<=40 && tempG[k][0]<=40 && tempG[k][1]<=40 && i-s>30)   //make sure all the values are in the boundary of the matrix definition
								{					
									if (perf_func_value_seq2[array_position]<99999)  //already calculated
									{
										temp_vv=perf_func_value_seq2[array_position]+delay_other_phase;
										//count++;
									}
									else
									{
										int f1_value=f1(k,planned_phase[k][1],planned_phase[k][0],tempG[k][1],tempG[k][0],i,s,stage,Num_veh,Num_Veh_Ratio);
										temp_vv=f1_value+delay_other_phase;
										perf_func_value_seq2[array_position]=f1_value;
									}
								}
								else
								{
									temp_vv=f1(k,planned_phase[k][1],planned_phase[k][0],tempG[k][1],tempG[k][0],i,s,stage,Num_veh,Num_Veh_Ratio)+delay_other_phase;
									//perf_func_value_seq2[array_position]=temp_vv;
								}

								//float temp_vv=f1(k,planned_phase[k][1],planned_phase[k][0],tempG[k][1],tempG[k][0],i,s,stage,Num_veh,Num_Veh_Ratio)+delay_other_phase;

								if (temp_vv<temp_v[k]) //calculate the delay
								{
									temp_v[k]=temp_vv; //save the best delay
									//save the best phase schedule
									opt_schedule[k][0]=tempG[k][1];
									opt_schedule[k][1]=tempG[k][0];
									opt_seq[k][0]=planned_phase[k][1];
									opt_seq[k][1]=planned_phase[k][0];

									for (int kkk=4*k;kkk<4+4*k;kkk++)
										temp_Q_perm[kkk]=Q_temp[i][kkk];
								}
								//calculate delay function produced by this combination!!!!!!!!!!!!!!!!!!!!
								//cout<<"tempG[0][0] is "<<tempG[k][0]<<endl;
								//cout<<"tempG[0][1] is "<<tempG[k][1]<<endl;
							}
						} 
						
					}
					//case 2: first phase is not skipped, second phase is skipped
					if(planned_phase[k][0]!=0 && planned_phase[k][1]==0)
					{
						tempG[k][0]=effectiveG[k];
						tempG[k][1]=0;
						if (tempG[k][0]<=MaxGreen[planned_phase[k][0]-1])
						{
							temp_v[k]=f1_r(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage,Num_veh,Num_Veh_Ratio);
							//save the best phase schedule
							opt_schedule[k][0]=tempG[k][0];
							opt_schedule[k][1]=tempG[k][1];
							opt_seq[k][0]=planned_phase[k][0];
							opt_seq[k][1]=planned_phase[k][1];
							for (int kkk=4*k;kkk<4+4*k;kkk++)
								temp_Q_perm[kkk]=Q_temp[i][kkk];
						}
					}
					//case 3: first phase is skipped, second phase is not skipped
					if(planned_phase[k][0]==0 && planned_phase[k][1]!=0)
					{
						tempG[k][0]=0;
						tempG[k][1]=effectiveG[k];
						if (tempG[k][1]<=MaxGreen[planned_phase[k][1]-1])
						{
							temp_v[k]=f1_r(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage,Num_veh,Num_Veh_Ratio);
							//save the best phase schedule
							opt_schedule[k][0]=tempG[k][0];
							opt_schedule[k][1]=tempG[k][1];
							opt_seq[k][0]=planned_phase[k][0];
							opt_seq[k][1]=planned_phase[k][1];
							for (int kkk=4*k;kkk<4+4*k;kkk++)
								temp_Q_perm[kkk]=Q_temp[i][kkk];
						}
					}
				}
				temp_total_v=temp_v[0]+temp_v[1]+v[stage-1][i-s];  //delay of this stage + delay at previous stage

				if (temp_total_v<v[stage][i])
				{
					v[stage][i]=temp_total_v;
					X[stage][i]=s;
					best_phase_schedule[stage][i][0][0]=opt_schedule[0][0];
				    best_phase_schedule[stage][i][0][1]=opt_schedule[0][1];
					best_phase_schedule[stage][i][1][0]=opt_schedule[1][0];
					best_phase_schedule[stage][i][1][1]=opt_schedule[1][1];
					
					best_phase_sequence[stage][i][0][0]=opt_seq[0][0];
				    best_phase_sequence[stage][i][0][1]=opt_seq[0][1];
					best_phase_sequence[stage][i][1][0]=opt_seq[1][0];
					best_phase_sequence[stage][i][1][1]=opt_seq[1][1];
					for (int kkk=0;kkk<P;kkk++)
						Q_perm[stage][i][kkk]=temp_Q_perm[kkk];

					//assign permenent queue
				}
			}
			//because of the min constraint, it is possible that planning the third stage result in worse consequence
			/*
			if(v[stage][i]>=v[stage-1][i])
			{
				v[stage][i]=v[stage-1][i];
				X[stage][i]=0;
				best_phase_schedule[stage][i][0][0]=0;
				best_phase_schedule[stage][i][0][1]=0;
				best_phase_schedule[stage][i][1][0]=0;
				best_phase_schedule[stage][i][1][1]=0;
				
				best_phase_sequence[stage][i][0][0]=0;
				best_phase_sequence[stage][i][0][1]=0;
				best_phase_sequence[stage][i][1][0]=0;
				best_phase_sequence[stage][i][1][1]=0;
				for (int kkk=0;kkk<P;kkk++)
					Q_perm[stage][i][kkk]=Q_perm[stage-1][i][kkk];
			}
			*/
		}
		else //if(i>=min_cons && i<=max_cons)
		{
 			X[stage][i]=0;
			v[stage][i]=v[stage-1][i];
			for (int kkk=0;kkk<P;kkk++)
				Q_perm[stage][i][kkk]=Q_perm[stage-1][i][kkk];
		}
		
		//deal with all four phases don't have requests!
		
	}  //for(i=1;i<T;i++)

free(perf_func_value_seq1);
free(perf_func_value_seq2);

}//end of planning stage 2 and 3



//retrive optimal solutions
		int opt_s[J];
		int Planed_barrier=0;
		int start_search_time;
		int end_search_time;

		int start_stage;   //with stage to start with to retrive the optimal solution
		//if second stage has no demand, then start retrive optimal solutions from first stage
		if(B_Min[J-1]==0 || B_Max[J-1]==0)
		{
			stage=J-2;
			start_stage=J-2;
		}
		else
		{
			stage=J-1;
			start_stage=J-1;
		}
	
		
		for(i=T-1;i>0;i--)
		{
			//if(v[stage][i]<99999)
			if(X[stage][i]!=0)
				break;
		}
		start_search_time=i;

		for(i=start_search_time;i>0;i--)
		{
			//if(v[stage][i]>=99999)
			if(X[stage][i]==0)
				break;
		}
		end_search_time=i+1;


		//find out hte place to trace back where have the changed residual queue and shortest time horizon
		int best_position=start_search_time;
		int residual_q[8];
		int flag=0;
		for(i=0;i<8;i++)
		{
			residual_q[i]=Q_perm[stage][start_search_time][i];
		}
		for(i=start_search_time;i>=end_search_time;i--)
		{
			for(j=0;j<8;j++)
			{
				if(Q_perm[stage][i][j]!=residual_q[j])
				{
					flag=1;
					break;
				}
			}
			if(flag==1)
				break;
		}
		best_position=i;
		
		

		//i already--
		//int start_time=min(T-1,i);
		//int time=min(T-1,i);
		
		int start_time=min(T-1,best_position);
		int time=min(T-1,best_position);
		
		
		//deal with the case only plan minimum length of the barrier
		if(X[stage][start_time]==0)
		{
			start_time++;
			time++;
		}
		
		
		
		while(stage>=0)
		{
			opt_s[Planed_barrier]=X[stage][time];
			//cout<<"stage "<<stage<<" time "<<time<<endl;
			//calculate the phase number in order to find out the Y+R time
			//cout<<"opt_s"<<opt_s[Planed_barrier]<<endl;
			if (opt_s[Planed_barrier]!=0)
			time=time-opt_s[Planed_barrier];
			Planed_barrier++;
			stage--;
		}


//	for(i=0;i<3;i++)
//	{	
//		for(j=0;j<121;j++)
//		{
//		sprintf(temp_log,"perminent queue at stage %d and time %d is %d %d %d %d %d %d %d %d\n",i,j,Q_perm[i][j][0],Q_perm[i][j][1],Q_perm[i][j][2],Q_perm[i][j][3],Q_perm[i][j][4],Q_perm[i][j][5],Q_perm[i][j][6],Q_perm[i][j][7]); 
//		outputlog(temp_log); cout<<temp_log;
//		}
//	}

	sprintf(temp_log,"perminent queue at start_time-1 is %d %d %d %d %d %d %d %d\n",Q_perm[J-1][start_time-1][0],Q_perm[J-1][start_time-1][1],Q_perm[J-1][start_time-1][2],Q_perm[J-1][start_time-1][3],Q_perm[J-1][start_time-1][4],Q_perm[J-1][start_time-1][5],Q_perm[J-1][start_time-1][6],Q_perm[J-1][start_time-1][7]); 
	outputlog(temp_log); cout<<temp_log;
	
sprintf(temp_log,"Start time is %d\n",start_time);   
outputlog(temp_log); cout<<temp_log;		
sprintf(temp_log,"Optimal Planning time is %d %d\n", opt_s[0],opt_s[1]);
outputlog(temp_log); cout<<temp_log;
		
//get the optimal signal plan
/////////////////////////////// for three stage cases!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*
//first barrier
for (i=0;i<2;i++)
	for (j=0;j<2;j++)
	{
	opt_sig_plan[i][j]=best_phase_schedule[0][start_time-opt_s[0]-opt_s[1]][i][j];
	opt_sig_seq[i][j]=best_phase_sequence[0][start_time-opt_s[0]-opt_s[1]][i][j];
	}
//second barrier
for (i=0;i<2;i++)
	for (j=0;j<2;j++)
	{
	opt_sig_plan[i][j+2]=best_phase_schedule[1][start_time-opt_s[0]][i][j];
	opt_sig_seq[i][j+2]=best_phase_sequence[1][start_time-opt_s[0]][i][j];
	}
//third barrier
for (i=0;i<2;i++)
	for (j=0;j<2;j++)
	{
	opt_sig_plan[i][j+4]=best_phase_schedule[2][start_time][i][j];
	opt_sig_seq[i][j+4]=best_phase_sequence[2][start_time][i][j];
	}
*/	
//////////////////////////for two stage cases!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

if (start_stage==J-1)
{
	for (i=0;i<2;i++)
		for (j=0;j<2;j++)
		{
		opt_sig_plan[i][j]=best_phase_schedule[0][start_time-opt_s[0]][i][j];
		opt_sig_seq[i][j]=best_phase_sequence[0][start_time-opt_s[0]][i][j];
		}
	//second barrier
	for (i=0;i<2;i++)
		for (j=0;j<2;j++)
		{
		opt_sig_plan[i][j+2]=best_phase_schedule[1][start_time][i][j];
		opt_sig_seq[i][j+2]=best_phase_sequence[1][start_time][i][j];
		}
}

if (start_stage==J-2)
{
	for (i=0;i<2;i++)
		for (j=0;j<2;j++)
		{
		opt_sig_plan[i][j]=best_phase_schedule[0][start_time][i][j];
		opt_sig_seq[i][j]=best_phase_sequence[0][start_time][i][j];
		}
	//second barrier
	for (i=0;i<2;i++)
		for (j=0;j<2;j++)
		{
		opt_sig_plan[i][j+2]=0;
		opt_sig_seq[i][j+2]=0;
		}
}
	
return 1;
}


float f_r(int ring, int phase1, int phase2, int g1, int g2, int total_length,int Num_Veh[8],float Num_Ratio[8])
{
	int i;
	int TotalDelay=0;
	//assign initial queue length
	//Q_temp[0][phase1-1]=ArrivalTable[0][phase1-1];
	//Q_temp[0][phase2-1]=ArrivalTable[0][phase2-1];

	float total_pre_delay[8]; //previous delay caused by queuing vehicle before planning

	float T_Delay_Phase[8]={0};    //Total delay of each phase in the ring
	float Ave_Delay_Phase[8]={0};  //Average Delay of each phase int the ring
	float Total_Ave_Delay=0;      //Total Average Delay of all non-planning phases
	
	for(i=4*ring;i<4+4*ring;i++)
	{
		Q_temp[0][i]=ArrivalTable[0][i];
		total_pre_delay[i]=Q_temp[0][i]*red_elapse_time[i]/2;
	}

	if(phase1==0)
	{
		for(i=1;i<=total_length;i++)
		{
			if(i<=g2) //phase 2 is green
			{
				//Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
				//deal with the green phase
				if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[i][phase2-1]-=LaneNo[phase2-1];  //discharging rate is based on the number of the lane, assuming queue at each lane are equal
					
					//if (phase2==2 || phase2==6)
					//Q_temp[i][phase2-1]-=3;  //queue discharge rate is 3 veh/2second for through phase 2 and 6, 2 veh/2second for phase 4 and 8, lanes and 1 veh/2 second for left turn phases
					//else if (phase2==4 || phase2==8)
					//Q_temp[i][phase2-1]-=2; 
					//else
					//Q_temp[i][phase2-1]--;
				}
				if (Q_temp[i][phase2-1]<=0)
					Q_temp[i][phase2-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1]; //Add the arrivel flow
				//deal with the red phase
				//Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1];
				TotalDelay+=Q_temp[i][phase2-1];
				T_Delay_Phase[phase2-1]+=Q_temp[i][phase2-1];
			}
			else   //all red
			{
				//Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase2-1];
				T_Delay_Phase[phase2-1]+=Q_temp[i][phase2-1];
			}

		}
		//Ave_Delay_Phase[phase2-1]=(total_pre_delay[phase2-1]+T_Delay_Phase[phase2-1])/Num_Veh[phase2-1];
		//if (Q_temp[g2][phase2-1]!=0)   //add residual queue penelty
			Ave_Delay_Phase[phase2-1]+=Q_temp[total_length][phase2-1];
	}
	if(phase2==0)
	{
		for(i=1;i<=total_length;i++)
		{
			if(i<=g1) //phase 2 is green
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
				//Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
				//deal with the green phase
				if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[i][phase1-1]-=LaneNo[phase1-1];  //discharging rate is based on the number of the lane, assuming queue at each lane are equal
					//if (phase1==2 || phase1==6)
					//Q_temp[i][phase1-1]-=3;  //queue discharge rate is 3 veh/2second for through phase 2 and 6, 2 veh/2second for phase 4 and 8, lanes and 1 veh/2 second for left turn phases
					//else if (phase1==4 || phase1==8)
					//Q_temp[i][phase1-1]-=2; 
					//else
					//Q_temp[i][phase1-1]--;
				}
				if (Q_temp[i][phase1-1]<=0)
					Q_temp[i][phase1-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1]; //Add the arrivel flow
				//deal with the red phase
				//Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1];
				T_Delay_Phase[phase1-1]+=Q_temp[i][phase1-1];
			}
			else   //all red
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
				//Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1];
				T_Delay_Phase[phase1-1]+=Q_temp[i][phase1-1];
			}
		}
		//Ave_Delay_Phase[phase1-1]=(total_pre_delay[phase1-1]+T_Delay_Phase[phase1-1])/Num_Veh[phase1-1];
		//if (Q_temp[g1][phase1-1]!=0)   //add residual queue penelty
			Ave_Delay_Phase[phase1-1]+=Q_temp[total_length][phase1-1];
	}

	//go through other 2 phases in the same ring: these two phases should be red
	for(i=4*ring;i<4+4*ring;i++)
	{
		if ((i+1)!=phase1 && (i+1)!=phase2)
		{
			//Q_temp[0][i]=ArrivalTable[0][i];
			for(int j=1;j<=total_length;j++)
			{
				Q_temp[j][i]=Q_temp[j-1][i]+ArrivalTable[j][i];
				TotalDelay+=Q_temp[j][i];
				T_Delay_Phase[i]+=Q_temp[j][i];
			}
			if(Num_Veh[i]!=0)
				Ave_Delay_Phase[i]=(total_pre_delay[i]+T_Delay_Phase[i])/Num_Veh[i];
			else
				Ave_Delay_Phase[i]=0;
		}
	}

	for (i=0;i<4;i++)
	{
		Total_Ave_Delay+=Ave_Delay_Phase[i+ring*4];
		//TotalDelay+=total_pre_delay[i+ring*4];
	}

	if(delay_type==1)
		return Total_Ave_Delay;
	if(delay_type==0)
		return TotalDelay;
}

float f1_r(int ring, int phase1, int phase2, int g1, int g2, int total_length, int B_L, int stage, int Num_Veh[8],float Num_Ratio[8])
{
	int i;
	int TotalDelay=0;
	//assign initial queue length
	//Q_temp[total_length-B_L][phase1-1]=Q_perm[stage-1][total_length-B_L][phase1-1];
	//Q_temp[total_length-B_L][phase2-1]=Q_perm[stage-1][total_length-B_L][phase2-1];

	float total_pre_delay[8]; //previous delay caused by queuing vehicle before planning

	float T_Delay_Phase[8]={0};    //Total delay of each phase in the ring
	float Ave_Delay_Phase[8]={0};  //Average Delay of each phase int the ring
	float Total_Ave_Delay=0;      //Total Average Delay of all non-planning phases
	
	for(i=4*ring;i<4+4*ring;i++)
	{
		Q_temp[total_length-B_L][i]=Q_perm[stage-1][total_length-B_L][i];
		total_pre_delay[i]=Q_temp[total_length-B_L][i]*(red_elapse_time[i]+total_length-B_L)/2;   //red_elapse_time+the first stage time
	}

	if(phase1==0)
	{
		for(i=total_length-B_L+1;i<=total_length;i++)
		{
			if(i<=g2+total_length-B_L) //phase 2 is green
			{
				//Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
				//deal with the green phase
				if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[i][phase2-1]-=LaneNo[phase2-1];  //discharging rate is based on the number of the lane, assuming queue at each lane are equal
					//if (phase2==2 || phase2==6)
					//Q_temp[i][phase2-1]-=3;  //queue discharge rate is 3 veh/2second for through phase 2 and 6, 2 veh/2second for phase 4 and 8, lanes and 1 veh/2 second for left turn phases
					//else if (phase2==4 || phase2==8)
					//Q_temp[i][phase2-1]-=2; 
					//else
					//Q_temp[i][phase2-1]--;
				}
				if (Q_temp[i][phase2-1]<=0)
					Q_temp[i][phase2-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1]; //Add the arrivel flow
				//deal with the red phase
				//Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1];
				TotalDelay+=Q_temp[i][phase2-1];
				T_Delay_Phase[phase2-1]+=Q_temp[i][phase2-1];
			}
			else   //all red
			{
				//Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase2-1];
				T_Delay_Phase[phase2-1]+=Q_temp[i][phase2-1];
			}
		}



		if (Num_Veh[phase2-1]!=0)
			//Ave_Delay_Phase[phase2-1]=(total_pre_delay[phase2-1]+T_Delay_Phase[phase2-1])/Num_Veh[phase2-1];
	//	if (Q_temp[g2+total_length-B_L][phase2-1]!=0)   //add residual queue penelty
			Ave_Delay_Phase[phase2-1]+=Q_temp[total_length][phase2-1];

	}
	if(phase2==0)
	{
		for(i=total_length-B_L+1;i<=total_length;i++)
		{
			if(i<=g1+total_length-B_L) //phase 2 is green
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
				//Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
				//deal with the green phase
				if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[i][phase1-1]-=LaneNo[phase1-1];  //discharging rate is based on the number of the lane, assuming queue at each lane are equal
					//if (phase1==2 || phase1==6)
					//Q_temp[i][phase1-1]-=3;  //queue discharge rate is 3 veh/2second for through phase 2 and 6, 2 veh/2second for phase 4 and 8, lanes and 1 veh/2 second for left turn phases
					//else if (phase1==4 || phase1==8)
					//Q_temp[i][phase1-1]-=2; 
					//else
					//Q_temp[i][phase1-1]--;
				}
				if (Q_temp[i][phase1-1]<=0)
					Q_temp[i][phase1-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1]; //Add the arrivel flow
				//deal with the red phase
				//Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1];
				T_Delay_Phase[phase1-1]+=Q_temp[i][phase1-1];
			}
			else   //all red
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
				//Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1];
				T_Delay_Phase[phase1-1]+=Q_temp[i][phase1-1];
			}
		}
		if (Num_Veh[phase1-1]!=0)
			//Ave_Delay_Phase[phase1-1]=(total_pre_delay[phase1-1]+T_Delay_Phase[phase1-1])/Num_Veh[phase1-1];
		//if (Q_temp[g1+total_length-B_L][phase1-1]!=0)   //add residual queue penelty
			Ave_Delay_Phase[phase1-1]+=Q_temp[total_length][phase1-1];
	}

	//go through other 2 phases in the same ring: these two phases should be red
	for(i=4*ring;i<4+4*ring;i++)
	{
		if ((i+1)!=phase1 && (i+1)!=phase2)
		{

			//Q_temp[total_length-B_L][i]=Q_perm[stage-1][total_length-B_L][i];
			for(int j=total_length-B_L+1;j<=total_length;j++)
			{
				Q_temp[j][i]=Q_temp[j-1][i]+ArrivalTable[j][i];
				TotalDelay+=Q_temp[j][i];
				T_Delay_Phase[i]+=Q_temp[j][i];
			}
			if(Num_Veh[i]!=0)
				Ave_Delay_Phase[i]=(total_pre_delay[i]+T_Delay_Phase[i])/Num_Veh[i];
			else
				Ave_Delay_Phase[i]=0;
		}
	}

	for (i=0;i<4;i++)
	{
		Total_Ave_Delay+=Ave_Delay_Phase[i+ring*4];
		//TotalDelay+=total_pre_delay[i+ring*4];
	}

	if(delay_type==1)
		return Total_Ave_Delay;
	if(delay_type==0)
		return TotalDelay;
}


float f(int ring, int phase1, int phase2, int g1, int g2, int total_length, int Num_Veh[8],float Num_Ratio[8])
{
	int i;
	int TotalDelay=0;
	float TotalDelay_P1=0;
	float TotalDelay_P2=0;
	float Ave_Delay=0;

	//assign initial queue length
	Q_temp[0][phase1-1]=ArrivalTable[0][phase1-1];
	Q_temp[0][phase2-1]=ArrivalTable[0][phase2-1];

	float total_pre_delay_P1=Q_temp[0][phase1-1]*red_elapse_time[phase1-1]/2; //previous delay caused by queuing vehicle before planning

	float total_pre_delay_P2=Q_temp[0][phase2-1]*red_elapse_time[phase2-1]/2; //previous delay caused by queuing vehicle before planning

	for(i=1;i<=total_length;i++)
	{
		if(i<=g1)  //phase 1 is green
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
			//deal with the green phase
			if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
			{
				Q_temp[i][phase1-1]-=LaneNo[phase1-1];  //discharging rate is based on the number of the lane, assuming queue at each lane are equal
				//if (phase1==2 || phase1==6)
				//Q_temp[i][phase1-1]-=3;  //queue discharge rate is 3 veh/2second for through phase 2 and 6, 2 veh/2second for phase 4 and 8, lanes and 1 veh/2 second for left turn phases
				//else if (phase1==4 || phase1==8)
				//Q_temp[i][phase1-1]-=2; 
				//else
				//Q_temp[i][phase1-1]--;
			}
			if (Q_temp[i][phase1-1]<=0)
				Q_temp[i][phase1-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
			else
				Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1]; //Add the arrivel flow
			//deal with the red phase
			Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1];
			TotalDelay_P1+=Q_temp[i][phase1-1];
			TotalDelay_P2+=Q_temp[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>g1 && i<=g1+Yellow[phase1-1]+Red[phase1-1])  //transition time in phase 1
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
			TotalDelay_P1+=Q_temp[i][phase1-1];
			TotalDelay_P2+=Q_temp[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>g1+Yellow[phase1-1]+Red[phase1-1] && i<=g1+Yellow[phase1-1]+Red[phase1-1]+g2) //phase 2 is green
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
			//deal with the green phase
			if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
			{
				Q_temp[i][phase2-1]-=LaneNo[phase2-1];  //discharging rate is based on the number of the lane, assuming queue at each lane are equal
				//if (phase2==2 || phase2==6)
				//Q_temp[i][phase2-1]-=3;  //queue discharge rate is 3 veh/2second for through phase 2 and 6, 2 veh/2second for phase 4 and 8, lanes and 1 veh/2 second for left turn phases
				//else if (phase2==4 || phase2==8)
				//Q_temp[i][phase2-1]-=2; 
				//else
				//Q_temp[i][phase2-1]--;
			}
			if (Q_temp[i][phase2-1]<=0)
				Q_temp[i][phase2-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
			else
				Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1]; //Add the arrivel flow
			//deal with the red phase
			Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1];
			TotalDelay_P1+=Q_temp[i][phase1-1];
			TotalDelay_P2+=Q_temp[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>g1+Yellow[phase1-1]+Red[phase1-1]+g2 && i<=total_length)  //all red
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
			TotalDelay_P1+=Q_temp[i][phase1-1];
			TotalDelay_P2+=Q_temp[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
	}


	float Ave_Delay_P1=0;
	float Ave_Delay_P2=0;
	if (Num_Veh[phase1-1]!=0)
		//Ave_Delay_P1=(TotalDelay_P1+total_pre_delay_P1)/Num_Veh[phase1-1];

	//if (Q_temp[g1][phase1-1]!=0)   //add residual queue penelty
		Ave_Delay_P1+=Q_temp[total_length][phase1-1];

	if (Num_Veh[phase2-1]!=0)
		//Ave_Delay_P2=(TotalDelay_P2+total_pre_delay_P1)/Num_Veh[phase2-1];

	//if (Q_temp[g1+Yellow[phase1-1]+Red[phase1-1]+g2][phase2-1]!=0)   //add residual queue penelty
		Ave_Delay_P2+=Q_temp[total_length][phase2-1];


	//TotalDelay=TotalDelay+total_pre_delay_P1+total_pre_delay_P2;

	Ave_Delay= Ave_Delay_P1+Ave_Delay_P2;

	if(delay_type==1)
		return Ave_Delay;
	if(delay_type==0)
		return TotalDelay;
}

float f_other(int ring, int phase1, int phase2, int total_length,int Num_Veh[8],float Num_Ratio[8])
{
int i;
int TotalDelay=0;
float T_Delay_Phase[8]={0};    //Total delay of each phase in the ring
float Ave_Delay_Phase[8]={0};  //Average Delay of each phase int the ring
float Total_Ave_Delay=0;      //Total Average Delay of all non-planning phases

float total_pre_delay[8];  //previous delay caused by queuing vehicle before planning

	for(i=4*ring;i<4+4*ring;i++)
	{
		if ((i+1)!=phase1 && (i+1)!=phase2)
		{
			Q_temp[0][i]=ArrivalTable[0][i];
			total_pre_delay[i]=Q_temp[0][i]*red_elapse_time[i]/2;
			for(int j=1;j<=total_length;j++)
			{
				Q_temp[j][i]=Q_temp[j-1][i]+ArrivalTable[j][i];
				TotalDelay+=Q_temp[j][i];
				T_Delay_Phase[i]+=Q_temp[j][i];
			}
			if (Num_Veh[i]!=0)
			{
				//Ave_Delay_Phase[i]=(T_Delay_Phase[i]+total_pre_delay[i])/Num_Veh[i];
				//cout << Ave_Delay_Phase[i] << " = " << T_Delay_Phase[i] << " / " << Num_Veh[i] << endl;
				Ave_Delay_Phase[i]=Q_temp[total_length][i];
			}
			else
				Ave_Delay_Phase[i]=0;
			Total_Ave_Delay+=Ave_Delay_Phase[i];
			//TotalDelay+=total_pre_delay[i];
		}
	}

//char temp_log[512];
//sprintf(temp_log,"Total_Delay %d Total_Ave_Delay is: %f, ring %d phase1 %d phase2 %d total_length %d Ave_Delay_Phase %f %f %f %f %f %f %f %f\n",TotalDelay,Total_Ave_Delay,ring,phase1,phase2,total_length,Ave_Delay_Phase[0],Ave_Delay_Phase[1],Ave_Delay_Phase[2],Ave_Delay_Phase[3],Ave_Delay_Phase[4],Ave_Delay_Phase[5],Ave_Delay_Phase[6],Ave_Delay_Phase[7]);
//	cout<<temp_log;
//sprintf(temp_log,"Num Vehicles %d %d %d %d %d %d %d %d\n",Num_Veh[0],Num_Veh[1],Num_Veh[2],Num_Veh[3],Num_Veh[4],Num_Veh[5],Num_Veh[6],Num_Veh[7]);
//cout<<temp_log;


if(delay_type==1)
	return Total_Ave_Delay;
if(delay_type==0)
	return TotalDelay;
}

float f1_other(int ring, int phase1, int phase2, int total_length, int B_L, int stage, int Num_Veh[8],float Num_Ratio[8])
{
int i;
int TotalDelay=0;
float T_Delay_Phase[8]={0};    //Total delay of each phase in the ring
float Ave_Delay_Phase[8]={0};  //Average Delay of each phase int the ring
float Total_Ave_Delay=0;      //Total Average Delay of all non-planning phases

float total_pre_delay[8];  //previous delay caused by queuing vehicle before planning

	for(i=4*ring;i<4+4*ring;i++)
	{
		if ((i+1)!=phase1 && (i+1)!=phase2)
		{
			Q_temp[total_length-B_L][i]=Q_perm[stage-1][total_length-B_L][i];
			total_pre_delay[i]=Q_temp[total_length-B_L][i]*(red_elapse_time[i]+total_length-B_L)/2;
			for(int j=total_length-B_L+1;j<=total_length;j++)
			{
				Q_temp[j][i]=Q_temp[j-1][i]+ArrivalTable[j][i];
				TotalDelay+=Q_temp[j][i];
				T_Delay_Phase[i]+=Q_temp[j][i];
			}
			if (Num_Veh[i]!=0)
				//Ave_Delay_Phase[i]=(T_Delay_Phase[i]+total_pre_delay[i])/Num_Veh[i];
				Ave_Delay_Phase[i]=Q_temp[total_length][i];
			else
				Ave_Delay_Phase[i]=0;
			Total_Ave_Delay+=Ave_Delay_Phase[i];
			//TotalDelay+=total_pre_delay[i];
		}
	}

if(delay_type==1)
	return Total_Ave_Delay;
if(delay_type==0)
	return TotalDelay;
}


float f1(int ring, int phase1, int phase2, int g1, int g2, int total_length, int B_L, int stage, int Num_Veh[8],float Num_Ratio[8])
{
	int i;
	int TotalDelay=0;

	float TotalDelay_P1=0;
	float TotalDelay_P2=0;
	float Ave_Delay=0;


	//assign initial queue length
	Q_temp[total_length-B_L][phase1-1]=Q_perm[stage-1][total_length-B_L][phase1-1];
	Q_temp[total_length-B_L][phase2-1]=Q_perm[stage-1][total_length-B_L][phase2-1];

	float total_pre_delay_P1=Q_temp[total_length-B_L][phase1-1]*(red_elapse_time[phase1-1]+total_length-B_L)/2; //previous delay caused by queuing vehicle before planning

	float total_pre_delay_P2=Q_temp[total_length-B_L][phase2-1]*(red_elapse_time[phase2-1]+total_length-B_L)/2; //previous delay caused by queuing vehicle before planning


	for(i=total_length-B_L+1;i<=total_length;i++)
	{
		if(i<=total_length-B_L+g1)  //phase 1 is green
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
			//deal with the green phase
			if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
			{
				Q_temp[i][phase1-1]-=LaneNo[phase1-1];  //discharging rate is based on the number of the lane, assuming queue at each lane are equal
				//if (phase1==2 || phase1==6)
				//Q_temp[i][phase1-1]-=3;  //queue discharge rate is 3 veh/2second for through phase 2 and 6, 2 veh/2second for phase 4 and 8, lanes and 1 veh/2 second for left turn phases
				//else if (phase1==4 || phase1==8)
				//Q_temp[i][phase1-1]-=2; 
				//else
				//Q_temp[i][phase1-1]--;
			}
			if (Q_temp[i][phase1-1]<=0)
				Q_temp[i][phase1-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
			else
				Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1]; //Add the arrivel flow
			//deal with the red phase
			Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			TotalDelay_P1+=Q_temp[i][phase1-1];
			TotalDelay_P2+=Q_temp[i][phase2-1];
		}
		if(i>total_length-B_L+g1 && i<=g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1])  //transition time in phase 1
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			TotalDelay_P1+=Q_temp[i][phase1-1];
			TotalDelay_P2+=Q_temp[i][phase2-1];
		}
		if(i>g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1] && i<=g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1]+g2) //phase 2 is green
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
			//deal with the green phase
			if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
			{
				Q_temp[i][phase2-1]-=LaneNo[phase2-1];  //discharging rate is based on the number of the lane, assuming queue at each lane are equal
				//if (phase2==2 || 2==6)
				//Q_temp[i][phase2-1]-=3;  //queue discharge rate is 3 veh/2second for through phase 2 and 6, 2 veh/2second for phase 4 and 8, lanes and 1 veh/2 second for left turn phases
				//else if (phase2==4 || phase2==8)
				//Q_temp[i][phase2-1]-=2; 
				//else
				//Q_temp[i][phase2-1]--;
			}
			if (Q_temp[i][phase2-1]<=0)
				Q_temp[i][phase2-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
			else
				Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1]; //Add the arrivel flow
			//deal with the red phase
			Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			TotalDelay_P1+=Q_temp[i][phase1-1];
			TotalDelay_P2+=Q_temp[i][phase2-1];
		}
		if(i>g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1]+g2 && i<=total_length)  //all red
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			TotalDelay_P1+=Q_temp[i][phase1-1];
			TotalDelay_P2+=Q_temp[i][phase2-1];
		}
	}


	float Ave_Delay_P1=0;
	float Ave_Delay_P2=0;
	if (Num_Veh[phase1-1]!=0)
		//Ave_Delay_P1=(TotalDelay_P1+total_pre_delay_P1)/Num_Veh[phase1-1];
	
	//if (Q_temp[total_length-B_L+g1][phase1-1]!=0)   //add residual queue penelty
		Ave_Delay_P1+=Q_temp[total_length][phase1-1];

	if (Num_Veh[phase2-1]!=0)
		//Ave_Delay_P2=(TotalDelay_P2+total_pre_delay_P1)/Num_Veh[phase2-1];

	//if (Q_temp[g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1]+g2][phase2-1]!=0)   //add residual queue penelty
		Ave_Delay_P2+=Q_temp[total_length][phase2-1];


	//TotalDelay=TotalDelay+total_pre_delay_P1+total_pre_delay_P2;


	Ave_Delay= Ave_Delay_P1+Ave_Delay_P2;

	if(delay_type==1)
		return Ave_Delay;
	if(delay_type==0)
		return TotalDelay;
}




int add_missing_phase(int barrier)
{
	if (planned_phase[0][0]==0 && planned_phase[0][1]==0)  //no phase planned in ring 1
	{
		if(barrier==0)
			planned_phase[0][1]=2;
		else
			planned_phase[0][1]=4;
	}
	if (planned_phase[1][0]==0 && planned_phase[1][1]==0)  //no phase planned in ring 1
	{
		if(barrier==0)
			planned_phase[1][1]=6;
		else
			planned_phase[1][1]=8;
	}
	return 0;
}
