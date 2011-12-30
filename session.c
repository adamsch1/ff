#include "route.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "ccgi.h"
#include "session.h"
#include "array.h"

/* Moved to a multi proc data structure later */
struct array_t *sessions;

/* Current session */
struct array_t *sess;

/**
 * Initalize session system 
 **/
void session_init()  {
  sessions = array_new();
}

void session_start()  {
  CGI_varlist *cookies = NULL;
  const char *value;
  char *temp;

  sess = NULL;

  cookies = CGI_get_cookie(NULL);
  if( cookies )  {
    value = CGI_lookup( cookies, "FSESSIONID" );
    if( value )  {
      /* They gave us a session  */
      temp = strdup(value);
      sess = array_get_obj( sessions, temp ); 
      if( sess == NULL )  {
        sess = array_new();
        array_add_obj( sessions, temp, sess ); 
      }
      free(temp);
    } else {
      printf("Set-Cookie: FSESSIONID=1234\r\n");
    }
  } 
}
