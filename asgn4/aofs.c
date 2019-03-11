
#include "aofs.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int bit_at(uint8_t map[], int idx) {
  int el_size    = sizeof(uint8_t) * 8;
  int bit_offset = idx % el_size;
  int byte_num   = idx / el_size;
  return (map[byte_num] >> bit_offset) & 1U;
}

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
    return -1;
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
    return -1;
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
    clear_block(&fs->blocks[iter]);
  }
  return 0;
}


int read_fs(const char* filename, AOFS* fs) {
  int disk = open(filename, O_RDWR | O_CREAT, 0777);
  if (disk < 0) {
    printf("There was a problem opening the disk image in %s\n",__func__);
    return -1;
  }
  const int super_off = 0;
  const int block_offset = sizeof(SuperBlock);
  lseek(disk, SUPER_BLOCK_OFFSET, SEEK_SET);
  read(disk, &fs->sb, sizeof(SuperBlock));

  for (int b = 0; b < fs->sb.totalblocks; b++) {
    lseek(disk, BLOCK_OFFSET(b), SEEK_SET);
    read(disk, &fs->blocks[b], sizeof(Block));
  }
  close(disk);
  return 0;
}

int write_fs(const char* filename, AOFS* fs) {
  int disk = open(filename, O_RDWR | O_CREAT, 0777);
  if (disk < 0) {
    printf("There was a problem opening the disk image in %s\n",__func__);
    return -1;
  }
  const int super_off = 0;
  const int block_offset = sizeof(SuperBlock);
  lseek(disk, SUPER_BLOCK_OFFSET, SEEK_SET);
  write(disk, &fs->sb, sizeof(SuperBlock));

  for (int b = 0; b < fs->sb.totalblocks; b++) {
    lseek(disk, BLOCK_OFFSET(b), SEEK_SET);
    write(disk, &fs->blocks[b], sizeof(Block));
  }
  close(disk);
  return 0;
}

int read_block(int fd, int block_num, Block* block) {
  lseek(fd, BLOCK_OFFSET(block_num), SEEK_SET);
  return read(fd, block, sizeof(Block));
}

int read_super_block(int fd, SuperBlock* sb) {
  lseek(fd, SUPER_BLOCK_OFFSET, SEEK_SET);
  return read(fd, sb, sizeof(SuperBlock));
}

int write_block(int fd, int block_num, Block* block) {
  lseek(fd, BLOCK_OFFSET(block_num), SEEK_SET);
  return write(fd, block, sizeof(Block));
}

int write_super_block(int fd, SuperBlock* sb) {
  lseek(fd, SUPER_BLOCK_OFFSET, SEEK_SET);
  return write(fd, sb, sizeof(SuperBlock));
}


int clear_block(Block* block) {
  strcpy(block->dbm.filename, "");
  block->dbm.next = -1;
  block->dbm.head = false;
  
  block->dbm.st_atim = time(NULL);
  block->dbm.st_mtim = time(NULL);
  block->dbm.st_ctim = time(NULL);
  block->dbm.st_birthtim = time(NULL);
  block->dbm.st_size = 0;
  block->dbm.st_blocks = 0;
  block->dbm.st_blksize = BLOCK_SIZE;
  block->dbm.st_flags = 0;
  
  
  byte* i_b = block->data;
  memset((char*)i_b, 0, BLOCK_DATA*sizeof(i_b[0]));
  char empty[2] = "\0";
  strcpy((char*)i_b, empty);
  return 0;
}

int aofs_allocate_block() {
  SuperBlock sb;
  int disk = OPEN_DISK;
  if (disk < 0) {
    printf("There was a problem opening the disk image in %s\n", __func__);
    return -1;
  }
  read_super_block(disk, &sb);
  uint8_t* map = sb.bitmap;
  int block_num = find_free_bit(map,sb.totalblocks);
  if (block_num == -1) {
    return -1;
  }
  set_bit(map, block_num);
  Block b;
  clear_block(&b);
  write_block(disk, block_num, &b);
  write_super_block(disk, &sb);
  close(disk);
  return block_num;
}

int aofs_create_file(const char* filename) {
  int block_num = aofs_allocate_block();
  if (block_num == -1) {
    printf("There was a problem allocating a new block in %s\n", __func__);
    return -1;
  }
  Block b;
  clear_block(&b);
  strcpy(b.dbm.filename, filename);
  b.dbm.head = true;
  printf("Head block for file %s initialized at block %d\n", filename,block_num);
  int disk = OPEN_DISK;
  write_block(disk, block_num, &b);

  SuperBlock sb;
  read_super_block(disk, &sb);
  int first_free = find_free_bit(sb.bitmap, sb.totalblocks);
  printf("first free after creation: %d\n", first_free);

  
  close(disk);
  return block_num;
}

int aofs_find_file_head(const char* filename, Block* block) {
  SuperBlock sb;
  int disk = OPEN_DISK;
  read_super_block(disk, &sb);
  uint8_t* bitmap = sb.bitmap;
  int block_num;
  Block* b = malloc(sizeof(Block));
  for (block_num = 0; block_num < BLOCK_NUM; block_num++) {
    if (bit_at(bitmap, block_num)) {
      read_block(disk, block_num, b);
      if (b->dbm.head && !strcmp(b->dbm.filename,filename)) { // is a head block with matching filename
	memcpy(block, b, sizeof(Block));
	close(disk);
	return block_num;
      }
    }
  }
  close(disk);
  return -1;
}

/*
int aofs_write_file(const char* filename, const char* buf, size_t size, AOFS* fs) {
  int bytes_to_write = size;
  int start_byte;
  Block *block = NULL;
  int block_num = aofs_find_file_head(filename, fs);
  if (block_num < 0) {
    block_num = aofs_create_file(filename, fs);
  }
  block = &fs->blocks[block_num];
  while (bytes_to_write > 0) {
    start_byte = size - bytes_to_write;
    aofs_write_to_block(&buf[start_byte], block, bytes_to_write);
    bytes_to_write -= BLOCK_DATA;
    if (bytes_to_write > 0 && block->dbm.next == NULL) {
      int b = aofs_allocate_block(fs);
      Block* bp = &fs->blocks[b];
      strcpy(bp->dbm.filename,filename);
      block->dbm.next = bp;
    }
    block = block->dbm.next;
  }  
  return 0;
}

int aofs_write_to_block(const char* buf, Block* block, int bytes_to_write) {
  if (bytes_to_write > BLOCK_DATA)
    bytes_to_write = BLOCK_DATA;
  memcpy(block->data, buf, bytes_to_write);
  return 0;
}
*/
int aofs_write_file(const char* path, char* buf, size_t size, off_t offset) {

  int fd = OPEN_DISK;
  Block curblock;
  char filename[257];
  //fix path to just be filename
  strcpy(filename, path);
  memmove(filename, filename+1, strlen(filename));
 
  //find head block for file
  if(aofs_find_file_head(filename, &curblock) < 0) {
    printf("Can't file head block for %s\n", filename);
    close(fd);
    return -ENOENT;
  }

  //Calculate positions to write
  
  //write everything possible in first block


  return 0;
}

int aofs_read_file(const char* path, char* buf, size_t size, off_t offset) {
  printf("aofs read_file\n");

  //open up disk image
  int fd = OPEN_DISK;

  char filename[257];
  //fix path to just be filename
  strcpy(filename, path);
  //  memmove(filename, filename+1, strlen(filename));
  
  //find head block for file
  Block curblock;
  int head_block = aofs_find_file_head(filename, &curblock);
  printf("%s: head block = %d\n",__func__, head_block);
  //calculate where buf should read from
  int start_block, num_blocks, block_offset;
  size_t size_f, size_l;
  int bytes_read = 0;
  /*
  // NOTE: blocks indexed at 0, num_blocks = additional blocks beyond head block
  // Example (BLOCK_DATA = 3)
  // [...]-[.S.]-[...]-[.E.]
  // start_block = 4 / 3 => 1
  // block_offset = 4 - (1 * 3) => 1
  // num_blocks = (1 + 7) / 3 => 2
  // size_f = 3 - 1 => 2
  // size_l = 7 - (2 + ((2 - 1) * 3)) => 2
  */ 

  //number of blocks away from the head block
  start_block = offset / BLOCK_DATA;
  //true offset considering start_block to offset from
  block_offset = offset - (start_block * BLOCK_DATA);
  //number of blocks to read (-1 in case exactly BLOCK_DATA read)
  num_blocks = (block_offset + size - 1) / BLOCK_DATA;
  //size of the first block
  size_f = ((num_blocks == 0) ? size : BLOCK_DATA - block_offset);
  //size of the last block
  size_l = ((num_blocks == 0) ? 0 : size - (size_f + ((num_blocks - 1) * BLOCK_DATA)));

    printf("+------------------READ------------+\n");
    printf("| size_t size     : %9zu |\n", size);
    printf("| off_t offset    : %9zu |\n", offset);
    printf("| start_block     : %9d |\n", start_block);
    printf("| block_offset    : %9d |\n", block_offset);
    printf("| num_blocks      : %9d |\n", num_blocks);
    printf("| size_f          : %9zu |\n", size_f);
    printf("| size_l          : %9zu |\n", size_l);
    printf("+----------------------------------+\n");


  
   //get the first block from which to begin reading.
   for(int i = 0; i < start_block; ++i) {
     int next_block = curblock.dbm.next;
     if(next_block < 0) {
       close(fd);
       return 0;
     }
     read_block(fd, next_block, &curblock);
   }
  
  

  //read first block
  // [...]-[.RR]-[...]-[.E.]
  memcpy(buf, curblock.data + block_offset, size_f);
  bytes_read = size_f;
  
  //read additional full blocks
  // [...]-[.S.]-[RRR]-[.E.]
  for(int i = 1; i < num_blocks; ++i) {
    int next_block = curblock.dbm.next;
    if(next_block < 0) {
      close(fd);
      return bytes_read;
    }
    read_block(fd, next_block, &curblock);
    memcpy(buf + block_offset + (BLOCK_DATA * i), curblock.data, BLOCK_DATA);
    bytes_read += BLOCK_DATA;
  }
  //read last block
  // [...]-[.S.]-[...]-[RR.]
  if(num_blocks > 0) {
    int next_block = curblock.dbm.next;
    if(next_block < 0) {
      close(fd);
      return bytes_read;
    }
    read_block(fd, next_block, &curblock);
    memcpy(buf + block_offset + (BLOCK_DATA * (num_blocks - 1)), curblock.data, size_l);
    bytes_read += size_l;
  }

  //all blocks read to buffer
  // buf = [...]-[.BB]-[BBB]-[BB.]
  close(fd);
  return bytes_read;
}

