#pragma once
//  Solution from : TestConfig 
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <string.h>

using namespace std;


/*
*** Array has n elements, find the index of value equal data.
*** Return the index used by an array directly, which means should be (real count -1)
*/
template <class T>
int FindIndexArray(T *Array,int n,T data)
    {
    int index=-1;
    for(int i=0;i<n;i++)
        {
        if (data==Array[i])
            {
            index=i;
            break;
            }
        }

    if(index==-1)
        {
        cout<<"***Error*** Data is not in the Array !\n";
        return index;
        }
    else
        return index;
    }



/*
*** "Array" has "n" elements, find the "nTh" index of value equal "data"
*/
template <class T>
int FindIndexArray(T *Array,int n,T data, int nTh)
{
	int index=-1;
	int FoundTimes=0;
	for(int i=0;i<n;i++)
	{
		if (data==Array[i])
		{
			FoundTimes++;
			if(FoundTimes==nTh)
			{
				index=i;
				break;		
			}
		}
	}

	return index;
}

/*
*** Summation of an array from StartIdx till EndIdx, EndIdx not included.
*/
template <class T>
T SumArray(T *Array,int n,int StartIdx,int EndIdx)
{
	T SumValue=0;
	if(StartIdx> (n-1) || EndIdx>(n-1) || StartIdx>EndIdx)
	{
		cout<<"There is problem with the index!!!\n";
		exit(1);
	}
	else
	{
		for(int i=StartIdx;i<EndIdx;i++)
		{
			SumValue+=Array[i];			
		}

		return(SumValue);		
	}
}


template <class T>
void PrintArray(T *Array,int n)
    {
    for(int i=0;i<n;i++)
        {
        cout<<Array[i]<<"\t";
        }
    cout<<endl;
    }


//void GeneratePhaseArray(int StartPhase, int Number, int Cycle ,int *PhaseSeq);
void GeneratePhaseArray(int startphase, int number, int cycle ,int *phase_seq,int *ResultSeq);

//int *GeneratePhaseArray(int StartPhase, int *PhaseSeq, int N, int TotalNo, int NoRepeat=0);
	
int BarrierPhaseIsMissing(int ReqPhase,int *InitPhase,int size);
int BarrierPhaseIsMissing2(int ReqPhase,int *InitPhase,int size);
int BarrierPhaseIsMissing(int ReqPhase,vector<int> Phase_vc);

int AllSameElementOfVector(vector<int> EV_Phase_vc);
void bubbleSort(int arr[], int n);
int removeDuplicates(int a[], int array_size);
void selectionSort(int a[], int size);
void binary(int number);
void xTimeStamp( char * pc_TimeStamp_ );

int msleep(unsigned long milisec);
int FindCycleIdx(double timeStamp[3],double timer);// 3 for cycles
