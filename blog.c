#include "ff.h"
#include "template.h"
#include "array.h"
#include "ccgi.h"
#include "util.h"
#include "form.h"
#include "session.h"
#include <stdlib.h>

void blog();
void comment();
void blog_post();
void logout();

/**
 * Tell ff what we provide
 */
struct ff_controller_t head[] = {
  { "/blog", blog  },
  { "/blog/post", blog_post },
  { "/blog/comments", comment },
  { "/blog/logout", logout },
  { 0 }
};

/**
 * Initialize template cache
 */
void init()  {
  template_init();
}

/**
 * Ubiquitous, dumbass blog 
 * 
 * Going forward - don't allocate memory if possiblem ask FF for resource
 * it can free everything after the call
 */
void blog()  {
  struct array_t *arr = array_new();

  template_run("test.tpl", arr);
}

void logout()  {
  fprintf( stderr, "Logout: Calling session_destroy\n");
  session_destroy();
}

void blog_post() {
  struct form_t *form = form_new();

  form_set_rule( form, "email", "Email", RULE_REQUIRED|RULE_EMAIL )  ;
  form_set_rule_sval( form, "password", "Password", 
                      RULE_REQUIRED|RULE_SMIN, 6 );

  if( form_validate( form ) )  {
    template_run( "test.tpl", 0 );
    printf("<b>Success</b><br> \n");
  } else {
    template_run_form( "test.tpl", form, 0 );
  }

  form_free( form );
}

void comment() {
  CGI_varlist *varlist;
  CGI_value *value;
  const char *name;
  int k;


  varlist = CGI_get_post(0,0);
  printf("Content-type: text/html\r\n\r\n");
  if( varlist ) { 
    for( name=CGI_first_name(varlist); name != 0; 
         name=CGI_next_name(varlist))  {
      value = CGI_lookup_all(varlist, 0);
      for( k=0; value[k] != 0; k++ )  {
        printf("%s [%d] = %s\r\n", name, k, value[k]);
      }
    }
  }
}

