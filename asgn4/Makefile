CC=cc
FUSE=`pkgconf fuse --cflags --libs`
EXE= fs_run fs_init

make:
	@echo No target specified for make.

fs_init: init_disk.c aofs.o
	${CC} init_disk.c aofs.o -o $@  $(FUSE)

fs_run: run_fs.c aofs.o 
	${CC} run_fs.c aofs.o  -o $@  $(FUSE)

aofs.o:  aofs.c aofs.h
	${CC} -c aofs.c

clean:
	rm ${EXE} *.o
