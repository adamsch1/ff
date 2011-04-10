#include <stdlib.h>
#include "sglib.h"
#include <string.h>

typedef struct node_t {
  char *key;
  char *value;

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
 *  Add string - we take ownership of freeing key/value
 */
void array_add_str( struct array_t * arr, char *key, char *value )  {
  node *n = calloc(1, sizeof(node));
  n->key = key;
  n->value = value;
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
