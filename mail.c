#include <libesmtp.h>
#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/**
 * Send an email - inspired by PHP mail function
 */
int mail(char *to, char *subject, char *message, struct array_t *headers )  {
  node *n;
  FILE *fp;
  smtp_session_t sess = smtp_create_session();
  smtp_message_t mess = smtp_add_message( sess );

  smtp_set_server( sess, "localhost:25" );
  smtp_set_header( mess, "Subject", NULL, subject );
  smtp_add_recipient( mess, to );

  if( headers )  {
    for( n = array_first( headers ); n != NULL; 
         n = array_next(headers) )  {
      smtp_set_header( mess, NULL, n->value );
    }
  }

  if ((fp = fopen ("./test.tpl", "r")) == NULL)
    {
      fprintf (stderr, "can't open  %s\n", strerror (errno));
      exit (1);
    }
  smtp_set_message_fp (mess, fp);

  if( !smtp_start_session( sess ) )  {
    char buff[255];
    printf("Could not send email: %s\n", smtp_strerror( smtp_errno(), buff,
           sizeof(buff)));
  }

  smtp_destroy_session(sess);
}


int main() {
  mail( "shane@localhost", "Dude", "DUDE", NULL );
}
