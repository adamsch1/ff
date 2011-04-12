#ifndef __TEMPLATE__H
#define __TEMPLATE__H

#include "array.h"
#include "form.h"

struct chunk_t {
  char *text;
  int  macro: 1;
  int  isif: 1;
  int  endif: 1;
  struct chunk_t *next;
};

int template_init();
struct chunk_t * template_parse( char * source );
struct chunk_t * template_load( char * source );

/* Display this shit */
void template_run( char *source, struct array_t *arr );
void template_run_form( char *source, struct form_t *form, 
                        struct array_t *arr );
#endif
