
Author(s): Matt Ovenden, Ryan Blelloch

# Assignment 3 Design Document

## Brief Description
Changed kernel source files to implement a FIFO ordered memory
queue for all paging. This means on page faults the oldest page will be
removed.

Implemented a single FIFO queue by funneling all incoming pages into the
inactive queue, rendering the active queue permanently vacant.

FIFO insertion and removal was applied to the inactive queue, such
that a page is only ever inserted at the tail and only ever removed from
the head of the queue.

It is important to note that while iterating through the inactive queue
all busy, held, and marker pages are left in place, and are discounted
from page counts and total FIFO ordering.

The printouts that will be seen in the console occur upon every call to
vm_pageout_scan(). The print message complies with the following format:

NEWPAGE num_pages= {FIFO queue size} head_page= {page# at queue HEAD} tail_page= {page# at queue TAIL}

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
      page ordering, but, in rare cases, can cause incontinuities.

    - Removed all functionality that scans the active page queue, since
      it will always be empty.

    - Removed functionality of transferring referenced pages into
      the active queue.

    - For statistics:
      Upon every call to vm_pageout_scan the following is printed:
      queue size, front-page's age, and tail-page's age.
      
      __NOTE__: We chose printing statistics here rather than every page
      fault for performance reasons. It is expensive to iterate 
      through the entire queue for the number of page faults, so we chose
      to print less often but still during relevent times.

  - Changes outside of vm_pageout_scan are meant to funnel pages into
    the inactive queue that would be placed in the active queue
    otherwise.
    
### vm/vm_page.c
  - All changes in this file are meant to serve one of two purposes:
    - Redirect pages from the active queue to the inactive queue.
    - update an incoming page's page count. 

  - variable `static unsigned long pg_cnt` is a global monotinically
    increasing counter that tracks the number of pages that have
    entered the FIFO page queue. This variable tracks page order within
    the queue, and is the key mechanism for displaying proper FIFO
    functionality.

### vm/vm_page.h
  - The only change to this file is an additional field in the vm_page
    struct. `vm_page->id` is a monotonically increasing variable, assigned
    to the page when it enters the FIFO page queue, to represent it's birthday 🎂.

## Benchmark:

 - **Build**:
  `$ make benchmark`
    
  - **Run** (as Root):
  `$ ./benchmark`

  - **Usage**:
   On start up, specify number of MBs to allocate. Enter 0 to use a 
    calculated amount based on the size of memory. If the benchmark is
    overfilling swap space and crashing the system, or is too small
    to cause page faults, this can be used to augment or reduce memory allocation.

 - **Description** - The system log is timestamped for the new test. The benchmark will allocate a vector 
 that spans that many MBs, and then access in 3 different ways:
              
     + Single Page Access - accesses random elements of the vector that span less than one page, 
              reducing paging out.
              
      + Cache Ideal Access - accesses the 0th element of the vector exclusively. This reduces paging
              out, and encourage ideal cache utilization.
              
      + Page Thrashing - accesses vector elements at random. This eliminates all locality, and forces 
              page faults to occur.
	      
	The results in the system log are read and analyzed, starting at the system log line with the timestamp.	      
              
    
  - **Results**: Following the completion of the page thrashing stage, pageout queue messages printed to the system
    log will be analyzed to ensure FIFO ordering. Any FIFO discrepancies observed will be printed to the terminal.
    Ideally, discrepancies should be 0.

   - **Discrepancies**: This will describe the number of violations of FIFO order. A violation of FIFO ordering is
   observed when either the tail of the queue is younger than the head (tail's id (birthdate) is higher than head's),
   or when a page at the head of the queue is younger than a previous head (newer head's id (birthdate) is higher than
   the older head's). 
