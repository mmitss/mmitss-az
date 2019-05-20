

/*NOTICE:  Copyright 2017 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

/*  Brakes.h  
 *  Created by Mehdi Zamanipour on 5/15/17.
 *  University of Arizona
 *  ATLAS Research Center
 *  College of Engineering
 *
 *  This code was develop under the supervision of Professor Larry Head
 *  in the ATLAS Research Center.
 *
 *  Revision History:
 *  The file provides a dummy interface for using Savari gps librararies in case the PriorityRequestServer application is being used in lab environment where there is no Savari device.
 *  
 */

#pragma once

struct savari_gps_data_t {
    double time;

};

struct gps_data_t {

};

gps_data_t *gps_handle;
savari_gps_data_t gps;

void printgpscsv();

void read_gps();

int gps_init() {
    return 0;
}

void printgpscsv() {

}

void read_gps() {

}

void populate_gps_values() {

}

gps_data_t *savari_gps_open(int *a, int b) {
    return 0;
}


double savari_gps_read(savari_gps_data_t *gps, gps_data_t *gps_handle) {
    return 0;
}
		
