
Author(s): Matt Ovenden, Ryan Blelloch

# Assignment 3 Design Document

## Brief Description
We have changed kernel source files to implement a FIFO ordered memory
queue for all paging. This means on page faults the oldest page will be
removed.

Implemented a single FIFO queue by funneling all incoming pages into the
inactive queue, rendering the active queue permanently vacant.

FIFO insertion and removal was then applied to the inactive queue,such
that a page is only ever inserted at the tail and only ever removed from
the head of the queue.

It is important to note that while iterating through the inactive queue
all busy, held, and marker pages are left in place, and are discounted
from page counts and total FIFO ordering.

The printouts that will be seen in the console occur upon every call to
vm_pageout_scan(). The print message complies with the following format:

NEWPAGE < FIFO queue size> <page# at queue HEAD> <page# at queue TAIL>

IMPORTANT NOTE:

	For reasons we are unable to diagnose, the machine has
	infrequently crashed while running the benchmark. There has not
	yet been an occurrence of two crash failures in a row.
	

## All files in REPO:

* README                - Directions for all compilation/running

* design_doc.txt        - Full explanation of asgn3 implementation

* Makefile              - Makefile with options to build and install 
                          kernel, and build benchmark
			  
* vm/vm_pageout.c	- Modified vm_pageout.c kernel source file 

* vm/vm_page.c		- Modified vm_page.c kernel source file

* vm/vm_page.h		- Modified vm_page.h kernel source file

* stress.cpp    	- Benchmark C++ source file


## Source Files changed/explanation

### vm/vm_pageout.c
  - Majority of the changes take place in function vm_pageout_scan().

    - vm_pageout_scan() iterates through the inactive queue, placing
      pages into the laundry queue one-by-one until the page deficit is
      supplemented.

    - Marker pages, held pages, and busy pages are skipped over. This
      design decision was suggested by Professor Nawab. Through the
      majority of our tests, this has caused no irregularities to FIFO
      page ordering, but, in rare cases, cause incontinuities.

    - Removed all functionality that scans the active page queue, since
      it will always be empty.

    - Removed functionality of transferring referenced pages into
      the active queue.

    - For statistics:
      Upon every call to vm_pageout_scan the following is printed:
      queue size, front-page's id, and tail-page's id.
      
  - Changes outside of vm_pageout_scan are meant to funnel pages into
    the inactive queue that would be placed in the active queue
    otherwise.
    
### vm/vm_page.c
  - All changes in this file are meant to serve one of two purposes:
    - Redirect pages from the active queue to the inactive queue.
    - update an incoming page's page count. 

  - variable static unsigned long pg_cnt is a global monotinically
    increasing counter that tracks the number of pages that have
    entered the FIFO page queue. This variable tracks page order within
    the queue, and is the key mechanism for displaying proper FIFO
    functionality.

### vm/vm_page.h
  - The only change to this file is an additional field in the vm_page
    struct. vm_page->id is a monotonically increasing variable, assigned
    to the page when it enters the FIFO page queue. 

## Benchmark:

  Build:
    `$ make benchmark`
    
  Run (as Root):
    `$ ./benchmark`

  Description:
    Allocates a large amount of memory to force virtual memory.
    Then accesses the memory three different ways. The third way will
    thrash and cause many page faults. 
    
  Results:
    Following the completion of
    the thrashing run, pageout queue messages printed to the system
    log will be analyzed to ensure FIFO queue. Any discrepencies will
    be reported and assumed to be special pages. For the most part, 
    the end page minus the starting page will be the number of pages
    in the queue.
