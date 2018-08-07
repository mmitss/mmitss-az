#pragma once

#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <sys/time.h>
using namespace std;

#define RED 1
#define GREEN 3
#define YELLOW 4


#define numPhases 8//Number of considering phases
#define numPhaseRing 4//Number of phases in a ring

#define PhaseSeq 1234 //phase sequence:1234 or 2143 or 24
const int RedInterval=2; // 2 seconds
const int YellowInteval=3; // 3 Seconds


double GetSeconds();   


enum Color {R=1, G=3, Y=4};


//PhaseStatus phase_read={0};

struct PhaseStatus
    {
    int phaseColor[numPhases];
    };



class Phase //current phase status: PhaseRecord in the Windows code
{
public:
    PhaseStatus Phase_Status[2]; //[0]: for previous; [1]: for current/new 
	double	StartTime[numPhases];	    
	int CurPhase[2];  //current timing phase 0-7 
	int InitPhase[2];   // Real phase should +1: Ring 1:0-3; Ring 2:4-7
    double InitTime[2];    // with InitPhase: for GLPK Solver.
	double ColorTime[numPhases];
	//Color color[numPhases];

public:
	void UpdatePhase(PhaseStatus newPhaseStatus);
	void Display();
    void RecordPhase(char *filename);
	Phase& operator=(Phase& newPhase);
	Phase();
    };
