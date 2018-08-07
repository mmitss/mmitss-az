#pragma once

#include <iostream>
#include <time.h>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>
#include "Array.h"

using namespace std;


typedef struct
    {
    int MissPhase[2],MP_Relate[2],MP_Ring[2]; // It is possible that at most one phase missing
    int Ring1No,Ring2No;
    int *Phase_Seq_R1,*Phase_Seq_R2;  // Phase_Seq_R[1][2] {0-3}
    double Yellow[8],Red[8],Gmin[8],Gmax[8];
    } RSU_Config;

//int MissPhase,MP_Relate;  // Missing phase and related phase in the other ring:real phase -1
//int MP_Ring;   // The Ring where missing phase at: 0 or 1
//MissPhase=3;  // Missing phase Ring 1 "4";
//MP_Relate=7;  // Related phase Ring 2 "8";
//MP_Ring=0;    // Missing Ring "1"

extern RSU_Config ConfigIS;


void PrintRSUConfig();
void PrintRSUConfig(RSU_Config configIS);
void PrintRSUConfig2File(RSU_Config configIS,char *filename);
void ReadInConfig(char *filename);   // Read parameters into global variable ["ConfigIS"]

// Will return RSU_Congif: if New=1, Only consider the exsitng phases, no missing phases.
RSU_Config ReadInConfig(char *filename,int New);

void RSUConfig2ConfigFile(char *filename,int *PhaseInfo,int Num);
void RSUConfig2ConfigFile(char *filename,int *PhaseInfo,int Num,RSU_Config configIS);




