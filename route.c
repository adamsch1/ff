#include <glib.h>
#include "route.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <dlfcn.h>

struct controller_t {
  struct ff_controller_t *head;
  struct controller_t *next;
}

struct controller_t *controllers = 0;

void load_controller( const char *fp );

/**
 * Import controllers from controllers subdirectory
 */
void route_import_controllers( const char *path )  {
  struct dirent *dp;
  DIR *dir = opendir( path );
  char *str;

  if( dir == NULL )  {
    fprintf(stderr, "Could not load any .so\n");
  }

  while((dp=readdir(dir)) != NULL )  {
		if( fnmatch("*.so", dp->d_name, FNM_NOESCAPE) == 0 )  {
      str = alloca( (strlen("controllers")+strlen(dp->d_name))+10 );
      sprintf( str, "controllers/%s", dp->d_name );
      load_controller( str );
    }
  }
}

/**
 * Read in the .so specified by fp.  Links it into our global table of routes
 */
void load_controller( const char *fp )  {
  void *handle;
  void (*ptr)();
  void *d;
  struct ff_controller_t *head;
  struct controller_t *node;

  handle = dlopen( fp, RTLD_LAZY);
  if( !handle )  {
    fprintf(stderr, "Could not load %s\n", fp);
    return;
  }
  head = dlsym(handle, "head");
  if( !head )  {
    fprintf(stderr, "Could not find handlers in %s\n", fp);
    return;
  }

  // Allocate and link
  node = (struct controller_t *)calloc(1,sizeof(struct controller_t));
  node->head = head;
  node->next = controllers;
  controllers = node;
}

/**
 * Given path - go for it
 */
int route_invoke( struct controller_t *controller )  {
  char *uri;
  struct controller_t *iter = controllers;

  while( iter )  {
    head = iter->  
  }
  
  while( head->route )  {
    printf("here");
    if( strstr( uri, head->route ) == uri )  {
      head->ptr();
    }
    head++;
  }

  return -1;
}

