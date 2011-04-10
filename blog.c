#include "ff.h"
#include "template.h"
#include "array.h"

void blog();
void comment();

struct ff_controller_t head[] = {
  { "/blog", blog  },
  { "/comments", comment },
  { 0 }
};

void init()  {
  template_init();
}

void blog()  {
  struct array_t *arr = array_new();

  array_add_str( arr, "dog", "GONZO!"); 
  array_add_str( arr, "house", "sucks"); 
  template_run("test.tpl", arr);
}

void comment() {
}

