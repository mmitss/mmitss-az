/***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  GeoCoord.h
  Created by: K. Larry Head  
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

***************************************************************************************/

/*geoCoord is a utility class to translate between WGS84 and ECEF
  Reference:

  */
#include <cmath>
class geoCoord {
private:
	//parameters for conversion
	double a{}; //meters
	double b{}; //meters
	double f{};
	double inv_f{};
	double e{};
	double N{};
	double pi{};

		//longitude and latitude origin

	double longitude_init{};
	double latitude_init{};
	double altitude_init{};
	double ex_init{};
	double ey_init{};
	double ez_init{};
	double	R_sinLat{};
	double	R_cosLat{};
	double	R_sinLong{};
	double	R_cosLong{};


	
		//longutude and latitude in radians
	double longitude_r{};
	double latitude_r{};

public:
	//longitude, latitude, and altitude 
		//double longitude, latitude, altitude ; /* decimal representation */
	double longitude_degrees{};
	double longitude_minutes{};
	double  longitude_seconds{};
	int latitude_degrees{};
	int latitude_minutes{};
	double  latitude_seconds{};
	
 geoCoord::geoCoord() {
		a = 6378137.0 ; //meters
		b = 6356752.3142 ; //meters
		f= 1.0 - (b/a) ; //(a-b)/a ; 
		inv_f = 1.0/f ;
		e = sqrt(f*(2-f)) ;
		pi = (double) 4.0*atan( (double)1.0);
 }

public:
	/* init(...) used to set the base location used for all transformations and represents the origin in the local tanget plane. 
	This method must be called prior to any other conversion. */
	
	void init(double longitude, double latitude, double altitude) ;

	void local2ecef(double x, double y, double z, double *ex, double *ey, double *ez) ;

	/* dms2d(...) converts a position from degree, minutes, seconds to degrees */

	double dms2d(double degree, double minutes, double seconds) ;

	/*	lla2ecef(...) converts longitude, latitude, and altitude to earth-centered, earth-fixed using 
		the base location as the origin */

	void lla2ecef(double longitude, double latitude, double altitude, double *x, double *y, double *z) ;
	
	/*	ecef2lla(...) converts from earth-center, earth-fixed to longitude, latitude, and altitude in degrees*/
	
	void ecef2lla(double x, double y, double z, double *longitude, double *latitude, double *altitude) ;
	//double d2dms(double degree, int idegree, double minutes, double seconds) ;


} ;

