# FreeBSD-Messaround

## A Repository for messing around with the different functionalities of the FreeBSD kernel.

Version 2.7 of FreeBSD was used for all kernel operations, and due to subtle differences between v2.7 and the newer v3.0, these adjustments may not compile within other versions of FreeBSD.

### What do things mean?

The repository is split into isolated pieces, each totally orthogonal to the other.

* [Part 1](asgn1): A simple shell program that uses FreeBSD system calls to implement command line functions with multiple arguments, input and output redirection and piping between processes.

* [Part 2](asgn2): An altering of the FreeBSD low level process scheduling algorithm. The standard FreeBSD scheduling algorithm relies on multiple FIFO process queues, each having priority lower or higher than another queue. We instead implement a lottery scheduling algorithm for all timeshare (non-root) processes.

* [Part 3](asgn3): We override the native FreeBSD pageout algorithm with an absolute FIFO queue. The native algorithm combines the Not Recently Used algorithm with second chance in an attempt to minimize page-outs. While this algorithm is very efficient, it is very costly in the case of a large number of random page accessses that cover more pages than can fit in memory. The false expectation of spacial locality greatly increases page thrashing when compared to an ignorant and simplistic FIFO page queue.


      Disclaimer: The FIFO page algorithms is a terrible way to manage virtual memory in any 
      realistic system. It is ONLY advantageous when there is absolutely no locality expressed
      by the pattern of page accesses.
      
* [Part 4](asgn4): This final part is not a modification of the FreBSD kernel. Instead it is a ground-up implementation of an All-in-One-File-System. This file system has only 1 top-level directory, with all files inside of it.

Within each folder ([asgn1](asgn1), [asgn2](asgn2), [asgn3](asgn3), [asgn4](asgn4)) there are more extensive README files with installation instructions, and a design_document that describes the architecture of each part.
