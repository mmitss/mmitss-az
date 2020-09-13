/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  MAPEntry.cpp  
 *  Created by Mehdi Zamanipour
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
#pragma  once
#include <iostream>
#include <string>
using namespace std;
class MAPEntry
{
	public:
		long ID;	 // ID of the parsed MAP
		time_t AtTime;              //  Time stamp recording when the entry added 
	public:
		MAPEntry();
		MAPEntry(long id, time_t time);
		MAPEntry(MAPEntry& that);
		MAPEntry& operator=(MAPEntry& that);
	public:
		~MAPEntry();
		friend ostream &operator <<(ostream &stream, MAPEntry e)
		{
			stream<<"MAP ID is: "<<e.ID<<", at time: "<<e.AtTime<<endl;
			return stream;
		}
};
