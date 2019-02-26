###########################################################################
#							  		  #
#  Copyright (C) 2018  Matt Ovenden, Ryan Blelloch. All rights reserved.  #
#  You may note use, distribute, publish, or modify this code without	  #
#  the express permission of the copyright holder(s).			  #
#							  		  #
###########################################################################

CORES?=1

SYS=/usr/src/sys
CC=g++
CFLAGS=-std=c++11
EXE=benchmark

PAGEH=vm/vm_page.h
PAGEC=vm/vm_page.c
PAGEOUT=vm/vm_pageout.c

nothing: # in case no make target is specified
	@echo No target specified

${EXE}: stress.cpp
	pkg install gcc
	@$(CC) stress.cpp -o ${EXE} $(CFLAGS)

build: copyFiles
	@cd /usr/src/; make buildkernel -j$(CORES)

copyFiles:
	@cp $(PAGEC) $(SYS)/$(PAGEC)
	@cp $(PAGEOUT) $(SYS)/$(PAGEOUT)
	@cp $(PAGEH) $(SYS)/$(PAGEH)

install:
	@cd /usr/src/; make installkernel

clean:
	@rm benchmark *~ *\#
