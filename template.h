#ifndef __TEMPLATE__H
#define __TEMPLATE__H

#include "array.h"

struct chunk_t {
  char *text;
  int  macro: 1;
  int  isif: 1;
  struct chunk_t *next;
};

int template_init();
struct chunk_t * template_parse( char * source );
struct chunk_t * template_load( char * source );

/* Display this shit */
void template_run( char *source, struct array_t *arr );
#endif
