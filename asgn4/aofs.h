#ifndef AOFS_H
#define AOFS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define byte unsigned char
#define EMPTY 0x00

//Magic number
#define MAGICNUM 0xfa19283e
//Block size
#define BLOCK_SIZE 4096
//Number of blocks
#define BLOCK_NUM 4096

#define BITMAP_SIZE (BLOCK_NUM / sizeof(uint8_t))

typedef struct SuperBlock {
  uint32_t magicnum;
  uint32_t totalblocks;
  uint32_t blocksize;
  uint8_t bitmap[BITMAP_SIZE];
} Superblock;

struct Block;

typedef struct BlockMeta {
  char filename[256];
  struct Block* next;
  bool head;
} BlockMeta;

//Datablock metadata size
#define BLOCK_META sizeof(BlockMeta)
#define BLOCK_DATA (BLOCK_SIZE - BLOCK_META)

typedef struct Block {
  BlockMeta dbm;
  byte data[BLOCK_DATA];
} Block;

typedef struct AOFS {
  Superblock sb;
  Block blocks[BLOCK_NUM];
} AOFS;

/* returns the index of the first free block */
int find_free_bit(uint8_t map[], int num_bits);
/* sets the specified bit in the bitmap to 1 */
int set_bit(uint8_t map[], int bit_idx);
/* sets the specified but in the bitmap to 0 */
int clear_bit(uint8_t map[], int bit_idx);
/* reads an AOFS structure in fs from specified file */
int read_fs(const char* filename, AOFS* fs);
/* writes the AOFS structure fs to a specified file */
int write_fs(const char* filename, AOFS* fs);
/* Initializes a new AOFS structure to default and empty values */
int init_fs(AOFS* fs);

#endif



