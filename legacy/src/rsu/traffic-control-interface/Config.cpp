/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   



/* Config.cpp
*  Created by :Jun Ding
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.

*/

//#include "stdafx.h"
#include "Config.h"


void PrintRSUConfig()
{
    for(int i=0;i<2;i++)
    {
        cout<<"In Ring "<<(i+1)<<" The MissPhase is: "<<ConfigIS.MissPhase[i]+1<<"  MP_Relate : "<<ConfigIS.MP_Relate[i]+1<<"  Ring "<<ConfigIS.MP_Ring[i]<<endl;
    }
    

    cout<<"Ring 1 number are: "<<ConfigIS.Ring1No<<"  Details: ";
    PrintArray<int>(ConfigIS.Phase_Seq_R1,ConfigIS.Ring1No);
    cout<<"Ring 2 number are: "<<ConfigIS.Ring2No<<"  Details: ";
    PrintArray<int>(ConfigIS.Phase_Seq_R2,ConfigIS.Ring2No);

    cout<<"Yellow:  "; PrintArray<double>(ConfigIS.Yellow,8);
    cout<<"Red:     "; PrintArray<double>(ConfigIS.Red,8);
    cout<<"Gmin:    "; PrintArray<double>(ConfigIS.Gmin,8);
    cout<<"Gmax:    "; PrintArray<double>(ConfigIS.Gmax,8);
}

void PrintRSUConfig(RSU_Config configIS)
{
        //cout<<"The MissPhase is: "<<configIS.MissPhase+1<<"  MP_Relate : "<<configIS.MP_Relate+1<<"  Ring "<<configIS.MP_Ring<<endl;
    for(int i=0;i<2;i++)
    {
        cout<<"In Ring "<<(i+1)<<" The MissPhase is: "<<configIS.MissPhase[i]+1<<"  MP_Relate : "<<configIS.MP_Relate[i]+1<<"  Ring "<<configIS.MP_Ring[i]<<endl;
    }
    cout<<"Ring 1 number are: "<<configIS.Ring1No<<"  Details: ";
    PrintArray<int>(configIS.Phase_Seq_R1,configIS.Ring1No);
    cout<<"Ring 2 number are: "<<configIS.Ring2No<<"  Details: ";
    PrintArray<int>(configIS.Phase_Seq_R2,configIS.Ring2No);

    cout<<"Yellow:  "; PrintArray<double>(configIS.Yellow,8);
    cout<<"Red:     "; PrintArray<double>(configIS.Red,8);
    cout<<"Gmin:    "; PrintArray<double>(configIS.Gmin,8);
    cout<<"Gmax:    "; PrintArray<double>(configIS.Gmax,8);

}

void PrintRSUConfig2File(RSU_Config configIS,char *filename)
{
    fstream fs_Fileout;
    fs_Fileout.open(filename,ios::out);

    for(int i=0;i<2;i++)
    {
        fs_Fileout<<"In Ring "<<(i+1)<<" The MissPhase is: "<<configIS.MissPhase[i]+1<<"  MP_Relate : "<<configIS.MP_Relate[i]+1<<"  Ring "<<configIS.MP_Ring[i]<<endl;
    }

    fs_Fileout<<"Ring 1 number are: "<<configIS.Ring1No<<"  Details: ";

    //PrintArray<int>(configIS.Phase_Seq_R1,configIS.Ring1No);
    for(int i=0;i<configIS.Ring1No;i++)
    {
        fs_Fileout<<configIS.Phase_Seq_R1[i]<<"\t";
    }
    fs_Fileout<<endl;

    
    fs_Fileout<<"Ring 2 number are: "<<configIS.Ring2No<<"  Details: ";

    //PrintArray<int>(configIS.Phase_Seq_R2,configIS.Ring2No);
    for(int i=0;i<configIS.Ring2No;i++)
    {
        fs_Fileout<<configIS.Phase_Seq_R2[i]<<"\t";
    }
    fs_Fileout<<endl;


    

    fs_Fileout<<"Yellow:  "; 
    //PrintArray<double>(configIS.Yellow,8);
    for(int i=0;i<8;i++)
    {
        fs_Fileout<<configIS.Yellow[i]<<"\t";
    }
    fs_Fileout<<endl;

    fs_Fileout<<"Red:     "; 
    //PrintArray<double>(configIS.Red,8);
    for(int i=0;i<8;i++)
    {
        fs_Fileout<<configIS.Red[i]<<"\t";
    }
    fs_Fileout<<endl;

    fs_Fileout<<"Gmin:    "; 
    //PrintArray<double>(configIS.Gmin,8);
    for(int i=0;i<8;i++)
    {
        fs_Fileout<<configIS.Gmin[i]<<"\t";
    }
    fs_Fileout<<endl;

    fs_Fileout<<"Gmax:    "; 
    //PrintArray<double>(configIS.Gmax,8);
    for(int i=0;i<8;i++)
    {
        fs_Fileout<<configIS.Gmax[i]<<"\t";
    }
    fs_Fileout<<endl;

    fs_Fileout.close();


}

void RSUConfig2ConfigFile(char *filename,int *PhaseInfo,int Num)
{
    //---------------PhaseInfo has the information from the requests.txt, including the requested phases.
    //---------------The Num is the total number of the requested phases, No Repeated case(DOES NOT MATTER).
    //---------------**IMportant**We should have the assumption that the phase in PhaseInfo should be in the ConfigIS.Phase_Seq_R1&2
    //---- First to sort the request phase information--------****Probably not necessary****
    
    //bubbleSort(PhaseInfo,Num);   // the element in PhaseInfo should be the truephase, not truephase-1

    int TotalPhase[8]={0};

    int Phase[8];
    double Yellow[8],Red[8],Gmin[8],Gmax[8];


    for(int i=0;i<Num;i++)
    {
        int tempPhase=PhaseInfo[i]-1;

        if(FindIndexArray(ConfigIS.Phase_Seq_R1,ConfigIS.Ring1No,tempPhase)!=-1
         ||FindIndexArray(ConfigIS.Phase_Seq_R2,ConfigIS.Ring2No,tempPhase%4)!=-1)

        TotalPhase[tempPhase]=1;
    }

    int TotalNum=0;

    for(int i=0;i<8;i++)
    {
        if(TotalPhase[i]==1)
        {
            TotalNum++;
            Yellow[i]= ConfigIS.Yellow[i];
            Red[i]   = ConfigIS.Red[i];
            Gmin[i]  = ConfigIS.Gmin[i];
            Gmax[i]  = ConfigIS.Gmax[i];
        }
        else
        {
            Yellow[i]= 0;
            Red[i]   = 0;
            Gmin[i]  = 0;
            Gmax[i]  = 0;
        }             
    }



    fstream fs_Fileout;

    fs_Fileout.open(filename,ios::out);

    fs_Fileout<<"Phase_Num "<< TotalNum <<endl;
    fs_Fileout<<"Phase_Seq";

    for(int i=0;i<8;i++)
    {
        if(TotalPhase[i]==1)
            fs_Fileout<<" "<<(i+1);
        else
            fs_Fileout<<" "<<0;         
    }
    fs_Fileout<<endl;
    
    fs_Fileout<<"Gmin";
    for(int i=0;i<8;i++)
        fs_Fileout<<"\t"<<Gmin[i];
    fs_Fileout<<endl;

    fs_Fileout<<"Yellow";
    for(int i=0;i<8;i++)
        fs_Fileout<<"\t"<<Yellow[i];
    fs_Fileout<<endl;

    fs_Fileout<<"Red";
    for(int i=0;i<8;i++)
        fs_Fileout<<"\t"<<Red[i];
    fs_Fileout<<endl;

    fs_Fileout<<"Gmax";
    for(int i=0;i<8;i++)
        fs_Fileout<<"\t"<<Gmax[i];
    fs_Fileout<<endl;

    fs_Fileout.close();



}


void RSUConfig2ConfigFile(char *filename,int *PhaseInfo,int Num,RSU_Config configIS)
{
    //---------------PhaseInfo has the information from the requests.txt, including the requested phases.
    //---------------The Num is the total number of the requested phases, No Repeated case(DOES NOT MATTER).
    //---------------**IMportant**We should have the assumption that the phase in PhaseInfo should be in the ConfigIS.Phase_Seq_R1&2
    //---- First to sort the request phase information--------****Probably not necessary****
    
    //bubbleSort(PhaseInfo,Num);   // the element in PhaseInfo should be the truephase, not truephase-1

    int TotalPhase[8]={0};

    int Phase[8];
    double Yellow[8],Red[8],Gmin[8],Gmax[8];


    for(int i=0;i<Num;i++)
    {
        int tempPhase=PhaseInfo[i]-1;

        if(FindIndexArray(configIS.Phase_Seq_R1,configIS.Ring1No,tempPhase)!=-1
         ||FindIndexArray(configIS.Phase_Seq_R2,configIS.Ring2No,tempPhase%4)!=-1)

        TotalPhase[tempPhase]=1;
    }

    int TotalNum=0;

    for(int i=0;i<8;i++)
    {
        if(TotalPhase[i]==1)
        {
            TotalNum++;
            Yellow[i]= configIS.Yellow[i];
            Red[i]   = configIS.Red[i];
            Gmin[i]  = configIS.Gmin[i];
            Gmax[i]  = configIS.Gmax[i];
        }
        else
        {
            Yellow[i]= 0;
            Red[i]   = 0;
            Gmin[i]  = 0;
            Gmax[i]  = 0;
        }             
    }



    fstream fs_Fileout;

    fs_Fileout.open(filename,ios::out);

    fs_Fileout<<"Phase_Num "<< TotalNum <<endl;
    fs_Fileout<<"Phase_Seq";

    for(int i=0;i<8;i++)
    {
        if(TotalPhase[i]==1)
            fs_Fileout<<" "<<(i+1);
        else
            fs_Fileout<<" "<<0;         
    }
    fs_Fileout<<endl;
    
    fs_Fileout<<"Gmin";
    for(int i=0;i<8;i++)
        fs_Fileout<<"\t"<<Gmin[i];
    fs_Fileout<<endl;

    fs_Fileout<<"Yellow";
    for(int i=0;i<8;i++)
        fs_Fileout<<"\t"<<Yellow[i];
    fs_Fileout<<endl;

    fs_Fileout<<"Red";
    for(int i=0;i<8;i++)
        fs_Fileout<<"\t"<<Red[i];
    fs_Fileout<<endl;

    fs_Fileout<<"Gmax";
    for(int i=0;i<8;i++)
        fs_Fileout<<"\t"<<Gmax[i];
    fs_Fileout<<endl;

    fs_Fileout.close();



}



void ReadInConfig(char *filename)
{
    //-------------------------------------------------------------------------------//
    //* Important * Read the configuration: Find out the Missing phase and MP_Relate
    fstream FileRead;
    FileRead.open(filename,ios::in);
    //FileRead.open("ConfigurationInfo1268.txt",ios::in);

    if(!FileRead)
    {
        cerr<<"Unable to open file!"<<endl;
        exit(1);
    }

    for(int i=0;i<2;i++)
    {
        ConfigIS.MissPhase[i]=-1;
        ConfigIS.MP_Relate[i]=-1;
        ConfigIS.MP_Ring[i]=-1;
    }

    int PhaseNo;
    int PhaseSeq[8];
    char TempStr[16];
    string lineread;
    vector<int> P11,P12,P21,P22;

    //----------------- Read in the parameters---------------
    while(!FileRead.eof())
    {
        getline(FileRead,lineread);

        if (lineread.size()!=0)
        {
            sscanf(lineread.c_str(),"%s",TempStr);
            if(strcmp(TempStr,"Phase_Num")==0)
            {					
                sscanf(lineread.c_str(),"%*s %d ",&PhaseNo);
            }
            else if(strcmp(TempStr,"Phase_Seq")==0)
            {
                sscanf(lineread.c_str(),"%*s %d %d %d %d %d %d %d %d",
                    &PhaseSeq[0],&PhaseSeq[1],&PhaseSeq[2],&PhaseSeq[3],
                    &PhaseSeq[4],&PhaseSeq[5],&PhaseSeq[6],&PhaseSeq[7]);
            }
            else if(strcmp(TempStr,"Yellow")==0)
            {
                sscanf(lineread.c_str(),"%*s %lf %lf %lf %lf %lf %lf %lf %lf",
                    &ConfigIS.Yellow[0],&ConfigIS.Yellow[1],
                    &ConfigIS.Yellow[2],&ConfigIS.Yellow[3],
                    &ConfigIS.Yellow[4],&ConfigIS.Yellow[5],
                    &ConfigIS.Yellow[6],&ConfigIS.Yellow[7]);
            }
            else if(strcmp(TempStr,"Red")==0)
            {
                sscanf(lineread.c_str(),"%*s %lf %lf %lf %lf %lf %lf %lf %lf",
                    &ConfigIS.Red[0],&ConfigIS.Red[1],
                    &ConfigIS.Red[2],&ConfigIS.Red[3],
                    &ConfigIS.Red[4],&ConfigIS.Red[5],
                    &ConfigIS.Red[6],&ConfigIS.Red[7]);                
            }
            else if(strcmp(TempStr,"Gmin")==0)
            {
                sscanf(lineread.c_str(),"%*s %lf %lf %lf %lf %lf %lf %lf %lf",
                    &ConfigIS.Gmin[0],&ConfigIS.Gmin[1],
                    &ConfigIS.Gmin[2],&ConfigIS.Gmin[3],
                    &ConfigIS.Gmin[4],&ConfigIS.Gmin[5],
                    &ConfigIS.Gmin[6],&ConfigIS.Gmin[7]);                
            }
            else if(strcmp(TempStr,"Gmax")==0)
            {
                sscanf(lineread.c_str(),"%*s %lf %lf %lf %lf %lf %lf %lf %lf",
                    &ConfigIS.Gmax[0],&ConfigIS.Gmax[1],
                    &ConfigIS.Gmax[2],&ConfigIS.Gmax[3],
                    &ConfigIS.Gmax[4],&ConfigIS.Gmax[5],
                    &ConfigIS.Gmax[6],&ConfigIS.Gmax[7]);
            }

        }
    }
    FileRead.close();
    //-------------Handle the parameters for non-complete phases case----// 
    for(int i=0;i<8;i++)
    {
        if (PhaseSeq[i]>0)
        {
            switch (PhaseSeq[i])
            {
            case 1:
            case 2:
                P11.push_back(PhaseSeq[i]);
                break;
            case 3:
            case 4:
                P12.push_back(PhaseSeq[i]);
                break;
            case 5:
            case 6:
                P21.push_back(PhaseSeq[i]);
                break;
            case 7:
            case 8:
                P22.push_back(PhaseSeq[i]);
                break;
            }
        } 
    }
    //--------Here is different from the one within the GeneratePriReqMod--------//
    if (P11.size()==0)
    {
        P11.push_back(2);// If P11 is missing, put a dummy "6" which means "2" in it
        ConfigIS.MissPhase[0]=1;
        ConfigIS.MP_Relate[0]=5;
        ConfigIS.MP_Ring[0]=0;
    }
    if (P12.size()==0)
    {
        P12.push_back(4);// If P12 is missing, put a dummy "8" which means "4" in it
        ConfigIS.MissPhase[0]=3;
        ConfigIS.MP_Relate[0]=7;
        ConfigIS.MP_Ring[0]=0;
    }
    if (P21.size()==0)
    {
        P21.push_back(6);// If P21 is missing, put a dummy "2" which means "6" in it
        ConfigIS.MissPhase[1]=5;
        ConfigIS.MP_Relate[1]=1;
        ConfigIS.MP_Ring[1]=1;
    }
    if (P22.size()==0)
    {
        P22.push_back(8);// If P21 is missing, put a dummy "4" which means "8" in it
        ConfigIS.MissPhase[1]=7;
        ConfigIS.MP_Relate[1]=3;
        ConfigIS.MP_Ring[1]=1;
    }


    for(vector<int>::iterator it=P12.begin();it<P12.end();it++)
    {
        P11.push_back(*it);
    }
    for(vector<int>::iterator it=P22.begin();it<P22.end();it++)
    {
        P21.push_back(*it);
    }

    int R1No=P11.size(); 
    int R2No=P21.size();

    //int SP1=StartPhase[0]-1,SP2=StartPhase[1]-1;  // Real phase -1
    int *Phase_Seq_R1=new int[R1No];  // Real phase-1
    int i=0;
    for(vector<int>::iterator it=P11.begin();it<P11.end()&& i<R1No;it++,i++)
    {
        Phase_Seq_R1[i]=*it-1;
    }

    int *Phase_Seq_R2=new int[R2No];
    i=0;
    for(vector<int>::iterator it=P21.begin();it<P21.end()&& i<R2No;it++,i++)
    {
        Phase_Seq_R2[i]=*it-1;
    }

    ConfigIS.Ring1No=R1No;
    ConfigIS.Ring2No=R2No;

    ConfigIS.Phase_Seq_R1=new int[R1No];
    ConfigIS.Phase_Seq_R2=new int[R2No];

    //cout<<"MissPhase= "<<ConfigIS.MissPhase<<"  MP_Relate:= "<<ConfigIS.MP_Relate
    //	<<" Phase Num:="<<PhaseNo<<endl;

    //cout<<"Continue to Parsing...";
    //cin.get();


    for(int i=0;i<2;i++)
    {
        if(ConfigIS.MP_Relate[i]>=0)
        {
            //cout<<"HAVE Missed phase:"<<Config.MissPhase<<endl;
            ConfigIS.Yellow[ConfigIS.MissPhase[i]]=ConfigIS.Yellow[ConfigIS.MP_Relate[i]];
            ConfigIS.Red[ConfigIS.MissPhase[i]]=ConfigIS.Red[ConfigIS.MP_Relate[i]];
            ConfigIS.Gmin[ConfigIS.MissPhase[i]]=ConfigIS.Gmin[ConfigIS.MP_Relate[i]];
            ConfigIS.Gmax[ConfigIS.MissPhase[i]]=ConfigIS.Gmax[ConfigIS.MP_Relate[i]];
        }
    }

    for (int i=0;i<R1No;i++)
    {
        ConfigIS.Phase_Seq_R1[i]=Phase_Seq_R1[i];
    }
    for (int i=0;i<R2No;i++)
    {
        ConfigIS.Phase_Seq_R2[i]=Phase_Seq_R2[i]%4;
    }
}


    //-------------------------------------------------------------------------------//

//**** Read into a RSU_Config ConfigIS_EV from the Temp configuration file. ****//
//**** The difference is that the phase sequence will not have the missing phase, only have the exsiting phases, which will be used by
//**** reading new schedule.
RSU_Config ReadInConfig(char *filename,int New)
{
    //-------------------------------------------------------------------------------//
    RSU_Config configIS;
    cout<<"Read the configure file to a structure: "<<New<<endl;
    //* Important * Read the configuration: Find out the Missing phase and MP_Relate
    fstream FileRead;
    FileRead.open(filename,ios::in);
    //FileRead.open("ConfigurationInfo1268.txt",ios::in);

    if(!FileRead)
    {
        cerr<<"Unable to open file!"<<endl;
        exit(1);
    }

    for(int i=0;i<2;i++)
    {
        configIS.MissPhase[i]=-1;
        configIS.MP_Relate[i]=-1;
        configIS.MP_Ring[i]=-1;
    }

    int PhaseNo;
    int PhaseSeq[8];
    char TempStr[16];
    string lineread;
    vector<int> P11,P12,P21,P22;

    //----------------- Read in the parameters---------------
    while(!FileRead.eof())
    {
        getline(FileRead,lineread);

        if (lineread.size()!=0)
        {
            sscanf(lineread.c_str(),"%s",TempStr);
            if(strcmp(TempStr,"Phase_Num")==0)
            {					
                sscanf(lineread.c_str(),"%*s %d ",&PhaseNo);
            }
            else if(strcmp(TempStr,"Phase_Seq")==0)
            {
                sscanf(lineread.c_str(),"%*s %d %d %d %d %d %d %d %d",
                    &PhaseSeq[0],&PhaseSeq[1],&PhaseSeq[2],&PhaseSeq[3],
                    &PhaseSeq[4],&PhaseSeq[5],&PhaseSeq[6],&PhaseSeq[7]);
            }
            else if(strcmp(TempStr,"Yellow")==0)
            {
                sscanf(lineread.c_str(),"%*s %lf %lf %lf %lf %lf %lf %lf %lf",
                    &configIS.Yellow[0],&configIS.Yellow[1],
                    &configIS.Yellow[2],&configIS.Yellow[3],
                    &configIS.Yellow[4],&configIS.Yellow[5],
                    &configIS.Yellow[6],&configIS.Yellow[7]);
            }
            else if(strcmp(TempStr,"Red")==0)
            {
                sscanf(lineread.c_str(),"%*s %lf %lf %lf %lf %lf %lf %lf %lf",
                    &configIS.Red[0],&configIS.Red[1],
                    &configIS.Red[2],&configIS.Red[3],
                    &configIS.Red[4],&configIS.Red[5],
                    &configIS.Red[6],&configIS.Red[7]);                
            }
            else if(strcmp(TempStr,"Gmin")==0)
            {
                sscanf(lineread.c_str(),"%*s %lf %lf %lf %lf %lf %lf %lf %lf",
                    &configIS.Gmin[0],&configIS.Gmin[1],
                    &configIS.Gmin[2],&configIS.Gmin[3],
                    &configIS.Gmin[4],&configIS.Gmin[5],
                    &configIS.Gmin[6],&configIS.Gmin[7]);                
            }
            else if(strcmp(TempStr,"Gmax")==0)
            {
                sscanf(lineread.c_str(),"%*s %lf %lf %lf %lf %lf %lf %lf %lf",
                    &configIS.Gmax[0],&configIS.Gmax[1],
                    &configIS.Gmax[2],&configIS.Gmax[3],
                    &configIS.Gmax[4],&configIS.Gmax[5],
                    &configIS.Gmax[6],&configIS.Gmax[7]);
            }

        }
    }
    FileRead.close();
    //-------------Handle the parameters for non-complete phases case----// 
    for(int i=0;i<8;i++)
    {
        if (PhaseSeq[i]>0)
        {
            switch (PhaseSeq[i])
            {
            case 1:
            case 2:
                P11.push_back(PhaseSeq[i]);
                break;
            case 3:
            case 4:
                P12.push_back(PhaseSeq[i]);
                break;
            case 5:
            case 6:
                P21.push_back(PhaseSeq[i]);
                break;
            case 7:
            case 8:
                P22.push_back(PhaseSeq[i]);
                break;
            }
        } 
    }

    //--------Here is different from the one above--------//
    if (P11.size()==0)
    {
        if(New==0)
        {
            P11.push_back(2);
        }
        //P11.push_back(2);// If P11 is missing, put a dummy "6" which means "2" in it
        configIS.MissPhase[0]=1;
        configIS.MP_Relate[0]=5;
        configIS.MP_Ring[0]=0;
    }
    if (P12.size()==0)
    {
        if(New==0)
        {
            P12.push_back(4);
        }
        //P12.push_back(4);// If P12 is missing, put a dummy "8" which means "4" in it
        configIS.MissPhase[0]=3;
        configIS.MP_Relate[0]=7;
        configIS.MP_Ring[0]=0;
    }
    if (P21.size()==0)
    {
        if(New==0)
        {
            P21.push_back(6);
        }
        //P21.push_back(6);// If P21 is missing, put a dummy "2" which means "6" in it
        configIS.MissPhase[1]=5;
        configIS.MP_Relate[1]=1;
        configIS.MP_Ring[1]=1;
    }
    if (P22.size()==0)
    {
        if(New==0)
        {
            P22.push_back(8);
        }
        //P22.push_back(8);// If P21 is missing, put a dummy "4" which means "8" in it
        configIS.MissPhase[1]=7;
        configIS.MP_Relate[1]=3;
        configIS.MP_Ring[1]=1;
    }


    for(vector<int>::iterator it=P12.begin();it<P12.end();it++)
    {
        P11.push_back(*it);
    }
    for(vector<int>::iterator it=P22.begin();it<P22.end();it++)
    {
        P21.push_back(*it);
    }

    int R1No=P11.size(); 
    int R2No=P21.size();

    //int SP1=StartPhase[0]-1,SP2=StartPhase[1]-1;  // Real phase -1
    int *Phase_Seq_R1=new int[R1No];  // Real phase-1
    int i=0;
    for(vector<int>::iterator it=P11.begin();it<P11.end()&& i<R1No;it++,i++)
    {
        Phase_Seq_R1[i]=*it-1;
    }

    int *Phase_Seq_R2=new int[R2No];
    i=0;
    for(vector<int>::iterator it=P21.begin();it<P21.end()&& i<R2No;it++,i++)
    {
        Phase_Seq_R2[i]=*it-1;
    }

    configIS.Ring1No=R1No;
    configIS.Ring2No=R2No;

    configIS.Phase_Seq_R1=new int[R1No];
    configIS.Phase_Seq_R2=new int[R2No];

    //cout<<"MissPhase= "<<configIS.MissPhase<<"  MP_Relate:= "<<configIS.MP_Relate
    //	<<" Phase Num:="<<PhaseNo<<endl;

    //cout<<"Continue to Parsing...";
    //cin.get();


    for(int i=0;i<2;i++)
    {
        if(configIS.MP_Relate[i]>=0)
        {
            //cout<<"HAVE Missed phase:"<<configIS.MissPhase<<endl;
            configIS.Yellow[configIS.MissPhase[i]]=configIS.Yellow[configIS.MP_Relate[i]];
            configIS.Red[configIS.MissPhase[i]]=configIS.Red[configIS.MP_Relate[i]];
            configIS.Gmin[configIS.MissPhase[i]]=configIS.Gmin[configIS.MP_Relate[i]];
            configIS.Gmax[configIS.MissPhase[i]]=configIS.Gmax[configIS.MP_Relate[i]];
        }

    }


    for (int i=0;i<R1No;i++)
    {
        configIS.Phase_Seq_R1[i]=Phase_Seq_R1[i];
    }
    for (int i=0;i<R2No;i++)
    {
        configIS.Phase_Seq_R2[i]=Phase_Seq_R2[i]%4;
    }

    return configIS;
}
