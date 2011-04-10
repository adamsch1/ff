#include <stdlib.h>
#include <string.h>
#include "form.h"
#include "array.h"

/**
 * Heavily inspired by Codeigniter which I like alot.
 */
struct rule_t {
  char *display_name;
  char *rule;
};

void set_rule( struct form_t *form, char *field_name, char *display_name, 
               char *rule )  {

  struct rule_t * r = calloc(1,sizeof(struct rule_t));
  r->display_name = strdup(display_name);
  r->rule = strdup(rule);
  array_add_obj( form->arr, field_name, r );
}

struct form_t * form_new()  {
  struct form_t *form = calloc(1,sizeof(struct form_t));
  form->arr = array_new();
  form->setrule = set_rule;
}
