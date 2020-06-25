/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   

/*  GetInfo.h 
*  Created by Mehdi Zamanipour
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.
*
*  Revision History:
* 
*  
*/
#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <istream>
#include <math.h>

#include "Signal.h"


using namespace std;

#define NumPhases 8

//---Store the file name of the Config file.
//--- For Example: ConfigInfo_Hastings.txt, which has the minGrn and maxGrn etc.
extern char ConfigInfo[256]	   ;
extern char ConfigFile[256];
extern int CombinedPhase[8];
extern char IPInfo[64]			   ;
extern char rsuid_filename[64] ;
extern string RSUID;
extern char INTip[128];// = "150.135.152.23";

extern char INTport[16];
extern char tmp_log[256];
extern int outputlog(char *output);

void get_rsu_id();
void get_configfile();
void get_ip_address();
