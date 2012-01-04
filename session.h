#ifndef __SESSION__H
#define __SESSION__H

/* patterend from PHP sessions */

void session_init();
void session_start();
struct array_t * session_get();
void session_destroy();

#endif
