#ifndef __FF__H
#define __FF__H

#include <fcgi_stdio.h>
/**
 * 
 *
 *
 */
struct ff_controller_t {
  char * route;
  void (*ptr)();
}; 

#endif
