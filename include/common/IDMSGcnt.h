/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   



/* IDMSGcnt.h
*  Created by :Jun Ding
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.

*/

#pragma once

#include <fstream>
#include <iostream>

using namespace std;

class IDMSGcnt
{
public:
	int TempID;
	int MSGcnt;  // 0-127
public:
	IDMSGcnt()
	{
		TempID=0;
		MSGcnt=0;
	}
	IDMSGcnt(int id,int msgcnt)
	{
		TempID=id;
		MSGcnt=msgcnt;
	}
	IDMSGcnt(const IDMSGcnt& that)
	{
		TempID=that.TempID;
		MSGcnt=that.MSGcnt;
	}
public:
	~IDMSGcnt(){}
	friend ostream &operator <<(ostream &stream, IDMSGcnt e)
	{
		stream<<"TempID is: "<<e.TempID<<", MSGcnt is: "<<e.MSGcnt<<endl;
		return stream;
	}
};