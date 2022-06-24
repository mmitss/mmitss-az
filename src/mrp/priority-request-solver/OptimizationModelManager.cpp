/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  OptimizationModelManager.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script contains method to create mod file for different types of priority requests.
*/

#include "OptimizationModelManager.h"

OptimizationModelManager::OptimizationModelManager(double Flexibility_Weight)
{
    FlexibilityWeight = Flexibility_Weight;
}

/*
    - Method for generating Mod File for transit and truck
*/
void OptimizationModelManager::generateModFile(int noOfPhase, vector<int> PhaseNumber, vector<int> P11, vector<int> P12, vector<int> P21, vector<int> P22)
{
    ofstream FileMod;
    FileMod.open("/nojournal/bin/OptimizationModel.mod", ios::out);
    // =================Defining the sets ======================

    if (P11.size() == 1)
        FileMod << "set P11:={" << P11[0] << "}; \n";

    else if (P11.size() == 2)
        FileMod << "set P11:={" << P11[0] << "," << P11[1] << "};  \n";

    if (P12.size() == 1)
        FileMod << "set P12:={" << P12[0] << "}; \n";

    else if (P12.size() == 2)
        FileMod << "set P12:={" << P12[0] << "," << P12[1] << "};  \n";

    if (P21.size() == 1)
        FileMod << "set P21:={" << P21[0] << "}; \n";

    else if (P21.size() == 2)
        FileMod << "set P21:={" << P21[0] << "," << P21[1] << "};  \n";

    if (P22.size() == 1)
        FileMod << "set P22:={" << P22[0] << "}; \n";

    else if (P22.size() == 2)
        FileMod << "set P22:={" << P22[0] << "," << P22[1] << "};  \n";

    FileMod << "set P:={";
    for (int i = 0; i < noOfPhase; i++)
    {
        if (i != noOfPhase - 1)
            FileMod << " " << PhaseNumber[i] << ",";
        else
            FileMod << " " << PhaseNumber[i];
    }
    FileMod << "};\n";

    FileMod << "set K  := {1..3};\n"; //Only two cycles ahead are considered in the model. But we should count the third cycle in the cycle set. Because, assume we are in the midle of cycle one. Therefore, we have cycle 1, 2 and half of cycle 3.
    FileMod << "set J  := {1..15};\n"; //Priority Type for all the priority requests in the list
    FileMod << "set P2 := {1..8};\n";
    FileMod << "set T  := {1..10};\n"; //The model can serve at most 10 different types of vehicle simultaneously among which EVs are 1, Transits are 2, Trucks are 3, DIlemmaZoneRequests are 4, CoordinationRequests are 5, and EVSplitRequests are 4, 
    FileMod << "set E:={1,2};\n";
    FileMod << "\n";

    //========================Parameters=========================
    FileMod << "param y    {p in P}, >=0,default 0;\n";
    FileMod << "param red  {p in P}, >=0,default 0;\n";
    FileMod << "param gmin {p in P}, >=0,default 0;\n";
    FileMod << "param gmax {p in P}, >=0,default 0;\n";
    FileMod << "param init1,default 0;\n";
    FileMod << "param init2,default 0;\n";
    FileMod << "param Grn1, default 0;\n";
    FileMod << "param Grn2, default 0;\n";
    FileMod << "param SP1,  integer,default 0;\n";
    FileMod << "param SP2,  integer,default 0;\n";
    FileMod << "param EarlyReturnValue1, default 0;\n";
    FileMod << "param EarlyReturnValue2, default 0;\n";
    FileMod << "param CoordinatedPhase1, default 2;\n";
    FileMod << "param CoordinatedPhase2, default 6;\n";
    FileMod << "param M:=9999,integer;\n";
    FileMod << "param Rl{p in P, j in J}, >=0,  default 0;\n";
    FileMod << "param Ru{p in P, j in J}, >=0,  default 0;\n";

    FileMod << "param PrioType { t in T}, >=0, default 0;  \n";
    FileMod << "param PrioWeight { t in T}, >=0, default 0;  \n";
    FileMod << "param priorityType{j in J}, >=0, default 0;  \n";
    FileMod << "param priorityTypeWeight{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeight[t] else 0);  \n";
    FileMod << "param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);\n";
    FileMod << "param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);\n";
    FileMod << "param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);\n";
    FileMod << "param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);\n";
    FileMod << "param ReqNo:=sum{p in P,j in J} active_pj[p,j];\n";
    FileMod << "param gmaxPerRng{p in P,k in K}, := (if ((k==1 and p==CoordinatedPhase1)) then (gmax[p]+EarlyReturnValue1) else (if ((k==1 and p==CoordinatedPhase2)) then (gmax[p]+EarlyReturnValue2) else	gmax[p]));\n";
    FileMod << "\n";

    // ==================== Variables =======================
    FileMod << "var t{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var g{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var v{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var d{p in P,j in J}, >=0;\n";
    FileMod << "var theta{p in P,j in J}, binary;\n";
    FileMod << "var ttheta{p in P,j in J}, >=0;\n";
    FileMod << "var PriorityDelay;\n";
    FileMod << "var Flexibility;\n";

    FileMod << "\n";

    // ===================Constraints==============================

    FileMod << "s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  \n";
    FileMod << "s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  \n";
    FileMod << "s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  \n";
    // # constraints in the same cycle in same ring-barrier group for cycle,k=1
    FileMod << "s.t. PrecedenceConstraint_From_P11_To_P11_Cycle1{e in E,p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P12_To_P12_Cycle1{e in E,p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P21_To_P21_Cycle1{e in E,p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P22_To_P22_Cycle1{e in E,p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    // # constraints in the same cycle in connecting
    FileMod << "s.t. PrecedenceConstraint_From_P11_To_P12_Cycle1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P11_To_P22_Cycle1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P21_To_P12_Cycle1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P21_To_P22_Cycle1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
    // #================ END of cycle 1======================#

    // # constraints in the same cycle in same ring-barrier group for cycle,k>1
    FileMod << "s.t. PrecedenceConstraint_From_P11_To_P11_Cycle2and3{e in E,p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P12_To_P12_Cycle2and3{e in E,p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P21_To_P21_Cycle2and3{e in E,p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P22_To_P22_Cycle2and3{e in E,p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";

    // # constraints in the same cycle in connecting
    FileMod << "s.t. PrecedenceConstraint_From_P11_To_P12_Cycle2and3{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P11_To_P22_Cycle2and3{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P21_To_P12_Cycle2and3{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P21_To_P22_Cycle2and3{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";

    // # constraints in connecting in different cycles
    FileMod << "s.t. PrecedenceConstraint_From_P12_To_P11_Cycle2and3{e in E,p in P11, k in K: (card(P11)+p+1)=4 and k>1 }:    t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P22_To_P11_Cycle2and3{e in E,p in P11, k in K: (card(P11)+p+1+4)=8 and k>1 }:  t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P12_To_P21_Cycle2and3{e in E,p in P21, k in K: (card(P21)+p+1-4)=4 and k>1 }:  t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";
    FileMod << "s.t. PrecedenceConstraint_From_P22_To_P21_Cycle2and3{e in E,p in P21, k in K: (card(P21)+p+1)=8 and k>1 }:    t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";

    FileMod << "s.t. PhaseDuration{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];\n";
    FileMod << "s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";
    FileMod << "s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";

    FileMod << "s.t. PriorityConstraint1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; \n";
    FileMod << "s.t. PriorityConstraint2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n";
    FileMod << "s.t. PriorityConstraint3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];\n";
    FileMod << "s.t. PriorityConstraint4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);\n";
    FileMod << "s.t. PriorityConstraint5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];\n";
    FileMod << "s.t. PriorityConstraint6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];\n";
    FileMod << "s.t. PriorityConstraint7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];\n";
    FileMod << "s.t. PriorityConstraint8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; \n";
    FileMod << "s.t. PriorityConstraint9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";

    FileMod << "s.t. FlexibilityConstraint: Flexibility= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];\n ";
    FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeight[j,tt]*active_pj[p,j]*d[p,j] ) )  - " << FlexibilityWeight << "*Flexibility; \n "; // The coeficient to Flexibility should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points

    FileMod << "  minimize delay: PriorityDelay;     \n";
    //=============================Writing the Optimal Output into the /nojournal/bin/OptimizationResults.txt file ==================================
    FileMod << "  \n";
    FileMod << "solve;  \n";
    FileMod << "  \n";
    FileMod << "printf \" \" > \"/nojournal/bin/OptimizationResults.txt\";  \n";
    FileMod << "printf \"%3d  %3d \\n \",SP1, SP2 >>\"/nojournal/bin/OptimizationResults.txt\";  \n";
    FileMod << "printf \"%5.2f  %5.2f %5.2f  %5.2f \\n \",init1, init2,Grn1,Grn2 >>\"/nojournal/bin/OptimizationResults.txt\";  \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,1] else 0  >>\"/nojournal/bin/OptimizationResults.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/OptimizationResults.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,2] else 0  >>\"/nojournal/bin/OptimizationResults.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/OptimizationResults.txt\";\n";
    FileMod << " } \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,1] else 0  >>\"/nojournal/bin/OptimizationResults.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/OptimizationResults.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,2] else 0  >>\"/nojournal/bin/OptimizationResults.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/OptimizationResults.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "printf \"%3d \\n \", ReqNo >>\"/nojournal/bin/OptimizationResults.txt\";  \n";
    FileMod << "  \n";
    FileMod << "for {p in P,j in J : Rl[p,j]>0}  \n";
    FileMod << " {  \n";
    FileMod << "   printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", p, Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>\"/nojournal/bin/OptimizationResults.txt\";\n"; // the  term " coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1))" is used to know the request is served in which cycle. For example, aasume there is a request for phase 4. If the request is served in firsr cycle, the term will be 4, the second cycle, the term will be 14 and the third cycle, the term will be 24
    FileMod << " } \n";

    FileMod << "printf \"%5.2f \\n \", PriorityDelay + 0.01*Flexibility>>\"/nojournal/bin/OptimizationResults.txt\"; \n";

    FileMod << "printf \"%5.2f \\n \", Flexibility >>\"/nojournal/bin/OptimizationResults.txt\"; ";

    //------------- End of Print the Main body of mode----------------
    FileMod << "end;";
    FileMod.close();
}


/*
    -Dynamic mod file for EV. If there is multiple EV and they are requesting for different phase 
*/
void OptimizationModelManager::generateEVModFile(vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan_EV, vector<int> EV_P11, vector<int> EV_P12, vector<int> EV_P21, vector<int> EV_P22)
{
    int phasePosition{};
    ofstream FileMod;

    FileMod.open("/nojournal/bin/OptimizationModel_EV.mod", ios::out);
    // =================Defining the sets ======================

    if (EV_P11.size() == 1)
        FileMod << "set P11:={" << EV_P11[0] << "}; \n";

    else if (EV_P11.size() == 2)
        FileMod << "set P11:={" << EV_P11[0] << "," << EV_P11[1] << "};  \n";

    if (EV_P12.size() == 1)
        FileMod << "set P12:={" << EV_P12[0] << "}; \n";

    else if (EV_P12.size() == 2)
        FileMod << "set P12:={" << EV_P12[0] << "," << EV_P12[1] << "};  \n";

    if (EV_P21.size() == 1)
        FileMod << "set P21:={" << EV_P21[0] << "}; \n";

    else if (EV_P21.size() == 2)
        FileMod << "set P21:={" << EV_P21[0] << "," << EV_P21[1] << "};  \n";

    if (EV_P22.size() == 1)
        FileMod << "set P22:={" << EV_P22[0] << "}; \n";

    else if (EV_P22.size() == 2)
        FileMod << "set P22:={" << EV_P22[0] << "," << EV_P22[1] << "};  \n";

    FileMod << "set P:={";
    
    for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
    {
        if (i == trafficSignalPlan_EV.size() - 1)
            FileMod << trafficSignalPlan_EV[i].phaseNumber;
        
        else
            FileMod << trafficSignalPlan_EV[i].phaseNumber << "," << " ";
    }

    FileMod << "};\n";

    FileMod << "set K  := {1..3};\n"; //Only two cycles ahead are considered in the model. But we should count the third cycle in the cycle set. Because, assume we are in the midle of cycle one. Therefore, we have cycle 1, 2 and half of cycle 3.
    FileMod << "set J  := {1..15};\n"; //Priority Type for all the priority requests in the list
    FileMod << "set P2 := {1..8};\n";
    FileMod << "set T  := {1..10};\n"; //The model can serve at most 10 different types of vehicle simultaneously among which EV are 1, Transit are 2, Trucks are 3, DilemmaZoneRequests are 4, Coordination are 5, and EVSplitRequest are 6
    FileMod << "set E:={1,2};\n";
    FileMod << "\n";
    //========================Parameters=========================

    FileMod << "param y    {p in P}, >=0,default 0;\n";
    FileMod << "param red  {p in P}, >=0,default 0;\n";
    FileMod << "param gmin {p in P}, >=0,default 0;\n";
    FileMod << "param gmax {p in P}, >=0,default 0;\n";
    FileMod << "param init1,default 0;\n";
    FileMod << "param init2,default 0;\n";
    FileMod << "param Grn1, default 0;\n";
    FileMod << "param Grn2, default 0;\n";
    FileMod << "param SP1,  integer,default 0;\n";
    FileMod << "param SP2,  integer,default 0;\n";
    FileMod << "param M:=9999,integer;\n";
    FileMod << "param alpha:=100,integer;\n";
    FileMod << "param Rl{p in P, j in J}, >=0,  default 0;\n";
    FileMod << "param Ru{p in P, j in J}, >=0,  default 0;\n";
    
    FileMod << "param PrioType { t in T}, >=0, default 0;  \n";
    FileMod << "param PrioWeight { t in T}, >=0, default 0;  \n";
    FileMod << "param priorityType{j in J}, >=0, default 0;  \n";
    FileMod << "param priorityTypeWeight{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeight[t] else 0);  \n";
    FileMod << "param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);\n";
    FileMod << "param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);\n";
    FileMod << "param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);\n";
    FileMod << "param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);\n";
    FileMod << "param ReqNo:=sum{p in P,j in J} active_pj[p,j];\n";
    FileMod << "param gmaxPerRng{p in P,k in K}, := gmax[p];\n";
    FileMod << "\n";
    // ==================== Variables =======================
    FileMod << "var t{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var g{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var v{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var d{p in P,j in J}, >=0;\n";
    FileMod << "var theta{p in P,j in J}, binary;\n";
    FileMod << "var ttheta{p in P,j in J}, >=0;\n";
    FileMod << "var PriorityDelay;\n";

    FileMod << "var Flexibility;\n";

    FileMod << "\n";

    // ===================Constraints==============================

    FileMod << "s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  \n";
    FileMod << "s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  \n";
    FileMod << "s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  \n";
    
    // # constraints in the same cycle in same ring and barrier group, for cycle,k=1
    if (EV_P11.size() > 1)
        FileMod << "s.t. PrecedenceConstraint_From_P11_To_P11_Cycle1{e in E,p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P12.size() > 1)
        FileMod << "s.t. PrecedenceConstraint_From_P12_To_P12_Cycle1{e in E,p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P21.size() > 1)
        FileMod << "s.t. PrecedenceConstraint_From_P21_To_P21_Cycle1{e in E,p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P22.size() > 1)
        FileMod << "s.t. PrecedenceConstraint_From_P22_To_P22_Cycle1{e in E,p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    
    // # constraints in the same cycle in connecting
    if (EV_P11.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P11_To_P12_Cycle1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[" << EV_P11[phasePosition] << ",1,e]+v[" << EV_P11[phasePosition] << ",1,e];\n";
    }

    if (EV_P11.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P11_To_P22_Cycle1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[" << EV_P11[phasePosition] << ",1,e]+v[" << EV_P11[phasePosition] << ",1,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P21_To_P12_Cycle1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[" << EV_P21[phasePosition] << ",1,e]+v[" << EV_P21[phasePosition] << ",1,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P21_To_P22_Cycle1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[" << EV_P21[phasePosition] << ",1,e]+v[" << EV_P21[phasePosition] << ",1,e];\n";
    }

    // #================ END of cycle 1======================#

    // # constraints in the same cycle in same ring and barrier group, for cycle,k>1
    if (EV_P11.size() > 1)
        FileMod << "s.t. PrecedenceConstraint_From_P11_To_P11_Cycle2and3{e in E,p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P12.size() > 1)
        FileMod << "s.t. PrecedenceConstraint_From_P12_To_P12_Cycle2and3{e in E,p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P21.size() > 1)
        FileMod << "s.t. PrecedenceConstraint_From_P21_To_P21_Cycle2and3{e in E,p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P22.size() > 1)
        FileMod << "s.t. PrecedenceConstraint_From_P22_To_P22_Cycle2and3{e in E,p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";

    // # constraints in the same cycle in connecting

    if (EV_P11.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P11_To_P12_Cycle2and3{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=" << static_cast<int>(EV_P12.size()) + EV_P12[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P11[phasePosition] << ",k,e]+v[" << EV_P11[phasePosition] << ",k,e];\n";
    }

    if (EV_P11.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P11_To_P22_Cycle2and3{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=" << static_cast<int>(EV_P22.size()) + EV_P22[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P11[phasePosition] << ",k,e]+v[" << EV_P11[phasePosition] << ",k,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P21_To_P12_Cycle2and3{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=" << static_cast<int>(EV_P12.size()) + EV_P12[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P21[phasePosition] << ",k,e]+v[" << EV_P21[phasePosition] << ",k,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P21_To_P22_Cycle2and3{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=" << static_cast<int>(EV_P22.size()) + EV_P22[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P21[phasePosition] << ",k,e]+v[" << EV_P21[phasePosition] << ",k,e];\n";
    }

    // # constraints in connecting in different cycles
    if (EV_P12.size() > 0 && EV_P11.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P12.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P12_To_P11_Cycle2and3{e in E,p in P11, k in K: (card(P11)+p+1)=" << static_cast<int>(EV_P11.size()) + EV_P11[0] + 1 << " and k>1 }:    t[p,k,e]=t[" << EV_P12[phasePosition] << ",k-1,e]+v[" << EV_P12[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P22.size() > 0 && EV_P11.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P22.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P22_To_P11_Cycle2and3{e in E,p in P11, k in K: (card(P11)+p+1+4)=" << static_cast<int>(EV_P11.size()) + EV_P11[0] + 1 + 4 << " and k>1 }:  t[p,k,e]=t[" << EV_P22[phasePosition] << ",k-1,e]+v[" << EV_P22[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P12.size() > 0 && EV_P21.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P12.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P12_To_P21_Cycle2and3{e in E,p in P21, k in K: (card(P21)+p+1-4)=" << static_cast<int>(EV_P21.size()) + EV_P21[0] + 1 - 4 << " and k>1 }:  t[p,k,e]=t[" << EV_P12[phasePosition] << ",k-1,e]+v[" << EV_P12[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P22.size() > 0 && EV_P21.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P22.size()) - 1;
        FileMod << "s.t. PrecedenceConstraint_From_P22_To_P21_Cycle2and3{e in E,p in P21, k in K: (card(P21)+p+1)=" << static_cast<int>(EV_P21.size()) + EV_P21[0] + 1 << " and k>1 }:    t[p,k,e]=t[" << EV_P22[phasePosition] << ",k-1,e]+v[" << EV_P22[phasePosition] << ",k-1,e];\n";
    }

    FileMod << "s.t. PhaseDuration{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];\n";
    FileMod << "s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";
    FileMod << "s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";

    FileMod << "s.t. PriorityConstraint1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; \n";
    FileMod << "s.t. PriorityConstraint2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n";
    FileMod << "s.t. PriorityConstraint3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];\n";
    FileMod << "s.t. PriorityConstraint4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);\n";
    FileMod << "s.t. PriorityConstraint5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];\n";
    FileMod << "s.t. PriorityConstraint6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];\n";
    FileMod << "s.t. PriorityConstraint7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];\n";
    FileMod << "s.t. PriorityConstraint8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; \n";
    FileMod << "s.t. PriorityConstraint9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";

    FileMod << "s.t. FlexibilityConstraint: Flexibility= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];\n ";
    FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeight[j,tt]*active_pj[p,j]*d[p,j] ) )  - " << FlexibilityWeight << "*Flexibility; \n "; // The coeficient to Flexibility should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points

    FileMod << "  minimize delay: PriorityDelay;     \n";

    //=============================Writing the Optimal Output into the /nojournal/bin/OptimizationResults.txt file ==================================
    FileMod << "  \n";
    FileMod << "solve;  \n";
    FileMod << "  \n";
    FileMod << "printf \" \" > \"/nojournal/bin/OptimizationResults.txt\";  \n";
    FileMod << "printf \"%3d  %3d \\n \",SP1, SP2 >>\"/nojournal/bin/OptimizationResults.txt\";  \n";
    FileMod << "printf \"%5.2f  %5.2f %5.2f  %5.2f \\n \",init1, init2,Grn1,Grn2 >>\"/nojournal/bin/OptimizationResults.txt\";  \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,1] else 0  >>\"/nojournal/bin/OptimizationResults.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/OptimizationResults.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,2] else 0  >>\"/nojournal/bin/OptimizationResults.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/OptimizationResults.txt\";\n";
    FileMod << " } \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,1] else 0  >>\"/nojournal/bin/OptimizationResults.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/OptimizationResults.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,2] else 0  >>\"/nojournal/bin/OptimizationResults.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/OptimizationResults.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";

    FileMod << "printf \"%3d \\n \", ReqNo >>\"/nojournal/bin/OptimizationResults.txt\";  \n";
    FileMod << "  \n";
    FileMod << "for {p in P,j in J : Rl[p,j]>0}  \n";
    FileMod << " {  \n";
    FileMod << "   printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", p, Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>\"/nojournal/bin/OptimizationResults.txt\";\n"; 
    FileMod << " } \n";
    
    FileMod << "printf \"%5.2f \\n \", PriorityDelay + 0.01*Flexibility>>\"/nojournal/bin/OptimizationResults.txt\"; \n";

    FileMod << "printf \"%5.2f \\n \", Flexibility >>\"/nojournal/bin/OptimizationResults.txt\"; \n";
    //------------- End of Print the Main body of mode----------------
    FileMod << "end;\n";
    FileMod.close();
}


OptimizationModelManager::~OptimizationModelManager()
{
}