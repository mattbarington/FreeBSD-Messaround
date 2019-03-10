#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "aofs.h"
#include <inttypes.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static const char* hola_path = "/HolaMundo.txt";

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

//static AOFS* get_context() {
//  return ((AOFS *) fuse_get_context()->private_data);
//}
static int aofs_getattr(const char *path, struct stat *stbuf)
{
  printf("aofs_getattr. Path: %s\n",path);
//  print_stbuf(stbuf);
  int res = 0;
  memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  } else {
    Block b;
    int file_head = aofs_find_file_head(path, &b);
    printf("file head at %d. This is where we copy over the metadata\n", file_head);
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
      print_stbuf(stbuf);
    }
    //file does not exist
    else {
      return -ENOENT;
    }
  }
  return res;
}

static int aofs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi)
{
  printf("readdir\n");

  (void) offset;
  (void) fi;
  
  if (strcmp(path, "/") != 0) {
    //    return -ENOENT;
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
    return -1;
  }

  //AOFS* fs = malloc(sizeof(AOFS));
  //  read_fs(FS_FILE_NAME, fs);

  //  uint8_t* bitmap = fs->sb.bitmap;
  
  read_super_block(disk, &sb);
  uint8_t* bitmap = sb.bitmap;
  for (int block_num = 0; block_num < BLOCK_NUM; block_num++) {
    if (bit_at(bitmap, block_num)) {
      //    memcpy(&block, &fs->blocks[block_num], sizeof(Block));
      read_block(disk, block_num, &block);
      bm = &block.dbm;
      //      printf("found a occupied boi[%d]:\n%s\n",block_num,bm->filename);
      if (bm->head) {
  	filler(buf, bm->filename + 1, NULL, 0);
      }
    }
  }
  return 0;
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

static int aofs_open(const char *path, struct fuse_file_info *fi) {

  printf("aofs_open\n");
  printf("OPEN path: %s\n", path);

  //  AOFS* fs = get_context();
//  print_fi_flags(fi);  
  fi->flags = O_RDWR | O_TRUNC | O_CREAT;
//  print_fi_flags(fi);
  fi->fh = 0;
//  printf("File descriptor: %" PRIu64 "\n", fi->fh);

  //check to see file exists
  Block b;
  if(aofs_find_file_head(path,&b) != -1) {
    fi->nonseekable = 0;
    fi->direct_io   = 0;
    //fi->fh          = open(path, fi->flags, S_IRWXU | S_IRWXG | S_IRWXO);
    fi->flush       = 0;
    fi->writepage   = 0;
    printf("Set fuse_file_info\n");
    printf("File descriptor: %" PRIu64 "\n", fi->fh);
    return ((fi->fh < 0) ? -ENOENT : 0);
  } else {  
    printf("aofs_open error. couldn't find specified file :/\n");
    return -ENOENT;
  }
}

static int aofs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

  printf("aofs_create\n");

  //  AOFS* fs = get_context();

  //if file already exists, run open operation instead??
  //if(aofs_find_file_head(path, fs)) {
  //  return aofs_open(path, fi);
  //}

  //Create file at first avaliable block
  int x = aofs_create_file(path);
  printf("new file at: %d\n", x);
  if(x >= 0) {
    return 0;
  } else { return -1;}  
}


static int aofs2_read(const char *path, char *buf, size_t size, off_t offset,
		     struct fuse_file_info *fi) {
  printf("aofs_read\n");
  printf("size: %lu, offset: %lu\n", size, offset);
  //read_file(path, buf, size, offset);
  return 0;
}

static int aofs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *file_info) {
  printf("aofs_write\n");
  return 0;
}
static int aofs_statfs(const char* path, struct statvfs* stbufi) {
  printf("aofs_statfs\n");
  return 0;
}

static int aofs_release(const char* path, struct fuse_file_info *fi) {
  printf("aofs_release\n");
  //printf("File descriptor: %" PRIu64 "\n", fi->fh);
  //close(fi->fh);
  return 0;
}

static struct fuse_operations aofs_oper = {
	.getattr	= aofs_getattr,
	.readdir	= aofs_readdir,
	.create   = aofs_create,
  .read     = aofs2_read,
	.open     = aofs_open,
  .write    = aofs_write,
  .statfs   = aofs_statfs,
  .release  = aofs_release,
};

int main(int argc, char *argv[])
{
  
  //check number of arguments
  if(argc < 2) {
        printf("Incorrect number of arguments.\n");
    //    printf("Incorrect number of arguments.\nMust Provide <disk_img> <mount folder>");
    return 1;
  }

  //initialize foldername
  if(strlen(argv[1]) > MAX_FILENAME) {
    printf("Folder name must be less than %d characters.\n", MAX_FILENAME);
    return 1;
  }

  //printf("Launching AOFS file system from img %s in %s\n", argv[1], argv[2]);

  AOFS* aofs = calloc(1,sizeof(AOFS));
  //char* buf = malloc(sizeof(AOFS));
  char* filename = FS_FILE_NAME;
  if (read_fs(filename, aofs) == -1) {
    printf("There was a problem loading the disk image\n");
    exit(1);
  }

  aofs_create_file("ayylmao");
  
  int disk = OPEN_DISK;
  if (disk < 0)
    printf("error opening disk >:(\n");

  SuperBlock super;
  Block* block = malloc(sizeof(Block));
  read_super_block(disk, &super);
  int first_free = find_free_bit(super.bitmap, super.totalblocks);
  printf("First free should be like 1? %d\n", first_free);
  free(block);
  fuse_main(argc, argv, &aofs_oper, NULL);

  return 0;
}



