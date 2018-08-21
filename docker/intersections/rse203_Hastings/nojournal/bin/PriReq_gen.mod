set P11:={1,2};  
set P12:={4}; 
set P21:={6}; 
set P22:={8}; 
set P:={ 1, 2, 4, 6, 8};
set K  := {1,2,3};
set J  := {1..12};
set P2  :={1..8};  
  
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
param M:=99999,integer;  
param alpha:=100,integer;  
param Rl{p in P, j in J}, >=0,  default 0;  
param Ru{p in P, j in J}, >=0,  default 0;  
param Tq{p in P, j in J}, >=0,  default 5;  
  
param active_pj{p in P, j in J}, integer, :=(  if Rl[p,j]>0 then	1  else	0);  
param coef{p in P,k in K}, integer,:=(   if ((((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1))then   	0   else   	1);  
param MinGrn1{p in P,k in K},:=(  if (((p==SP1 and p<5) and k==1))then   	Grn1   else   	0);  
param MinGrn2{p in P,k in K},:=(  if (((p==SP2 and p>4 ) and k==1))then   	Grn2   else   	0);  
param ReqNo:=sum{p in P,j in J} active_pj[p,j];  
param queue_pj{p in P, j in J}, integer, :=(  if Tq[p,j]>0 then	1  else	0);  
  
var t{p in P,k in K}, >=0;    # starting time vector  
var g{p in P,k in K}, >=0;  
var a{p in P,k in K}, >=0;  
var v{p in P,k in K}, >=0;  
var d{p in P,j in J,k in K}, >=0;  
var theta{p in P,j in J,k in K}, binary;  
var RealDelay,>=0;  
  
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
  
s.t. vv{p in P, k in K}:  v[p,k]=(g[p,k]+y[p]+a[p,k]+red[p])*coef[p,k];  
  
s.t. RD: RealDelay=sum{p in P,j in J,k in K} active_pj[p,j]*d[p,j,k];  
  
s.t. selection_1{p in P,j in J,k in K: active_pj[p,j]>0 }: Ru[p,j]<=t[p,k]+g[p,k]*coef[p,k]+M*(1-theta[p,j,k]);  
  
s.t. selection_2{p in P,j in J,k in K: (active_pj[p,j]>0 and k >1 )}: Ru[p,j]>=t[p,k-1]+g[p,k-1]*coef[p,k-1]-M*(1-theta[p,j,k]);  
  
s.t. queue_minG{p in P,j in J,k in K: queue_pj[p,j]>0 } :   g[p,k]*coef[p,k] >= Tq[p,j] - M*(1-theta[p,j,k]);    
  
s.t. delayPlus{p in P,j in J,k in K: active_pj[p,j]>0 }: d[p,j,k]>=t[p,k]-Rl[p,j]-M*(1-theta[p,j,k]);  
  
s.t. sumtheta{p in P,j in J: active_pj[p,j]>0 }: sum{k in K} theta[p,j,k]=1;  
  
s.t. GrnMax{p in P,k in K}:  g[p,k]+a[p,k]<=gmax[p];  
  
s.t. GrnMin{p in P,k in K}:  g[p,k]>=(gmin[p]-MinGrn1[p,k]-MinGrn2[p,k])*coef[p,k];  
  
minimize delay: sum{p in P,j in J,k in K}  alpha*active_pj[p,j]*d[p,j,k]-sum{p in P,k in K} a[p,k];  
  
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
  
for {k in K}   
 { 
     for {p in P2} 
        { 
           printf "%5.2f  ", if(p in P)  then a[p,k] else 0  >>"/nojournal/bin/Results.txt";   
        } 
        printf " \n ">>"/nojournal/bin/Results.txt";
 } 
  
printf "%3d \n ", ReqNo >>"/nojournal/bin/Results.txt";  
  
for {k in K,p in P,j in J:theta[p,j,k]>0}  
 {  
   printf "%d  %5.2f  %5.2f %5.2f \n ", (p+10*(k-1)), Rl[p,j],Ru[p,j],Tq[p,j]>>"/nojournal/bin/Results.txt";
 } 
  
printf "%5.2f", RealDelay >>"/nojournal/bin/Results.txt";  

end;
