#include <string>
#include <sys/time.h>

std::string getTimestamp()
{
    timeval curTime;

    gettimeofday(&curTime, NULL);

    int milli = curTime.tv_usec / 1000;
    char buf[sizeof "2011-10-08 07:07:09.000"];
    strftime(buf, sizeof buf, "%F %T", gmtime(&curTime.tv_sec));
    sprintf(buf, "%s.%d", buf, milli);

    return buf;
}  

