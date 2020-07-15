/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   

/* GetInfo.cpp 
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

#include "GetInfo.h"

void get_rsu_id() 
{
    fstream fs;

    fs.open(rsuid_filename);

    char temp[128];

    getline(fs,RSUID);

    if(RSUID.size()!=0)
    {
        //std::cout<< "Current Vehicle ID:" << RSUID <<std::endl;
        sprintf(temp,"Current RSU ID %s\n",RSUID.c_str());
        outputlog(temp);
    }
    else
    {
        sprintf(temp,"Reading RSU ID %s problem",rsuid_filename);
        outputlog(temp);
        exit(0);
    }

    fs.close();
}

void get_configfile()
{
	char tmp_log[256];
	int phase_num;
    fstream fs;
    fstream fs_phase; //*** Read in all phases in order to find the combined phase information.***//
    fs.open(ConfigInfo);
    string lineread;	getline(fs,lineread);
    if(lineread.size()!=0)
    {
        sprintf(ConfigFile,"%s",lineread.c_str());
        outputlog(ConfigFile);    
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
        outputlog(tmp_log);
        exit(0);
    }
    outputlog("\n");
    fs.close();
}

void get_ip_address()
{
	char tmp_log[256];
    fstream fs;	fs.open(IPInfo);
    string lineread; 	
    getline(fs,lineread);
    outputlog("Controller IP Address is:\t");
    if(lineread.size()!=0)
    {
        sprintf(INTip,"%s",lineread.c_str());
       	outputlog(INTip);
    }
    else
    {
        sprintf(tmp_log,"Reading IPinfo file %s problem",IPInfo);
       	outputlog(tmp_log);
        exit(0);
    }
    outputlog("\n");
	getline(fs,lineread);
    outputlog("Controller Port is:\t");
    if(lineread.size()!=0)
    {
        sprintf(INTport,"%s",lineread.c_str());
        outputlog(INTport);
    }
    else
    {
        sprintf(tmp_log,"Reading IPinfo file %s problem",INTport);
       	outputlog(tmp_log);
        exit(0);
    }
    outputlog("\n");
    fs.close();
}
