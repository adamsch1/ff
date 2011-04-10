#include <unistd.h>
#include <stdlib.h>
#include <fcgi_stdio.h>
#include <string.h>

struct timeval start_time;
struct timeval end_time;
struct timeval difference_time;
extern char **environ;

// Make a NULL pointer into a empty string
char * nully( char * p )  {
  if( p == NULL )  {
    p = malloc(1);
    *p = 0;
  }
  return p;
}

/* Strip trailing / and query params etc */
char * cleanrequest( char * request_uri )  {
  char * r = strdup( request_uri );
  char *p;

  /* Cut past query params */
  p = strchr( r, '?' );
  if( p ) *p = 0;
  else return r;

  /* Strip trailing slash */
  if( p != r && *(p-1) == '/' )  {
    *(p-1) = 0;
  }
 
  return r;
}

/* Dump all CGI env variables */
void dumpcgi()  {
  char **e = environ;
  int k=0;

  printf("Content-type: text/html\r\n\r\n");

  while( e[k] )  { 
    printf( "CGI: %s\n", e[k] );
    k++;
  }
}

/* Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.  
   Got this from GNU documents for libc
*/

int
timeval_diff (result, x, y)
     struct timeval *result, *x, *y;
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

#if 0
void main()  {
  dump_cgi();
}
#endif
