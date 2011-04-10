#include <stdlib.h>
#include "sglib.h"
#include <string.h>

#define IS_STR 0x1
#define IS_OBJ 0x2

typedef struct node_t {
  char *key;
  char *value;
  void *obj;

  int  bits;

  /* Sglib stuff */
  char color_field;
  struct node_t *left;
  struct node_t *right;
} node;

struct array_t {
  node *head;
};

#define COMPARATOR(x,y) (strcmp(x->key,y->key))
SGLIB_DEFINE_RBTREE_PROTOTYPES( node, left, right, color_field, COMPARATOR );
SGLIB_DEFINE_RBTREE_FUNCTIONS( node, left, right, color_field, COMPARATOR );


struct array_t * array_new()  {
  struct array_t *arr = calloc(1,sizeof(struct array_t));
  return arr;
}

/**
 * Get a named value
 */
void * array_get_obj( struct array_t *arr, char *key )  {
  node *n;
  node entry;

  entry.key = key; 
  if( (n=sglib_node_find_member( arr->head, &entry )) == NULL ) { 
    return n->obj;
  } else {
    return NULL;
  }
}

/**
 * Get a named value
 */
char * array_get( struct array_t *arr, char *key )  {
  node *n;
  node entry;

  entry.key = key; 
  if( (n=sglib_node_find_member( arr->head, &entry )) == NULL ) { 
    return NULL;
  } else {
    return n->value;
  }
}

/**
 *  
 */
void array_add_obj( struct array_t *arr, char *key, void *obj ) { 
  node *n = calloc(1, sizeof(node));
  n->key = strdup(key);
  n->obj = obj;
  n->bits |= IS_OBJ;
  sglib_node_add( &arr->head, n );
}

/**
 *  Add string - we take ownership of freeing key/value
 */
void array_add_str( struct array_t * arr, char *key, char *value )  {
  node *n = calloc(1, sizeof(node));
  n->key = strdup(key);
  n->value = strdup(value);
  n->bits |= IS_STR;
  sglib_node_add( &arr->head, n );
}

/**
 * Free everything, including keys and values
 */
void array_free( struct array_t *arr ) { 
  struct sglib_node_iterator it;
  node *np;

  for( np=sglib_node_it_init(&it, arr->head); np != NULL; 
    np=sglib_node_it_next(&it))  {
    free(np->key);
    free(np->value);
    free(np);
  }

  free(arr);
}

#if 0

void main() {
  struct array_t *arr = array_new();
  array_add_str(arr, strdup("key"), strdup("value"));
  array_free( arr);
}
#endif 
