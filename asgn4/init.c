#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "oper_fs.h"

	
int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, NULL, NULL);
}



