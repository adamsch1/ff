#ifndef __RULE__H
#define __RULE__H

#define BITNUM( n ) ( 0x1 << n )

#define RULE_REQUIRED BITNUM(0)
#define RULE_EMAIL    BITNUM(1)
#define RULE_IMIN     BITNUM(2)
#define RULE_IMAX     BITNUM(3)
#define RULE_FMIN     BITNUM(4)
#define RULE_FMAX     BITNUM(5)

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
void form_set_rule_ival( struct form_t *form, char *field_name, char *display_name, int rule, int val );
void form_set_rule_fval( struct form_t *form, char *field_name, char *display_name, int rule, float val );
int form_validate( struct form_t *form );
int form_validate( struct form_t *form );
int form_free( struct form_t *form );

#endif
