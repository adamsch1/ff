#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "sglib.h"
#define _GNU_SOURCE
#include <stdio.h>

#define COMPARATOR(x,y) (strcmp(x->key,y->key))
SGLIB_DEFINE_RBTREE_FUNCTIONS( node, left, right, color_field, COMPARATOR );

struct array_t * array_new()  {
  struct array_t *arr = calloc(1,sizeof(struct array_t));
  return arr;
}

/**
 * Returns true of array is empty
 */
int array_empty( struct array_t *arr )  {
  return arr->head == NULL ;
}

/**
 * Get array node
 */
node * array_get_node( struct array_t *arr, char *key) { 
  node *n;
  node entry;

  entry.key = key; 
  if( (n=sglib_node_find_member( arr->head, &entry )) != NULL ) { 
    return n;
  } else {
    return NULL;
  }
}

/**
 * Get a named value
 */
void * array_get_obj( struct array_t *arr, char *key )  {
  node *n;
  node entry;

  entry.key = key; 
  if( (n=sglib_node_find_member( arr->head, &entry )) != NULL ) { 
    return n->obj;
  } else {
    return NULL;
  }
}

/**
 * Convert a serialized array back to a C datastructure.
 */
int array_deserialize( struct array_t *arr, char *input )  {
  char *p;
  char *skey;
  char *svalue;

  skey = svalue = p = input;
  while( *p )  {
    if( *p == '\1' )  {
      *p = 0;
      svalue = p+1;
    } else if( *p == '\2' )  {
      *p = 0;
      array_add_str( arr, skey, svalue );
      skey = p+1;
    }
    p++;
  }

  return 0;
}

/**
 * Convert array to string.  We separate keys with 0x1 and values with 0x2
 */
int array_serialize( struct array_t *arr, char **outs )  {
  node *n;
  int count = 0;
  char *str = 0;

  for( n=array_first( arr ); n != NULL;
       n = array_next( arr ) ) {
    str = *outs;
    /* Not portable but my world always involves GNU or FreeBSD */
    count = asprintf( outs, "%s%s\1%s\2", str == NULL ? "" : str,
                      n->key, n->value  );
    if( str ) free(str);
  }

  return count; 
}

/**
 * Macros could be a list of keys, each terminated by 0 with an 
 * extra 0 added to terminate the list.
 */
char * array_get_macro( struct array_t *arr, char *key )  {
  node *n;
  char *p=key;
  struct array_t *iter;

  n = array_get_node( arr, key );
  while( p && n && n->bits & IS_OBJ ) { 
    p += strlen(p)+1;
    if( p )  {
      /* More */
      iter = (struct array_t*)n->obj;
      n = array_get_node( iter, p );
    } else {
      /* Walked the key */
      return n->value;
    }  
  }

  if( n && n->bits & IS_STR )  {
    return n->value;
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
 * Remove the thing - free value if str return OBJ if object or NULL if str
 */
void * array_remove( struct array_t *arr, char *key )  {
  node entry;
  node *result;
  void *obj=NULL;
  int rc;

  entry.key = key;
  rc=sglib_node_delete_if_member( &arr->head, &entry, &result );
  if( rc )  {
    free(result->key);
    if( result->bits & IS_STR ) free(result->value);
    obj = result->obj;
    free(result);
  }
  return obj;
}

/**
 * Add obj.  We allocate copy of key  
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
void array_add_str( struct array_t * arr, char *key, const char *value )  {
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
    if( np->bits & IS_STR ) free(np->value);
    free(np);
  }

  free(arr);
}

node * array_first( struct array_t *arr )  {
  node *np;

  np = sglib_node_it_init(&arr->it, arr->head);
  return np;
}

node * array_next( struct array_t *arr )  {
  return sglib_node_it_next(&arr->it);
}


/**
 * Walk the array, you give the callback
 */
void array_walk( struct array_t *arr, void (*callback)(struct array_t *arr, 
                 int k, node *n ) )  {
  struct sglib_node_iterator it;
  node *np;
  int k=0;

  for( np=sglib_node_it_init(&it, arr->head); np != NULL; 
    np=sglib_node_it_next(&it),k++)  {
    callback( arr, k, np );
  }
  
}

#if 0

void main() {
  struct array_t *arr = array_new();
  int k=0;
  char *out=NULL;
  struct array_t *arr2 = array_new();
  node *n;

  for(k=0; k<10; k++ )  {
    array_add_str( arr, "k", "v" );
  }

  k=array_serialize( arr, &out );
  printf("%d %s \n", k, out );
  
  array_deserialize( arr2, out );
  
  for( n=array_first(arr2); n!=NULL; n = array_next(arr2))  {
    printf("%s %s\n", n->key, n->value );
  }  
 
  free(out);
  array_free( arr);
  array_free( arr2);
}
#endif 
