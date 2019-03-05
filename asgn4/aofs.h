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

typedef struct Superblock {
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
  Block db[BLOCK_NUM];
} AOFS;

#endif



