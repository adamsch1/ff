#include<stdlib.h>

#include <stdio.h>

struct foo_t {
  struct foo_t *next;
  struct foo_t *child;
  struct foo_t *parent;
};

#define HSIZE ( sizeof(struct foo_t) )
#define B( p ) ( (struct foo_t*)((char*)p - HSIZE) )


/**
 * Poor man's hierarchical allocator 
 *
 * Allocate first passing in NULL for ctx
 *
 * data = falloc( NULL, 100 )
 * string1 = falloc( data, 10 )
 * string2 = falloc( data, 10 )
 *
 * ffree(data)
 *
 * Both string1 and string2 and data are free'd
 *
 **/
void * falloc( void *ctx, size_t size )  {
  struct foo_t *t;

  if( !ctx )  {
    t = malloc(size + HSIZE);
    if( !t ) return 0;
    t->child = 0;
    t->next = 0;
    t->parent = 0;
    return (char*)t + HSIZE;
  } else {
    t = malloc(size+HSIZE);
    if( !t ) return 0;
    t->child = 0;
    t->next = B(ctx)->child;
    t->parent = B(ctx);
    B(ctx)->child = t;
    return (char*)t + HSIZE;
  }
}


/**
 * This only deals with ctx that points to beginning of mem block
 **/
static void _ffree( struct foo_t *ctx )  {
  struct foo_t *iter;
  struct foo_t *prior;

  if( ctx->child )  {
    _ffree( ctx->child );
  } 

  if( ctx->next ) {
    _ffree( ctx->next );
  }

  if( ctx->parent == NULL )  {
    free(ctx); 
  } else {
    iter = ctx->parent->child;
    while( iter )  {
      prior = iter;
      iter = iter->next;
      if( iter == ctx )  {
        prior->next = iter->next; 
      }
    }   
    free( ctx );
  }
}

void ffree( void *ctx )  {
  /* Pass in pointer to start of memory block to free */
  if( ctx ) _ffree( B(ctx) );
}


#if 1
int main() { 
  char *p = NULL;
  char *s = NULL;

  p = falloc( NULL, 255 );
  s = falloc( p, 255 ); 
  printf("p=%x\n",p);
  printf("s=%x\n",s);

  ffree(s);
  ffree(p);
  return 0;
}
#endif
