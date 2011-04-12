#ifndef __ARRAY__H
#define __ARRAY__H

#include "sglib.h"




#define IS_STR 0x1
#define IS_OBJ 0x2

typedef struct node_t {
  /* Key, value - we allocate and free except for obj */
  char *key;
  char *value;
  void *obj;

  /* And with IS_ above to figure out what it is */
  int  bits;

  /* Sglib stuff, don't touch */
  char color_field;
  struct node_t *left;
  struct node_t *right;
} node;

/* SGLIB declarations - arrays here are actaully red-black trees */
SGLIB_DEFINE_RBTREE_PROTOTYPES( node, left, right, color_field, COMPARATOR );

struct array_t {
  node * head;
  struct sglib_node_iterator it;
};

struct array_t * array_new();
void array_add_str( struct array_t *arr, char *key, char *value );
void array_free( struct array_t *arr );
node * array_get_node( struct array_t *arr, char *key );
char * array_get( struct array_t *arr, char *key );
char * array_get_macro( struct array_t *arr, char *key );
void * array_get_obj( struct array_t *arr, char *key );
int array_empty( struct array_t *arr );

/* Iterator support */
node * array_first( struct array_t *arr );
node * array_next( struct array_t *arr );

#endif
