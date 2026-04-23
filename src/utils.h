#ifndef UTILS_H
#define UTILS_H

#include <time.h>	   /* timespec */

double timespec_diff_in_ms(struct timespec start, struct timespec end);
double ping_sqrt(double number);


#endif /* UTILS_H */
