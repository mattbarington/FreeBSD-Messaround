#ifndef AOFS_H
#define AOFS_H

#include <stdbool.h>

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

typedef struct BlockMeta {
  char filename[256];
  Block* next;
  bool head;
} Blockmeta;

//Datablock metadata size
#define DB_BLOCK_META sizeof(BlockMeta)
#define DB_BLOCK_DATA (DB_BLOCK_SIZE - DB_BLOCK_META)

typedef struct Block {
  byte data[DATA_SIZE]
} Block;

typedef struct AOFS {
  Superblock sb;
  Block blocks[BLOCK_NUM];
} AOFS;

#endif



