###########################################################################
#							  		  #
#  Copyright (C) 2018  Matt Ovenden, Ryan Blelloch. All rights reserved.  #
#  You may note use, distribute, publish, or modify this code without	  #
#  the express permission of the copyright holder(s).			  #
#							  		  #
###########################################################################

CORES?=1

SYS=/usr/src/sys/vm

nothing: # in case no make target is specified

benchmark:
	@cc benchmark.c -o benchmark -lm

build: copyFiles
	@cd /usr/src/; make buildkernel -j$(CORES) #-DKERNFAST

copyFiles:
	@./copyFiles.sh
#	@cp $(runq) $(SYS)/$(runq)
#	@cp $(switch) $(SYS)/$(switch)
#	@cp $(ule) $(SYS)/$(ule)

install:
	@cd /usr/src/; make installkernel

clean:
	@rm benchmark *~ *\#
