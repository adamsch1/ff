#ifndef __ARRAY__H
#define __ARRAY__H


struct array_t {
  void * head;
};

struct array_t * array_new();
void array_add_str( struct array_t *arr, char *key, char *value );
void array_free( struct array_t *arr );
char * array_get( struct array_t *arr, char *key );
void * array_get_obj( struct array_t *arr, char *key );

#endif
