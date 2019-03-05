#ifndef AOFS_H
#define AOFS_H


//Magic number
#define MAGICNUM 0xfa19283e
//Block size
#define BLOCK_SIZE 4096
//Number of blocks
#define BLOCK_NUM 512
	 

typedef Superblock {
  uint32_t magicnum;
  uint32_t totalblocks;
  uint32_t blocksize;
  uint8_t bitmap[BLOCK_NUM / sizeof(uint8_t)];
} Superblock;


#endif



