#ifndef AOFS_H
#define AOFS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/_types.h>
#include <fcntl.h>
#include <limits.h>

#define byte unsigned char
#define EMPTY 0x00

#define FS_FILE_NAME "/FS_FILE"   //Stick FS_FILE in top level directory so that it's always reachable
#define MAX_FILENAME 255

//Magic number
#define MAGICNUM 0xfa19283e
//Block size
#define BLOCK_SIZE 4096
//Number of blocks
#define BLOCK_NUM 4096

#define BITMAP_SIZE (BLOCK_NUM / sizeof(uint8_t))
#define SUPER_BLOCK_OFFSET 0
#define BLOCK_OFFSET(block_num) sizeof(SuperBlock) + (block_num*sizeof(Block))
#define OPEN_DISK open(FS_FILE_NAME, O_RDWR, 0777)

typedef struct SuperBlock {
  uint32_t magicnum;
  uint32_t totalblocks;
  uint32_t blocksize;
  uint8_t bitmap[BITMAP_SIZE];
} SuperBlock;

struct Block;

typedef struct BlockMeta {
  char filename[256];
  int next;
  bool head;
  __mode_t    st_mode;              /* inode protection mode */
  uint64_t  st_atim;              /* time of last access */
  uint64_t  st_mtim;              /* time of last data modification */
  uint64_t  st_ctim;              /* time of last file status change */
  off_t     st_size;              /* file size, in bytes */
  __blkcnt_t st_blocks;             /* blocks allocated for file */
  __blksize_t st_blksize;           /* optimal blocksize for I/O */
  __fflags_t  st_flags;             /* user defined flags for file */
  uint64_t  st_birthtim;          /* time of file creation */
} BlockMeta;

//Datablock metadata size
#define BLOCK_META sizeof(BlockMeta)
#define BLOCK_DATA (BLOCK_SIZE - BLOCK_META)

typedef struct Block {
  BlockMeta dbm;
  byte data[BLOCK_DATA];
} Block;

typedef struct AOFS {
  SuperBlock sb;
  Block blocks[BLOCK_NUM];
} AOFS;

/* prints out the head block for each file and their block_num in Blocks[] */
void print_aofs(int disk);

void print_block(int disk, int block_num);
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

/* Get block block_num from disk  and stores in provided buffer */
int read_block(int fd, int block_num, Block* block);
/* Get SuperBLock from disk and stores in provided buffer */
int read_super_block(int fd, SuperBlock*);
/* Writes provided block to disk img */
int write_block(int fd, int block_num, Block* block);
/* Deletes file blocks in chain, starting at block_num */
int delete_chain(int fd, int block_num);

/* Initializes a new AOFS structure to default and empty values */
int init_fs(AOFS* fs);
/* Wipes block's data and metadata */
int clear_block(Block*);
/* Finds the first available block, marks it as unavailable, and returns its
 * block address
*/
int allocate_block(int fd);
/* removes block at index block_num from the file system, returning the block's next index */
int deallocate_block(int fd, int block_num);

/* Returns the block index of a file's head block */
int aofs_find_file_head(int fd, const char* filename, Block* block);
/* Initializes a single file block for a file */
int aofs_create_file(int fd, const char* filename);
/* Reads size bytes of file into buffer starting from offset. Returns number of
 *bytes read 
*/
int aofs_read_file(int fd, const char* path, char* buf, size_t size, off_t offset);
/* Writes to a file with filename 'filename'. Creates file if needed */
int aofs_write_file(int fd, const char* filename, const char* buf, size_t size, off_t offset);
/* Deletes a file with filename 'path' */
int aofs_delete_file(int fd, const char* path);
/* Truncates file to specified size. If size > filesize the difference is padded with 0 */
int aofs_truncate_file(int fd, const char* path, off_t size);

#endif



