#include "Signal.h"


void FindPhaseNext(int PhaseNext[2],int CurPhase[2])
{
    PhaseNext[0]=-1; // initial as 1 if there is no phase next
    PhaseNext[1]=-1; // initial as 5 if there is no phase next

    for(int j=0;j<2;j++)
    {
        for(int i=0;i<4;i++)
        {
            if(CurPhaseStatus[i+j*4]==2)
            {
                PhaseNext[j]=(i+j*4);
            }

            if(CurPhaseStatus[i+j*4]==5 || CurPhaseStatus[i+j*4]==6 || CurPhaseStatus[i+j*4]==7)
            {
                CurPhase[j] =(i+j*4);
            }
        }
    }

    
}

double GetSeconds()
{
	struct timeval tv_tt;
	gettimeofday(&tv_tt, NULL);
	return (tv_tt.tv_sec+tv_tt.tv_usec/1.0e6);    
}

char *GetDate()
{
    time_t rawtime;

    time ( &rawtime );

    return ctime(&rawtime);
}



int FindPhaseIndex(int *phase, int N, int SP)
// N is the number of the array phase[], SP is the dedired phase to be found in the array
{
	for(long i=0;i<N;i++)
	{
		if (SP==phase[i])
			return i;    // Find the index and return
	}
	// If the SP is not found in the phase[], then return -1;
	cout<<"There is no SP in Phase array!\n";
	return -1;	
}

void Phase::UpdatePhase(PhaseStatus newPhaseStatus)
{
	/*
	* newPhaseStatus is the status reading from the controller.
	*/
	Phase_Status[1]=newPhaseStatus;

    int PhaseNext[2];
    
    FindPhaseNext(PhaseNext,CurPhase);


	//CurPhase[0]=-1;
	//CurPhase[1]=-1;

	for (int i=0;i<numPhases;i++)
	{        	
		if (Phase_Status[0].phaseColor[i]==RED && Phase_Status[1].phaseColor[i]==GREEN )
		{//---- Previous state is Red, and current is Green
			int indx=int(i/4);  // Ring index
			//CurPhase[indx]=i; //-----Critical---------: {0-7} real phase should +1
			StartTime[i] = GetSeconds(); 
			ColorTime[i] = 0;
			//cout<<"Phase: "<<i+1<<" Ring: "<<indx+1<<"..........Turn to Green!\n";
		}

		else if (Phase_Status[0].phaseColor[i]==GREEN && Phase_Status[1].phaseColor[i]==YELLOW )
		{ //---- Previous state is Green, and current is Yellow
			int indx=int(i/4); // Ring index

			//CurPhase[indx]=i; //-----Critical---------: {0-7} real phase should +1

			StartTime[i] = GetSeconds();
			ColorTime[i]=0; 
			//cout<<"Phase: "<<i+1<<" Ring: "<<indx+1<<"..........Turn to Yellow!\n";
		}  

		else if (Phase_Status[0].phaseColor[i]==YELLOW && Phase_Status[1].phaseColor[i]==RED )
		{ //---- Previous state is Yellow, and current is Red	
			int indx=int(i/4); // Ring index

			//CurPhase[indx]=i; //-----Critical---------: 0-7 real phase should +1

			StartTime[i] = GetSeconds();
			ColorTime[i]=0; 
			//cout<<"Phase: "<<i+1<<" Ring: "<<indx+1<<"..........Turn to Red!\n";
		}
	}

     /*
            if (CurPhase[1-ConfigIS.MP_Ring]==ConfigIS.MP_Relate)
        {
            CurPhase[ConfigIS.MP_Ring]=ConfigIS.MissPhase;
            StartTime[ConfigIS.MissPhase]=StartTime[ConfigIS.MP_Relate];
            ColorTime[ConfigIS.MissPhase]=ColorTime[ConfigIS.MP_Relate];
        }
    //*/

    for(int i=0;i<2;i++) // 2 rings
    {
        if (CurPhase[1-ConfigIS.MP_Ring[i]]==ConfigIS.MP_Relate[i])
        {
            CurPhase[ConfigIS.MP_Ring[i]]   =ConfigIS.MissPhase[i];
            StartTime[ConfigIS.MissPhase[i]]=StartTime[ConfigIS.MP_Relate[i]];
            ColorTime[ConfigIS.MissPhase[i]]=ColorTime[ConfigIS.MP_Relate[i]];
        }

    }
	

	for (int i=0;i<numPhases;i++)
	{
		if(Phase_Status[1].phaseColor[i]!=RED)
		{
			CurPhase[i/4]=i;
			PrePhase[i/4]=CurPhase[i/4];

		}
		else  //added by YF 01.17.2014
		{
			CurPhase[i/4]=PrePhase[i/4];
		}

		ColorTime[i]=GetSeconds()-StartTime[i];

		//-----GET the initial phase and time for GLPK solver----//// We use PhaseNext on 2012.4.23
		for(int j=0;j<2;j++) //--- 2 Rings-------
		{
			int CP=CurPhase[j];  // {0-7}

			GrnElapse[j]=0;

			if(CP==ConfigIS.MissPhase[j]) // TODO: think more
			{
				CP=ConfigIS.MP_Relate[j];
			}

			if(Phase_Status[1].phaseColor[CP]==GREEN)
			{
				InitTime[j]=0;
				InitPhase[j]=CurPhase[j];
				GrnElapse[j]=ColorTime[CP];
			}

			else if(Phase_Status[1].phaseColor[CP]==YELLOW)
			{
				if(j==0)
				{
					int *phaseSeq=ConfigIS.Phase_Seq_R1;//{0,1,3};  //1,2,4
					int phase=CP%4; // consider two rings.

					int phaseIdx=FindPhaseIndex(phaseSeq,ConfigIS.Ring1No,phase);

					InitPhase[j]=phaseSeq[(phaseIdx+1)%ConfigIS.Ring1No]+4*j;

                    if(InitPhase[j]!=PhaseNext[j] && PhaseNext[j]>=0)
                    {
                        InitPhase[j]=PhaseNext[j];
                    }
					//InitTime[j]=ConfigIS.Yellow[InitPhase[j]]-ColorTime[CP]+ConfigIS.Red[InitPhase[j]];
                    InitTime[j]=max(ConfigIS.Yellow[InitPhase[j]]-ColorTime[CP]+ConfigIS.Red[InitPhase[j]],0.0);
				}
				else  //j==1
				{
                    int *phaseSeq=ConfigIS.Phase_Seq_R2;
                    int phase=CP%4; // consider two rings. CP {4-7}

                    int phaseIdx=FindPhaseIndex(phaseSeq,ConfigIS.Ring2No,phase);

                    if(phaseIdx>=0)
                    {
                        InitPhase[j]=phaseSeq[(phaseIdx+1)%ConfigIS.Ring2No]+4*j;

                        if(InitPhase[j]!=PhaseNext[j] && PhaseNext[j]>=0)
                        {
                            InitPhase[j]=PhaseNext[j];
                        }
                    }

					//InitTime[j]=ConfigIS.Yellow[InitPhase[j]]-ColorTime[CP]+ConfigIS.Red[InitPhase[j]];
                    InitTime[j]=max(ConfigIS.Yellow[InitPhase[j]]-ColorTime[CP]+ConfigIS.Red[InitPhase[j]],0.0);
				}                        
			}

			else if(Phase_Status[1].phaseColor[CP]==RED)
			{
				if(j==0)
				{
					int *phaseSeq=ConfigIS.Phase_Seq_R1;//{0,1,3};  //1,2,4
					int phase=CP%4; // consider two rings.
					int phaseIdx=FindPhaseIndex(phaseSeq,ConfigIS.Ring1No,phase);

                    if (phaseIdx>=0)
                    {
                        InitPhase[j]=phaseSeq[(phaseIdx+1)%ConfigIS.Ring1No];

                        if(InitPhase[j]!=PhaseNext[j] && PhaseNext[j]>=0)
                        {
                            InitPhase[j]=PhaseNext[j];
                        }
                    }                   

					InitTime[j]=max(ConfigIS.Red[InitPhase[j]]-ColorTime[CP],0.0);
				}  
				else // j==1
				{
					int *phaseSeq=ConfigIS.Phase_Seq_R2;
					int phase=CP%4; // consider two rings.
					int phaseIdx=FindPhaseIndex(phaseSeq,ConfigIS.Ring2No,phase);
					if(phaseIdx>=0)
					{
						InitPhase[j]=phaseSeq[(phaseIdx+1)%ConfigIS.Ring2No]+4*j;
                        if(InitPhase[j]!=PhaseNext[j] && PhaseNext[j]>=0)
                        {
                            InitPhase[j]=PhaseNext[j];
                        }
					}

					InitTime[j]=max(ConfigIS.Red[InitPhase[j]]-ColorTime[CP],0.0);
				}
			}
		}

	}

    /*
	if (CurPhase[1-ConfigIS.MP_Ring]==ConfigIS.MP_Relate)
	{ 
		ColorTime[ConfigIS.MissPhase]=ColorTime[ConfigIS.MP_Relate];
	} //*/

    for(int i=0;i<2;i++)   // 2 rings
    {
        if (CurPhase[1-ConfigIS.MP_Ring[i]]==ConfigIS.MP_Relate[i])
        { 
            ColorTime[ConfigIS.MissPhase[i]]=ColorTime[ConfigIS.MP_Relate[i]];
        }
    }    

	Phase_Status[0]=Phase_Status[1]; // WORK
}


Phase::Phase(void)
{
	for(int i=0;i<numPhases;i++)
	{
		Phase_Status[0].phaseColor[i]=0;
		Phase_Status[1].phaseColor[i]=0;
		StartTime[i]=GetSeconds();
		ColorTime[i]=0;
	}
	for(int i=0;i<2;i++)
	{
		CurPhase[i]=0;
		InitPhase[i]=0;
		InitTime[i]=0;        
		//color[i]=R;                
	}
}



Phase& Phase::operator=(Phase& newPhase)
{
	for(int i=0;i<2;i++)
	{
		Phase_Status[i]=newPhase.Phase_Status[i];
		CurPhase[i]=newPhase.CurPhase[i];        

		InitPhase[i]=newPhase.InitPhase[i];
		InitTime[i] =newPhase.InitTime[i];
	}
	for(int i=0;i<numPhases;i++)
	{
		StartTime[i]=newPhase.StartTime[i];
		ColorTime[i]=newPhase.ColorTime[i];
	}
	return *this;
}

/*
void Phase::RecordPhase(char *filename)
{
fstream fs;
fs.open(filename,fstream::out|fstream::app);

fs<<"At Time: "<<time(NULL)<<"  Current Phase: "<<CurPhase[0]+1<<"\t"<<ColorTime[CurPhase[0]]<<"\t"
<<CurPhase[1]+1<<"\t"<<ColorTime[CurPhase[1]]<<"\t";
cout<<" Current Phase: "<<CurPhase[0]+1<<" "<<CurPhase[1]+1<<"\t"
<<"Initial Phase:\t"<< InitPhase[0]+1<<"  "<< InitPhase[1]+1<<"\n";

for(int i=0;i<numPhases;i++)
{
//fs<<" "<<Phase_Status[1].phaseColor[i]<<" "<<StartTime[i]<<" "<<Phase_Status[0].phaseColor[i]<<endl;
fs<<"\t"<<Phase_Status[1].phaseColor[i]<<"\t"<<ColorTime[i];
cout<<" "<<Phase_Status[1].phaseColor[i];
}
fs<<endl;

fs.close();
}
*/

void Phase::RecordPhase(char *filename)
{
	FILE *fs;
	fs=fopen(filename,"a+");

	//fs<<"At Time: "<<time(NULL)<<"  Current Phase: "<<CurPhase[0]+1<<"\t"<<ColorTime[CurPhase[0]]<<"\t"
	//    <<CurPhase[1]+1<<"\t"<<ColorTime[CurPhase[1]]<<"\t";

	fprintf(fs,"At %.2f \t CP:\t %d \t %.2f \t  %d \t %.2f ",GetSeconds(),CurPhase[0]+1,
		ColorTime[CurPhase[0]],(CurPhase[1]+1),ColorTime[CurPhase[1]] );

	cout<<" Current Phase: "<<CurPhase[0]+1<<" "<<CurPhase[1]+1<<"\t"
		<<"Initial Phase:\t"<< InitPhase[0]+1<<"  "<< InitPhase[1]+1<<"\n";

	for(int i=0;i<numPhases;i++)
	{
		//fs<<"\t"<<Phase_Status[1].phaseColor[i]<<"\t"<<ColorTime[i];
		fprintf(fs,"\t %d \t %.2f",Phase_Status[1].phaseColor[i],ColorTime[i]);
	}

	for(int j=0;j<2;j++)
	{
		fprintf(fs,"\t %d \t %.2f",InitPhase[j]+1,InitTime[j]);
	}

	for(int j=0;j<2;j++)
	{
		fprintf(fs,"\t %d \t %.2f",InitPhase[j]+1,GrnElapse[j]);
	}

	fprintf(fs,"\n");

	fclose(fs);
}




void Phase::Display()
{
	char szTemp[256];


	for (int i=0;i<numPhases;i++)
	{

		if(Phase_Status[1].phaseColor[i]==RED)
		{
			sprintf(szTemp,"Phase %d: R :%lf:%lf",i+1,StartTime[i],ColorTime[i]);
			cout<<szTemp<< endl;
		}
		else if (Phase_Status[1].phaseColor[i]==GREEN)
		{
			sprintf(szTemp,"Phase %d: G :%lf:%lf",i+1,StartTime[i],ColorTime[i]);
			cout<<szTemp<< endl;
		}
		else if (Phase_Status[1].phaseColor[i]==YELLOW)
		{
			sprintf(szTemp,"Phase %d: Y :%lf:%lf",i+1,StartTime[i],ColorTime[i]);
			cout<<szTemp<< endl;
		}
		else
		{
			sprintf(szTemp,"ERROR POLL PHASE! Multiple color for this phase %d",i+1);
			cout<<szTemp<< endl;

		}

	}
	sprintf(szTemp,"Current phase: %d & %d",CurPhase[0]+1,CurPhase[1]+1);
	cout<<szTemp<< endl;

}
