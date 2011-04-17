#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>
#include <stdint.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "array.h"
#include "repl.h"

struct global_t {
  int max_file_size;
} global;

/* Everything we need to know about a datafile */
struct chunk_file_t {
  char *fname;
  int fd;
  off_t offset;
  long bytes_written;
  int  corrupted:1;
};

/* File we are currently writing too, or currently loading from */
///struct chunk_file_t * current_chunk_file ;


/* K/V we store in memory */
struct mkeymaster_t {
  time_t ts;
  struct chunk_file_t *file;
  off_t    vpos;
  uint32_t vsize; 
};

/* Header we write for each record */
struct fkeymaster_t {
  time_t ts; 
  uint32_t ksize;
  uint32_t vsize;
};

int mark_current_file_corrupted()  {
  exit(1);
  return 0;
}

/**
 * Append data to the file
 */
int repl_append( struct repl_t *r, time_t now, char *key, char *data )  {
  struct iovec iov[3];
  struct mkeymaster_t *mkey;
  int    attempted_count;
  int    rc;
  struct fkeymaster_t fkey;

  fkey.ts = now;
  fkey.ksize = strlen(key)+1;
  fkey.vsize = strlen(data)+1;
 
  iov[0].iov_base = &fkey;
  iov[0].iov_len = sizeof(struct fkeymaster_t);

  iov[1].iov_base = key;
  iov[1].iov_len = strlen(key)+1;
 
  iov[2].iov_base = data;
  iov[2].iov_len = strlen(data)+1;

  rc=writev( r->current_chunk_file->fd, iov, 3 );
  if( rc == -1 )  {
    fprintf(stderr, "Could not write to %s, errno=%d, %s\n", 
            r->current_chunk_file->fname, errno, strerror(errno) );
    return -1;
  }

  attempted_count = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len;

  if( rc != attempted_count )  {
    fprintf(stderr, "Could not write %s bytes, disk full?, errno=%d\n", 
            r->current_chunk_file->fname, errno );
    /* Mark current chunk as corrupted and close it */
    mark_current_file_corrupted();
    return -1;
  }

  //fsync(r->current_chunk_file->fd );
  r->current_chunk_file->bytes_written += attempted_count;

  mkey = (struct mkeymaster_t *)malloc(sizeof(struct mkeymaster_t));
  if( mkey == NULL )  {
    fprintf(stderr,"Malloc failed: %s:%d\n", __FILE__, __LINE__);
    return -1;
  }

  /* Copy over easy stuff */
  mkey->ts = now;
  mkey->file = r->current_chunk_file;
  mkey->vsize = iov[2].iov_len;

  /* Length of key with null */
  mkey->vpos = r->current_chunk_file->bytes_written - iov[2].iov_len;

  array_add_obj( r->kv, key, mkey );

  return repl_check_close(r);
}

/**
 * Convenient way to name files, sortable don't have to track a counter etc
 */
char * makefilename()  {
  char buff[PATH_MAX];
  time_t timer = time(0);
  struct tm *the_time=localtime(&timer);

  strftime( buff, sizeof(buff), "%Y%m%M%S.data", the_time );

  return strdup(buff);
}

/**
 * If current file is too big, close it and open another one 
 * This should be seamless and without anyone's strict knowledge. 
 */
int repl_check_close( struct repl_t *r)  {
  char buff[PATH_MAX];

  if( r->current_chunk_file == NULL )  {
    struct chunk_file_t *fp =(struct chunk_file_t*)calloc(1, 
      sizeof(struct chunk_file_t));
    char *filename = makefilename();
    load_chunk( r, fp, filename );
    fp->fname = filename;
    r->current_chunk_file = fp;
  } else if( r->current_chunk_file->bytes_written > global.max_file_size )  {
    if( r->callback ) r->callback(r);
    r->current_chunk_file = NULL;
    return repl_check_close(r);
  }

  /* Always return goodness */
  return 0; 
}

int repl_init( struct repl_t *r, const char *chunkpath ) { 
  struct dirent *dp;
  DIR *dir;
  char *str;
  char buff[PATH_MAX];

  dir = opendir( chunkpath );
  if( dir == NULL )  {
    fprintf(stderr,"Could not load chunks from %s, %s:%d\n", 
            chunkpath, __FILE__, __LINE__);
    return -1;
  }

  /* Init array to hold all file data */
  r->file_chunks = array_new();
  r->kv = array_new();

  /* Read listing of files in the subdirectory */
  while((dp=readdir(dir)) != NULL )  {
    /* Look specifically for .data files */
    if( fnmatch("*.data", dp->d_name, FNM_NOESCAPE) == 0 )  {
      struct chunk_file_t *fp =(struct chunk_file_t*)calloc(1, 
        sizeof(struct chunk_file_t));
      if( fp == NULL )  {
        fprintf(stderr,"Malloc failed: %s:%d\n", __FILE__, __LINE__);
        return -1;
      }
      load_chunk( r, fp, dp->d_name );
      array_add_obj( r->file_chunks, dp->d_name, fp );
      /* IF this file can still be written to set it as current, if we 
         don't find a suitable file we will create a new one when we call
         repl_check_close */
      if( fp->bytes_written < global.max_file_size ) r->current_chunk_file = fp;
    }
  }


  closedir(dir);

  return repl_check_close(r);
}

int repl_get( struct repl_t *r, char *key, char **data )  {
  struct mkeymaster_t *km;
  int rc; 
  struct node_t *n;

  km = array_get_obj( r->kv, key );
  if( !km )  {
    return 0;
  }

  *data = (char*)malloc( km->vsize );
  if( !*data )  {
    fprintf(stderr,"Malloc failed: %s:%d\n", __FILE__, __LINE__);
    return -1;
  }

  rc = lseek( km->file->fd, km->vpos, 0 );
  if( rc != km->vpos )  {
    fprintf(stderr,"Could not seek: %s offs=%d  %s:%d\n", km->file->fname, 
            km->vpos, __FILE__, __LINE__);
    free(*data);
    return -1;
  }

  rc = read( km->file->fd, *data, km->vsize );
  if( rc != km->vsize )  {
    fprintf(stderr,"Could not read: %s offs=%d count=%d %s:%d\n", 
            km->file->fname, km->vpos, km->vsize, __FILE__, __LINE__);
    free(*data);
    return -1;
  }

  return 1;
}

int read_data_from_chunk( struct repl_t *r, struct chunk_file_t *fp )  {
  void *addr;
  char *p;
  struct fkeymaster_t fkey;
  long offset = 0;
  struct mkeymaster_t *mkey;

  p = addr = mmap( 0, fp->bytes_written, PROT_READ, MAP_PRIVATE, fp->fd,
               0 );
  if( addr == NULL )  {
    fprintf(stderr,"Could not read: %s %s:%d\n", fp->fname, __FILE__, 
            __LINE__);
    fp->corrupted |= 0x1;
    return -1;
  }

  /* While we at least have enough space to read in the fkey */
  while( offset + sizeof(struct fkeymaster_t) < fp->bytes_written )  {
    memcpy( &fkey, addr+offset, sizeof(struct fkeymaster_t));

    /* Skip past fix length header now at start of key/avlue */
    offset += sizeof(struct fkeymaster_t) ;

    /* Copy over easy stuff */
    mkey = (struct mkeymaster_t *)malloc(sizeof(struct mkeymaster_t));
    mkey->ts = fkey.ts;
    mkey->file = fp;
    mkey->vsize = fkey.vsize;

    /* Get position of value from file */
    mkey->vpos = offset + fkey.ksize;

    array_add_obj( r->kv, addr+offset, mkey );

    offset += fkey.ksize + fkey.vsize;
  }

  munmap( addr, fp->bytes_written ); 
}

/**
 * Read through file - create in memory key map
 */
int load_chunk( struct repl_t *r, struct chunk_file_t *fp, char *fpath )  {
  char buff[PATH_MAX];
  struct stat st;

  snprintf( buff, sizeof(buff), "logs/%s", fpath );
  fp->fd = open( buff,O_RDWR | O_APPEND |O_CREAT, 0666 );
  fp->bytes_written = 0;
  if( fp->fd == -1 ) { 
    fprintf(stderr,"Could not open: %s %s:%d\n", buff, __FILE__, 
            __LINE__);
    return -1;
  }

  /* Stat file to get the mtime and size */
  if( stat( buff, &st )  < 0 )  {
    fprintf(stderr,"Could not stat: %s %d %s:%d\n", buff, errno, __FILE__,
            __LINE__ );
    return -1;
  }

  fp->fname = strdup(fpath);
  fp->bytes_written = st.st_size;

  /* Hmm a non emtpy chunk */
  if( fp->bytes_written > 0 )  {
     return read_data_from_chunk( r, fp );
  }

  return 0; 
}

int repl_quit( struct repl_t *r ) {
  node *n;
  struct chunk_file_t *cp;

  for( n=array_first( r->file_chunks ); n != NULL; 
       n = array_next( r->file_chunks ) ) { 
    cp = (struct chunk_file_t *)n->obj;
    close(cp->fd);
    free(cp->fname);
    free(cp);
  }
 
  for( n=array_first( r->kv ); n != NULL;
       n=array_next( r->kv ) )  {
    struct mkeymaster_t *km = n->obj;
    free(km);
  } 
  array_free( r->file_chunks );
  array_free( r->kv );
  return 0; 
}

void spill( struct repl_t *r ) {
  printf("Spill\n");
}

int main(int argc, char *argv[])  {
  char * value;
  struct repl_t repl = {0};

  global.max_file_size = 1000;

  repl_init( &repl, "logs" );
  repl.callback = spill; 
  repl_append( &repl, time(0), "b", "d" );
  repl_append( &repl, time(0), "bob", "dic" );

  if( repl_get( &repl, "bob", &value ) )  {
    printf("%s\n", value );
    free(value);
  }

  repl_quit( &repl );
} 
