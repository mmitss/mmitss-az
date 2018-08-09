/*NOTICE:  Copyright 2014 /Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  PriorityRequest.h  
 *  Created by Mehdi Zamanipour on 2/19/15
 *  University of Arizona/
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
#include <iostream>
using namespace std;

class PriorityRequest
{
	public:
		int iPhaseCycle; // the phase number in the ring that can be {0,1,2,3,10,11,12,13} for both rings
		double dRl;
		double dRu;
		int iType;
		double dReqDelay;
	public:
		PriorityRequest(void);
		PriorityRequest(const PriorityRequest& PR);
		PriorityRequest& operator=(PriorityRequest& PR);
		PriorityRequest(int phaseInRing,double rl,double ru,double delay,int type);
		~PriorityRequest(void);
};
