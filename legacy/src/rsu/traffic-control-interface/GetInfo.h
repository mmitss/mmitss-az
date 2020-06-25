/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   



/* GetInfo.h
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
extern char INTip[64];// = "150.135.152.23";
extern char INTport[8];
extern char tmp_log[256];

extern int outputlog(char *output);

void get_rsu_id();
void get_configfile();
void get_ip_address();





void get_rsu_id() //DJ add 06/24/2011
{
    fstream fs;

    fs.open(rsuid_filename);

    char temp[128];

    getline(fs,RSUID);

    if(RSUID.size()!=0)
    {
        //std::cout<< "Current Vehicle ID:" << RSUID <<std::endl;
        sprintf(temp,"Current RSU ID %s\n",RSUID.c_str());
        cout<<temp<<endl;
        outputlog(temp);
    }
    else
    {
        sprintf(temp,"Reading RSU ID %s problem",rsuid_filename);
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }

    fs.close();
}

void get_configfile()
{
	char tmp_log[256];
    fstream fs;
    fstream fs_phase; //*** Read in all phases in order to find the combined phase information.***//
    fs.open(ConfigInfo);

    string lineread;	getline(fs,lineread);

    if(lineread.size()!=0)
    {
        //std::cout<< "Current Vehicle ID:" << RSUID <<std::endl;
        sprintf(ConfigFile,"%s",lineread.c_str());
        cout<<ConfigFile<<endl; outputlog(ConfigFile);

        int phase_num;

        fs_phase.open(ConfigFile);

        getline(fs_phase,lineread); //*** Read the first line to get the number of all phases.
        sscanf(lineread.c_str(),"%*s %d ",&phase_num);

        getline(fs_phase,lineread); //*** Read the second line of the combined phase into array CombinedPhase[8]
        //*** If the phase exsits, the value is not 0; if not exists, the default value is '0'.
        sscanf(lineread.c_str(),"%*s %d %d %d %d %d %d %d %d",
            &CombinedPhase[0],&CombinedPhase[1],&CombinedPhase[2],&CombinedPhase[3],
            &CombinedPhase[4],&CombinedPhase[5],&CombinedPhase[6],&CombinedPhase[7]);

        fs_phase.close();

    }
    else
    {
        sprintf(tmp_log,"Reading configure file %s problem",ConfigInfo);
        cout<<tmp_log<<endl; outputlog(tmp_log);
        exit(0);
    }

    outputlog("\n");

    fs.close();
}

void get_ip_address()
{
	  char tmp_log[256];
    fstream fs;	fs.open(IPInfo);

    string lineread; 	getline(fs,lineread);
    outputlog("Controller IP Address is:\t");
    if(lineread.size()!=0)
    {
        sprintf(INTip,"%s",lineread.c_str());
        cout<<INTip<<endl;		outputlog(INTip);
    }
    else
    {
        sprintf(tmp_log,"Reading IPinfo file %s problem",IPInfo);
        cout<<tmp_log<<endl;		outputlog(tmp_log);
        exit(0);
    }
    getline(fs,lineread);
    if(lineread.size()!=0)
    {
        sprintf(INTport,"%s",lineread.c_str());
        cout<<INTport<<endl;		outputlog(INTip);
    }

    outputlog("\n");

    fs.close();
}
