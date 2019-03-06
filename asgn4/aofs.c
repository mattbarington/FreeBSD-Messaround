#include "aofs.h"
#include <stdlib.h>
#include <stdio.h>


int find_free_bit(uint8_t map[], int num_bits) {
   int el_size = sizeof(uint8_t) * 8;
   int bit_offset, byte_num, bit;
   for (int i = 0; i < num_bits; ++i) {
     byte_num   = i / el_size;
     bit_offset = i % el_size;
     bit = (map[byte_num] >> bit_offset) & 1U;
     if (!bit){
       return i;
     }
   }
   return -1;
}

int set_bit(uint8_t map[], int bit_idx) {
  int el_size    = sizeof(uint8_t) * 8;
  int bit_offset = bit_idx % el_size;
  int byte_num   = bit_idx / el_size;
  if ((map[byte_num] >> bit_offset) & 1U) { //Bit at bit_idx is already set
    return 1;
  } else {
    int mask = 1 << bit_offset;
    map[byte_num] |= mask;
    return 0;
  }
}

int clear_bit(uint8_t map[], int bit_idx) {
  int el_size    = sizeof(uint8_t) * 8;
  int bit_offset = bit_idx % el_size;
  int byte_num   = bit_idx / el_size;
  if (!((map[byte_num] >> bit_offset) & 1U)) { //Bit at bit_idx is already 0
    return 1;
  } else {
    int mask = ~(1U << bit_offset);
    map[byte_num] &= mask;
    return 0;
  }
}

int init_fs(AOFS* fs) {

  int iter;
  byte* i_b;

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
    strcpy(fs->blocks[iter].dbm.filename, "");
    fs->blocks[iter].dbm.next = NULL;
    fs->blocks[iter].dbm.head = false;
    
    i_b = fs->blocks[iter].data;
    memset(i_b, 0, BLOCK_DATA*sizeof(i_b[0]));
  }

  return 0;
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

  printf("Creating File System...\n");

  //Create file system
  AOFS* my_fs;
  my_fs = malloc(1*sizeof(AOFS));

  printf("Initializing File System...\n");

  //Initialize file system
  init_fs(my_fs);

  printf("Writing File System to file...\n");
 
  //write file system to file
  write_fs("fs.dat", my_fs);

  printf("Freeing File System Memory...\n");
 
  //free file system from memory
  free(my_fs);

  printf("Reading in File System from file...\n");

  //create new file system to read into
  AOFS new_fs;

  //read in file system from file
  read_fs("fs.dat", &new_fs);

  return 0;
}
