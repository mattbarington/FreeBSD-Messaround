
FUSE=`pkgconf fuse --cflags --libs`

fs_run: init.c oper_fs.o aofs.o 
	cc init.c aofs.o oper_fs.o -o $@  $(FUSE) 

aofs.o:  aofs.c aofs.h
	cc -c aofs.c

oper_fs.o: oper_fs.c oper_fs.h aofs.o 
	cc -c oper_fs.c 
