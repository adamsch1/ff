#ifndef __ARRAY__H
#define __ARRAY__H



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

struct array_t {
  node * head;
};

struct array_t * array_new();
void array_add_str( struct array_t *arr, char *key, char *value );
void array_free( struct array_t *arr );
char * array_get( struct array_t *arr, char *key );
void * array_get_obj( struct array_t *arr, char *key );

#endif
