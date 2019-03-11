
#include "aofs.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

void print_aofs(int disk) {
  SuperBlock sb;
  read_super_block(disk, &sb);
  Block block;
  for (int b = 0; b < sb.totalblocks; b++) {
    if (bit_at(sb.bitmap, b)){
      read_block(disk, b, &block);
      if (block.dbm.head) {
        printf("Blocks[%d]: %s\n", b, block.dbm.filename);
      }
    }
  }
}

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

int aofs_allocate_block(int disk) {
  SuperBlock sb;
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
  return block_num;
}

int aofs_deallocate_block(int disk, int block_num) {
  SuperBlock sb;
  read_super_block(disk, &sb);
  uint8_t* map = sb.bitmap;
  clear_bit(map, block_num);
  Block b;
  clear_block(&b);
  write_block(disk, block_num, &b);
  write_super_block(disk, &sb);
  return b.dbm.next;
}

int aofs_create_file(int disk, const char* filename) {
  int block_num = aofs_allocate_block(disk);
  if (block_num == -1) {
    printf("There was a problem allocating a new block in %s\n", __func__);
    return -1;
  }
  Block b;
  clear_block(&b);
  strcpy(b.dbm.filename, filename);
  b.dbm.head = true;
  printf("Head block for file %s initialized at block %d\n", filename,block_num);
  //  int disk = OPEN_DISK;
  write_block(disk, block_num, &b);

  SuperBlock sb;
  read_super_block(disk, &sb);
  int first_free = find_free_bit(sb.bitmap, sb.totalblocks);
  printf("first free after creation: %d\n", first_free);

  
  //  close(disk);
  return block_num;
}

int aofs_find_file_head(int disk, const char* filename, Block* block) {
  SuperBlock sb;
  read_super_block(disk, &sb);
  uint8_t* bitmap = sb.bitmap;
  int block_num;
  Block* b = malloc(sizeof(Block));
  for (block_num = 0; block_num < BLOCK_NUM; block_num++) {
    if (bit_at(bitmap, block_num)) {
      read_block(disk, block_num, b);
      if (b->dbm.head && !strcmp(b->dbm.filename,filename)) { //is a head block with matching filename
	memcpy(block, b, sizeof(Block));
	return block_num;
      }
    }
  }
  return -1;
}

int write_to_block(const char* buf, Block* block, int bytes_to_write, off_t offset) {
  if (offset > BLOCK_DATA)
    return BLOCK_DATA; //offset is beyond this block
  if (offset < 0)
    offset = 0;    // this is just silly
  if (bytes_to_write + offset > BLOCK_DATA) // offset is in this block. 
    bytes_to_write = BLOCK_DATA - offset;   // can't write all bytes, so write as many as will fit
  memcpy(&block->data[offset], buf, bytes_to_write);
  return bytes_to_write;
}

int delete_chain(int disk, int blockidx) {
  do {
    blockidx = aofs_deallocate_block(disk, blockidx);
  } while(blockidx != -1);
  return 0;
}

int aofs_write_file(int disk, const char* filename, char* buf, size_t size, off_t offset) {


  int bytes_to_write = size;
  int start_byte;
  Block* block = malloc(sizeof(Block));
  Block* bp = malloc(sizeof(Block));
  int block_num = aofs_find_file_head(disk, filename, block);
  //if file doesn't exist create it
  if (block_num < 0) {                      
    block_num = aofs_create_file(disk, filename);
    read_block(disk, block_num, block);
  }
  //if offset > file size, then set offset to file size
  if (offset > block->dbm.st_size)
    offset = block->dbm.st_size;
  // new file size will be number of bytes written + offset starting pos
  block->dbm.st_size = size + offset;
  // we don't "write" the offset but we can consider this as padding buf.
  bytes_to_write += offset;
  while (bytes_to_write > 0) {
    
    write_to_block(buf, block, bytes_to_write, offset);
    printf("writing block %d to disk\n", block_num);
    write_block(disk, block_num, block);
    bytes_to_write -= BLOCK_DATA;
    if (bytes_to_write > 0 && block->dbm.next == -1) {
      printf("still more to write and no next. better allocate another block\n");
      int b = aofs_allocate_block(disk);
      strcpy(bp->dbm.filename,filename);
      block->dbm.next = b;
    }
    printf("block's next = %d. bytes_left = %d\n", block->dbm.next, bytes_to_write);
    block_num = block->dbm.next;
  }

  printf("after write:\n");
  print_aofs(disk);
  
  return 0;
  


  
  //  aofs_create_file(disk, filename);
  //return 0;
  printf("ah, you want to write '%s' to '%s\n", buf, filename);
  Block curblock;
  
  //find head block for file
  int head_block = aofs_find_file_head(disk, filename, &curblock);
  if(head_block == -1) {
    printf("Can't file head block for %s\n", filename);
    head_block = aofs_create_file(disk, filename);
    printf("Allocated head block %d for '%s'\n", head_block, filename);
    read_block(disk, head_block, &curblock);
  }
  //First: assume <= 1 block, offset = 0
  curblock.dbm.st_size = size;
  memcpy(&curblock.data, buf, size);
  write_block(disk, head_block, &curblock);
  
  //Calculate positions to write
  
  //write everything possible in first block
}

 

int aofs_read_file(int disk, const char* path, char* buf, size_t size, off_t offset) {
  printf("aofs read_file\n");

  //open up disk image
  //  int fd = OPEN_DISK;

  //  char filename[257];
  //fix path to just be filename
  //  strcpy(filename, path);
  //  memmove(filename, filename+1, strlen(filename));
  
  //find head block for file
  Block curblock;
  int head_block = aofs_find_file_head(disk, path, &curblock);
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
       //       close(fd);
       return 0;
     }
     read_block(disk, next_block, &curblock);
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
      //      close(fd);
      return bytes_read;
    }
    read_block(disk, next_block, &curblock);
    memcpy(buf + block_offset + (BLOCK_DATA * i), curblock.data, BLOCK_DATA);
    bytes_read += BLOCK_DATA;
  }
  //read last block
  // [...]-[.S.]-[...]-[RR.]
  if(num_blocks > 0) {
    int next_block = curblock.dbm.next;
    if(next_block < 0) {
      //      close(fd);
      return bytes_read;
    }
    read_block(disk, next_block, &curblock);
    memcpy(buf + block_offset + (BLOCK_DATA * (num_blocks - 1)), curblock.data, size_l);
    bytes_read += size_l;
  }

  //all blocks read to buffer
  // buf = [...]-[.BB]-[BBB]-[BB.]
  //  close(fd);
  return bytes_read;
}

int aofs_delete_file(int disk, const char* path) {

  Block curblock;
  int blockidx;

  //find file head
  blockidx = aofs_find_file_head(disk, path, &curblock); 
  if (blockidx < 0) {
    return -ENOENT;
  }

  //remove file recursively
  delete_chain(disk, blockidx);

  return 0;
}

