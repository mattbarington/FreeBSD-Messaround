CC=cc
FUSE=`pkgconf fuse --cflags --libs`
EXE= fs_run fs_init

make:
	@echo No target specified for make.
all: init run

init: init_disk.c aofs.o
	${CC} init_disk.c aofs.o -o fs_$@  $(FUSE)

run: run_fs.c aofs.o 
	${CC} run_fs.c aofs.o -o fs_$@  $(FUSE)

aofs.o:  aofs.c aofs.h
	${CC} -c aofs.c

clean:
	rm ${EXE} *.o
