/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   



/* Signal.h
*  Created by :Jun Ding
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.

*/

#pragma once

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "Config.h"   //extern RSU_Config ConfigIS;

using namespace std;

//enum Color {R=1, G=3, Y=4};
#define numPhases 8//Number of considering phases

typedef struct 
    {
    int phaseColor[numPhases];
    } PhaseStatus;

#define RED 1
#define YELLOW 4
#define GREEN 3


double GetSeconds();   // Get the current time: seconds in float point number

char *GetDate();     // Get the current date as: "Sat May 20 15:21:51 2000"

extern int CurPhaseStatus[8];

void FindPhaseNext(int PhaseNext[2],int CurPhase[2]);

class Phase //current phase status: PhaseRecord in the Windows code
{
public:
    PhaseStatus Phase_Status[2]; //[0]: for previous; [1]: for current/new 
	double	StartTime[numPhases];	    
	int CurPhase[2];        //current timing phase {0-7} in Ring[2]
	int PrePhase[2];        //timing phase of previous time step, used for all red situation added by YF 01.17.2014
	int InitPhase[2];       // Real phase should +1: Ring 1:0-3; Ring 2:4-7
    double InitTime[2];     // with InitPhase: for GLPK Solver.
	double GrnElapse[2];
	double ColorTime[numPhases];
	

public:
	void UpdatePhase(PhaseStatus newPhaseStatus);
	void Display();
    void RecordPhase(char *filename);
	Phase& operator=(Phase& newPhase);
	Phase();
    };

extern Phase Phases;  //----GLOBAL:Important one----//
