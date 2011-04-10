#include "ff.h"
#include "template.h"
#include "array.h"
#include "ccgi.h"
#include "util.h"

void blog();
void comment();

/**
 * Tell ff what we provide
 */
struct ff_controller_t head[] = {
  { "/blog", blog  },
  { "/blog/comments", comment },
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


  array_add_str( arr, "dog", "GONZO!"); 
  array_add_str( arr, "house", "sucks"); 
  template_run("test.tpl", arr);
  comment();
}

void comment() {
  CGI_varlist *varlist;
  CGI_value *value;
  const char *name;
  int k;

  printf("Content-type: text/html\r\n\r\n");

  varlist = CGI_get_all(0);
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

