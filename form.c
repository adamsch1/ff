#include <stdlib.h>
#include <stdio.h>
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

/**
 * Give field name, display name for errors and a | deliminted set of rules
 * when I figure out what those are I'll document them
 */
void form_set_rule( struct form_t *form, char *field_name, char *display_name, 
                    char *rule )  {

  struct rule_t * r = calloc(1,sizeof(struct rule_t));
  r->display_name = strdup(display_name);
  r->rule = strdup(rule);
  array_add_obj( form->arr, field_name, r );
}

/**
 * Make an ew form
 */
struct form_t * form_new()  {
  struct form_t *form = calloc(1,sizeof(struct form_t));
  form->arr = array_new();
  return form;
}

int errors=0;

static void walk( struct array_t *arr, int k, node *n )  {
  printf("%d %s\n", k,n->key );
}

/**
 * Validate the form - applies each rule, cleans up input etc.
 * returns 0 on success, -1 if failure;
 */
int form_validate( struct form_t *form )  {

  array_walk( form->arr, walk );

  return errors == 0 ? 0 :  -1;
}

/**
 * Free the rule
 */
static int free_rule( struct array_t *arr, int k, node *n )  {
  if( n->bits & IS_OBJ )  {
    struct rule_t *r = n->obj;
    free( r->display_name );
    free( r->rule );
    free( n->obj );
  }
}

/**
 * Free the form
 */
int form_free( struct form_t *form )  {
  array_walk( form->arr, free_rule );
  array_free( form->arr );
  free(form);
}

#if 1

#include <stdio.h>
void main()  {
  struct form_t *f = form_new();
  int v;

  form_set_rule( f, "email", "Email", "required");
  form_set_rule( f, "password", "Password", "password");

  v = form_validate( f );
  form_free( f );

  printf("Validated: %d\n", v );
}

#endif
