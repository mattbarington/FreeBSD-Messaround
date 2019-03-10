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

* benchmark.c           - Source file for benchmark code

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

### benchmark.c
  - Code to run system calls that test simple filesystem operations.
  - Note: The benchmark expects the AOFS filesystem to already be built.
  - All folders and files are created and removed during the benchmark, nothing is left behind.
  - Three tests are run:
    + Create 100 Files
    + Write file
    + Read file
  - Time taken to run tests is compared in microseconds

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

Storage across disjoint potentially non-contiguous blocks complicates writing/reading
of files that are larger than the size of a block. We deal with large files by 
chaining blocks together in a linked-list of blocks.

  - Each file block stores a index `next` of the next block in the chain.
  - We first find the head block of a file.
  - If no more data can be written to the current block we store the changes onto
    disk and load the next block using the block's `next` field. If `next == -1`
    then the block is the last in the chain and another must be allocated on the
    fly to accomodate the additional data.

### Handling Write/Read Offset
  - If the offset is larger than the file size then the offset is truncated to 
    the file size (i.e., `offset = min(offset, filesize)`).
  - If the offset is smaller than a block size (3,776) then we can start to copy
    the buffer to the first block at the offset'th byte.
  - If the offset is beyond the first data block size we iterate through the 
    blocks until we find the block containing the offset'th byte and begin 
    copying the buffer.
  - The same protocol is followed to handle offset on read operations.

### Reading Files
  - The protocol to handle reads is similar to the protocol to handle writes, 
    but differs in implementation. One significant difference is that read will
    not allocate new blocks if the `size` parameter is larger than the file size.
    Instead it will copy as many bytes as possible into the buffer and return 
    the number of bytes read.
    

### Deleting Blocks
  - Block deletion and deallocation is done by deallocating a block and 
    "recursively" deallocating the next block in the chain (the implementation
    is iterative).
  - A block index is passed into the function `deletechain(disk,int)`, the block
    index is marked as available in the bitmap, and the same is done to the 
    `block.next`, and so on, until `block.next == -1`.

### Deleting Files
  - Deletion of files is simple given the above block deletion mechanism. All we
    have to do is find the head block for a given filename, and pass its block
    number to `deletechain(disk,block_num)`.

### File Truncation
  - Truncation combines the strategies the logic of writing with an offset and 
    block chain deletion. Truncation changes the size of a file, which can lead
    to one of two cases that need be handled.
    
     - **Case 1**: Truncation size is larger than the file size. In this case we
                pad the rest of the file with 0. If this new filesize requires
                additional blocks, then additional blocks will be allocated and
                chained behind the existing ones. These new blocks are filled
                with padding 0s.
     
     - **Case 2**: Truncation size is smaller than the file size. In this case
                the block with the offset'th byte is located. The remainder of 
                the block is padded with 0s, and any trailing blocks are 
                deallocated.
