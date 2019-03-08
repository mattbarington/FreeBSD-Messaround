#define FUSE_USE_VERSION 26

//#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "aofs.h"


int main(int argc, char *argv[])
{
  char filename[256] = "";
  int iter;
  AOFS* new_aofs;

  //check number of arguments
  if(argc > 2) {
    printf("Incorrect number of arguments.\n");
    return 1;
  }

  //initialize filename
  if(argc == 2) {
    if(strlen(argv[1]) > MAX_FILENAME) {
      printf("Filename must be less than %d characters.\n", MAX_FILENAME);
      return 1;
    } else {
      strcpy(filename, argv[1]);
    }
  } else {
    strcpy(filename, FS_FILE_NAME);
  }

  new_aofs = malloc(1*sizeof(AOFS));
  if(init_fs(new_aofs) != 0) {
    printf("Unable to initialize AOFS.\n");
    return 1;
  }
  if(write_fs(filename, new_aofs) != 0) {
    printf("Unable to write AOFS.\n");
    return 1;
  }
  free(new_aofs);
  printf("Successfully created new file system in %s.\n", filename);
  
  return 0;
}



