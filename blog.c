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
  struct chunk_t *t = template_load("test.tpl");
  while( t )  {
    printf("%s",t->text);
    t=t->next;
  }
}

void comment() {
}





