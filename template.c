#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "template.h"
#include "sglib.h"

/** 
 * Hold stuff about the template
 */
typedef struct template_t {
  char *path;
  struct chunk_t *head;
  time_t mtime;

  /* For Sglib */
  char color_field;
  struct template_t *left;
  struct template_t *right;
} template;

#define COMPARATOR(x,y) (strcmp(x->path,y->path))
SGLIB_DEFINE_RBTREE_PROTOTYPES( template, left, right, color_field, COMPARATOR );
SGLIB_DEFINE_RBTREE_FUNCTIONS( template, left, right, color_field, COMPARATOR );


char *path;
template *template_cache = NULL;

int template_init()  {
  path = getcwd(NULL, 0 );
  if( !path )  {
    fprintf(stderr,"Could not get current path: %d\n", errno );
    return -1;
  }
}

struct chunk_t * chunk_new( char * text )  {
  struct chunk_t *t = (struct chunk_t*)malloc(sizeof(struct chunk_t));
  t->text = strdup(text);
  t->next = 0;
}

struct chunk_t * template_load( char * source )  {
  struct stat st;
  char *bigbuff;
  char *p;
  char ch;
  FILE *fp;
  struct chunk_t *chunk;
  struct chunk_t *iter;
  template *tp;
  template entry;
  char finalpath[PATH_MAX];

  printf("%d\n", sprintf( finalpath, "%s/%s", path, source ));     

  /* Stat file to get the mtime and size */
  if( stat( finalpath, &st )  < 0 )  {
    fprintf(stderr,"Could not stat: %s %d\n", finalpath, errno );
    return NULL;
  }

  /* See if we already have this sucker */ 
  entry.path = finalpath;
  if( (tp=sglib_template_find_member( template_cache, &entry )) != NULL )  {
    /* We do if it hasn't modified - exit */
    if( tp->mtime == st.st_mtime ) return tp->head;
    /* Modified, free old data */
    iter = tp->head;
    while( iter )  {
      chunk = iter;
      iter = iter->next; 
      free(chunk->text);
      free(chunk);
    }
  } else {
    /* New template */
    tp = calloc(1, sizeof(template));
    if( tp == NULL ) { 
      fprintf(stderr,"Could not allocate memory for template: %s %d\n", finalpath, errno );
      return NULL;
    }
    tp->path = strdup(finalpath);
    tp->mtime = st.st_mtime;
  }

  /* Allocate buffer for the template and read it in from the file */ 
  bigbuff = malloc(st.st_size+1);
  if( bigbuff == NULL )  {
    fprintf(stderr,"Could not allocate memory to read: %s %d\n", finalpath, errno );
    return NULL;
  }  

  fp = fopen( finalpath, "r");
  if( fp == NULL ) { 
    fprintf(stderr,"Could not open to read: %s %d\n", finalpath, errno );
    free(bigbuff);
    return NULL;
  }

  /* Possibly mmap and memcpy later */
  p = bigbuff;
  while( (ch=fgetc(fp)) != EOF )  {
    *p = ch; p++;
  }
  *p = 0;
       
  /* Parse the template, link in the chunk header */ 
  chunk =  template_parse( bigbuff );

  tp->head = chunk;
  sglib_template_add(&template_cache, tp);
 
  fclose(fp); 
  free(bigbuff);
  return chunk;
}

/** 
 * Parse macro and simple if statement
 */
struct chunk_t * parse_logic( char ** them )  {
  char *p = *them;
  char *start;
  int have_if = 0;
  struct chunk_t * chunk=NULL;
  char ch;
  while( *p )  {

    /* White space */
    while( *p && *p == '\t' && *p == ' ' ) p++;
    if( !*p ) return NULL;

    if( *p == '$' )  {
      start = p+1;
      /* We have a straight expansion */
      while( *p != '\t' && *p != ' ' ) p++;
      if( !*p ) return NULL;
      ch = *p;
      *p = 0;/* Isolate macro name */
      chunk = chunk_new( start );
      chunk->macro = 1;
      chunk->isif  = have_if;  
      *p = ch;
      *them = p;
      return chunk;
    } else if( *p == '%' && *(p+1) == '>' )  {
      /* Cloes tag */
      fprintf( stderr, "Close tag without macro\n");
      p += 2; 
      *them = p;
      return chunk;
    } else if( *p == 'i' && *(p+1) == 'f' )  {
      /* boolean logic */
      have_if = 1;
      p+=2; 
      continue;     
    }
    p++; 
  } 
  *them = p;
  return NULL;
}

struct chunk_t * template_parse( char * source )  {
  char *p = source;
  char *start = p;
  struct chunk_t *head;
  struct chunk_t *iter;
  struct chunk_t *temp; 
  iter = head = chunk_new( "" );

  while( *p )  {
    if( *p == '<' && *(p+1) == '%' )  {
      /* Open tag  */
      if( p != start )  {
        /* Grab everthing after the last close tag up to the start tag */
        *p = 0;
        iter->next = chunk_new( start );
        iter = iter->next;
        *p = '<';
      }
      /* Skip the <% literal and parse the logic inside this tag */
      p += 2;
      temp = parse_logic( &p );
      if( temp )  { 
        /* valid logic link it in */
        iter->next = temp;
        iter = iter->next;
      }
    } else if( *p == '%' && *(p+1) == '>' )  {
      /* Record start of next text chunk */
      start = p+2;
    }
    p++;
  }
  /* Grab any trailing text */
  iter->next = chunk_new( start );
  iter = iter->next;
  return head;
}

#if 0
char temp2[] = "<% $dude %> Ouch <html></html>";
char temp3[] = "<% %> Ouch <html></html>";
char temp4[] = "<% if $d %> <% $crap %> Ouch <html></html>";
char temp1[] = " if $d %>";
int main()  {
  struct chunk_t *t;

  template_init();

  t = template_parse( temp1 );

  while( t )  {
    printf("{%s} ismacro: %x isif: %x\n", t->text, t->macro!=0, t->isif!=0);
    t = t->next;
  }

  t = template_load("test.tpl");
  while( t )  {
    printf("{%s} ismacro: %x isif: %x\n", t->text, t->macro!=0, t->isif!=0);
    t = t->next;
  }
}
#endif





