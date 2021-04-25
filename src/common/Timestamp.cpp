#include <string>
#include <sys/time.h>

std::string getVerboseTimestamp()
{
    timeval curTime;

    gettimeofday(&curTime, NULL);

    long int milli = curTime.tv_usec / 1000;
    char buf[sizeof "2011-10-08 07:07:09.000"];
    strftime(buf, sizeof buf, "%F %T", gmtime(&curTime.tv_sec));
    sprintf(buf, "%s.%ld", buf, milli);

    return buf;
}  

double getPosixTimestamp()
{

	struct timeval tv_tt;
	gettimeofday(&tv_tt, NULL);
	return (static_cast<double>(tv_tt.tv_sec)+static_cast<double>(tv_tt.tv_usec)/1.0e6);  

}
