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
char *id;

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
      } else {
        fprintf(stderr, "Fetching prior session from memory: %s\n",
          value);
      }
      free(temp);
    } else {
      char buff[255];
      /* XXX Fix */
      snprintf( buff, sizeof(buff), "%ld", time(0));
      sess = array_new();
      array_add_str( sess, "FSESSIONID", buff );
      array_add_obj( sessions, buff, sess ); 
      printf("Set-Cookie: FSESSIONID=%s\r\n", buff );
    }
  } 
}

struct array_t * session_get()  {
  return sess;
}

void session_destroy()  {
  char *id;
  fprintf( stderr, "session_destroy()\n");
  if( sess )  {
    fprintf( stderr, "have session\n");
    id = array_get( sess, "FSESSIONID" );  
    if( !id ) { 
      fprintf(stderr, "Have session but no FSESSIONID set!\n");
    } else {
      fprintf( stderr, "Removing session: %s\n", id );
      array_remove( sessions, id );
      array_free( sess );
    }
    printf("Set-Cookie: FSESSIONID=None; expires=Friday, 31-Dec-2010 08:00:00 GMT\r\n" );
  }
}




