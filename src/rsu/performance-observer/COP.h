
#include <stdio.h>

#define		Gamma	2    /*  minimum green time  */
#define		Delta	1    /*  dead time between phase changes  */
#define		theta	10   /*  maximum green ... not implemented  */
#define		T       30   /*  total planned time  */
#define		dt	1   
#define		N       3    /*  number of cycles  */
#define		P	8    /*  number of phases  */  
#define		J	12    /*  number of stages  J = N*P  */

int MinGreen[8]={7,7,7,7,7,7,7,7};
int MaxGreen[8]={35,35,35,35,35,35,35,35};
int Yellow[8]={3,3,3,3,3,3,3,3};
int Red[8]={2,2,2,2,2,2,2,2};

extern char temp_log[512];
extern int ArrivalTable[30][8];
extern int opt_s[12];

//int InitPhase[2]={2,6};


		
		//int		a[T][P];	/*arrival data*/
		int	v[N*P+1][T];	/*table of values*/
		int	X[N*P+1][T];	/*table of decisions*/
		int Q_temp[T][P];  //temporary standing queue at each stage
		int Q_perm[J][T][P];  //permanent standing queue at each stage
		int Total_Phases[P]={1,2,3,4,5,6,7,8};      //total 8 phases
		int Veh_No_Phase[P]={0};    //No. of vehicles requesting each phase   //added 1.30.2014
		
		
int COP(int InitPhase[2]);
int f1(int j,  int s, int phase, int x);  //calculate delay for the later stages
int f(int j,  int s, int phase);   //calculate delay for the first stage
int f_r(int j, int s, int phase);  //calculate delay if every phase is not allowed to pass: in yellow/red clearance state
int g(int z);
extern int outputlog(char *output);

int COP(int InitPhase[2])
{
		int 	j;		/*current stage number*/
		int i,k,kkk; //the iterator
		int	s;		/*state variable*/
		int	phase;		/*current phases*/
        int     temp ;
		int t; //time horizon
		int temp_X;  //temp decision variable
		
		//FILE *Data, *OutPut;
		
	/*read the data*/
	
		//Data=fopen("DP.dat","r");
		//OutPut=fopen("DP.out","w");
         //       printf("files open - read and write");

		//for (t=0; t<T; ++t) {
      //             printf(".") ;
		//    fscanf(Data,"%d %d %d %d %d %d %d %d",&a[t][0], &a[t][1],&a[t][2],&a[t][3],&a[t][4],&a[t][5],&a[t][6],&a[t][7]) ;
		//    fprintf(OutPut,"%d %d %d %d %d %d %d %d",&a[t][0], &a[t][1],&a[t][2],&a[t][3],&a[t][4],&a[t][5],&a[t][6],&a[t][7]) ;
		//}
      //          printf("\n%d read complete\n", t) ;
		    
	/*Begin the forward recursion*/
	
        //        fprintf(OutPut, "Value Function from Forward Recursion \n");
                printf("Value Function from Forward Recursion \n");

				
		//added 1.30.2014
		//calculate Total number of vehicles each phase
		//reset
		for (i=0;i<P;i++)
		{
			Veh_No_Phase[i]=0;
		}
		for (i=0;i<P;i++)
			for(j=0;j<T;j++)
			{
				Veh_No_Phase[i]+=ArrivalTable[j][i];
			}
		sprintf(temp_log,"The No. of vehicle requesting each phase is %d %d %d %d %d %d %d %d\n",Veh_No_Phase[0],Veh_No_Phase[1],Veh_No_Phase[2],Veh_No_Phase[3],Veh_No_Phase[4],Veh_No_Phase[5],Veh_No_Phase[6],Veh_No_Phase[7]);
		outputlog(temp_log); cout<<temp_log;
		
				
		phase = InitPhase[0] ; //start at initial pahse
		
		//Forward recursion
		//Stage 1
		j=0;
		for(s=0;s<T;s++)
		{
			//reset Q_temp
			for (i=0;i<T;i++)
				for(k=0;k<P;k++)
				{
					Q_temp[i][k]=0;
				}
				
			//assign initial queue to Q_Temp
			for (k=0;k<P;k++)
			{
				Q_temp[0][k]=ArrivalTable[0][k];
			}
			

			if(s+1-Yellow[InitPhase[0]-1]-Red[InitPhase[0]-1]<=0)  //first stage when X=0
			{
				X[j][s]=0;
				v[j][s]=f_r(j,s,phase);
				for (i=0;i<P;i++)
					Q_perm[j][s][i]=Q_temp[s][i];  //assign permenent queue of stage 1
			}
			else   //first stage when X>0
			{
				X[j][s] = s+1-Yellow[InitPhase[0]-1]-Red[InitPhase[0]-1];
				v[j][s] = f(j,s,phase);  //change!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				for (i=0;i<P;i++)
					Q_perm[j][s][i]=Q_temp[s][i];  //assign permenent queue of stage 1
			}
		}

		//The later stages
        for (j=1;j<J;j++)
		{
			//change phase: phase sequence is 1,2,3,4,1,2,3,4...
			phase++;
			//phase=phase%(P+1);
			phase=phase%(4+1);   //changed!!!!!!!!!!!!! need to check further!!!!!!!!!!!!!
			if(phase==0)
				phase++;


			for(s=0;s<T;s++)
			{
				if (s<MinGreen[phase-1]+Yellow[phase-1]+Red[phase-1]+Yellow[InitPhase[0]-1]+Red[InitPhase[0]-1]-1)
				{
					X[j][s] = 0;  //from the second stage, there is minimum green time limitation;
					v[j][s] = v[j-1][s];  //if not assigned time, then v is equal to the previous stage
					for (i=0;i<P;i++)
					Q_perm[j][s][i]=Q_perm[j-1][s][i];  //assign permenent queue of previous stage
				}
				else
				{
					v[j][s]=9999;    //v[j][s] start from a very big number
					for(i=MinGreen[phase-1];i<=s+1-Yellow[phase-1]-Red[phase-1]-Yellow[InitPhase[0]-1]-Red[InitPhase[0]-1];i++)
					{
						//reset Q_temp
						for (kkk=0;kkk<T;kkk++)
							for(k=0;k<P;k++)
							{
								Q_temp[kkk][k]=0;
							}
						
						//assign initial queue to Q-Temp
						for (kkk=0;kkk<P;kkk++)
						{
							Q_temp[s-Yellow[phase-1]-Red[phase-1]-i][kkk]=Q_perm[j-1][s-Yellow[phase-1]-Red[phase-1]-i][kkk];
						}

						temp_X=i;  //the green time allocated start from
						//if (j==1 && temp_X==6)
						//temp_X=5;
						//calculate the delay here
						int temp_v=f1(j,s,phase,temp_X)+v[j-1][s-temp_X-Yellow[phase-1]-Red[phase-1]];
						if(temp_v<v[j][s])
						{
							v[j][s]=temp_v;  
							X[j][s]=i;       //this is the optimal X;
							for (k=0;k<P;k++)
							Q_perm[j][s][k]=Q_temp[s][k];  //assign permenent queue of stage j
						}
					}
					if (v[j-1][s]<=v[j][s])  //s could still be zero
					{
						v[j][s]=v[j-1][s];
						X[j][s]=0;
						for (i=0;i<P;i++)
						Q_perm[j][s][i]=Q_perm[j-1][s][i];  //assign permenent queue of previous stage
					}
				}
			}

		//stop criteria
			if(j>=3)
			{
				if (v[j][T-1]==v[j-1][T-1] && v[j-1][T-1]==v[j-2][T-1] && v[j-2][T-1]==v[j-3][T-1])  //nothing is improved for one cycle
				{
					break;
				}
			}
		}

		//Retrival of Optimal Policy
		int opt_s_r[T];  //this is the optimal signal sequence in reverse order
		int Planed_phase=0;
		int stage=j-3;
		int time=T-1;
		while(stage>=0)
		{
			opt_s_r[Planed_phase]=X[stage][time];
			//calculate the phase number in order to find out the Y+R time
			int planned__cur_phase= (InitPhase[0]+stage)%5;
			if (planned__cur_phase==0)
				planned__cur_phase++;

			if (opt_s_r[Planed_phase]!=0)
			time=time-opt_s_r[Planed_phase]-Yellow[planned__cur_phase-1]-Red[planned__cur_phase-1];
			Planed_phase++;
			stage--;
		}
		//opt_s_r is in back order, opt_s_r[0] is the last phase
		sprintf(temp_log,"The optimal phase sequence is: ");
		char int_string[32];
		for (i=Planed_phase-1;i>=0;i--)
		{
			opt_s[Planed_phase-1-i]=opt_s_r[i];
		}
		for(i=0;i<Planed_phase;i++)
		{
			sprintf(int_string,"%d ",opt_s[i]);
			strcat(temp_log,int_string);	
		}
		strcat(temp_log,"\n");
		outputlog(temp_log); cout<<temp_log;

	  		 
	return Planed_phase;
} /*end of COP*/

int f(int j,  int s, int phase)  //calculate delay
{
	int k;
	int i;
	int phase_delay=0;  //delay of each phase, this is total delay divided by veh No
	int delay=0;      //This is the total average delay 
	
	
	for (i=0;i<P;i++)  //go through each phase to calcualte the delay
	{
		phase_delay=0;
		if (Total_Phases[i]==phase || Total_Phases[i]==phase+4)  // Ring one and ring 2
		{
			//add here to deal with the queue discharge!
			for(k=1;k<s+1-Yellow[phase-1]-Red[phase-1];k++)
			{
				Q_temp[k][i]=Q_temp[k-1][i];
				if ((k+1)%2==0)  //and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[k][i]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
				if (Q_temp[k][i]<=0)
					Q_temp[k][i]=0;  //if queue is 0, the arrival flow can pass, no need to add
				else
					Q_temp[k][i]+=ArrivalTable[k][i]; //Add the arrivel flow
				phase_delay+=Q_temp[k][i];
			}

			//This is the transition Y/R period
			for(k=s+1-Yellow[phase-1]-Red[phase-1];k<=s;k++)
			{
				Q_temp[k][i]=Q_temp[k-1][i]+ArrivalTable[k][i];
				phase_delay+=Q_temp[k][i];
			}
		}
		else    //the red phase
		{
				for (k=1;k<=s;k++)
				{
					Q_temp[k][i]=Q_temp[k-1][i]+ArrivalTable[k][i];
					phase_delay+=Q_temp[k][i];
				}
		}
	//if (Veh_No_Phase[i]!=0)
	//phase_delay=(int)phase_delay/Veh_No_Phase[i];
	//else
	//phase_delay=0;
	delay+=phase_delay;
	}
	
	return(delay);	

}


int f_r(int j,  int s, int phase)  //calculate delay for all phases
{
	int k;
	int i;
	int phase_delay=0;  //delay of each phase, this is total delay divided by veh No
	int delay=0;
	for (i=0;i<P;i++)  //go through each phase to calcualte the delay
	{
	phase_delay=0;
		for (k=0;k<=s;k++)
		{
			Q_temp[k][i]=Q_temp[k-1][i]+ArrivalTable[k][i];
			phase_delay+=Q_temp[k][i];
		}
	//if (Veh_No_Phase[i]!=0)
	//phase_delay=(int)phase_delay/Veh_No_Phase[i];
	//else
	//phase_delay=0;
	delay+=phase_delay;
	}
	return(delay);	

}

int f1(int j,  int s, int phase, int x) 
{
	int i,k;
	int phase_delay=0;  //delay of each phase, this is total delay divided by veh No
	int delay=0;
	
	
	for (i=0;i<P;i++)  //go through each phase to calcualte the total delay
	{
		phase_delay=0;
		if (Total_Phases[i]==phase || Total_Phases[i]==phase+4)  //The green phase
		{
			for(k=s+1-Yellow[phase-1]-Red[phase-1]-x;k<s+1-Yellow[phase-1]-Red[phase-1];k++)
			{
				Q_temp[k][i]=Q_temp[k-1][i];
				if (k%2==0 && k>s+1-Yellow[phase-1]-Red[phase-1]-x)  //k>1 means start-up delay, and k%2==0 means saturation flow rate is 2 sec/veh
				{
					Q_temp[k][i]--;  //discharging one vehicle  Note: here only consider one lane!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
				if (Q_temp[k][i]<=0)
					Q_temp[k][i]=0;  //if queue is 0, the arrival flow can pass
				else
					Q_temp[k][i]+=ArrivalTable[k][i]; //Add the arrival flow
				phase_delay+=Q_temp[k][i];
			}
			//delay when arrival at the transition period
			for(k=s+1-Yellow[phase-1]-Red[phase-1];k<=s;k++)
			{
				Q_temp[k][i]=Q_temp[k-1][i]+ArrivalTable[k][i];
				phase_delay+=Q_temp[k][i];
			}
		}
		else    //the red phase
		{
			int t_Q=Q_perm[j-1][s-x-Yellow[phase-1]-Red[phase-1]][Total_Phases[i]-1];
			for (k=s+1-Yellow[phase-1]-Red[phase-1]-x;k<=s;k++)
			{
				t_Q+=ArrivalTable[k][i];   //queue is green each time
				phase_delay+=t_Q;
			}
			Q_temp[s][i]=t_Q;
		}
	//if (Veh_No_Phase[i]!=0)
	//phase_delay=(int)phase_delay/Veh_No_Phase[i];
	//else
	//phase_delay=0;
	delay+=phase_delay;
	}

	return(delay);	
}


int g(int z)
{
        if(z == 0 ) return(0) ;
        else return(z+Delta);
}

