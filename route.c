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
};

struct controller_t *controllers = NULL;

static void load_controller( const char *fp );
static int route_invoke( const char *uri, struct controller_t *controller );

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
static void load_controller( const char *fp )  {
  void *handle;
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

int route_dispatch( char *uri)  {
  return route_invoke( uri, controllers );
}

/**
 * Given path - go for it
 */
static int route_invoke( const char *uri, struct controller_t *controller )  {
  struct ff_controller_t *head;

  /** Once and a while we get to do some recursion **/
  if( controller->next ) {
    route_invoke( uri, controller->next );
  }

  head = controller->head;  
  while( head && head->route )  {
    if( strstr( uri, head->route ) == uri )  {
      head->ptr();
      return 0;
    }
    head++;
  }

  return -1;
}

