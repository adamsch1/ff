#ifndef __UTIL__H
#define __UTIL__H

char * nully( char *);
void dumpcgi();
char * cleanrequest( char * request_uri );
long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time
            );


#endif
