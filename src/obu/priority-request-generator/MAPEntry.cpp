/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  MAPEntry.cpp  
 *  Created by Mehdi Zamanipour on 7/9/14.
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

#include "MAPEntry.h"
MAPEntry::MAPEntry(void)
{
	ID=0;
	AtTime=0;
	//Flag=-999;
}
MAPEntry::MAPEntry(long id, time_t time)
{
	ID=id;
	AtTime=time;
}

MAPEntry::MAPEntry(MAPEntry& that)
{
	ID=that.ID;
	AtTime=that.AtTime;
}
MAPEntry& MAPEntry::operator=(MAPEntry& that)
{
	ID=that.ID;
	AtTime=that.AtTime;
	return *this;
}
MAPEntry::~MAPEntry(void)
{
}
