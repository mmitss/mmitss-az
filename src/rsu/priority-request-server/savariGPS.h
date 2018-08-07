/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  savariGPS.h
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


#include <libgps.h>



gps_data_t *gps_handle;
savari_gps_data_t gps;




/*
int gps_init()
{
	gpsdata = gps_open("127.0.0.1", "2947");
	if (!gpsdata) {
		printf("Error: Failed to connect to gps\n");
		return -1;
	}
	gps_query(gpsdata, "o\n"); // update fix once 
	usleep(1000*1000); // give it sometime to get first fix 
	return 0;
}

*/
