set P11:={2}; 
set P12:={4}; 
set P21:={6}; 
set P22:={8}; 
set P:={2, 4, 6, 8};
set K  := {1..3};
set J  := {1..10};
set P2 := {1..8};
set T  := {1..10};
set E:={1,2};
set DZ:={1,2};

param y    {p in P}, >=0,default 0;
param red  {p in P}, >=0,default 0;
param gmin {p in P}, >=0,default 0;
param gmax {p in P}, >=0,default 0;
param init1,default 0;
param init2,default 0;
param Grn1, default 0;
param Grn2, default 0;
param SP1,  integer,default 0;
param SP2,  integer,default 0;
param M:=9999,integer;
param alpha:=100,integer;
param Rl{p in P, j in J}, >=0,  default 0;
param Ru{p in P, j in J}, >=0,  default 0;
param cycle, :=100;
param PrioType { t in T}, >=0, default 0;  
param PrioWeigth { t in T}, >=0, default 0;  
param priorityType{j in J}, >=0, default 0;  
param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  
param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);
param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);
param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);
param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);
param ReqNo:=sum{p in P,j in J} active_pj[p,j];
param Dl{p in P, dz in DZ}, >=0,  default 0;
param Du{p in P, dz in DZ}, >=0,  default 0;
param DilemmaZoneWeight, default 0;
param active_dilemmazone_p{p in P, dz in DZ}, integer, :=(if Dl[p,dz]>0 then 1 else	0);
param gmaxPerRng{p in P,k in K}, := gmax[p];

var t{p in P,k in K,e in E}, >=0;
var g{p in P,k in K,e in E}, >=0;
var v{p in P,k in K,e in E}, >=0;
var d{p in P,j in J}, >=0;
var theta{p in P,j in J}, binary;
var ttheta{p in P,j in J}, >=0;
var dilemmazone_d{p in P, dz in DZ}, >=0;
var dilemmazone_theta{p in P, dz in DZ}, binary;
var dilemmazone_ttheta{p in P, dz in DZ}, >=0;
var PriorityDelay;
var DilemmaZoneDelay;
var Flex;

s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  
s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  
s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  
s.t. Prec_11_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];
s.t. Prec_11_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];
s.t. Prec_21_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];
s.t. Prec_21_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];
s.t. Prec_11_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];
s.t. Prec_11_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];
s.t. Prec_21_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];
s.t. Prec_21_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];
s.t. Prec_12_11_c23{e in E,p in P11, k in K: (card(P11)+p+1)=4 and k>1 }:    t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];
s.t. Prec_22_11_c23{e in E,p in P11, k in K: (card(P11)+p+1+4)=8 and k>1 }:  t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];
s.t. Prec_12_21_c23{e in E,p in P21, k in K: (card(P21)+p+1-4)=4 and k>1 }:  t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];
s.t. Prec_22_21_c23{e in E,p in P21, k in K: (card(P21)+p+1)=8 and k>1 }:    t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];
s.t. PhaseLen{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];
s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; 
s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; 
s.t. PrioDelay1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; 
s.t. PrioDelay2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));
s.t. PrioDelay3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];
s.t. PrioDelay4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);
s.t. PrioDelay5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];
s.t. PrioDelay6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];
s.t. PrioDelay7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];
s.t. PrioDelay8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; 
s.t. PrioDelay9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; 
s.t. Flexib: Flex= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];
 s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) )  - 0.01*Flex; 
 s.t. DilemmaZoneDelay1{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    dilemmazone_d[p,dz]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Dl[p,dz]; 
s.t. DilemmaZoneDelay2{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    M*dilemmazone_theta[p,dz]>=Du[p,dz]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));
s.t. DilemmaZoneDelay3{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    dilemmazone_d[p,dz]>= dilemmazone_ttheta[p,dz]-Dl[p,dz]*dilemmazone_theta[p,dz];
s.t. DilemmaZoneDelay4{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Du[p,dz]-Dl[p,dz])*(1-dilemmazone_theta[p,dz]);
s.t. DilemmaZoneDelay5{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    dilemmazone_ttheta[p,dz]<=M*dilemmazone_theta[p,dz];
s.t. DilemmaZoneDelay6{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-dilemmazone_theta[p,dz])<=dilemmazone_ttheta[p,dz];
s.t. DilemmaZoneDelay7{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-dilemmazone_theta[p,dz])>=dilemmazone_ttheta[p,dz];
s.t. DilemmaZoneDelay8{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Du[p,dz]-Dl[p,dz])*dilemmazone_theta[p,dz]; 
s.t. DilemmaZoneDelay9{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    Du[p,dz]*dilemmazone_theta[p,dz] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; 
s.t. DD: DilemmaZoneDelay=( sum{p in P, dz in DZ} (DilemmaZoneWeight*active_dilemmazone_p[p,dz]*dilemmazone_d[p,dz] ) )  - 0.01*Flex; 
  minimize delay: PriorityDelay + DilemmaZoneDelay;     
  
solve;  
  
printf " " > "/nojournal/bin/Results.txt";  
printf "%3d  %3d \n ",SP1, SP2 >>"/nojournal/bin/Results.txt";  
printf "%5.2f  %5.2f %5.2f  %5.2f \n ",init1, init2,Grn1,Grn2 >>"/nojournal/bin/Results.txt";  
 
for {k in K}   
 { 
     for {p in P2} 
        { 
           printf "%5.2f  ", if(p in P)  then v[p,k,1] else 0  >>"/nojournal/bin/Results.txt";   
        } 
        printf " \n ">>"/nojournal/bin/Results.txt";
 } 
  
for {k in K}   
 { 
     for {p in P2} 
        { 
           printf "%5.2f  ", if(p in P)  then v[p,k,2] else 0  >>"/nojournal/bin/Results.txt";   
        } 
        printf " \n ">>"/nojournal/bin/Results.txt";
 } 
 
for {k in K}   
 { 
     for {p in P2} 
        { 
           printf "%5.2f  ", if(p in P)  then g[p,k,1] else 0  >>"/nojournal/bin/Results.txt";   
        } 
        printf " \n ">>"/nojournal/bin/Results.txt";
 } 
  
for {k in K}   
 { 
     for {p in P2} 
        { 
           printf "%5.2f  ", if(p in P)  then g[p,k,2] else 0  >>"/nojournal/bin/Results.txt";   
        } 
        printf " \n ">>"/nojournal/bin/Results.txt";
 } 
  
printf "%3d \n ", ReqNo >>"/nojournal/bin/Results.txt";  
  
for {p in P,j in J : Rl[p,j]>0}  
 {  
   printf "%d  %5.2f  %5.2f  %5.2f %d \n ", p, Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>"/nojournal/bin/Results.txt";
 } 
printf "%5.2f \n ", PriorityDelay + 0.01*Flex>>"/nojournal/bin/Results.txt"; 
printf "%5.2f \n ", Flex >>"/nojournal/bin/Results.txt"; 
printf " \n ">>"/nojournal/bin/Results.txt";
end;
