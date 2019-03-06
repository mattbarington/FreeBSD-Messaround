#include "aofs.h"
#include <stdlib.h>
#include <stdio.h>

int init_fs(AOFS* fs) {

  int iter;
  

  //initialize superblock
  
  fs->sb.magicnum    = MAGICNUM;
  fs->sb.totalblocks = BLOCK_NUM;
  fs->sb.blocksize   = BLOCK_SIZE;
  
  //initialize bitmap
  for(iter = 0; iter < BITMAP_SIZE; ++iter) {
    fs->sb.bitmap[iter] = EMPTY;
  }
  
  //initialize datablocks
  for(iter = 0; iter < BITMAP_SIZE; ++iter) {
    fs->blocks[iter].dbm.filename = "";
    fs->blocks[iter].dbm.next = NULL;
    fs->blocks[iter].dbm.head = false;
    
    memset(fs->blocks[iter].blocks, 0, BLOCK_DATA*sizeof(
  }

}

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
