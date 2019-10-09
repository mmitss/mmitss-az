set P11:={1,2};  
set P12:={3,4};  
set P21:={5,6};  
set P22:={7,8};  
set P:={ 1, 2, 3, 4, 5, 6, 7, 8};
set K  := {1,2};
set J  := {1..10};
set P2  :={1..8};  
set I  :={1..131};  
set T  := {1..10}; 	# at most 10 different types of vehicle may be considered , EV are 1, Transit are 2, Trucks are 3  
  
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
param epsilon:=0.00001;
#param alpha:=100,integer;  
param vehDis{p in P, j in J}, >=0,  default 0;  
param PrioType { t in T}, >=0, default 0;  
param PrioWeigth { t in T}, >=0, default 0;  
param priorityType{j in J}, >=0, default 0;  
param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  
param Arr{p in P, i in I}, >=0,  default 0;  
param active_arr{p in P, i in I}, integer, :=(if Arr[p,i]>0 then 1 else 0); 
param SumOfActiveArr, := (if (sum{p in P, i in I} Arr[p,i])>0 then (sum{p in P, i in I} Arr[p,i]) else 1); 
  
param active_pj{p in P, j in J}, integer, :=(  if vehDis[p,j]>0 then	1  else	0);  
param coef{p in P,k in K}, integer,:=(   if ((((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1))then   	0   else   	1);  
param MinGrn1{p in P,k in K},:=(  if (((p==SP1 and p<5) and k==1))then   	Grn1   else   	0);  
param MinGrn2{p in P,k in K},:=(  if (((p==SP2 and p>4 ) and k==1))then   	Grn2   else   	0);  
param ReqNo:=sum{p in P,j in J} active_pj[p,j];  
#param queue_pj{p in P, j in J}, integer, :=(  if Tq[p,j]>0 then	1  else	0);  

param lmax{j in J}, >=0, default 0;  
param n{j in J}, >=0, default 0;  
param vehSp{j in J}, >=0, default 0;  
param qSp{j in J}, >=0, default 0;  
param disChargSp{j in J}, >=0, default 0;  
  
var t{p in P,k in K}, >=0;    # starting time vector  
var g{p in P,k in K}, >=0;  
var v{p in P,k in K}, >=0;  
var d{p in P,j in J}, >=0;  
var PriorityDelay,>=0;  

var theta{p in P,j in J}, binary;  
var ttheta{p in P,j in J}, >=0; 
var gama{p in P,j in J}, binary; 
var tgama{p in P,j in J}, >=0; 
var zeta{p in P,j in J}, binary; 
var gzeta{p in P,j in J}, >=0; 
var lprim{p in P,j in J}, >=0; 
var thetalprim{p in P,j in J}, >=0; 
var lzegon, >=0; 
var lzegontheta{p in P,j in J}, >=0; 

#var miu{p in P,i in I}, binary; 
#var rd{p in P,i in I}, >=0; 
#var tmiu{p in P, i in I}, >=0; 
#var TotRegVehDel, >=0; 
#var QueClrTime{p in P, i in I},>=0; 
#var ttheta{p in P,j in J}, >=0; 
  
#================ Begin of cycle 1======================#  
s.t. initial{p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1]=0;  
s.t. initial1{p in P:p=SP1}: t[p,1]=init1;  
s.t. initial2{p in P:p=SP2}: t[p,1]=init2;  

 # constraints in the same cycle in same P??  
s.t. Prec_11_11_c1{p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1]=t[p,1]+v[p,1];  
s.t. Prec_12_12_c1{p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1]=t[p,1]+v[p,1];  
s.t. Prec_21_21_c1{p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1]=t[p,1]+v[p,1];  
s.t. Prec_22_22_c1{p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1]=t[p,1]+v[p,1];  
  
# constraints in the same cycle in connecting   
s.t. Prec_11_12_c1{p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1]=t[2,1]+v[2,1];  
s.t. Prec_11_22_c1{p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1]=t[2,1]+v[2,1];  
s.t. Prec_21_12_c1{p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1]=t[6,1]+v[6,1];  
s.t. Prec_21_22_c1{p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1]=t[6,1]+v[6,1];  
  
#================ END of cycle 1======================#  
  
# constraints in the same cycle in same P??  
s.t. Prec_11_11_c23{p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k]=t[p,k]+v[p,k];  
s.t. Prec_12_12_c23{p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k]=t[p,k]+v[p,k];  
s.t. Prec_21_21_c23{p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k]=t[p,k]+v[p,k];  
s.t. Prec_22_22_c23{p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k]=t[p,k]+v[p,k];  
  
# constraints in the same cycle in connecting   
s.t. Prec_11_12_c23{p in P12, k in K: (card(P12)+p)=5 and k>1 }:  t[p,k]=t[2,k]+v[2,k];
s.t. Prec_11_22_c23{p in P22, k in K: (card(P22)+p)=9 and k>1 }:  t[p,k]=t[2,k]+v[2,k];  
s.t. Prec_21_12_c23{p in P12, k in K: (card(P12)+p)=5 and k>1 }:  t[p,k]=t[6,k]+v[6,k];  
s.t. Prec_21_22_c23{p in P22, k in K: (card(P22)+p)=9 and k>1 }:  t[p,k]=t[6,k]+v[6,k];  
  
# constraints in connecting in different cycles  
s.t. Prec_12_11_c23{p in P11, k in K: (card(P11)+p+1)=4 and k>1 }:    t[p,k]=t[4,k-1]+v[4,k-1];  
s.t. Prec_22_11_c23{p in P11, k in K: (card(P11)+p+1+4)=8 and k>1 }:  t[p,k]=t[8,k-1]+v[8,k-1];  
s.t. Prec_12_21_c23{p in P21, k in K: (card(P21)+p+1-4)=4 and k>1 }:  t[p,k]=t[4,k-1]+v[4,k-1];  
s.t. Prec_22_21_c23{p in P21, k in K: (card(P21)+p+1)=8 and k>1 }:    t[p,k]=t[8,k-1]+v[8,k-1];  
  
#==================================================#  
  
 s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) );  
  
# s.t. TotRegVehDelay: TotRegVehDel=(sum{p in P, i in I} active_arr[p,i]*Arr[p,i]*rd[p,i])/SumOfActiveArr;  
   
s.t. PhaseLen{p in P, k in K}:  v[p,k]=(g[p,k]+y[p]+red[p])*coef[p,k];  
  
s.t. PhaseLen2{p in P, k in K}:  v[p,k]*coef[p,k]>=g[p,k]; 
s.t. GrnMax{p in P, k in K}:  g[p,k]<=gmax[p];    
#s.t. GrnMax{p in P : (p!= coordphase1 and p!=coordphase2)}:  g[p,1]<=gmax[p];    
#s.t. GrnMax2{p in P}:  (1-isItCoordinated)*g[p,2]<=gmax[p];   
#s.t. GrnMax3{p in P}:  (1-isItCoordinated)*g[p,1]<=gmax[p];  
s.t. GrnMin{p in P,k in K}:  			       g[p,k]>=(gmin[p]-MinGrn1[p,k]-MinGrn2[p,k])*coef[p,k];  
s.t. PriDel1{ p in P,j in J: active_pj[p,j]>0 }:       n[j]+M*gama[p,j]>=t[p,1];
s.t. PriDel2{ p in P,j in J: active_pj[p,j]>0 }:       n[j]*gama[p,j]<=tgama[p,j]+epsilon;
s.t. PriDel3{ p in P,j in J: active_pj[p,j]>0 }:       M*gama[p,j]>=tgama[p,j];
s.t. PriDel4{ p in P,j in J: active_pj[p,j]>0 }:       t[p,1]+M*(1-gama[p,j])>=tgama[p,j];
s.t. PriDel5{ p in P,j in J: active_pj[p,j]>0 }:       t[p,1]-M*(1-gama[p,j])<=tgama[p,j];
s.t. PriDel6{ p in P,j in J: active_pj[p,j]>0 }:       lprim[p,j]=lmax[j]*gama[p,j];
s.t. PriDel66{ p in P,j in J: active_pj[p,j]>0}:       theta[p,j]=0;
#s.t. PriDel7{ p in P,j in J: active_pj[p,j]>0}:       t[p,1]+lprim[p,j]/disChargSp[j]-(vehDis[p,j]-lprim[p,j])/vehSp[j] <= d[p,j]; 
s.t. PriDel7{ p in P,j in J: active_pj[p,j]>0 }:       t[p,1]+lprim[p,j]/disChargSp[j]-(vehDis[p,j])/vehSp[j] <= d[p,j]; 
s.t. PriDel8{ p in P,j in J: active_pj[p,j]>0 }:       M*theta[p,j]>=(lprim[p,j]/disChargSp[j]+(vehDis[p,j]-lprim[p,j])/vehSp[j])-(t[p,1]+g[p,1]);
s.t. PriDel9{ p in P,j in J: active_pj[p,j]>0 }:       lprim[p,j]/vehSp[j]+lprim[p,j]/disChargSp[j] - thetalprim[p,j]/vehSp[j]-thetalprim[p,j]/disChargSp[j] <=g[p,1];
s.t. PriDel10{ p in P,j in J: active_pj[p,j]>0}:       M*theta[p,j]>=thetalprim[p,j];
s.t. PriDel11{ p in P,j in J: active_pj[p,j]>0}:       lprim[p,j]+M*(1-theta[p,j])>=thetalprim[p,j];
s.t. PriDel12{ p in P,j in J: active_pj[p,j]>0}:       lprim[p,j]-M*(1-theta[p,j])<=thetalprim[p,j];
s.t. PriDel13{ p in P,j in J: active_pj[p,j]>0}:       ttheta[p,j]+lzegontheta[p,j]/disChargSp[j]-vehDis[p,j]*theta[p,j]/vehSp[j] <= d[p,j];
s.t. PriDel14{p in P, j in J: active_pj[p,j]>0}:       ttheta[p,j]<=M*theta[p,j]; 
s.t. PriDel15{p in P, j in J: active_pj[p,j]>0}:       t[p,2]-M*(1-theta[p,j])<=ttheta[p,j]; 
s.t. PriDel16{p in P, j in J: active_pj[p,j]>0}:       t[p,2]+M*(1-theta[p,j])>=ttheta[p,j]; 
s.t. PriDel17{p in P, j in J: active_pj[p,j]>0}:       lzegontheta[p,j]<=g[p,2];
s.t. PriDel18{p in P, j in J: active_pj[p,j]>0}:       lzegontheta[p,j]/disChargSp[j]+(vehDis[p,j]/vehSp[j])*theta[p,j]<=t[p,2]+g[p,2];
s.t. PriDel19{p in P, j in J: active_pj[p,j]>0}:       lzegontheta[p,j]<=M*theta[p,j];
s.t. PriDel20{p in P, j in J: active_pj[p,j]>0}:       lzegontheta[p,j]<=lzegon+M*(1-theta[p,j]);
s.t. PriDel21{p in P, j in J: active_pj[p,j]>0}:       lzegontheta[p,j]>=lzegon-M*(1-theta[p,j]);
s.t. PriDel22{p in P, j in J: active_pj[p,j]>0}:       lmax[j]-disChargSp[j]*g[p,1]<=M*(1-zeta[p,j]);
s.t. PriDel23{p in P, j in J: active_pj[p,j]>0}: 	   lmax[j]*zeta[p,j]-disChargSp[j]*gzeta[p,1]<=0;
s.t. PriDel24{p in P, j in J: active_pj[p,j]>0}: 	   lzegon=lmax[j]-lmax[j]*zeta[p,j]-disChargSp[j]*g[p,1]+disChargSp[j]*gzeta[p,j];
s.t. PriDel25{p in P, j in J: active_pj[p,j]>0}: 	   gzeta[p,j]<=M*zeta[p,j];
s.t. PriDel26{p in P, j in J: active_pj[p,j]>0}: 	   gzeta[p,j]<=g[p,1]+M*(1-zeta[p,j]);
s.t. PriDel27{p in P, j in J: active_pj[p,j]>0}: 	   gzeta[p,j]>=g[p,1]-M*(1-zeta[p,j]);

minimize delay:  PriorityDelay;      
#minimize delay:  TotRegVehDel+  PriorityDelay;     
  
solve;  
  
printf " " > "/nojournal/bin/Results.txt";  
printf "%3d  %3d \n ",SP1, SP2 >>"/nojournal/bin/Results.txt";  
printf "%5.2f  %5.2f %5.2f  %5.2f \n ",init1, init2,Grn1,Grn2 >>"/nojournal/bin/Results.txt";  
for {k in K}   
 { 
     for {p in P2} 
        { 
           printf "%5.2f  ", if(p in P)  then v[p,k] else 0  >>"/nojournal/bin/Results.txt";   
        } 
        printf " \n ">>"/nojournal/bin/Results.txt";
 } 
  
for {k in K}   
 { 
     for {p in P2} 
        { 
           printf "%5.2f  ", if(p in P)  then g[p,k] else 0  >>"/nojournal/bin/Results.txt";   
        } 
        printf " \n ">>"/nojournal/bin/Results.txt";
 } 
  
printf "%3d \n ", ReqNo >>"/nojournal/bin/Results.txt";  
  
for {p in P,j in J : vehDis[p,j]>0}  
 {  
   printf "%d %d %d %5.2f %5.2f %5.2f  %5.2f %5.2f %d \n ", ((theta[p,j])), gama[p,j], zeta[p,j],lzegon, lprim[p,j], t[p,1], t[p,2], d[p,j] , priorityType[j] >>"/nojournal/bin/Results.txt";
 } 
  
printf "%5.2f \n ", PriorityDelay >>"/nojournal/bin/Results.txt"; 
#printf "%5.2f \n ", TotRegVehDel >>"/nojournal/bin/Results.txt";  
 printf " \n ">>"/nojournal/bin/Results.txt";
end;
