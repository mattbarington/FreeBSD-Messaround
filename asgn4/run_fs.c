#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "oper_fs.h"


int main(int argc, char *argv[])
{
  //check number of arguments
  if(argc != 2) {
    printf("Incorrect number of arguments.\n");
    return 1;
  }

  //initialize foldername
  if(strlen(argv[1]) > MAX_FILENAME) {
    printf("Folder name must be less than %d characters.\n", MAX_FILENAME);
    return 1;
  }

  printf("Launching AOFS file system in %s\n", argv[1]);
  fuse_main(argc, argv, NULL, NULL);
  
  return 0;
}



