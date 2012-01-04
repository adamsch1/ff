#include<stdlib.h>

#include <stdio.h>

struct foo_t {
  struct foo_t *next;
  struct foo_t *child;
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
    return (char*)t + HSIZE;
  } else {
    t = malloc(size+HSIZE);
    if( !t ) return 0;
    t->child = 0;
    t->next = B(ctx)->child;
    B(ctx)->child = t;
    return (char*)t + HSIZE;
  }
}


/**
 * This only deals with ctx that points to beginning of mem block
 **/
static void _ffree( struct foo_t *ctx )  {

  if( ctx->child )  {
    _ffree( ctx->child );
  } 

  if( ctx->next ) _ffree( ctx->next );

  free( ctx );
}

void ffree( void *ctx )  {
  /* Pass in pointer to start of memory block to free */
  if( ctx ) _ffree( B(ctx) );
}


#if 0
int main() { 
  char *p = NULL;
  char *s = NULL;

  p = falloc( NULL, 255 );
  s = falloc( p, 255 ); 
  s = falloc( p, 255 ); 
  s = falloc( p, 255 ); 
  s = falloc( p, 255 ); 
  s = falloc( p, 255 ); 
  s = falloc( s, 255 ); 
  printf("p=%x\n",p);
  printf("s=%x\n",s);

  ffree(p);
  return 0;
}
#endif
