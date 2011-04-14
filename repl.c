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

#include "array.h"

struct repl_t {
  /* Array of struct chunk_file_t */
  struct array_t *file_chunks;

  /* Array of key/values */
  struct array_t *kv;
};

struct repl_t repl;

struct chunk_file_t {
  int file_id;
  int fd;
  off_t offset;
};

/* File we are currently writing too, or currently loading from */
struct chunk_file_t * current_chunk_file ;

/* K/V we store in memory */
struct mkeymaster_t {
  time_t ts;
  int file_id;
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
int repl_append( time_t now, char *key, char *data )  {
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

  rc=writev( current_chunk_file->fd, iov, 3 );
  if( rc == -1 )  {
    fprintf(stderr, "Could not write to %d, errno=%d\n", 
            current_chunk_file->file_id, errno );
    return -1;
  }

  attempted_count = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len;

  if( rc != attempted_count )  {
    fprintf(stderr, "Could not write %d bytes, disk full?, errno=%d\n", 
            current_chunk_file->file_id, errno );
    /* Mark current chunk as corrupted and close it */
    mark_current_file_corrupted();
    return -1;
  }

  mkey = (struct mkeymaster_t *)malloc(sizeof(struct mkeymaster_t));

  /* Copy over easy stuff */
  mkey->ts = now;
  mkey->file_id = current_chunk_file->file_id;

  /* Length of key with null */
  mkey->vpos = iov[0].iov_len + iov[1].iov_len; 

  array_add_obj( repl.kv, key, mkey );

  return 0;
}

/**
 * If current file is too big, close it and open another one 
 * This should be seamless and without anyone's strict knowledge. 
 */
int repl_check_close()  {

  if( current_chunk_file == NULL )  {
    struct chunk_file_t *fp =(struct chunk_file_t*)calloc(1, 
      sizeof(struct chunk_file_t));

    if( fp == NULL )  {
      fprintf(stderr,"Malloc failed: %s:%d\n", __FILE__, __LINE__);
      return -1;
    }

    fp->fd = open("logs/1.data",O_APPEND|O_CREAT, 
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    fp->file_id = 1;

    if( fp->fd == -1 ) { 
      fprintf(stderr,"Could not open logs/1.data: %s:%d\n", __FILE__, 
              __LINE__);
      return -1;
    }

    current_chunk_file = fp;
  }

  /* Always return goodness */
  return 0; 
}

int repl_init( struct repl_t *r, const char *chunkpath ) { 
  struct dirent *dp;
  DIR *dir = opendir( chunkpath );
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
      snprintf( buff, PATH_MAX, "chunkpath/%s", dp->d_name );
      load_chunk( fp, buff );
    }
  }


  closedir(dir);

  return repl_check_close();
}

/**
 * Read through file - create in memory key map
 */
int load_chunk( struct chunk_file_t *fp, char *fpath )  {

  current_chunk_file = fp;

  return 0; 
}

int repl_quit( struct repl_t *r ) {
  return 0; 
}

int main(int argc, char *argv[])  {

  repl_init( &repl, "logs" );

  repl_append( time(0), "bob", "ross" );
  repl_quit( &repl );
} 
