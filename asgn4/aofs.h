#ifndef AOFS_H
#define AOFS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define byte unsigned char

//Magic number
#define MAGICNUM 0xfa19283e
//Block size
#define DB_BLOCK_SIZE 4096
//Number of blocks
#define BLOCK_NUM 4096

typedef struct SuperBlock {
  uint32_t magicnum;
  uint32_t totalblocks;
  uint32_t blocksize;
  uint8_t bitmap[BLOCK_NUM / sizeof(uint8_t)];
} Superblock;

struct Block;

typedef struct BlockMeta {
  char filename[256];
  struct Block* next;
  bool head;
} BlockMeta;

//Datablock metadata size
#define DB_BLOCK_META sizeof(BlockMeta)
#define DB_BLOCK_DATA (DB_BLOCK_SIZE - DB_BLOCK_META)

typedef struct Block {
  BlockMeta dbm;
  byte data[DB_BLOCK_DATA];
} Block;

typedef struct AOFS {
  Superblock sb;
  Block blocks[BLOCK_NUM];
} AOFS;

int find_free_bit(uint8_t map[], int num_bits) {
   int el_size = sizeof(uint8_t) * 8;
   int bit_offset, byte_num, bit;
   for (int i = 0; i < num_bits; i++) {
     byte_num   = i / el_size;
     bit_offset = i % el_size;
     bit = (map[byte_num] >> bit_offset) & 1U;
     if (!bit){
       return i;
     }
   }
   return -1;
}

//
int set_bit(uint8_t map[], int bit_idx) {
  int el_size    = sizeof(uint8_t) * 8;
  int bit_offset = bit_idx % el_size;
  int byte_num   = bit_idx / el_size;
  if ((map[byte_num] >> bit_offset) & 1U) { //Bit at bit_idx is already set
    return 1;
  } else {
    int mask = 1 << bit_offset;
    map[byte_num] |= mask;
  }
  return 0;
}

int read_fs(const char* filename, AOFS* fs);

int write_fs(const char* filename, AOFS* fs);


#endif



