#ifndef __REPL__H
#define __REPL__H

#include "array.h"

struct repl_t {
  struct array_t *file_chunks;
  struct array_t *kv;

  /* Rollover callback */
  void (*callback)(struct repl_t *r);
};

int repl_init( struct repl_t *r, const char *chunkpath );
int repl_quit( struct repl_t *r );
int repl_append( time_t now, char *key, char *data );
int repl_rollover_callback( struct repl_t *r, 
      void (*calllback)( struct repl_t *r ) );
#endif
