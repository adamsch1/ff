#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct chunk_t {
  char *text;
  int  macro: 1;
  int  isif: 1;
  struct chunk_t *next;
};

struct chunk_t * chunk_new( char * text )  {
  struct chunk_t *t = (struct chunk_t*)malloc(sizeof(struct chunk_t));
  t->text = strdup(text);
  t->next = 0;
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
      /* Open tag but only create a chunk node if its at the first byte */
      if( p != start )  {
        /* Grab everthing after the last close tag up to the start tag */
        *p = 0;
        iter->next = chunk_new( start );
        iter = iter->next;
        *p = '<';
      }
      p += 2;
      temp = parse_logic( &p );
      if( temp )  { 
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

char temp2[] = "<% $dude %> Ouch <html></html>";
char temp3[] = "<% %> Ouch <html></html>";
char temp4[] = "<% if $d %> <% $crap %> Ouch <html></html>";
char temp1[] = " if $d %>";
int main()  {
  struct chunk_t *t;
  t = template_parse( temp1 );

  while( t )  {
    printf("{%s} ismacro: %x isif: %x\n", t->text, t->macro!=0, t->isif!=0);
    t = t->next;
  }
}







