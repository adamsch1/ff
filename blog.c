#include "ff.h"

void blog();
void comment();

struct ff_controller_t head[] = {
  { "/blog", blog  },
  { "/comments", comment },
  { 0 }
};

void blog()  {
  printf("HERE!");
}

void comment() {
}





