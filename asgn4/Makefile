CC=cc
FUSE=`pkgconf fuse --cflags --libs`
EXE= fs_run fs_init

make:
	@echo No target specified for make.
all: fs_init fs_run

benchmark: benchmark.c
	${CC} benchmark.c -o $@

fs_init: init_disk.c aofs.o
	${CC} init_disk.c aofs.o -o $@  $(FUSE)

fs_run: run_fs.c aofs.o 
	${CC} run_fs.c aofs.o -o $@  $(FUSE)

aofs.o:  aofs.c aofs.h
	${CC} -c aofs.c

clean:
	rm ${EXE} *.o

reset: clean
	rm /FS_FILE

setup: all
	./fs_init

fresh: reset all
	./fs_init

umount:
	umount /dev/fuse
