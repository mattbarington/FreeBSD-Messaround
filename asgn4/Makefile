CC=cc
FUSE=`pkgconf fuse --cflags --libs`
EXE= fs_run fs_init

make:
	@echo No target specified for make.

fs_init: init_disk.c oper_fs.o aofs.o
	${CC} init_disk.c aofs.o oper_fs.o -o $@  $(FUSE)

fs_run: run_fs.c oper_fs.o aofs.o 
	${CC} run_fs.c aofs.o oper_fs.o -o $@  $(FUSE)

aofs.o:  aofs.c aofs.h
	${CC} -c aofs.c

oper_fs.o: oper_fs.c oper_fs.h aofs.o
	${CC} -c oper_fs.c

clean:
	rm ${EXE} *.o
