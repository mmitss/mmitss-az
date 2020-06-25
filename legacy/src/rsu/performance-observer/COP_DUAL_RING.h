#include <stdio.h>
#include <iostream>
#include <math.h>


using namespace std;

#define		J       3    /*  number of barriers/stages planned  */
#define		P	8    /*  number of phases  */
#define		T       121   /*  total planned time +1  */

int MinGreen[8]={7,10,7,10,7,10,7,10};
int MaxGreen[8]={20,35,20,35,20,35,20,35};
int Yellow[8]={3,3,3,3,3,3,3,3};
int Red[8]={2,2,2,2,2,2,2,2};


extern char temp_log[512];
extern int ArrivalTable[121][8];
extern int opt_s[12];
extern int opt_sig_plan[2][3*2];


int	v[J][T];	/*table of values for barrier level*/
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


int Q_temp[T][P];  //temporary standing queue at each stage
int Q_perm[J][T][P];  //permanent standing queue at each stage

int COP_DUAL_RING(int InitPhase[2], int GrnElapse[2], int PassedPhase[2], int Skip_phase[8]);
int f(int ring, int phase1, int phase2, int g1, int g2, int total_length);  //calculate the delay when planning two phases at the first stage
int f1(int ring, int phase1, int phase2, int g1, int g2, int total_length, int B_L, int stage); //calculate the delay when planning two phases at the following stages the last parameter is the length of this planning barrier
int f_r(int ring, int phase1, int phase2, int g1, int g2, int total_length);  //calculate the delay when planning one phase at the first stage
int f1_r(int ring, int phase1, int phase2, int g1, int g2, int total_length, int B_L, int stage);  //calculate the delay when planning one phase at the following stages
//int delay(int phase1, int phase2, int g1, int g2, int total_length); //calculate the delay, the order of the phases are pre-ordered

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

stage=0;
//1. Decide which barrier we start from
start_barrier=(InitPhase[0]<3)?0:1; 

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

cout<<"The planned phases are:"<<planned_phase[0][0]<<" "<<planned_phase[0][1]<<" "<<planned_phase[1][0]<<" "<<planned_phase[1][1]<<endl;

//2.2 Based on the planned pahse, calculate min and max for the barrier
int ring1_min, ring2_min, ring1_max, ring2_max;
ring1_min=0; ring2_min=0; ring1_max=0; ring2_max=0;

for(i=0;i<2;i++)  //ring 1
{
	if(planned_phase[0][i]!=0 && planned_phase[0][i]!=InitPhase[0])  //not the current phase, but in plan
	{
		ring1_min+=MinGreen[planned_phase[0][i]-1]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
		ring1_max+=MaxGreen[planned_phase[0][i]-1]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
	}
	if (planned_phase[0][i]!=0 && planned_phase[0][i]==InitPhase[0])  //the current phase in plan
	{
		ring1_min+=Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
		ring1_max+=MaxGreen[planned_phase[0][i]-1]-GrnElapse[0]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
	}
}
for(i=0;i<2;i++)  //ring 2
{
	if(planned_phase[1][i]!=0 && planned_phase[1][i]!=InitPhase[1])
	{
		ring2_min+=MinGreen[planned_phase[1][i]-1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
		ring2_max+=MaxGreen[planned_phase[1][i]-1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
	}
	if(planned_phase[1][i]!=0 && planned_phase[1][i]==InitPhase[1])  //the current phase in plan
	{
		ring2_min+=Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
		ring2_max+=MaxGreen[planned_phase[1][i]-1]-GrnElapse[1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
	}
}
B_Min[stage]=max(ring1_min,ring2_min);
B_Max[stage]=max(ring1_max,ring2_max);  //if one ring exceeds the maximum green time, just put the last phase to green rest!!!!!

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
		//for both of two rings there are three cases
		int temp_v[2];
		temp_v[0]=99999; temp_v[1]=99999;
		for (k=0;k<2;k++)  //two rings
		{
			//cout<<"ring "<<k+1<<endl;
			//case 1: both phases are not skipped
			if(planned_phase[k][0]!=0 && planned_phase[k][1]!=0)
			{
				//the initial phase can ternimate at any time, so start from 0 to Gmax-GrnElapse
				if(planned_phase[k][0]==InitPhase[k])
				{
					for(j=0;j<=MaxGreen[planned_phase[k][0]-1]-GrnElapse[k];j++)
					{
						tempG[k][0]=j;
						tempG[k][1]=effectiveG[k]-tempG[k][0];
						if (tempG[k][1]>=MinGreen[planned_phase[k][1]-1] && tempG[k][1]<=MaxGreen[planned_phase[k][1]-1])  //feasible solution
						{
							//calculate delay function produced by this combination!!!!!!!!!!!!!!!!!!!!
							int temp_vv=f(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i);
							if (temp_vv<temp_v[k]) //calculate the delay
							{
								temp_v[k]=temp_vv; //save the best delay
								//save the best phase schedule
								best_phase_schedule[stage][i][k][0]=tempG[k][0];
								best_phase_schedule[stage][i][k][1]=tempG[k][1];
								for (int kkk=0;kkk<P;kkk++)
										Q_perm[stage][i][kkk]=Q_temp[i][kkk];
							}
							//cout<<"tempG[0][0] is "<<tempG[k][0]<<endl;
							//cout<<"tempG[0][1] is "<<tempG[k][1]<<endl;
						}
					}
				}
				if(planned_phase[k][1]==InitPhase[k])
				{
					for(j=0;j<=MaxGreen[planned_phase[k][1]-1]-GrnElapse[k];j++)
					{
						tempG[k][1]=j;
						tempG[k][0]=effectiveG[k]-tempG[k][1];
						if (tempG[k][0]>=MinGreen[planned_phase[k][0]-1] && tempG[k][0]<=MaxGreen[planned_phase[k][0]-1])  //feasible solution
						{
							//calculate delay function produced by this combination!!!!!!!!!!!!!!!!!!!!
							int temp_vv=f(k,planned_phase[k][1],planned_phase[k][0],tempG[k][0],tempG[k][1],i);
							if (temp_vv<temp_v[k]) //calculate the delay
							{
								temp_v[k]=temp_vv;  //save the best delay
								//save the best phase schedule
								best_phase_schedule[stage][i][k][0]=tempG[k][0];
								best_phase_schedule[stage][i][k][1]=tempG[k][1];
									for (int kkk=0;kkk<P;kkk++)
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
				temp_v[k]=f_r(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i);
				//save the best phase schedule
				best_phase_schedule[stage][i][k][0]=tempG[k][0];
				best_phase_schedule[stage][i][k][1]=tempG[k][1];
				for (int kkk=0;kkk<P;kkk++)
					Q_perm[stage][i][kkk]=Q_temp[i][kkk];
				//calculate delay!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
			//case 3: first phase is skipped, second phase is not skipped
			if(planned_phase[k][0]==0 && planned_phase[k][1]!=0)
			{
				tempG[k][0]=0;
				tempG[k][1]=effectiveG[k];
				//cout<<"tempG[0][0] is "<<tempG[k][0]<<endl;
				//cout<<"tempG[0][1] is "<<tempG[k][1]<<endl;
				temp_v[k]=f_r(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i);
				//save the best phase schedule
				best_phase_schedule[stage][i][k][0]=tempG[k][0];
				best_phase_schedule[stage][i][k][1]=tempG[k][1];
				for (int kkk=0;kkk<P;kkk++)
					Q_perm[stage][i][kkk]=Q_temp[i][kkk];
				//calculate delay!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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

for(stage=1;stage<3;stage++)
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
	cout<<"The planned phases are:"<<planned_phase[0][0]<<" "<<planned_phase[0][1]<<" "<<planned_phase[1][0]<<" "<<planned_phase[1][1]<<endl;


	ring1_min=0; ring2_min=0; ring1_max=0; ring2_max=0;

	for(i=0;i<2;i++)  //ring 1
	{
		if(planned_phase[0][i]!=0)
		{
			ring1_min+=MinGreen[planned_phase[0][i]-1]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
			ring1_max+=MaxGreen[planned_phase[0][i]-1]+Yellow[planned_phase[0][i]-1]+Red[planned_phase[0][i]-1];
		}
	}
	for(i=0;i<2;i++)  //ring 2
	{
		if(planned_phase[1][i]!=0)
		{
			ring2_min+=MinGreen[planned_phase[1][i]-1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
			ring2_max+=MaxGreen[planned_phase[1][i]-1]+Yellow[planned_phase[1][i]-1]+Red[planned_phase[1][i]-1];
		}
	}
	B_Min[stage]=max(ring1_min,ring2_min);
	B_Max[stage]=max(ring1_max,ring2_max);  //if one ring exceeds the maximum green time, just put the last phase to green rest!!!!!

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
	//2.3 for each fixed Barrier time, manipulate the green time between the two phases
	for(i=1;i<T;i++)
	{
		if(i>=min_cons+B_Min[stage] && i<=max_cons)
		{
			v[stage][i]=99999;
			int temp_total_v;
			int opt_seq[2][2];
			int temp_Q_perm[8];
			for(s=max(B_Min[stage],i-B_Max[stage-1]);s<=min(i-min_cons,B_Max[stage]);s++)
			{
				int temp_total_v;
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
				int temp_v[2];
				temp_v[0]=99999; temp_v[1]=99999;
				for (k=0;k<2;k++)  //two rings
				{
					
					//case 1: both phases are not skipped
					if(planned_phase[k][0]!=0 && planned_phase[k][1]!=0)
					{
						for(j=MinGreen[planned_phase[k][0]-1];j<=MaxGreen[planned_phase[k][0]-1];j++)
						{
							tempG[k][0]=j;
							tempG[k][1]=effectiveG[k]-tempG[k][0];
							if (tempG[k][1]>=MinGreen[planned_phase[k][1]-1] && tempG[k][1]<=MaxGreen[planned_phase[k][1]-1])  //feasible solution
							{
								int temp_vv=f1(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage);
								if (temp_vv<temp_v[k]) //calculate the delay
								{
									temp_v[k]=temp_vv; //save the best delay
									//save the best phase schedule
									opt_seq[k][0]=tempG[k][0];
									opt_seq[k][1]=tempG[k][1];

									for (int kkk=0;kkk<P;kkk++)
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
						temp_v[k]=f1_r(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage);
						//save the best phase schedule
						opt_seq[k][0]=tempG[k][0];
						opt_seq[k][1]=tempG[k][1];
						for (int kkk=0;kkk<P;kkk++)
							temp_Q_perm[kkk]=Q_temp[i][kkk];
						//calculate delay!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					}
					//case 3: first phase is skipped, second phase is not skipped
					if(planned_phase[k][0]==0 && planned_phase[k][1]!=0)
					{
						tempG[k][0]=0;
						tempG[k][1]=effectiveG[k];
						temp_v[k]=f1_r(k,planned_phase[k][0],planned_phase[k][1],tempG[k][0],tempG[k][1],i,s,stage);
						//save the best phase schedule
						opt_seq[k][0]=tempG[k][0];
						opt_seq[k][1]=tempG[k][1];
						for (int kkk=0;kkk<P;kkk++)
							temp_Q_perm[kkk]=Q_temp[i][kkk];
						//calculate delay!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					}
				}
				temp_total_v=temp_v[0]+temp_v[1]+v[stage-1][i-s];  //delay of this stage + delay at previous stage

				if (temp_total_v<v[stage][i])
				{
					v[stage][i]=temp_total_v;
					X[stage][i]=s;
					best_phase_schedule[stage][i][0][0]=opt_seq[0][0];
				    best_phase_schedule[stage][i][0][1]=opt_seq[0][1];
					best_phase_schedule[stage][i][1][0]=opt_seq[1][0];
					best_phase_schedule[stage][i][1][1]=opt_seq[1][1];
					for (int kkk=0;kkk<P;kkk++)
						Q_perm[stage][i][kkk]=temp_Q_perm[kkk];

					//assign permenent queue
				}
			}
			//because of the min constraint, it is possible that planning the third stage result in worse consequence
			if(v[stage][i]>=v[stage-1][i])
			{
				v[stage][i]=v[stage-1][i];
				X[stage][i]=0;
				for (int kkk=0;kkk<P;kkk++)
					Q_perm[stage][i][kkk]=Q_perm[stage-1][i][kkk];
			}

		}
		else //if(i>=min_cons && i<=max_cons)
		{
			X[stage][i]=0;
			v[stage][i]=v[stage-1][i];
			for (int kkk=0;kkk<P;kkk++)
				Q_perm[stage][i][kkk]=Q_perm[stage-1][i][kkk];
		}
	}

}//end of planning stage 2 and 3


//retrive optimal solutions
		int Planed_barrier=0;
            stage=J-1;
		int time=T-1;
		while(stage>=0)
		{
			opt_s[Planed_barrier]=X[stage][time];
			//calculate the phase number in order to find out the Y+R time

			if (opt_s[Planed_barrier]!=0)
			time=time-opt_s[Planed_barrier];
			Planed_barrier++;
			stage--;
		}
//get the optimal signal plan

//first barrier
for (i=0;i<2;i++)
	for (j=0;j<2;j++)
opt_sig_plan[i][j]=best_phase_schedule[0][T-1-opt_s[0]-opt_s[1]][i][j];
//second barrier
for (i=0;i<2;i++)
	for (j=0;j<2;j++)
opt_sig_plan[i][j+2]=best_phase_schedule[1][T-1-opt_s[0]][i][j];
//third barrier
for (i=0;i<2;i++)
	for (j=0;j<2;j++)
opt_sig_plan[i][j+4]=best_phase_schedule[2][T-1][i][j];

return 1;
}

int f_r(int ring, int phase1, int phase2, int g1, int g2, int total_length)
{
	int i;
	int TotalDelay=0;
	//assign initial queue length
	Q_temp[0][phase1-1]=ArrivalTable[0][phase1-1];
	Q_temp[0][phase2-1]=ArrivalTable[0][phase2-1];

	if(g1==0)
	{
		for(i=1;i<=total_length;i++)
		{
			if(i<=g2) //phase 2 is green
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
				//deal with the green phase
				if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[i][phase2-1]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
				if (Q_temp[i][phase2-1]<=0)
					Q_temp[i][phase2-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1]; //Add the arrivel flow
				//deal with the red phase
				Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1];
				TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			}
			else   //all red
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			}

		}
	}
	if(g2==0)
	{
		for(i=1;i<=total_length;i++)
		{
			if(i<=g1) //phase 2 is green
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
				//deal with the green phase
				if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[i][phase1-1]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
				if (Q_temp[i][phase1-1]<=0)
					Q_temp[i][phase1-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1]; //Add the arrivel flow
				//deal with the red phase
				Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			}
			else   //all red
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			}
		}
	}

	//go through other 2 phases in the same ring: these two phases should be red
	for(i=4*ring;i<4+4*ring;i++)
	{
		if ((i+1)!=phase1 && (i+1)!=phase2)
		{
			Q_temp[0][i]=ArrivalTable[0][i];
			for(int j=1;j<=total_length;j++)
			{
				Q_temp[j][i]=Q_temp[j-1][i]+ArrivalTable[j][i];
				TotalDelay+=Q_temp[j][i];
			}
		}
	}

	return TotalDelay;
}

int f1_r(int ring, int phase1, int phase2, int g1, int g2, int total_length, int B_L, int stage)
{
	int i;
	int TotalDelay=0;
	//assign initial queue length
	Q_temp[total_length-B_L][phase1-1]=Q_perm[stage-1][total_length-B_L][phase1-1];
	Q_temp[total_length-B_L][phase2-1]=Q_perm[stage-1][total_length-B_L][phase2-1];

	if(g1==0)
	{
		for(i=total_length-B_L+1;i<=total_length;i++)
		{
			if(i<=g2+total_length-B_L) //phase 2 is green
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
				//deal with the green phase
				if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[i][phase2-1]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
				if (Q_temp[i][phase2-1]<=0)
					Q_temp[i][phase2-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1]; //Add the arrivel flow
				//deal with the red phase
				Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1];
				TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			}
			else   //all red
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			}

		}
	}
	if(g2==0)
	{
		for(i=total_length-B_L+1;i<=total_length;i++)
		{
			if(i<=g1+total_length-B_L) //phase 2 is green
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
				//deal with the green phase
				if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[i][phase1-1]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
				if (Q_temp[i][phase1-1]<=0)
					Q_temp[i][phase1-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1]; //Add the arrivel flow
				//deal with the red phase
				Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			}
			else   //all red
			{
				Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
				Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
				TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
			}
		}
	}

	//go through other 2 phases in the same ring: these two phases should be red
	for(i=4*ring;i<4+4*ring;i++)
	{
		if ((i+1)!=phase1 && (i+1)!=phase2)
		{

			Q_temp[total_length-B_L][i]=Q_perm[stage-1][total_length-B_L][i];
			for(int j=total_length-B_L+1;j<=total_length;j++)
			{
				Q_temp[j][i]=Q_temp[j-1][i]+ArrivalTable[j][i];
				TotalDelay+=Q_temp[j][i];
			}
		}
	}

	return TotalDelay;
}


/*
int f(int p_phase1, int p_phase2, int Green1, int Green2, int barrier_length)
{
	if (barrier==0 && ring==0)  //phase 1 and 2
	{
		if(InitPhase==1)
		return	delay(1,2,Green1,Green2,barrier_length);
		if(InitPhase==2)
		return	delay(2,1,Green2,Green1,barrier_length);
	}
		if (barrier==0 && ring==1)  //phase 1 and 2
	{
		if(InitPhase==5)
		return	delay(5,6,Green1,Green2,barrier_length);
		if(InitPhase==6)
		return	delay(6,5,Green2,Green1,barrier_length);
	}
	if (barrier==1 && ring==0)  //phase 1 and 2
	{
		if(InitPhase==3)
		return	delay(3,4,Green1,Green2,barrier_length);
		if(InitPhase==4)
		return	delay(4,3,Green2,Green1,barrier_length);
	}
	if (barrier==1 && ring==1)  //phase 1 and 2
	{
		if(InitPhase==7)
		return	delay(7,8,Green1,Green2,barrier_length);
		if(InitPhase==8)
		return	delay(8,7,Green2,Green1,barrier_length);
	}

}
*/
int f(int ring, int phase1, int phase2, int g1, int g2, int total_length)
{
	int i;
	int TotalDelay=0;
	//assign initial queue length
	Q_temp[0][phase1-1]=ArrivalTable[0][phase1-1];
	Q_temp[0][phase2-1]=ArrivalTable[0][phase2-1];

	for(i=1;i<=total_length;i++)
	{
		if(i<=g1)  //phase 1 is green
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
			//deal with the green phase
			if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
			{
				Q_temp[i][phase1-1]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
			if (Q_temp[i][phase1-1]<=0)
				Q_temp[i][phase1-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
			else
				Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1]; //Add the arrivel flow
			//deal with the red phase
			Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>g1 && i<=g1+Yellow[phase1-1]+Red[phase1-1])  //transition time in phase 1
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>g1+Yellow[phase1-1]+Red[phase1-1] && i<=g1+Yellow[phase1-1]+Red[phase1-1]+g2) //phase 2 is green
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
			//deal with the green phase
			if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
			{
				Q_temp[i][phase2-1]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
			if (Q_temp[i][phase2-1]<=0)
				Q_temp[i][phase2-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
			else
				Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1]; //Add the arrivel flow
			//deal with the red phase
			Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>g1+Yellow[phase1-1]+Red[phase1-1]+g2 && i<=total_length)  //all red
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
	}

		//go through other 2 phases in the same ring: these two phases should be red
	for(i=4*ring;i<4+4*ring;i++)
	{
		if ((i+1)!=phase1 && (i+1)!=phase2)
		{
			Q_temp[0][i]=ArrivalTable[0][i];
			for(int j=1;j<=total_length;j++)
			{
				Q_temp[j][i]=Q_temp[j-1][i]+ArrivalTable[j][i];
				TotalDelay+=Q_temp[j][i];
			}
		}
	}

	return TotalDelay;
}

int f1(int ring, int phase1, int phase2, int g1, int g2, int total_length, int B_L, int stage)
{
	int i;
	int TotalDelay=0;
	//assign initial queue length
	Q_temp[total_length-B_L][phase1-1]=Q_perm[stage-1][total_length-B_L][phase1-1];
	Q_temp[total_length-B_L][phase2-1]=Q_perm[stage-1][total_length-B_L][phase2-1];

	for(i=total_length-B_L+1;i<=total_length;i++)
	{
		if(i<=total_length-B_L+g1)  //phase 1 is green
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
			//deal with the green phase
			if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
			{
				Q_temp[i][phase1-1]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
			if (Q_temp[i][phase1-1]<=0)
				Q_temp[i][phase1-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
			else
				Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1]; //Add the arrivel flow
			//deal with the red phase
			Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>total_length-B_L+g1 && i<=g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1])  //transition time in phase 1
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1] && i<=g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1]+g2) //phase 2 is green
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1];
			//deal with the green phase
			if ((i+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
			{
				Q_temp[i][phase2-1]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
			if (Q_temp[i][phase2-1]<=0)
				Q_temp[i][phase2-1]=0;  //if queue is 0, the arrival flow can pass, no need to add
			else
				Q_temp[i][phase2-1]+=ArrivalTable[i][phase2-1]; //Add the arrivel flow
			//deal with the red phase
			Q_temp[i][phase1-1]+=ArrivalTable[i][phase1-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
		if(i>g1+total_length-B_L+Yellow[phase1-1]+Red[phase1-1]+g2 && i<=total_length)  //all red
		{
			Q_temp[i][phase1-1]=Q_temp[i-1][phase1-1]+ArrivalTable[i][phase1-1];
			Q_temp[i][phase2-1]=Q_temp[i-1][phase2-1]+ArrivalTable[i][phase2-1];
			TotalDelay+=Q_temp[i][phase1-1]+Q_temp[i][phase2-1];
		}
	}

		//go through other 2 phases in the same ring: these two phases should be red
	for(i=4*ring;i<4+4*ring;i++)
	{
		if ((i+1)!=phase1 && (i+1)!=phase2)
		{
			Q_temp[total_length-B_L][i]=Q_perm[stage-1][total_length-B_L][i];
			for(int j=total_length-B_L+1;j<=total_length;j++)
			{
				Q_temp[j][i]=Q_temp[j-1][i]+ArrivalTable[j][i];
				TotalDelay+=Q_temp[j][i];
			}
		}
	}

	return TotalDelay;
}