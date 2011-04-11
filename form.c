#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcgi_stdio.h>
#include <regex.h>
#include "form.h"
#include "array.h"
#include "ccgi.h"

/**
 * Heavily inspired by Codeigniter which I like alot.
 */
struct rule_t {
  char *display_name;
  int rule;
};

int check_and_set_required( struct form_t *form, struct rule_t *rule, 
                             node * np  );
int check_and_set_email( struct form_t *form, struct rule_t *rule,
                         node *np );

/* All the canned rules */
struct rules_t {
  int check_flag;
  int (*checkf)( struct form_t *form, struct rule_t *rule, node *np );
} rules[] = {
  { RULE_REQUIRED, check_and_set_required },
  { RULE_EMAIL, check_and_set_email }
};

#define RULE_COUNT (sizeof( rules)/sizeof(struct rules_t))


/**
 * Give field name, display name for errors and a | deliminted set of rules
 * when I figure out what those are I'll document them
 */
void form_set_rule( struct form_t *form, char *field_name, char *display_name, 
                    int rule )  {

  struct rule_t * r = calloc(1,sizeof(struct rule_t));
  r->display_name = strdup(display_name);
  r->rule = rule;
  array_add_obj( form->arr, field_name, r );
}

/**
 * Make an ew form
 */
struct form_t * form_new()  {
  struct form_t *form = calloc(1,sizeof(struct form_t));
  form->arr = array_new();
  form->err = array_new();
  return form;
}

int errors=0;

#define CHECK( r, f ) ( r->rule & f )


int check_and_set_required( struct form_t *form, struct rule_t *rule, 
                             node * np  )  {
  char buff[255];

  const char * value = CGI_lookup( form->cgi, np->key );
  if( !value || strlen(value) == 0 )  {
    sprintf( buff, "%s is required", rule->display_name );
    array_add_str( form->err, np->key, buff );
    return 1;
  }
 
  return 0; 
}

int check_and_set_email( struct form_t *form, struct rule_t *rule,
                         node *np ) { 
  static regex_t re;
  static compiled=0;
  char buff[255];
  const char * value;

  if( compiled == 0 ) { 
    /* http://www.regular-expressions.info/email.html */
    regcomp(&re,"\\b[A-Z0-9._%-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", 
            REG_EXTENDED|REG_ICASE|REG_NOSUB );
    compiled = 1;
  }

  value = CGI_lookup( form->cgi, np->key );
  if( !value || regexec(&re, value, 0, NULL, 0 ) != 0 )   {
//  if( !value || strchr( value, '@') == NULL ) {
    sprintf( buff, "%s is not a valid email address", rule->display_name );
    array_add_str( form->err, np->key, buff );
    return 1;
  }

  return 0; 
}

/**
 * Validate the form - applies each rule, cleans up input etc.
 * returns 0 on success, 1 if failure;
 */
int form_validate( struct form_t *form )  {
  node *np;
  const char *vp;
  struct rule_t *rule;
  int k;

  /* Prep form */ 
  form->cgi = CGI_get_all(NULL); 
  form->success = 0;

  /* Iterate across all the rules */
  np = array_first( form->arr );
  while( np )  {
    /* Reference everythign we will need to check */
    rule = (struct rule_t*)np->obj;

    /* Run all the rules that match */
    for( k=0; k<RULE_COUNT; k++ )  {
      if( CHECK( rule, rules[k].check_flag ) ) {
        form->success |= rules[k].checkf( form, rule, np );
      }
    }

    np = array_next( form->arr );
  }  

  return form->success;
}

/**
 * Free the rule
 */
static int free_rule( struct array_t *arr, int k, node *n )  {
  if( n->bits & IS_OBJ )  {
    struct rule_t *r = n->obj;
    free( r->display_name );
    free( n->obj );
  }
}

/**
 * Free the form
 */
int form_free( struct form_t *form )  {
  array_walk( form->arr, free_rule );
  array_free( form->arr );
  array_walk( form->err, free_rule );
  array_free( form->err );
  free(form);
}

#if 0

#include <stdio.h>
void main()  {
  struct form_t *f = form_new();
  int v;

  form_set_rule( f, "email", "Email", RULE_REQUIRED);
  form_set_rule( f, "password", "Password", RULE_REQUIRED|RULE_EMAIL);

  v = form_validate( f );
  form_free( f );

  printf("Validated: %d\n", v );
}

#endif
