#include <iostream>
#include <ctime>
#include <sys/time.h>
using namespace std;
// #define MST (-7)
// #define UTC (0)
// #define CCT (+8)

int main()
{
	time_t t = time(NULL);
	tm* timePtr = gmtime(&t);
	cout << "seconds= " << timePtr->tm_sec << endl;
	cout << "minutes = " << timePtr->tm_min << endl;
	cout << "hours = " << timePtr->tm_hour << endl;
	cout << "day of month = " << timePtr->tm_mday << endl;
	cout << "month of year = " << timePtr->tm_mon << endl;
	cout << "year = " << timePtr->tm_year+1900 << endl;
	cout << "weekday = " << timePtr->tm_wday << endl;
	cout << "day of year = " << timePtr->tm_yday << endl;
	cout << "daylight savings = " << timePtr->tm_isdst << endl;

	int dayOfYear = timePtr->tm_yday;
	int currentHour = timePtr->tm_hour;
	int currentMinute = timePtr->tm_min;
	double currentSecond = timePtr->tm_sec;
	int minuteOfYear = (dayOfYear -1)*24*60 +currentHour*60+ currentMinute;
	double msOfMinute = currentSecond*1000.00;

	cout<<"minuteOfYear"<<minuteOfYear<<endl;
	cout<<"msOfMinute"<<msOfMinute<<endl;


	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	cout<<"msOfMinute"<<ms<<endl;


	
  // time_t rawtime;
  // struct tm * ptm;

  // time ( &rawtime );

  // ptm = gmtime ( &rawtime );

  // puts ("Current time around the World:");
  // printf ("Phoenix, AZ (U.S.) :  %2d:%02d\n", (ptm->tm_hour+MST)%24, ptm->tm_min);
  // printf ("Reykjavik (Iceland) : %2d:%02d\n", (ptm->tm_hour+UTC)%24, ptm->tm_min);
  // printf ("Beijing (China) :     %2d:%02d\n", (ptm->tm_hour+CCT)%24, ptm->tm_min);
	

  

  return 0;

}