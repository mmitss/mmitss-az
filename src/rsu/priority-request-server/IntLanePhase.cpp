#include "IntLanePhase.h"
#include <iostream>
#include <string>
using namespace std;
#define DEFAULT_ARRAY_SIZE 80

int IntLanePhase::ReadLanePhaseMap(char*filename)
{
	memset(iInLaneOutLanePhase, 0, sizeof(iInLaneOutLanePhase)); 
		
	ifstream lanePhase_file; 
    lanePhase_file.open(filename);

	    if (!lanePhase_file)
    {
        cout<<"Error: Open file "<<filename<<endl;
        return -1;
    }

	string lineread;
	char token_char [DEFAULT_ARRAY_SIZE];
	char temp_char  [DEFAULT_ARRAY_SIZE];
	
    for(int i=0;i<4;i++)
	{
		getline(lanePhase_file, lineread); // Read line by line
		sscanf(lineread.c_str(), "%s",token_char);
		//token.assign(token_char);
	    if(strcmp(token_char,"IntersectionID")==0)
		{
			sscanf(lineread.c_str(), "%*s %s",temp_char);
			iIntersectionID=atoi(temp_char);
		}
		else if(strcmp(token_char,"No_Approach")==0)
		{
			sscanf(lineread.c_str(), "%*s %s",temp_char);
			iTotalApproaches=atoi(temp_char);
		}
		else if(strcmp(token_char,"No_Phase")==0)
		{
			sscanf(lineread.c_str(), "%*s %s",temp_char);
			iTotalPhases=atoi(temp_char);
		}
		else if(strcmp(token_char,"No_Ingress")==0)
		{
			sscanf(lineread.c_str(), "%*s %s",temp_char);
			iTotalIngress=atoi(temp_char);
		}
		
	}
//end 
	for (int i=0 ; i<iTotalIngress; i++)
	{
		getline(lanePhase_file, lineread); // Read line by line
		sscanf(lineread.c_str(), "%s",token_char);
		if(strcmp(token_char,"Approach")==0)
		{   
			sscanf(lineread.c_str(), "%*s %s",temp_char);
			iApproach=atoi(temp_char);
			getline(lanePhase_file, lineread);
			sscanf(lineread.c_str(), "%s",token_char);
			if (strcmp(token_char,"No_Lane")==0)
			{	
				sscanf(lineread.c_str(), "%*s %s",temp_char);
				iNoRow=atoi(temp_char);
				for (int i=0;i<iNoRow;i++)
				{
					getline(lanePhase_file, lineread);
					sscanf(lineread.c_str(), "%s",token_char);
					iInLane=token_char[2]-48;
					sscanf(lineread.c_str(), "%*s %s",token_char);
					iOutApproach=token_char[0]-48;
					iOutLane=token_char[2]-48;
					sscanf(lineread.c_str(), "%*s %*s %s", temp_char);
					iPhase=atoi(temp_char); 
					iInLaneOutLanePhase[iApproach][iInLane][iOutApproach][iOutLane]=iPhase;
				}
			} //if (strcmp(token_char,"No_Lane")==0)
		}

		getline(lanePhase_file, lineread); // Read line by line
		sscanf(lineread.c_str(), "%s",token_char);
	}//for (int i=0 ; i<iTotalIngress; i++)
	//The end line
	getline(lanePhase_file, lineread); // Read line by line
	sscanf(lineread.c_str(), "%s",token_char);
	if(strcmp(token_char,"end_map")==0)
	{
		cout<<"Read Lane Phase Mapping file successfully!"<<endl;
	}
}
