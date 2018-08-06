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
#include <iostream>
#include <math.h>
#include <stdio.h>


void printgpscsv ();
void read_gps ();
gps_data_t *gps_handle;
savari_gps_data_t gps;
int gps_init();
void populate_gps_values();


extern int msleep(unsigned long milisec);
extern int outputlog(char *output);


// Vairables required for getting GPS 

extern double m_vehlat,m_vehlat_pre;
extern double m_vehlong,m_vehlong_pre;
extern float m_vehspeed,m_vehspeed_pre;
extern double dElevation,dElevation_pre;
extern double dgpsTime, dgpsTime_pre;
extern double dHeading,dHeading_pre;
extern char temp_log[1024];	




int gps_init() 
{
    int is_async = 0;
    int fd;
    gps_handle = savari_gps_open(&fd, is_async);
   if (gps_handle == 0) {
        printf("sorry no gpsd running\n");
        return -1;
    }
   return 0;
}
 



void printgpscsv () 
{
    static int first = 1;
    if (first) 
   {
         printf ("time,latitude,longitude,elevation,speed,heading\n");
         first = 0;
	} 
    printf ("%lf,", gps.time);
    printf ("%lf,", gps.latitude);
    printf ("%lf,", gps.longitude);
    printf ("%lf,", gps.altitude);
    printf ("%lf,", gps.speed);
    printf ("%lf\n", gps.heading);
}
 
 
 
 
 
void read_gps () 
{
     savari_gps_read (&gps, gps_handle);
     printgpscsv (); 
}



void populate_gps_values()
{
//get gps time
	if (isnan(gps.time)==0) 
	{
		dgpsTime=gps.time;
		dgpsTime_pre=dgpsTime;
	}
	else
		dgpsTime=dgpsTime_pre;	
	if (isnan(gps.latitude)==0 && isnan(gps.longitude)==0 && isnan(gps.altitude)==0  )  
	{	
		//m_vehspeed   =  gpsdata->fix.speed;  //------ gps_fix_t: m/s
		m_vehlat_pre=m_vehlat;
		m_vehlong_pre=m_vehlong;
		dElevation_pre=dElevation;
		if( isnan(gps.speed)!=0)
		{
			sprintf(temp_log,"--------GPS Speed Error.--------------At(%.2lf)\n",dgpsTime);
			outputlog(temp_log);
			m_vehspeed = m_vehspeed_pre;
		}
		else
		{
			//if(isnan(gpsdata->fix.speed)!=0)
			m_vehspeed    =  gps.speed;
			m_vehspeed_pre     =  m_vehspeed;
		}
		if( isnan(gps.heading)!=0)
		{
			sprintf(temp_log,"--------GPS Fix Error.--------------At(%.2lf)\n",dgpsTime);
			outputlog(temp_log);
			dHeading = dHeading_pre;
		}
		else
		{
			//if(isnan(gpsdata->fix.track)!=0)
			dHeading      =  gps.heading;  //------ range from [0,360)
			if(m_vehspeed>2.0)
			dHeading_pre         = dHeading;
		}
		m_vehlat	 =  gps.latitude;
		m_vehlong	 =  gps.longitude;
		dElevation   =  gps.altitude;
		if(m_vehspeed<2.0)
		{
			m_vehlat =m_vehlat_pre;
			m_vehlong=m_vehlong_pre;
			dElevation_pre=dElevation;
			dHeading  = dHeading_pre;
		}
	}
	else
	{
		// if there is any error with GPS
		if(isnan(gps.latitude)!=0) {sprintf(temp_log,"--------GPS Lat Error.--------------At(%.2lf)\n",dgpsTime); outputlog(temp_log); }
		if(isnan(gps.longitude)!=0) {sprintf(temp_log,"--------GPS Long Error.--------------At(%.2lf)\n",dgpsTime);outputlog(temp_log); }
		if(isnan(gps.altitude)!=0) {sprintf(temp_log,"--------GPS Altitude Error.--------------At(%.2lf)\n",dgpsTime);outputlog(temp_log); }
		if(isnan(gps.heading)!=0) {sprintf(temp_log,"--------GPS Fix Error.--------------At(%.2lf)\n",dgpsTime);outputlog(temp_log); }
		if(isnan(gps.speed)!=0) {sprintf(temp_log,"--------GPS Speed Error.--------------At(%.2lf)\n",dgpsTime);outputlog(temp_log); }
		msleep(200);
	//	continue;
	}
	sprintf(temp_log," .................................... GPS Position Obtianed Successfully............... "); 
	outputlog(temp_log); 
	sprintf(temp_log,"Lat %.8f Long %.8f Heading  %.3f Speed %.3f. At time %.2lf \n",m_vehlat,m_vehlong,dHeading,m_vehspeed,dgpsTime);		
	outputlog(temp_log);
}

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
