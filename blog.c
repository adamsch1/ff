#include "ff.h"
#include "template.h"
#include "array.h"

void blog();
void comment();

/**
 * Tell ff what we provide
 */
struct ff_controller_t head[] = {
  { "/blog", blog  },
  { "/comments", comment },
  { 0 }
};

/**
 * Initialize template cache
 */
void init()  {
  template_init();
}

/**
 * Ubiquitous, dumbass blog 
 * 
 * Going forward - don't allocate memory if possiblem ask FF for resource
 * it can free everything after the call
 */
void blog()  {
  struct array_t *arr = array_new();

  array_add_str( arr, "dog", "GONZO!"); 
  array_add_str( arr, "house", "sucks"); 
  template_run("test.tpl", arr);
}

void comment() {
}

