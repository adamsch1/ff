#include "route.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <dlfcn.h>

/**
 *  This file routes requests to the proper controller
 */

/**
 * Lets us maintain a list of controllers and their functions
 */
struct controller_t {
  struct ff_controller_t *head;
  struct controller_t *next;
};

/* Linked list head */
struct controller_t *controllers = NULL;

static void load_controller( const char *fp );
static int route_invoke( const char *uri, struct controller_t *controller );

/**
 * Import controllers from path subdirectory
 */
void route_import_controllers( const char *path )  {
  struct dirent *dp;
  DIR *dir = opendir( path );
  char *str;

  if( dir == NULL )  {
    fprintf(stderr, "Could not load any .so\n");
    return;
  }

  /* Read listing of files in the subdirectory */
  while((dp=readdir(dir)) != NULL )  {
    /* Look specifically for .so files */
		if( fnmatch("*.so", dp->d_name, FNM_NOESCAPE) == 0 )  {
      /* Construct a useable filepath and try load the .so properly */
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
  void (*initp)();

  /* Load the .so */
  handle = dlopen( fp, RTLD_LAZY);
  if( !handle )  {
    fprintf(stderr, "Could not load %s\n", fp);
    return;
  }

  /* Look for the symbol name */
  head = dlsym(handle, "head");
  if( !head )  {
    fprintf(stderr, "Could not find handlers in %s\n", fp);
    return;
  }


  initp = dlsym(handle, "init");
  if( !initp )  {
    fprintf(stderr, "Could not find init() in %s\n", fp);
    return;
  }
  initp(); 
 
  /* Allocate and link into our list of controllers */
  node = (struct controller_t *)calloc(1,sizeof(struct controller_t));
  node->head = head;
  node->next = controllers;
  controllers = node;
}

/**
 * Call this with the REQUEST_URI and it will invoke the correct 
 * controller/function
 */
int route_dispatch( char *uri)  {
  return route_invoke( uri, controllers );
}

/**
 * Walk our list of controllers and dispatch the correct one
 */
static int route_invoke( const char *uri, struct controller_t *controller )  {
  struct ff_controller_t *head;

  /** No path, most likely invoked form cmd line */
  if( !uri ) return -1;

  /** Once and a while we get to do some recursion **/
  if( controller->next ) {
    route_invoke( uri, controller->next );
  }

  /** Walk this controller list of functions **/
  head = controller->head;  
  while( head && head->route )  {
    /* Check if there is a match */
    if( strstr( uri, head->route ) == uri )  {
      /* There is so invoke the controller function */
      head->ptr();
      return 0;
    }
    head++;
  }

  return -1;
}

