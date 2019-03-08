
#include "aofs.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

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

  //pretend to put file "HolaMundo.txt" at block 42. This is for testing only.
  int block = 42;
  if (set_bit(fs->sb.bitmap, block)) {
    printf("Something went wrong while trying to make HolaMundo file\n");
  } else {
    Block* b = &fs->blocks[block];
    const char* buf = "Hola Mundo, come mis shorts\n";
    const char* hola_path = "/HolaMundo.txt";
    const char* dab = "DAB ON EM *dab* *dab* *dab*\n";
    const char* ayy_path = "/AyyLmao.txt";
    aofs_write_file(ayy_path, dab, strlen(dab), fs);
    aofs_write_file(hola_path, buf, strlen(buf), fs);
  }
  return 0;
}


int read_fs(const char* filename, AOFS* fs) {
  FILE* rf;
    
  rf = fopen(filename, "r");
  if(rf) {
    fread(fs, sizeof(AOFS), 1, rf);
    fclose(rf);
    return 0;
  } else {
    return -1;
  }
}

int write_fs(const char* filename, AOFS* fs) {
  FILE* wf;

  wf = fopen(filename, "w");
  if(wf) {
    fwrite(fs, sizeof(AOFS), 1, wf);
    fclose(wf);
    return 0;
  } else {
    return -1;
  }
}

int clear_block(Block* block) {
  strcpy(block->dbm.filename, "");
  block->dbm.next = NULL;
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
  memset(i_b, 0, BLOCK_DATA*sizeof(i_b[0]));
  return 0;
}

int aofs_allocate_block(AOFS* fs) {
  SuperBlock *sb = &fs->sb;
  uint8_t* map = fs->sb.bitmap;
  int block_num = find_free_bit(map, sb->totalblocks);
  if (block_num == -1) {
    return -1;
  }
  set_bit(map, block_num);
  Block *nb = &fs->blocks[block_num];
  clear_block(nb);
  return block_num;
}

int aofs_create_file(const char* filename, AOFS* fs) {
  int block_num = aofs_allocate_block(fs);
  if (block_num < 0)
    return -ENOMEM;
  
  Block *nb = &fs->blocks[block_num];
  BlockMeta *nb_meta = &nb->dbm;
  strcpy(nb_meta->filename, filename);
  nb_meta->head = true;
  return block_num;
}

int aofs_find_file_head(const char* filename, AOFS* fs) {
  //  printf("in findfile_head. Looking for %s\n",filename);
  BlockMeta* block = NULL;
  int block_num;
  for (block_num = 0; block_num < BLOCK_NUM; block_num++) {
    if (bit_at(fs->sb.bitmap, block_num)) {
      block = &fs->blocks[block_num].dbm;
      //      if (block->head)
      //	printf("is '%s' the file you're looking for?\n", block->filename);
      if (block->head && !strcmp(block->filename,filename)) { // is a head block with matching filename
	//	printf("find_file_head found '%s' head at %d\n", filename, block_num);
	return block_num;
      }
    }
  }
  return -1;
}

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
