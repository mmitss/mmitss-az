#include "Signal.h"

double GetSeconds()
    {
    struct timeval tv_tt;
    gettimeofday(&tv_tt, NULL);
    return (tv_tt.tv_sec+tv_tt.tv_usec/1.0e6);    
    }

//char rsuID[64]="MountaiCampbell";
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

    CurPhase[0]=-1;
    CurPhase[1]=-1;

    for (int i=0;i<numPhases;i++)
        {        	
        if (Phase_Status[0].phaseColor[i]==RED && Phase_Status[1].phaseColor[i]==GREEN )
            {//---- Previous state is Red, and current is Green
            int indx=int(i/4);
            CurPhase[indx]=i; //-----Critical---------: 0-7 real phase should +1
            StartTime[i] = GetSeconds(); 
            ColorTime[i] = 0;
            cout<<"Phase: "<<i+1<<" Ring: "<<indx+1<<"..........Turn to Green!\n";
            }

        else if (Phase_Status[0].phaseColor[i]==GREEN && Phase_Status[1].phaseColor[i]==YELLOW )
            { //---- Previous state is Green, and current is Yellow
            int indx=int(i/4);
            StartTime[i] = GetSeconds();
            ColorTime[i]=0; 
            cout<<"Phase: "<<i+1<<" Ring: "<<indx+1<<"..........Turn to Yellow!\n";
            }  

        else if (Phase_Status[0].phaseColor[i]==YELLOW && Phase_Status[1].phaseColor[i]==RED )
            { //---- Previous state is Yellow, and current is Red	
            int indx=int(i/4);         
            StartTime[i] = GetSeconds();
            ColorTime[i]=0; 
            cout<<"Phase: "<<i+1<<" Ring: "<<indx+1<<"..........Turn to Red!\n";
            }
        }

    for (int i=0;i<numPhases;i++)
        {
        if(Phase_Status[1].phaseColor[i]!=RED)
            {
            CurPhase[i/4]=i;
            }

        ColorTime[i]=GetSeconds()-StartTime[i];

        //-----GET the initial phase and time for GLPK solver----
        for(int j=0;j<2;j++) //--- 2 Rings-------
            {
            int CP=CurPhase[j];  // 0-7
            // In case of one phase is missing: for example {1,2,6,8}:No "4"
            if(CP<0) 
                {
                CP=CurPhase[(j+1)%2];  // Real phase -1: 0-7
                int jjj=int(pow(-1.0,j));
                CurPhase[j]=CurPhase[(j+1)%2]-4*jjj; // Assign to the missing phase
                }

            //Color Curcol=Phase_Status[1].phaseColor[CP];

            if(Phase_Status[1].phaseColor[CP]==G)
                {
                InitTime[j]=0;
                InitPhase[j]=CurPhase[j];
                }
            else if(Phase_Status[1].phaseColor[CP]==Y)
                {
                if(j==0)
                    {
                    int phaseSeq[3]={0,1,3};  //1,2,4
                    int phase=CP%4; // consider two rings.
                    int phaseIdx=FindPhaseIndex(phaseSeq,3,phase);
                    InitPhase[j]=phaseSeq[(phaseIdx+1)%3]+4*j;
                    InitTime[j]=YellowInteval-ColorTime[CP]+RedInterval;
                    }
                else
                    {
                    int phaseSeq[2]={1,3}; // 6,8 for Ring 2
                    int phase=CP%4; // consider two rings.
                    int phaseIdx=FindPhaseIndex(phaseSeq,2,phase);
                    InitPhase[j]=phaseSeq[(phaseIdx+1)%2]+4*j;
                    InitTime[j] =YellowInteval-ColorTime[CP]+RedInterval;
                    }                        
                }

            else if(Phase_Status[1].phaseColor[CP]==R)
                {
                if(j==0)
                    {
                    int phaseSeq[3]={0,1,3};  //1,2,4
                    int phase=CP%4; // consider two rings.
                    int phaseIdx=FindPhaseIndex(phaseSeq,3,phase);
                    InitPhase[j]=phaseSeq[(phaseIdx+1)%3]+4*j;
                    InitTime[j]=RedInterval-ColorTime[CP];
                    }
                else
                    {
                    int phaseSeq[2]={1,3}; // 6,8 for Ring 2
                    int phase=CP%4; // consider two rings.
                    int phaseIdx=FindPhaseIndex(phaseSeq,2,phase);
                    InitPhase[j]=phaseSeq[(phaseIdx+1)%2]+4*j;
                    InitTime[j]=RedInterval-ColorTime[CP];
                    }
                }
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
void Phase::RecordPhase(char *filename)
    {
    fstream fs;
    fs.open(filename,fstream::out|fstream::app);
    //fs<<"Signal_status "<<rsuID;
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo=localtime(&rawtime);

    /*    fs<<" "<<rawtime<<"  "<<asctime(timeinfo);
    fs<<" Current Phase: "<<CurPhase[0]+1<<" "<<ColorTime[0]<<" "<<CurPhase[1]+1<<" "<<ColorTime[0]<<endl;
    for(int i=0;i<numPhases;i++)
    {
    fs<<" "<<Phase_Status[1].phaseColor[i]<<" "<<StartTime[i]<<" "<<Phase_Status[0].phaseColor[i]<<endl;
    }
    fs<<endl;
    fs.close(); */  

    //fs<<" "<<rawtime<<"  "<<asctime(timeinfo);
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
