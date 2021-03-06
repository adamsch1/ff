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
#include "array.h"
#include "sglib.h"
#include "ff.h"
#include "form.h"

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

/**
 * Load, parse, evaulate, display, with a form
 */
void template_run_form( char *path, struct form_t *form, 
                        struct array_t *them ) { 
  struct array_t *arr = array_new();
  node *n;

  /* If not empty we validated */ 
  if( ! array_empty( form->err) )  {
    array_add_obj( arr, "errors", form->err );
  }

  for( n=array_first( form->clean ); n != NULL; 
       n = array_next( form->clean ) ) { 
//    printf("FUCK %s %s\n", n->key, n->value );
    array_add_str( arr, n->key, nully(n->value) ); 
  }

  template_run( path, arr );

  array_free(arr);   
}

/**
 * Walk the key, recall we break a key with . into a set of subkeys
 * isolated by \0
 */
char * walk_macro( struct array_t *arr, char *key )  {
  node *n = array_get_node( arr, key );
  char *p = key;

  while( n )  {
    p += strlen( key ) + 1;
    if( p )  {
      n = array_get_node( n->obj, p );   
    } else {
      /* Nothing else to walk */
      return n->value;
    }
  }
}

/**
 * Load, parse, evaulate, display
 */
void template_run( char *path, struct array_t *arr )  {
  struct chunk_t *chunk = template_load(path);
  char   *value;
  int    iftrue = 1;
  int    freeit = 1;

  if( arr == NULL ) arr = array_new();
  else freeit = 0;

  printf("Content-type: text/html\r\n\r\n");

  while( chunk )  {
    if( chunk->macro && chunk->isif == 0 )  {
      /* have macro not an if statement, echo it */
      if( (value=array_get_macro( arr, chunk->text )) != NULL )  {
        printf("%s", value );
      }
    } else if( chunk->isif )  {
      /* Have an if statement, see if macro evals to true/fales */
      value = array_get_macro( arr, chunk->text );
      if( value == NULL || *value == 0 || *value == '0' )  {
        /* Its false don't echo anything until we get endif */
        iftrue = 0;  
      } 
    } else if( chunk->endif )  {
      /* Turn output back on */
      iftrue = 1; 
    } else if( iftrue ) {
      /* Echo text by default or if last IF statement eval to true  */
      printf("%s", chunk->text );
    }
    chunk = chunk->next;
  }

  /* Free it here to make life easier in controller */
  if( freeit ) array_free( arr ); 
}

/**
 *  Macro chunk  - break into subkeys
 *  For macro,s "." indicates a sub object is expected so 
 *  to make this easier to walk data structures as a key we convert 
 *  to \0
 *  errors.password  becomes  errors\0password\0\0
 *  allowing us to walk the string for each subkey
 */
struct chunk_t * chunk_new_macro( char *text )  {
  struct chunk_t *t = (struct chunk_t*)calloc(1,sizeof(struct chunk_t));
  int len;
  char *p=text;
  char *d;
  if( text )  {
    /* Copy over data with extra space for null and end null */
    len = strlen(text);
    d = t->text = malloc(len+2);
    while( *p )  {
      *d = *p;
      if( *d == '.' ) *d = 0;
      p++; 
      d++;
    }
    *d = *p;
  }
  t->next = 0;
}

/* Regular text chunk */
struct chunk_t * chunk_new( char * text )  {
  struct chunk_t *t = (struct chunk_t*)calloc(1,sizeof(struct chunk_t));
  if( text ) t->text = strdup(text);
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

  sprintf( finalpath, "%s/%s", path, source );

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
      if( chunk->text ) free(chunk->text);
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
  int have_endif = 0;
  int have_while = 0;
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
      chunk = chunk_new_macro( start );
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
    } else if( strncmp( p, "while", 5 ) == 0 )  {
      /* While */
      have_while = 1;
      p+=5;
      continue;
    } else if( strncmp( p, "endwhile", 8 ) == 0 )  {
      /* While */
      chunk = chunk_new( NULL );
      chunk->endwhile = 1;
      p+=8;
      return chunk;
    } else if( *p == 'i' && *(p+1) == 'f' )  {
      /* boolean logic */
      have_if = 1;
      p+=2; 
      continue;     
    } else if( strncmp( p, "endif", 5 ) == 0 )  {
      /* close boolean logic */
      chunk = chunk_new( NULL );
      chunk->endif  = 1;
      return chunk;
      p+=5; 
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
  struct array_t *arr = array_new();

  template_init();

  t = template_parse( temp1 );

  while( t )  {
    printf("{%s} ismacro: %x isif: %x\n", t->text, t->macro!=0, t->isif!=0);
    t = t->next;
  }

  array_add_str( arr, "dog", "GONZO!");
//  array_add_str( arr, "house", "doctor!");
  template_run("test.tpl", arr);
}
#endif





