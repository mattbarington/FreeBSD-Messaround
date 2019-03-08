#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include  <string.h>
#include <errno.h>
#include <fcntl.h>
#include "oper_fs.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";


static int aofs_getattr(const char *path, struct stat *stbuf)
{
  int res = 0;
  memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    
    //  }else if (strcmp(path, hello_path) == 0) {
    //    stbuf->st_mode = S_IFREG | 0444;
    //    stbuf->st_nlink = 1;
    //    stbuf->st_size = strlen(hello_str);
  } else
    res = -ENOENT;
  printf("res = %d\n",res);
  return res;
}

static int aofs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi)
{
  printf("eraddir");
  (void) offset;
  (void) fi;
  
  if (strcmp(path, "/") != 0)
    return -ENOENT;
  
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  filler(buf, hello_path + 1, NULL, 0);
  printf(" returned 0\n");
  return 0;
}

static struct fuse_operations aofs_oper = {
	.getattr	= aofs_getattr,
	.readdir	= aofs_readdir,
};

int main(int argc, char *argv[])
{
  
  //check number of arguments
  if(argc < 2) {
    printf("Incorrect number of arguments.\n");
    return 1;
  }

  //initialize foldername
  if(strlen(argv[1]) > MAX_FILENAME) {
    printf("Folder name must be less than %d characters.\n", MAX_FILENAME);
    return 1;
  }

  printf("Launching AOFS file system in %s\n", argv[1]);
  fuse_main(argc, argv, &aofs_oper, NULL);
  
  return 0;
}



