#include "ff.h"
#include "template.h"

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



  template_run("test.tpl");
}

void comment() {
}





