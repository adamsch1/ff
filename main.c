#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

/* Include this or stdio will not work */
#include <fcgi_stdio.h>

#include "ff.h"
#include "route.h"
#include "session.h"

extern struct timeval start_time, end_time, difference_time;
int main () {
    int count = 0;
    char * request_uri;

    /* Load the controller .so from this sub directory */
    route_import_controllers("controllers/");

    /* Initialize session system */
    session_init();

    while (FCGI_Accept() >= 0) {
      /* Record start time */
      gettimeofday(&start_time,NULL);

      /* Initialize session for this request */
      session_start();

      /* Dispatch the request to the correct controller */
      request_uri = cleanrequest( getenv("REQUEST_URI"));
      route_dispatch(request_uri);
      free(request_uri);
     
      /* Calculate total runtime of the operation */
      gettimeofday(&end_time,NULL);
      timeval_diff( &difference_time, &end_time, &start_time );      

      printf("time: %ld.%06ld\n", difference_time.tv_sec, difference_time.tv_usec);
    }

    return 0;
}
