#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/* Include this or stdio will not work */
#include <fcgi_stdio.h>

#include "ff.h"
#include "route.h"

extern struct timeval start_time, end_time, difference_time;
int main () {
    int count = 0;

    /* Load the controller .so from this sub directory */
    route_import_controllers("controllers/");

    while (FCGI_Accept() >= 0) {
      /* Record start time */
      gettimeofday(&start_time,NULL);

      /* Dispatch the request to the correct controller */
      route_dispatch(getenv("REQUEST_URI"));

      /* Calculate total runtime of the operation */
      gettimeofday(&end_time,NULL);
      timeval_diff( &difference_time, &end_time, &start_time );      

      printf("time: %ld.%06ld\n", difference_time.tv_sec, difference_time.tv_usec);
    }

    return 0;
}

