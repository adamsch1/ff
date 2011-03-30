
#include "fcgi_config.h"
#include <stdlib.h>
#include <dlfcn.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_stdio.h"

#include "ff.h"
#include "route.h"

int main ()
{
    int count = 0;

    route_import_controllers("controllers/");

    while (FCGI_Accept() >= 0) {
      printf("Content-type: text/html\r\n"
            "\r\n"
            "<title>ff test</title>"
            "<h1>Testing basic controller dispatching</h1>\n"
            "Request number %d,  Process ID: %d<p> %s %s\n", ++count, getpid(),
							getenv("REQUEST_URI")
             );
      route_dispatch(getenv("REQUEST_URI"));
    }

    return 0;
}

