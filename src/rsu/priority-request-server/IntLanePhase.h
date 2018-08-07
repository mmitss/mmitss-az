#pragma once


#include <sstream>
#include <iostream>
#include <fstream>
#include <string.h>
#include "stdio.h"
#include "stdlib.h"

#ifndef MAX_APRROACH
    #define MAX_APRROACH 10
#endif
#ifndef MAX_LANE
    #define MAX_LANE 10
#endif

using namespace std;
class IntLanePhase
{
public:
	int iIntersectionID;
	int iTotalApproaches;
	int iTotalPhases;
	int iTotalIngress;
	int iApproach;
	int iNoRow;
	int iInLane;
	int iOutLane;
	int iPhase;
	int iOutApproach;
	int iInLaneOutLanePhase[MAX_APRROACH][MAX_LANE][MAX_APRROACH][MAX_LANE];

	//Methods
	int ReadLanePhaseMap(char* filename);
	int findThePhaseOfInLine(int ilane);

};
