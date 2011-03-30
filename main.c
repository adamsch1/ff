#include <stdlib.h>
#include <unistd.h>

/* Include this or stdio will not work */
#include <fcgi_stdio.h>

#include "ff.h"
#include "route.h"

int main () {
    int count = 0;

    /* Load the controller .so from this sub directory */
    route_import_controllers("controllers/");

    while (FCGI_Accept() >= 0) {
      printf("Content-type: text/html\r\n"
            "\r\n"
            "<title>ff test</title>"
            "<h1>Testing basic controller dispatching</h1>\n"
            "Request number %d,  Process ID: %d<p> %s %s\n", ++count, getpid(),
							getenv("REQUEST_URI")
             );

      /* Dispatch the request to the correct controller */
      route_dispatch(getenv("REQUEST_URI"));
    }

    return 0;
}

