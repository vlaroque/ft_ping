#include "utils.h"

double timespec_diff_in_ms(struct timespec start, struct timespec end)
{
	long seconds = end.tv_sec - start.tv_sec;
	long nano_seconds = end.tv_nsec - start.tv_nsec;

	if (nano_seconds < 0)
	{
		seconds -= 1;
		nano_seconds += 1000000000L;
	}

	return (double)seconds * 1000.0 + (double)nano_seconds / 1000000.0;
}

double ping_sqrt(double number)
{
	if ( number <= 0.0 )
		return (0.0);

	double precision = 0.000001;
	double low       = 0.0;
	double high      = (number < 1.0) ? 1.0 : number; 
	double mid       = 0.0;

	while ( high - low > precision )
	{
		mid = (low + high) / 2.0;

		if (mid * mid < number)
			low  = mid;
		else
			high = mid;
	}
	return (mid);
}
