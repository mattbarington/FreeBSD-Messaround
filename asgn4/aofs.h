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

typedef Superblock {
  uint32_t magicnum;
  uint32_t totalblocks;
  uint32_t blocksize;
  uint8_t bitmap[BLOCK_NUM / sizeof(uint8_t)];
} Superblock;

/*
typedef Datablockmeta {
  char filename[256];
  Datablock* next;
  bool head;
} Datablockmeta;

//Datablock metadata size
#define DB_BLOCK_META sizeof(Datablockmeta)
#define DB_BLOCK_DATA (DB_BLOCK_SIZE - DB_BLOCK_META)

typedef Datablock {
  byte data[DATA_SIZE]
} Datablock;

typedef Datablocks {
  Datablock[BLOCK_NUM];
} Datablocks;

typedef AOFS {
  Superblock sb;
  Datablocks db;
} AOFS;
*/
#endif



