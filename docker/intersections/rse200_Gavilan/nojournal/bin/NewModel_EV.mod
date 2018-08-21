set P11:={2}; 
set P12:={4}; 
set P21:={6}; 
set P22:={8}; 
set P:={ 2, 4, 6, 8};
set K  := {1,2};
set J  := {1..10};
set P2  :={1..8};  
set I  :={1..100};  
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
param alpha:=100,integer;  
param Rl{p in P, j in J}, >=0,  default 0;  
param Ru{p in P, j in J}, >=0,  default 0;  
param Tq{p in P, j in J}, >=0,  default 5;  
  
param current,>=0,default 0;   # current is  mod(time,cycle)  
param coord, :=1; 	# this paramter indicated where we consider coorrdination or not  
param cycle, :=70;    # if we have coordination, the cycle length  
param offset,:=0;   # if we have coordination, the offset  
param split, :=10;    # if we have coordination, the split  
param coordphase1,:=2;    # if we have coordination, thecoordinated phase in ring1  
param coordphase2,:=6;    # if we have coordination, thecoordinated phase in ring2  
param isCurPhCoord1, :=(if coordphase1=SP1 then 1 else 0);  
param isCurPhCoord2, :=(if coordphase2=SP2 then 1 else 0);  
param earlyReturnPhase1,  := (if (isCurPhCoord1=1 and current>gmax[coordphase1] and coordphase1>0) then 1 else 0); #  if we are in earlier coordinated phase green time, this earlyReturnPhase is 1 
param earlyReturnPhase2,  := (if (isCurPhCoord2=1 and current>gmax[coordphase2] and coordphase1>0) then 1 else 0);  
param PrioType { t in T}, >=0, default 0;  
param PrioWeigth { t in T}, >=0, default 0;  
param priorityType{j in J}, >=0, default 0;  
param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  
param Arr{p in P, i in I}, >=0,  default 0;  
param active_arr{p in P, i in I}, integer, :=(if Arr[p,i]>0 then 1 else 0); 
param SumOfActiveArr, := (if (sum{p in P, i in I} Arr[p,i])>0 then (sum{p in P, i in I} Arr[p,i]) else 1); 
  
param active_pj{p in P, j in J}, integer, :=(  if Rl[p,j]>0 then	1  else	0);  
param coef{p in P,k in K}, integer,:=(   if ((((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1))then   	0   else   	1);  
param MinGrn1{p in P,k in K},:=(  if (((p==SP1 and p<5) and k==1))then   	Grn1   else   	0);  
param MinGrn2{p in P,k in K},:=(  if (((p==SP2 and p>4 ) and k==1))then   	Grn2   else   	0);  
param ReqNo:=sum{p in P,j in J} active_pj[p,j];  
param queue_pj{p in P, j in J}, integer, :=(  if Tq[p,j]>0 then	1  else	0);  
  
var t{p in P,k in K}, >=0;    # starting time vector  
var g{p in P,k in K}, >=0;  
var v{p in P,k in K}, >=0;  
var d{p in P,j in J}, >=0;  
var theta{p in P,j in J}, binary;  
var PriorityDelay,>=0;  
 
var miu{p in P,i in I}, binary; 
var rd{p in P,i in I}, >=0; 
var tmiu{p in P, i in I}, >=0; 
var TotRegVehDel, >=0; 
var QueClrTime{p in P, i in I},>=0; 
var ttheta{p in P,j in J}, >=0; 
var dc1, >=0;  
var dc2, >=0;  
  
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
  
 s.t. TotRegVehDelay: TotRegVehDel=(sum{p in P, i in I} active_arr[p,i]*Arr[p,i]*rd[p,i])/SumOfActiveArr;  
  
s.t. vv{p in P, k in K}:  v[p,k]=(g[p,k]+y[p]+red[p])*coef[p,k];  
  
s.t. GrnMax{p in P : (p!= coordphase1 and p!=coordphase2)}:  g[p,1]<=gmax[p];    
  
  s.t. GrnMax2{p in P}:  g[p,2]<=gmax[p];   
  
  s.t. GrnMin{p in P,k in K}:  g[p,k]>=(gmin[p]-MinGrn1[p,k]-MinGrn2[p,k])*coef[p,k];  
  
  s.t. GrnMin2{p in P:(p=coordphase1 and isCurPhCoord1=1)} :g[p,1]>= split-current; 
  
  s.t. GrnMin3{p in P:(p=coordphase2 and isCurPhCoord2=1)} :g[p,1]>= split-current; 
  
  s.t. CoordPhasMax1{p in P: p=coordphase1}: (1-earlyReturnPhase1)*g[p,1]<=gmax[p]; 
  
  s.t. CoordPhasMax2{p in P: p=coordphase2}: (1-earlyReturnPhase2)*g[p,1]<=gmax[p]; 
  
s.t. CoordDelay1{p in P: p=coordphase1}: dc1>=((cycle-current+split) - (t[p,2]+v[p,2]) ); 
  
s.t. CoordDelay2{p in P: p=coordphase2}: dc2>=((cycle-current+split) - (t[p,2]+v[p,2]) ); 
  
s.t. CoordDelay5{p in P: ((p in P21)and current>gmax[coordphase1] and coordphase1>0)}: dc2>= ((cycle-current+split) - (t[coordphase2,1]+v[coordphase2,1]));   # I assumed the coordinated phase of ring 2 is in the second brrier 
  
s.t. CoordDelay6{p in P: ((p in P21)and current>gmax[coordphase2] and coordphase1>0)}: dc2>= ((cycle-current+split) - (t[coordphase2,1]+v[coordphase2,1]));   # I assumed the coordinated phase of ring 2 is in the second brrier 
  
  
	s.t. PrioDelay1{p in P,j in J: active_pj[p,j]>0 }:    d[p,j]>=t[p,1]-Rl[p,j];  
  
 s.t. PrioDelay2{p in P,j in J: active_pj[p,j]>0 }:    M*theta[p,j]>=Ru[p,j]-(t[p,1]+v[p,1]); 
  
 s.t. PrioDelay3{p in P,j in J: active_pj[p,j]>0 }:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j]; 
  
s.t. PrioDelay4{p in P, j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j]; 
  
s.t. PrioDelay5{p in P, j in J: active_pj[p,j]>0}:    t[p,2]-M*(1-theta[p,j])<=ttheta[p,j]; 
  
s.t. PrioDelay6{p in P, j in J: active_pj[p,j]>0}:    t[p,2]+M*(1-theta[p,j])>=ttheta[p,j]; 
  
  s.t. RegVehDel1{p in P, i in I: active_arr[p,i]>0} : rd[p,i] >= t[p,1]-i; 
  
s.t. RegVehDel2{p in P, i in I: active_arr[p,i]>0} : M*miu[p,i] >= i-(t[p,1]+v[p,1]); 
  
s.t. RegVehDel3{p in P, i in I: active_arr[p,i]>0} : rd[p,i] >=  tmiu[p,i]-i*miu[p,i]; 
  
s.t. RegVehDel4{p in P, i in I: active_arr[p,i]>0} : tmiu[p,i]<=M*miu[p,i]; 
  
s.t. RegVehDel5{p in P, i in I: active_arr[p,i]>0} : t[p,2]-M*(1-miu[p,i])<=tmiu[p,i]; 
  
s.t. RegVehDel6{p in P, i in I: active_arr[p,i]>0} : t[p,2]+M*(1-miu[p,i])>=tmiu[p,i]; 
  
minimize delay: PriorityDelay  ;
  
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
  
for {p in P,j in J:Rl[p,j]>0}  
 {  
   printf "%d  %5.2f  %5.2f %5.2f \n ", (p+10*(theta[p,j])), Rl[p,j],Ru[p,j],Tq[p,j]>>"/nojournal/bin/Results.txt";
 } 
  
printf "%5.2f ", PriorityDelay >>"/nojournal/bin/Results.txt";  

printf "%5.2f ", TotRegVehDel >>"/nojournal/bin/Results.txt";  
 printf "coordination delay %5.2f ", dc1+dc2 >>"/nojournal/bin/Results.txt";  
end;
