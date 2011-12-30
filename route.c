#include "route.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

/**
 *  This file routes requests to the proper controller
 */

/**
 * Lets us maintain a list of controllers and their functions
 */
struct controller_t {
  struct ff_controller_t *head;
  struct controller_t *next;
  void *handle;
  time_t time;
  char *fp;
};

/* Linked list head */
struct controller_t *controllers = NULL;

/* Where are we loading from */
char *load_path=0;

static void load_controller( const char *fp, struct controller_t *prior );
static int route_invoke( const char *uri, struct controller_t *controller );

/**
 * Import controllers from path subdirectory
 */
void route_import_controllers( const char *path )  {
  struct dirent *dp;
  DIR *dir = opendir( path );
  char *str;

  if( load_path )  {
    free( load_path );
    load_path = strdup(path); 
  }

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
      load_controller( str, NULL );
    }
  }

  closedir(dir);
}

/**
 * Read in the .so specified by fp.  Links it into our global table of routes
 */
static void load_controller( const char *fp, struct controller_t *prior )  {
  void *handle;
  struct ff_controller_t *head;
  struct controller_t *node;
  void (*initp)();

  if( prior )  {
    dlclose(prior->handle);
  }

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


  /* Call init immediatly */
  initp = dlsym(handle, "init");
  if( !initp )  {
    fprintf(stderr, "Could not find init() in %s\n", fp);
    return;
  }
  initp(); 

  if( prior )  {
    /* This was already loaded so re-init */
    prior->head = head;
    prior->time = time(0);
    prior->handle = handle;
  } else { 
    /* Allocate and link into our list of controllers */
    node = (struct controller_t *)calloc(1,sizeof(struct controller_t));
    node->head = head;
    node->next = controllers;
    node->handle = handle;
    node->fp = strdup(fp);
    node->time = time(0);
    controllers = node;
  }
}

/**
 * Check if so needs reloading
 **/
static int needs_reloading(struct controller_t *controller )  {
  struct stat sbuf;
  int rc;

  rc = stat( controller->fp, &sbuf );
  if( rc != 0 )  {
    fprintf(stderr, "Could not restat: %s\n", controller->fp );
    return 0;
  }

  if( sbuf.st_mtime > controller->time )  {
    printf("Need to reload\n");
    return 1;
  }

  return 0;
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
again:
  head = controller->head;  
  while( head && head->route )  {
    /* Check if there is a match */
    if( strcmp( uri, head->route ) == 0 )  {
      if( needs_reloading( controller ) )  {
        /* We need reloading - start over */
        load_controller( controller->fp, controller );
        goto again;
      }
      head->ptr();
      return 0;
    }
    head++;
  }

  return -1;
}

