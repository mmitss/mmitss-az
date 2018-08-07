/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  PriorityRequest.cpp  
 *  Created by Jun Ding on 2/19/12
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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "Signal.h"
#include "Array.h"

using namespace std;
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
#define PEDES_CALL	 		"1.3.6.1.4.1.1206.4.2.1.1.4.1.8.1"  // AT this time just use vehicle call

//Phase control
#define MIB_PHASE_HOLD 	 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.4.1"
#define MIB_PHASE_FORCEOFF 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.5.1"
#define MIB_PHASE_OMIT 	 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.2.1"
#define MIB_PHASE_VEH_CALL 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.6.1"

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


#define PHASE_GROUP_STATUS_GREEN   "1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1"  //which phase/phases is green
#define PHASE_GROUP_STATUS_RED     "1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1"  //which phase/phases is red
#define PHASE_GROUP_STATUS_YELLOW  "1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1"  //which phase/phases is yellow
#define PHASE_GROUP_STATUS_NEXT    "1.3.6.1.4.1.1206.4.2.1.1.4.1.11.1"  //which phase/phases is Next


//define the different phase control types
#define PHASE_FORCEOFF 0
#define PHASE_OMIT 1
#define PHASE_VEH_CALL 2
#define	PHASE_HOLD 3

#define MAX_ITEMS 50
#define PORT 15020 //PREEMPTION port
#define BROADCAST_ADDR "192.168.255.255"

////----intersection ASC controller ip port
//extern char INTip[64];// = "150.135.152.23";
//extern char INTport[16];
////extern char ConfigFile[256];
//extern PhaseStatus phase_read;
//extern int CurPhaseStatus[8];
//extern int PhaseDisabled[8];


//------------------END OF DEFINITION--------------------------------//

int GetSignalColor(int PhaseStatus);
void PhaseTimingStatusRead();
int  CurTimingPlanRead();  // For IntersectionConfigRead()
void IntersectionConfigRead(int CurTimingPlanNo,char *ConfigOutFile);
void IntersectionPhaseControl(int phase_control, int Total,char YES);

