
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
        printf("Blocks[%d].h ->%3d: %s\n", b, block.dbm.next, block.dbm.filename);
      } else {
	printf("Blocks[%d].t ->%3d: %s\n", b, block.dbm.next, block.dbm.filename);
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
  block->dbm.st_blksize = BLOCK_DATA;
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
  //  printf("Head block for file %s initialized at block %d\n", filename,block_num);
  //  int disk = OPEN_DISK;
  write_block(disk, block_num, &b);

  SuperBlock sb;
  read_super_block(disk, &sb);
  int first_free = find_free_bit(sb.bitmap, sb.totalblocks);
  //  printf("first free after creation: %d\n", first_free);

  
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
  //  if (offset < 0)
  //    offset = 0;    // this is just silly
  if (bytes_to_write + offset > BLOCK_DATA) // offset is in this block. 
    bytes_to_write = BLOCK_DATA - offset;   // can't write all bytes, so write as many as will fit
  printf("copying bytes from  %ld of length %d to block\n", offset, bytes_to_write);
  memcpy(&block->data[offset], buf, bytes_to_write);
  return bytes_to_write;
}

int delete_chain(int disk, int blockidx) {
  do {
    blockidx = aofs_deallocate_block(disk, blockidx);
  } while(blockidx != -1);
  return 0;
}

int aofs_write_file(int disk, const char* filename, const char* buf, size_t size, off_t offset) {
  int bytes_to_write = size;
  int bytes_written = 0;
  Block* block = malloc(sizeof(Block));
  Block* next_block = malloc(sizeof(Block));
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
  write_block(disk, block_num, block);
  //  printf("size = %zu\n", size+offset);
  bytes_to_write = size;
  while (bytes_to_write > 0) {
    if (offset > BLOCK_DATA) {  //traverse down block list until finding the block with offset
      offset -= BLOCK_DATA;
      read_block(disk, block->dbm.next, block);
      continue;
    }
    //    printf("about to write to block %d for %s=%s\n", block_num, filename,block->dbm.filename);
    int wr = write_to_block(buf, block, bytes_to_write, offset);
    //    printf("just  wrote to block %d for %s=%s\n", block_num, filename,block->dbm.filename);
    //    printf("%s\n", block->data);
    bytes_to_write -= wr;
    buf = &buf[wr];
    bytes_written += wr;
    offset = 0;
    if (bytes_to_write > 0 && block->dbm.next == -1) {
      int b = aofs_allocate_block(disk);
      block->dbm.next = b;
      read_block(disk, b, next_block);
      strcpy(next_block->dbm.filename, filename);
      //      printf("allocating block %d for file %s\n",b,filename);
    } else {
      read_block(disk, block->dbm.next, next_block);
    }
    //    printf("storing block %d for %s=%s\n", block_num, filename,block->dbm.filename);
    write_block(disk, block_num, block);
    //    printf("just  wrote to block(2) %d for %s=%s\n", block_num, filename,block->dbm.filename);
    Block bb;
    read_block(disk, block_num, &bb);
    //    printf("checking that the fetch matches for block %d, %s=%s\n", block_num, filename, bb.dbm.filename);
    //    printf("%s\n", bb.data);
    block_num = block->dbm.next;
    memcpy(block, next_block, sizeof(Block));
    
  }

  aofs_find_file_head(disk, filename, block);
  //  printf("ending file size %s = %ld\n", block->dbm.filename, block->dbm.st_size);
  
  print_aofs(disk);
  free(block);
  free(next_block);
  return bytes_written;
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
  printf("copying [first] block %d to position 0\n", start_block);
  //  printf("\n\nREAD 1ST BLOCK OF %s with offset %ld:\n%s",path, offset, curblock.data);
  //  return bytes_read;
  
  //read additional full blocks
  // [...]-[.S.]-[RRR]-[.E.]
  for(int i = 0; i < num_blocks - 1; ++i) {
    int next_block = curblock.dbm.next;
    if(next_block < 0) {
      //      close(fd);
      return bytes_read;
    }
    read_block(disk, next_block, &curblock);
    //    buf = &buf[bytes_read];
    //    printf("copying to bytes from %d of range %d\n", buf + block_offset + (BLOCK_DATA * i), BLOCK_DATA);
    printf("copying [middle] block %d to position %lu\n", next_block, block_offset + (BLOCK_DATA * i));
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
    printf("copying [last] block %d to position %lu\n", next_block, block_offset + (BLOCK_DATA *( num_blocks)));
    printf("copying last amount: %zu\n",size_l);

    memcpy(buf + block_offset + (BLOCK_DATA * (num_blocks)), curblock.data, size_l);
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

int aofs_truncate_file(int fd, const char* path, off_t size) {

  Block headblock, curblock;
  int file_head;
  off_t cur_size;
  int cur_blocks, new_blocks, diff_blocks;
  int prev, next;
  int i;

  file_head = aofs_find_file_head(fd, path, &headblock);
  if(file_head == -1) {
    return -ENOENT;
  }

  read_block(fd, file_head, &curblock);
  cur_size = curblock.dbm.st_size;
  cur_blocks = (cur_size - 1) / BLOCK_DATA;
  new_blocks = (size - 1) / BLOCK_DATA;

  //break down file
  if(size < cur_size) {
    //delete extra blocks
    if(cur_blocks - new_blocks) {
      //read to last block to keep
      for(i = 0; i < new_blocks; ++i) {
        read_block(fd, curblock.dbm.next, &curblock);
      }
      //remove all blocks after
      next = curblock.dbm.next;
      while(next != -1) {
        next = aofs_deallocate_block(fd, next);
      }
    }

 }

  //expand file
  else if(size > cur_size) {
    //add extra blocks
    diff_blocks = new_blocks - cur_blocks;
    if(diff_blocks) {
      //seek to last block
      prev = file_head;
      while(curblock.dbm.next != -1) {
        prev = curblock.dbm.next;
        read_block(fd, curblock.dbm.next, &curblock);
      }
      //allocate new blocks
      for(i = 0; i < diff_blocks; ++i) {
        curblock.dbm.next = aofs_allocate_block(fd);
        //if new block couldn't be allocated
        if(curblock.dbm.next == -1) {
          return -1;
        } else {
          //write previous block with changed next pointer
          write_block(fd, prev, &curblock);
          //initialize new block
          prev = curblock.dbm.next;
          read_block(fd, prev, &curblock);
          clear_block(&curblock);
          strcpy(curblock.dbm.filename, path);
          write_block(fd, prev, &curblock);
        }
      }
    }
  }
 
  //change size in head block
  headblock.dbm.st_size = size;
  write_block(fd, file_head, &headblock);
 
  return 0;
  
}
