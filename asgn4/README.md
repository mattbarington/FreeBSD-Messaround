# In soviet russia, repository READYOU

## How to setup FUSE with AOFS. The fast version:

- `$su -` then enter the password.

- `$ pkg install fusefs-libs`

- `git clone https://gitlab.soe.ucsc.edu/gitlab/cmps111/winter19-01/movenden.git`.

- `$ cd movenden/asgn4`.

- `$ kldload fuse`.

- `$ make setup`.

- `$ mkdir <directory name>`.

- `$ ./fs_run <directory name> [-f]`. `-f` is optional.

- `$ make benchmark`.

- `$ ./benchmark`.

- Have fun and remember to wear a helmet when going outside.

## Makefile Description

- `$ make ` - does nothing

- `$ make fs_init` - compiles `init_disk.c` into executable `fs_init`.

- `$ make fs_run` - compiles `run_fs.c` into executable `fs_run`.

- `$ make all` - runs `make fs_init` and `make fs_run` in one step.

- `$ make setup` - runs `make all` and runs `fs_init`, compiling and initializing
            FS_FILE in one tidy step.
	    
- `$ make clean` - Removes executables `fs_run` and `fs_init` and any `*.o` files.

- `$ make reset` - Calls `make clean` and removes `/FS_FILE`, cleaing up all
                trace that AOFS had ever been run.
		
- `$ make fresh` - Calls `make reset` and `make setup`, cleansing the environment
                from any previous instantiations of AOFS, recompiling the 
                the execuatables, and initializing a new `/FS_FILE` disk.

- `$ make benchmark` - Creates the benchmark executable. Can be made without previous 
                     makes. Must only be run after AOFS has been made (but not 
                     currently running). 
		
- `$ make umount` - Unmounts a mounted FUSE file system.

