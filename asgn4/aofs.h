#ifndef AOFS_H
#define AOFS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define byte unsigned char
#define EMPTY 0x00

#define FS_FILE_NAME "FS_FILE"
#define MAX_FILENAME 255

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
} SuperBlock;

struct Block;

typedef struct BlockMeta {
  char filename[256];
  struct Block* next;
  bool head;
  int64_t create_time;
  int64_t access_time;
} BlockMeta;

//Datablock metadata size
#define BLOCK_META sizeof(BlockMeta)
#define BLOCK_DATA (BLOCK_SIZE - BLOCK_META)

typedef struct Block {
  BlockMeta dbm;
  byte data[BLOCK_DATA];
} Block;

typedef struct AOFS {
  int present;
  SuperBlock sb;
  Block blocks[BLOCK_NUM];
} AOFS;

/* returns the value of bit at idx */
int bit_at(uint8_t map[], int bit_idx);
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
/* Wipes block's data and metadata */
int clear_block(Block*);
/* Finds the first available block, marks it as unavailable, and returns its block address */
int aofs_allocate_block(AOFS* fs);
/* Initializes a single file block for a file */
int aofs_create_file(const char* filename, AOFS* fs);
/* Returns the block index of a file's head block */
int aofs_find_file_head(const char* filename, AOFS* fs);
/* Writes to a file */
int aofs_write(const char* filename, const char* buf, size_t size, AOFS* fs);
/* Attempts to write buffer into block. Returns the number of bytes written */
int aofs_write_to_block(const char* buf, Block* block, int bytes_to_write);

#endif



