
## How to FIFO schedule memory pages. The fast version:
  * `su -` then enter password.
    
  * `git clone https://gitlab.soe.ucsc.edu/gitlab/cmps111/winter19-01/movenden.git`.
    	 You've probably already done that.
  
  * `cd movenden/asgn3`.
    
  * `make build` or `make build CORES=<CPUs you want to use>`
    
  * `make install`
  
  * `pkg install gcc`- if you don't have C++ installed.
    
  * `make benchmark`
      
  * `reboot`
  
  * `su -` and enter password
    
  * `cd back to <repo>/asgn3`.
      
  * `./benchmark`.
  
  * The benchmark takes 1 input: the number of Bytes to be allocated. Please enter `0` for a calculated amount, or any other number if the calculated amount overflows swap space or is too small to cause page faults. 
 
 ## Makefile Description
  The makefile provides some intuitive services for the user. The syntax and result of 
  each are listed below.
    
 - `make`           - does nothing.
 
 - `make benchmark` - compiles stress.cpp to executable benchmark. Also attempts to install gcc if not installed already.
 
 - `make build`     - copies all kernel files from vm/ to the corresponding 
                      files in the kernel.
                      make build also support multicore compilation.
                      
 - `make build CORES=X` - compile the kernel using X cores. 
                          If omitted CORES defaults to 1.
 - `make copyFiles` - copies all kernel files from vm/ to the corresponding files in the kernel. 
                      Does not build kernel.
                      
 - `make install`   - installs the compiled kernel.
 
  ## How to install
 - **Root**    - Make sure to `su -` into the root user, otherwise you won't 
            have adequate file permissions to do anything useful.
            
 - **Compile** - If you have allocated more than 1 CPU to your VM you can
            compile kernel faster with `make build CORES=X` where X is
            the number of CPUs available.
            - If you want to or must use 1 CPU you can compile with
            `make build` or `make build CORES=1`.
            - Don't worry about copying files over with `make copyFiles`.
            It's there for you convenience, but files are copied into 
            the kernel with `make build` (by calling `copyFiles`).

 - **Install** - To install the now compiled kernel go ahead and type 
            `make install`. 
            
 - **Reboot**  - Go ahead and type `reboot` and your VM should reboot into
            the new kernel with FIFO pageout scheduling.
            
            
  ## How to run benchmark
 - **Install Kernel**  - If you haven't already installed the new kernel and rebooted, go back 
                   to the previous section before running benchmark.
 
 - **Install C++** - The benchmark file is a C++ file, so it's important that it be able to compile it on 
               your machine.
               
   This can be accomplished with `pkg install gcc`, and following the instructions. You likely have to
   first enter root user via `su -` to perform an install.
            
 - **Compile**  - `make benchmark` will compile the stress.cpp into an the executable `benchmark`. This also attempts install gcc if it is absent from the system. 
            
 - **Run Benchmark** - The benchmark will prompt you to enter some number of megabytes (MB). You can enter `0` and the benchmark will attempt to calculate the goldilox amount to use but not overfill swap space. Please use `0` unless swap space overflows or remains untouched (the calculation is static and therefore can't account for other prexisting memory allocations on your VM). The benchmark 
              will allocate a vector that spans that many MBs, and then access in 3 different ways:
              
     + Single Page Access - accesses random elements of the vector that span less than one page, 
              reducing paging out.
              
      + Cache Ideal Access - accesses the 0th element of the vector exclusively. This reduces paging
              out, and encourage ideal cache utilization.
              
      + Page Thrashing - accesses vector elements at random. This eliminates all locality, and forces 
              page faults to occur.
              
     We find that entering the size of your allocated base memory works well. Since some of the memory
     is wired with kernel pages, some of the allocated vector will have to live in swap space. We included variable entry incase the default calculation is inoptimal for the grader's VM.
