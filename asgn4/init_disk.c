#define FUSE_USE_VERSION 26

//#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "oper_fs.h"

typedef enum run_option {NONE, BUILD, RUN, ALL} Option;
	
void print_usage(void) {
  printf("usage\n");
}

int main(int argc, char *argv[])
{
  char filename[256] = "";
  int iter;
  Option fs_op = NONE;

  //handle command line arguments  
  for(iter = 1; iter < argc; ++iter) {
    if((argv[iter][0] == '-') && (strlen(argv[iter]) == 2)) {
      switch(argv[iter][1]) {
        case 'c':
          fs_op = BUILD; break;
        case 'r':
          fs_op = RUN; break;
        case 'a':
          fs_op = ALL; break;
        default:
          printf("Unrecognized flag: %s\n", argv[iter]);
          print_usage();
          return 1;
      }
    } else {
      if(!strcmp(filename, "")) {
        if(strlen(argv[iter]) > MAX_FILENAME) {
          printf("Filename >%d characters.\n", MAX_FILENAME);
        } else {
          strcpy(filename, argv[iter]);
        }
      } else {
        printf("Unrecognized argument: %s\n", argv[iter]);
        print_usage();
        return 1;
      }
    }
  }

  //check if option was selected
  if(fs_op == NONE) {
    printf("No run option selected.\n");
    print_usage();
    return 1;
  }

  //set default filename if no file was given
  if(!strcmp(filename, "")) {
    strcpy(filename, FS_FILE_NAME);
  }

  //build filesystem
  if(fs_op != RUN) {
    printf("building filesystem...\n");
  }

  //run filesystem
  if(fs_op != BUILD) {
    printf("running filesystem...\n");
    //return fuse_main(argc, argv, NULL, NULL);
  }

  return 0;
}



