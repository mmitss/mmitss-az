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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "Signal.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

using namespace std;

//tell ramp or intersection
#define RAMP 0
#define INTERSECTION 1
//define the different phase control types
#define	PHASE_HOLD 0
#define PHASE_FORCEOFF 1
#define PHASE_OMIT 2
#define PHASE_VEH_CALL 3

#define MAX_ITEMS 20
//#define numPhases 8


//****The FOLLOWING should be defined in MAIN()*************//
//intersection asc controller ip port
extern char INTip[64];
//char INTip[64];// = "150.135.152.23";
extern char INTport[16];
extern char tmp_log[512];
extern char logfilename[256];
//****The FOLLOWING should be defined in MAIN()*************//


//------------------START OF DEFINITION--------------------------------//
//ASC INTERSECTION MIB :NTCIP 1202
//1.3.6.1.4.1.1206.4.1.3.1.1.3
#define RED_GROUP		 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1"                     //Object
#define YELLOW_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1"
#define GREEN_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1"
#define DONOTWALK_GROUP  	"1.3.6.1.4.1.1206.4.2.1.1.4.1.5.1"
#define PEDCLEAR_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.6.1"
#define WALK_GROUP 		 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.7.1"
#define VEHICLE_CALL	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.8.1"
//#define VEHICLE_DET_CALL	"1.3.6.1.4.1.1206.3.5.2.12.2.1.3"

#define PEDES_CALL	 		"1.3.6.1.4.1.1206.4.2.1.1.4.1.8.1"  // AT this time just use vehicle call

//Phase control
#define MIB_PHASE_HOLD 	 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.4.1"
#define MIB_PHASE_FORCEOFF 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.5.1"
#define MIB_PHASE_OMIT 	 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.2.1"
#define MIB_PHASE_VEH_CALL 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.6.1"
#define MIB_PHASE_PED_CALL 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.7.1"

// Controller configure information
#define MAX_PHASE_NO	 	"1.3.6.1.4.1.1206.4.2.1.1.1"
#define PHASE_NUMBER	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.1.1"   // phase number: last ".X" is phase number, the return value is also X

#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

#define PHASE_ENABLED       "1.3.6.1.4.1.1206.4.2.1.1.2.1.21."  // Phase options: last ".X" is phase, the last bit of return result is "0", the phase is not enabled.
//------------The following from standard: only read PLAN 1---------//
#define PHASE_MIN_GRN	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.4."   // need last "x" is the phase number: return the minimun green of phase x
#define PHASE_MAX_GRN	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.6."
#define PHASE_RED_CLR	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.9."
#define PHASE_YLW_XGE	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.8."
//------------The following from ASC3: WILL BE USED---------//
#define PHASE_MIN_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.9."   // need last "x.p" x is the timing plan number,p is the phase number: x get from CUR_TIMING_PLAN
#define PHASE_MAX_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.15."
#define PHASE_RED_CLR_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.19."
#define PHASE_YLW_XGE_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.18."

//**********asc3PhaseStatusTiming
//T (1):       Phase Timing
//N (2):       Phase Next
//- (3):       Phase Not Enabled
//(space) (4): Phase Not Timing or Next
#define PHASE_STA_TIME_ASC  "1.3.6.1.4.1.1206.3.5.2.1.18.1.1."  //NEED last "p"  for the phase
//**********asc3PhaseStatusTiming2
// (1) X: XPED timing
// (2) N: Phase Next
// (3) -: Phase Not enabled
// (4) .: Phase Not Timing
// (5) R: Phase Timing RED
// (6) Y: Phase Timing YEL
// (7) G: Phase Timing GREEN

// (8) D: Phase Timing DELAY GREEN
// (9) O: Phase Timing YEL & RED
//(10) g: Phase Timing FLASHING GREEN
//(11) y: Phase Timing FLASHING YELLOW "
#define PHASE_STA_TIME2_ASC  "1.3.6.1.4.1.1206.3.5.2.1.18.1.6."  //NEED last "p"  for the phase

#define MIB_RING_STOPTIME	"1.3.6.1.4.1.1206.4.2.1.7.5.1.2.1"

#define asc3ViiMessageEnable	"1.3.6.1.4.1.1206.3.5.2.9.44.1.1"  //Set controller to broadcast SPAT


int GetSignalColor(int PhaseStatus);
int outputlog(char *output);



class SignalController
{
public:
	Phase Phases;
	PhaseStatus phase_read;
	int CurTimingPlan;

public:
	SignalController(void);
	~SignalController(void);

	// phase_control is the CMD applied to some phases, AS defined above PHASE_HOLD, FORCEOFF, etc.
	// The Total is the number for commanded phases: if a phase is CMD, SET it to 1, otherwise 0
	// For Example: CALL phase 2&6: 00100010--> TOTAL=34
	void PhaseControl(int phase_control, int Total,char YES);

	//read phase information through NTCIP: group=Red or Yellow or Green
	void PhaseRead();

	//
	int CurTimingPlanRead();

	//
	//void ConfigRead();

	// Updating the member "Phases"
	void UpdatePhase();
	
	//Make the controller to broadcast the SPAT through UDP
	void SPATSet(int value);

};
