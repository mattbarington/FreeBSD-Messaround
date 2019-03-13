#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "aofs.h"
#include <inttypes.h>


void print_stbuf(struct stat *stbuf) {
  printf("----------<Current stbuf>----------\n");
  printf("st_dev:   %d\n", stbuf->st_dev);
  printf("st_ino:   %d\n", stbuf->st_ino);
  printf("st_nlink: %d\n", stbuf->st_nlink);
  printf("st_mode:  %x\n", stbuf->st_mode);
  printf("st_atime: %ld\n", stbuf->st_atime);
  printf("st_mtime: %ld\n", stbuf->st_mtime);
  printf("st_ctime: %ld\n", stbuf->st_ctime);
  printf("st_size:  %ld\n", stbuf->st_size);
  printf("-----------------------------------\n");
}

void print_fi_flags(struct fuse_file_info *fi) {
  int mask = 1;
  for(int i = 0; i < sizeof(int)*8; ++i) {
    if(mask & fi->flags) {
      printf("1");
    } else {
      printf("0");
    }
    mask = mask << 1;
  }
  printf("\n");
}

static int aofs_getattr(const char *path, struct stat *stbuf)
{
  int disk;
  if ((disk = OPEN_DISK) < 0) {
    printf("Unable to open disk img in %s\n",__func__);
    return disk;
  }
  printf("$$$ aofs_getattr. Path: %s\n",path);
  //  print_stbuf(stbuf);
  int res = 0;
  memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  } else {
    Block b;
    int file_head = aofs_find_file_head(disk, path, &b);
    if (file_head != -1) {
      BlockMeta* bm = &b.dbm;
      //stbuf->st_mode = S_IFREG | 0444;
      stbuf->st_mode    = S_IFREG | 0777;
      stbuf->st_nlink   = 1;
      stbuf->st_ino     = 0; //don't have, so init to 0
      stbuf->st_dev     = 0; //don't know what this is
      stbuf->st_atime   = bm->st_atim;
      stbuf->st_mtime   = bm->st_mtim;
      stbuf->st_ctime   = bm->st_ctim;
      stbuf->st_size    = bm->st_size;
      stbuf->st_blocks  = bm->st_blocks;
      stbuf->st_blksize = bm->st_blksize;
      stbuf->st_flags   = bm->st_flags;
      stbuf->st_uid     = getuid();
      stbuf->st_gid     = getgid();
      //stbuf->st_mode = S_IFREG | 0444 | 0777;
      //stbuf->st_mode = S_IFREG | 0777 | 0777;
      //stbuf->st_mode    = 0040777;
      stbuf->st_mode    = S_IFREG | 0444 | 0666;
      printf("Finished setting file.\n");	
      //print_stbuf(stbuf);
    }
    //file does not exist
    else {
      close(disk);
      return -ENOENT;
    }
  }
  close(disk);
  return res;
}

static int aofs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi)
{
  printf("$$$ aofs_readdir\n");

  (void) offset;
  (void) fi;
  int lgg = open("log.txt", O_CREAT | O_RDWR);
  if (lgg < 0) {
    printf("couldn't open log\n");
  }
  write(lgg, path, strlen(path));
  if (strcmp(path, "/") != 0) {
    filler(buf, path + 1, NULL, 0);
    return 0;
  }
  
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  //Find all file names with iterative loop *barf*
  Block block;
  SuperBlock sb;
  BlockMeta* bm;
  int disk = OPEN_DISK;
  if (disk < 0) {
    printf("There was a problem reading the disk image in %s\n", __func__);
    return disk;
  }
  
  read_super_block(disk, &sb);
  uint8_t* bitmap = sb.bitmap;
  for (int block_num = 0; block_num < BLOCK_NUM; block_num++) {
    if (bit_at(bitmap, block_num)) {
      read_block(disk, block_num, &block);
      bm = &block.dbm;
      if (bm->head) {
  	filler(buf, bm->filename + 1, NULL, 0);
      }
    }
  }
  close(disk);
  return 0;
}


static int aofs_open(const char *path, struct fuse_file_info *fi) {
  int disk = OPEN_DISK;
  if (disk < 0) {
    printf("Unable to open disk image in %s\n",__func__);
    return disk;
  }
  printf("$$$ aofs_open\n");
  printf("OPEN path: %s\n", path);

  fi->fh = 0;
  Block b;
  if(aofs_find_file_head(disk, path, &b) != -1) {
    close(disk);
    return 0;
  } else {  
    printf("aofs_open error. couldn't find specified file :/\n");
    close(disk);
    return -ENOENT;
  }
}

static int aofs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
  int disk = OPEN_DISK;
  if (disk < 0) {
    printf("Unable to open disk image in %s\n",__func__);
    return disk;
  }
  
  printf("$$$ aofs_create\n");

  //Create file at first avaliable block
  int x = aofs_create_file(disk, path);
  close(disk);
  printf("new file at: %d\n", x);
  if(x >= 0) {
    return 0;
  } else { return -1;}  
}


static int aofs_read(const char *path, char *buf, size_t size, off_t offset,
		     struct fuse_file_info *fi) {
  int disk = OPEN_DISK;
  if (disk < 0) {
    printf("Unable to open disk in %s\n",__func__);
    return disk;
  }    
  printf("$$$ aofs_read\n");
  printf("size: %lu, offset: %lu\n", size, offset);
  int bytes_read = aofs_read_file(disk, path, buf, size, offset);
  //  printf("Read %d bytes: '%s'\n", bytes_read, buf);
  close(disk);
  return bytes_read;
}

static int aofs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *file_info) {
  printf("$$$ aofs_write\n");
  int disk = OPEN_DISK;
  if (disk < 0) {
    printf("Unable to open disk image in %s\n",__func__);
    return disk;
  }
  printf("size: %lu, offset: %lu\n", size, offset);
  int bytes_written = aofs_write_file(disk, path, buf, size, offset);
  close(disk);
  return bytes_written;
}

static int aofs_release(const char* path, struct fuse_file_info *fi) {
  printf("$$$ aofs_release\n");
  return 0;
}

static int aofs_unlink(const char* path) {

  int disk = OPEN_DISK;
  if (disk < 0) {
    printf("Unable to open disk image in %s\n",__func__);
    return disk;
  }
  int res = aofs_delete_file(disk, path);
  close(disk);
  return res;
}

static int aofs_truncate(const char* path, off_t size) {

  printf("$$$ aofs_truncate\n");
  int disk = OPEN_DISK;
  if (disk < 0) {
    printf("Unable to open disk image in %s\n",__func__);
    return disk;
  }
 
  int res = aofs_truncate_file(disk, path, size);
  close(disk);
  return res;
}

static struct fuse_operations aofs_oper = {
	.getattr	= aofs_getattr,
	.readdir	= aofs_readdir,
	.create   = aofs_create,
	.read     = aofs_read,
	.open     = aofs_open,
	.write    = aofs_write,
	.release  = aofs_release,
  .unlink   = aofs_unlink,
  .truncate = aofs_truncate,
};

int main(int argc, char *argv[])
{
  
  //check number of arguments
  if(argc < 2) {
        printf("Incorrect number of arguments.\n");
    return 1;
  }

  //initialize foldername
  if(strlen(argv[1]) > MAX_FILENAME) {
    printf("Folder name must be less than %d characters.\n", MAX_FILENAME);
    return 1;
  }

  //make sure the file is readable and valid
  AOFS* aofs = calloc(1,sizeof(AOFS));
  char* filename = FS_FILE_NAME;
  if (read_fs(filename, aofs) == -1) {
    printf("There was a problem loading the disk image\n");
    exit(1);
  } else {
    free(aofs);
  }
  fuse_main(argc, argv, &aofs_oper, NULL);

  return 0;
}



