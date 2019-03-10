#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "aofs.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static const char* hola_path = "/HolaMundo.txt";

static AOFS* get_context() {
  return ((AOFS *) fuse_get_context()->private_data);
}
static int aofs_getattr(const char *path, struct stat *stbuf)
{
  printf("aofs_getattr. Path: %s\n",path);
  int res = 0;
  memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  } else {
    AOFS* fs = get_context();
    int file_head = aofs_find_file_head(path, fs);
    printf("file head at %d. This is where we copy over the metadata\n", file_head);
    if (file_head != -1) {
      BlockMeta* bm = &fs->blocks[file_head].dbm;
      //stbuf->st_mode = S_IFREG | 0444;
      stbuf->st_mode    = S_IFREG | 0777;
      stbuf->st_nlink   = 1;
      stbuf->st_ino     = 0; //don't have, so init to 0
      stbuf->st_dev     = 0; //don't know what this is
      stbuf->st_atime   = bm->st_atim;
      stbuf->st_mtime   = bm->st_mtim;
      stbuf->st_ctime   = bm->st_ctim;
      stbuf->st_size    = bm->st_size;
      stbuf->st_blocks  = bm->st_blocks;
      stbuf->st_blksize = bm->st_blksize;
      stbuf->st_flags   = bm->st_flags;
      stbuf->st_uid     = getuid();
      stbuf->st_gid     = getgid();
      //stbuf->st_mode = S_IFREG | 0444 | 0777;
      //stbuf->st_mode = S_IFREG | 0777 | 0777;
      //stbuf->st_mode    = 0040777;
      stbuf->st_mode    = S_IFREG | 0444 | 0666;
      printf("Finished setting file.\n");	
    }
    //file does not exist
    else {
      return -ENOENT;
    }
  }
  return res;
}

static int aofs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi)
{
  printf("readdir\n");

  (void) offset;
  (void) fi;
  
  if (strcmp(path, "/") != 0) {
    //    return -ENOENT;
    filler(buf, path + 1, NULL, 0);
    return 0;
  }
  
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  //Find all file names with iterative loop *barf*
  AOFS* fs = get_context();
  BlockMeta* bm;
  
  for (int block_num = 0; block_num < BLOCK_NUM; block_num++) {
    if (bit_at(fs->sb.bitmap, block_num)) {
      bm = &fs->blocks[block_num].dbm;
      if (bm->head) {
	//	printf("found file '%s'\n", bm->filename);
  	filler(buf, bm->filename + 1, NULL, 0);
      }
    }
  }
  return 0;
}

static int aofs_open(const char *path, struct fuse_file_info *fi) {

  printf("aofs_open\n");

  AOFS* fs = get_context();
  
  //check to see file exists
  if(aofs_find_file_head(path, fs) != -1) {
    fi->nonseekable = 0;
    fi->direct_io   = 0;
    fi->fh          = 0;
    fi->flags       = 0;
    fi->flush       = 0;
    fi->writepage   = 0;
    printf("Set fuse_file_info\n");
    return 0;
  } else {
    printf("aofs_open error. couldn't find specified file :/\n");
    return -ENOENT;
  }
}

static int aofs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

  printf("aofs_create\n");

  AOFS* fs = get_context();

  //if file already exists, run open operation instead??
  //if(aofs_find_file_head(path, fs)) {
  //  return aofs_open(path, fi);
  //}

  //Create file at first avaliable block
  int x = aofs_create_file(path, fs); 
  printf("new file at: %d\n", x);
  if(x >= 0) {
    return 0;
  } else { return -1;}  
}


static int aofs2_read(const char *path, char *buf, size_t size, off_t offset,
		     struct fuse_file_info *fi) {
  printf("aofs_run\n");
  return 0;
}

static int aofs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *file_info) {
  printf("aofs_write\n");
  return 0;
}
static int aofs_statfs(const char* path, struct statvfs* stbufi) {
  printf("aofs_statfs\n");
  return 0;
}

static struct fuse_operations aofs_oper = {
	.getattr	= aofs_getattr,
	.readdir	= aofs_readdir,
	.create   = aofs_create,
  .read     = aofs2_read,
	.open     = aofs_open,
  .write    = aofs_write,
  .statfs   = aofs_statfs,
};

int main(int argc, char *argv[])
{
  
  //check number of arguments
  if(argc < 2) {
        printf("Incorrect number of arguments.\n");
    //    printf("Incorrect number of arguments.\nMust Provide <disk_img> <mount folder>");
    return 1;
  }

  //initialize foldername
  if(strlen(argv[1]) > MAX_FILENAME) {
    printf("Folder name must be less than %d characters.\n", MAX_FILENAME);
    return 1;
  }

  //printf("Launching AOFS file system from img %s in %s\n", argv[1], argv[2]);

  AOFS* aofs = calloc(1,sizeof(AOFS));
  //char* buf = malloc(sizeof(AOFS));
  char* filename = FS_FILE_NAME;
  if (read_fs(filename, aofs) == -1) {
    printf("There was a problem loading the disk image\n");
    exit(1);
  }
  fuse_main(argc, argv, &aofs_oper, aofs);

  return 0;
}



