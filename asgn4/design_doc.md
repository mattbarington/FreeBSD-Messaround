Author(s): Matt Ovenden, Ryan Blelloch

# Assignment 4 Design Document

## Brief Description:
  - This is a simple AOFS (All-In-One Folder File System) implemented using 
    FUSE. It has basic functionality of reading and writing files are arbitrary 
    size.

## All files in REPO:

* README                - Directions for all compilation/running

* aofs.c                - All implementation for AOFS low-level functions

* aofs.h                - Header file for AOFS low-level functions

* run_fs.c              - All implementation for FUSE operation functions, 
                           including fuse_main entry

* init_disk.c           - Standalone implementation for creating new FS_FILE

## Source Files explanation

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
