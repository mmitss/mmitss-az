/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  PriorityRequest.cpp  
 *  Created by Mehdi Zamanipour on 2/19/15
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

#include "PriorityRequest.h"

PriorityRequest::PriorityRequest(void)
{
	iPhaseCycle=0;
	dRl=0.0;	
	dRu=0.0;
	iType=0;
	dReqDelay=0.0;
}

PriorityRequest::PriorityRequest(int phaseInRingNo,double rl,double ru,double delay,int type)
{
	iPhaseCycle=phaseInRingNo;
	dRl=rl;
	dRu=ru;
	iType=type;
	dReqDelay=delay;
}

PriorityRequest::PriorityRequest(const PriorityRequest& PR)
{
    iPhaseCycle=PR.iPhaseCycle;
	dRl=PR.dRl;
	dRu=PR.dRu;
	iType=PR.iType;
	dReqDelay=PR.dReqDelay;
}

PriorityRequest& PriorityRequest::operator=(PriorityRequest& PR)
{
    iPhaseCycle=PR.iPhaseCycle;
	dRl=PR.dRl;
	dRu=PR.dRu;
	iType=PR.iType;
	dReqDelay=PR.dReqDelay;
    return *this;
}

PriorityRequest::~PriorityRequest(void)
{
}
