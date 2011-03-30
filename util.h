#ifndef __UTIL__H
#define __UTIL__H

long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time
            );


#endif
