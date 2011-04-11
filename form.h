#ifndef __RULE__H
#define __RULE__H

#define RULE_REQUIRED 0x1
#define RULE_EMAIL    0x2

#include "array.h"
#include "ccgi.h"

struct form_t {
  struct array_t *arr;
  struct array_t *err;
  int success;
  CGI_varlist *cgi;
};

struct form_t * form_new();
void form_set_rule( struct form_t *form, char *field_name, char *display_name,
                    int rule );
int form_validate( struct form_t *form );
int form_free( struct form_t *form );

#endif
