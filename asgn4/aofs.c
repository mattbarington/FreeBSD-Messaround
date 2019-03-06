#include "aofs.h"
#include <stdlib.h>
#include <stdio.h>

int read_fs(const char* filename, AOFS* fs) {
  FILE* rf;
    
  rf = fopen(filename, "r");
  if(rf) {
    fread(fs, sizeof(AOFS), 1, rf);
    fclose(rf);
    return 0;
  } else {
    return -1;
  }
}

int write_fs(const char* filename, AOFS* fs) {
  FILE* wf;

  wf = fopen(filename, "w");
  if(wf) {
    fwrite(fs, sizeof(AOFS), 1, wf);
    fclose(wf);
    return 0;
  } else {
    return -1;
  }
}

int main() {

  printf("starting...\n");

  //Create file system
  AOFS* my_fs;
  my_fs = malloc(1*sizeof(AOFS));
 
  //write file system to file
  write_fs("fs.dat", my_fs);
 
  //free file system from memory
  free(my_fs);

  //create new file system to read into
  AOFS new_fs;

  //read in file system from file
  read_fs("fs.dat", &new_fs);

  return 0;
}
