#include "time.h"

/*
 * return:	[0-9]{4}|[0-9]{6} -- sec|usec
 */
long int
gettime()
{
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec % 10000) * 100000 + tv.tv_usec/10;
}
