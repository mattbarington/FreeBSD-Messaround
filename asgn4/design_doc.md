Author(s): Matt Ovenden, Ryan Blelloch

# Assignment 4 Design Document

## Brief Description:
  - This is a simple AOFS (All-In-One Folder File System) implemented using 
    FUSE. It has basic functionality of reading and writing files are arbitrary 
    size.

## All files in REPO:

* README                - Directions for all compilation/running

* aofs.h                - Header file for AOFS low-level functions

* aofs.c                - All implementation for AOFS low-level functions

* run_fs.c              - All implementation for FUSE operation functions, 
                           including fuse_main entry

* init_disk.c           - Standalone implementation for creating new FS_FILE

## Source Files explanation

### aofs.h
  - Header declaration for all aofs functions.
  - Contains definition for structs:
    + AOFS - all encompassing structure of the file system
    + SuperBlock - contains magicnum, block allocation bitmap, totalblocks in
                  the system, and the blocksize
    + Block - byte[] of size BLOCK_DATA and BlockMeta
    + BlockMeta - metainformation about the block including filename, 
                timestamps, and the index of the next block in the file
                
  - Constants such as BLOCK_SIZE and FS_FILE_NAME
 
### aofs.c
  - All low-level functions that read and write to the filesystem are found here. 
  - Functionalilty includes: AOFS initialization, file reads/writes, block 
    reads/writes
  - These functions are called by FUSE oper functions to interface with the 
    actual AOFS filesystem

### run_fs.c
  - All FUSE operation functions defined here, as well as the main() that will 
    call fuse_main
  - Each FUSE operation is implemented to interface with our AOFS filesystem. 
    The operation structure is passed into fuse_main
  - Implemented an interface with the following FUSE endpoints:
    + .getattr
    + .readdir
    + .create
    + .read
    + .open
    + .write
    + .release
    + .unlink
    + .truncate

### init_disk.c
  - Initializes an fresh AOFS structure and writes it to /FS_FILE on disk for
    persistent storage.

## AOFS Design: The Nitty and the Gritty

### File System Structure

AOFS is split into 2 sections: The SuperBLock and everything else.

  The SuperBlock contains the filesystem's metadata:
  * Magic number
  * Bitmap of block allocation
  * Total number of blocks that can be in the system. Our system can currently
    support storage of 4,096 blocks. This number is defined in aofs.h with the
    macro BLOCK_NUM.
  * Block size
  
The everything else is the file data: the Blocks. 

The blocks live in a Block
    array. Each block is 4,096 Bytes including space for the block's
    metadata which takes up 320 Bytes. This leaves 3,776 Bytes available for
    raw data storage in each block.


### File Creation

File creation calls `aofs_create_file(disk,filename)`, which loads the superblock
from `disk`, searches for a free space in the bitmap, flips that 1 to a 0,
clears and timestamps the new block and finally returns the block_number of the 
newly allocated file block. 

### Writing Files

Writing a file falls into two categories: writing to an existing file and writing
to a file which does not yet exists. AOFS handles both of these by creating a 
file on the fly if needed (it is important to note that FUSE tends to call 
create then write immediately after if a file does not yet exist).

Storage across disjoint potentially non-contiguous blocks complicates writing of
files that are larger than the size of a block. We deal with large files by 
chaining blocks together in a linked-list of blocks.

  - Each file block stores a index `next` of the next block in the chain.
  - We first find the head block of a file.
  - If no more data can be written to the current block we store the changes onto
    disk and load the next block using the block's `next` field. If `next == -1`
    then the block is the last in the chain and another must be allocated on the
    fly to accomodate the additional data.

### Reading Files

### Deleting Files