#ifndef __RULE__H
#define __RULE__H

struct form_t {
  struct array_t *arr;
  void (*setrule)( struct form_t *form, char *field_name, char *display_name, 
                   char *rule);
};

struct form_t * form_new();

#endif
