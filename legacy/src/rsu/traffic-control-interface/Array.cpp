//#include "stdafx.h"
#include "Array.h"

int BarrierPhaseIsMissing(int ReqPhase,int *InitPhase,int size)
{
    // return 0 means not missing, other means barrier group is missing.
    int BarrierPhaseMissing=-1;

    switch(ReqPhase)
    {
    case 1:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==5 || InitPhase[i]==6)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=6;
        }
        break;        

    case 2:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==5 || InitPhase[i]==6)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=6;
        }
        break; 
    case 3:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==7 || InitPhase[i]==8)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=8;
        }
        break;        

    case 4:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==7 || InitPhase[i]==8)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=8;
        }
        break;

    case 5:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==1 || InitPhase[i]==2)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=2;
        }
        break;        

    case 6:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==1 || InitPhase[i]==2)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=2;
        }
        break; 
    case 7:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==3 || InitPhase[i]==4)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=4;
        }
        break;        

    case 8:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==3 || InitPhase[i]==4)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=4;
        }
        break;
    }

    return BarrierPhaseMissing;
}



int BarrierPhaseIsMissing2(int ReqPhase,int *InitPhase,int size)
{
    // return 0 means not missing, other means barrier group is missing.
    int BarrierPhaseMissing=-1;

    switch(ReqPhase)
    {
    case 1:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==5 || InitPhase[i]==6)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=InitPhase[1];
        }
        break;        

    case 2:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==5 || InitPhase[i]==6)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=InitPhase[1];
        }
        break; 
    case 3:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==7 || InitPhase[i]==8)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=InitPhase[1];
        }
        break;        

    case 4:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==7 || InitPhase[i]==8)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=InitPhase[1];
        }
        break;

    case 5:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==1 || InitPhase[i]==2)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=InitPhase[0];
        }
        break;        

    case 6:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==1 || InitPhase[i]==2)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=InitPhase[0];
        }
        break; 
    case 7:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==3 || InitPhase[i]==4)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=InitPhase[0];
        }
        break;        

    case 8:
        for(int i=0;i<size;i++)
        {
            if(InitPhase[i]==3 || InitPhase[i]==4)
            {
                BarrierPhaseMissing=0;
            }            
        }

        if(BarrierPhaseMissing!=0)
        {
            BarrierPhaseMissing=InitPhase[0];
        }
        break;
    }

    return BarrierPhaseMissing;
}


int BarrierPhaseIsMissing(int ReqPhase,vector<int> Phase_vc)
{
    //If not missing, return 0; missing, return the missing phase
    int size=Phase_vc.size();
    int *PhaseArray=new int[size];
    for(int i=0;i<size;i++)
    {
        PhaseArray[i]=Phase_vc[i];
    }

   int BarrierPhaseMissing=BarrierPhaseIsMissing(ReqPhase,PhaseArray,size);
   delete []PhaseArray;

   return BarrierPhaseMissing;

}
int AllSameElementOfVector(vector<int> EV_Phase_vc)
{
// Return 1 is the elements are all the same;
// If size<=1, return 0;
    int size=EV_Phase_vc.size();
    int SameElement=1;

    if(size>1)
    {
        int FirstValue=EV_Phase_vc[0];

        for(int i=1;i<size;i++)
        {
            if(FirstValue!=EV_Phase_vc[i])
            {
                SameElement=0;
                break;
            }    
        }
    }
    else
    {
        SameElement=0;
    }

    return SameElement;
}


void bubbleSort(int arr[], int n)
{
    //-------Bubble Sort------
    //-------http://www.algolist.net/Algorithms/Sorting/Bubble_sort//
    
    bool swapped = true;

      int j = 0;
      int tmp;
      while (swapped) 
      {
            swapped = false;
            j++;
            for (int i = 0; i < n - j; i++) 
            {
                  if (arr[i] > arr[i + 1]) 
                  {
                        tmp = arr[i];
                        arr[i] = arr[i + 1];
                        arr[i + 1] = tmp;
                        swapped = true;
                  }
            }
      }
}

void selectionSort(int a[], int size)
    {
    int m;
    double hold;

    for (int k=0; k<=size-2; k++)
        {
        m = k;
        for(int j=k+1; j<size-1; ++j)
            {
            if (a[j] < a[m])
                m=j;
            }
        hold = a[m];
        a[m] = a[k];
        a[k] = hold;
        }
    }


// Function to remove the duplicates: LATER when we use the array, we also need the new size
int removeDuplicates(int a[], int array_size) 
    { 
    int i, j;
    j = 0;
    // Remove the duplicates ...
    for (i = 1; i < array_size; i++) 
        { 
        if (a[i] != a[j]) 
            { 
            j++; 
            a[j] = a[i]; // Move it to the front 
            } 
        } 
    // The new array size..
    array_size = (j + 1);
    // Return the new size...
    return(j + 1); 
    }

void GeneratePhaseArray(int startphase,int number, int cycle, int *phase_seq,int *ResultSeq)
    {
    // N is the phase number in a cycle for a ring, so for 8 phase
    // It is 4, Cycle is the Cycle number
    int idx=FindIndexArray<int>(phase_seq,number,startphase);

    for(int i=0;i<number;i++)
        {
        for(int j=0;j<cycle;j++)
            {
            ResultSeq[i+j*number]=phase_seq[(idx+i+j*number)%number];
            }
        }  
    }





// If NoRepeat=0: will generate {0,1,2,3,0,1}; NoRepeat=1: will generate {0,1,2,3,10,11}
/*
int *GeneratePhaseArray(int StartPhase, int *PhaseSeq,int N, int TotalNo, int NoRepeat)
{
	if(NoRepeat==0)
	{
		// N is the phase number in a cycle for a ring, so for 8 phase, it is 4, TotalNo is the total number of phases in this sequence.
		int idx=FindIndexArray(PhaseSeq,N, StartPhase);
		int Cycle=TotalNo/N+1;
		int *PhaseArray1=new int[N*Cycle];
		for(int i=0;i<N;i++)
		{
			for(int j=0;j<Cycle;j++)
			{
				PhaseArray1[i+j*N]=PhaseSeq[(idx+i+j*N)%N];//+j*10;
			}
		}
		int *ArrayReturn=new int[TotalNo];
		for(int i=0;i<TotalNo;i++)
		{
			ArrayReturn[i]=PhaseArray1[i];
		}
		//PhaseArray[N*Cycle]=PhaseSeq[(idx+N*Cycle)%N]+cycle*10;
		return  ArrayReturn;

	}
	else //
	{
		// N is the phase number in a cycle for a ring, so for 8 phase, it is 4, TotalNo is the total number of phases in this sequence.
		int idx=FindIndexArray(PhaseSeq,N, StartPhase);
        int nCycles=4;
        int *phase_array=new int[nCycles*N]; // At most n cycles, each cycle has N elements
        for(int i=0;i<nCycles;i++)
        {
            for(int j=0;j<N;j++)
            {
                phase_array[i*N+j]=PhaseSeq[j]+i*10;
            }                
        }
		int *ArrayReturn=new int[TotalNo];
		for(int i=0;i<TotalNo;i++)
		{
			ArrayReturn[i]=phase_array[i+idx];
		}
		return  ArrayReturn;
	}

}
*/

/* //****OLD staff---//
int *GeneratePhaseArray(int StartPhase, int *PhaseSeq,int N, int TotalNo, int NoRepeat)
{
	// N is the phase number in a cycle for a ring, so for 8 phase, it is 4, TotalNo is the total number of phases in this sequence.
	int idx=FindIndexArray(PhaseSeq,N, StartPhase);

	int Cycle=TotalNo/N+1;
	int *PhaseArray1=new int[N*Cycle];

	for(int i=0;i<N;i++)
	{
		for(int j=0;j<Cycle;j++)
		{
			PhaseArray1[i+j*N]=PhaseSeq[(idx+i+j*N)%N];//+j*10;
		}
	}

	int *ArrayReturn=new int[TotalNo];

	for(int i=0;i<TotalNo;i++)
	{
		ArrayReturn[i]=PhaseArray1[i];
	}


	//PhaseArray[N*Cycle]=PhaseSeq[(idx+N*Cycle)%N]+cycle*10;

	return  ArrayReturn;

}

//*/
void binary(int number)
    {// Conversion from decimal to binary: we need add "endl" after calling it.
    int remainder;

    if(number <= 1)
        {
        cout << number;
        return;
        }

    remainder = number%2;
    binary(number >> 1);
    cout << remainder;
    }



void xTimeStamp( char * pc_TimeStamp_ )
    {
    struct tm  * ps_Time;
    time_t       i_CurrentTime;
    char         ac_TmpStr[256];

    i_CurrentTime =  time(NULL);
    ps_Time = localtime( &i_CurrentTime );

    //year
    sprintf(ac_TmpStr, "%d", ps_Time->tm_year + 1900);
    strcpy(pc_TimeStamp_, ac_TmpStr);

    //month
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_mon + 1 );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //day
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_mday );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //hour
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_hour  );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //min
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_min );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //sec
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_sec );
    strcat(pc_TimeStamp_, ac_TmpStr);
    }

int msleep(unsigned long milisec)
    {
    struct timespec req={0};
    time_t sec=(int)(milisec/1000);
    milisec=milisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*1000000L;
    while(nanosleep(&req,&req)==-1)
        continue;
    return 1;
    }

int FindCycleIdx(double timeStamp[3],double timer)
{
	//------ 3 for cycles
	int CycleIdxFound=-1;

	if(timer>=0 && timer <timeStamp[0])
	{
		CycleIdxFound=0;
	}

	if(timer>=timeStamp[0] && timer <timeStamp[1])
	{
		CycleIdxFound=1;
	}
	if(timer>=timeStamp[1] && timer <timeStamp[2])
	{
		CycleIdxFound=2;
	}

	return CycleIdxFound;

}