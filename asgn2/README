# Important note:
    Our our scheduler puts ONLY NON-ROOT timeshare threads into the
    lottery queue. All other threads will be processed normally by the
    FreeBSD scheduler. Due to this, the benchmark MUST NOT be run by
    root user.

How to lottery schedule: the fast version:
    `su -` then enter password.
    `git clone https://gitlab.soe.ucsc.edu/gitlab/cmps111/winter19-01/movenden.git`.
    You've probably already done that
    `make build` or `make build CORES=<CPUs you want to use>`
    `make install`
    `make benchmark`
    `mv benchmark <somewhere outside of repo`. On our computers the file 
                permissions were all wonky, and `benchmark` couldn't run
                properly within the repo.
    `reboot`
    `cd <that place where you put benchmark`
    `./benchmark <some small int, or nothing` This determines the test length.
                If an integer is omitted it will default to 2.
                
Makefile Description:
    The makefile provides some intuitive services for the user. 
    The syntax and result of each are listed below.
    
 - `make`           - does nothing
 
 - `make benchmark` - compiles benchmark.c to executable benchmark
 
 - `make build`     - copies all kernel files from kern/ and sys/ 
                      to the corresponding files in the kernel.
                      make build also support multicore compilation.
                      
 - `make build CORES=X` - compile the kernel using X cores. 
                          If omitted CORES defaults to 1.
 - `make copyFiles` - copies all kernel files from kern/ and sys/ 
 
                      to the corresponding files in the kernel.
                      Does not build kernel.
                      
 - `make install`   - installs the compiled kernel.
 
 
 How to install:
 - Root    - Make sure to `su -` into the root user, otherwise you won't 
            have adequate file permissions to do anything useful.
            
 - Compile - If you have allocated more than 1 CPU to your VM you can
            compile kernel faster with `make build CORES=X` where X is
            the number of CPUs available.
            - If you want to or must use 1 CPU you can compile with
            `make build` or `make build CORES=1`.
            - Don't worry about copying files over with `make copyFiles`.
            It's there for you convenience, but files are copied into 
            the kernel with `make build` (it actually calls `copyFiles`).

 - Install - To install the now compiled kernel go ahead and type 
            `make install` to install 
            
 - Reboot  - Go ahead and type `reboot` and your VM should reboot into
            the new kernel with lottery scheduling.
            
How to run benchmark
 - Install  - If you haven't already installed the new kernel and
            rebooted, go back to the previous section before running
            benchmark.
            
 - Compile  - `make benchmark` will compile the benchmark.c into an 
            the executable `benchmark`.
            
 - To root,
  or not to root?
            - This is important!! The scheduler only puts root 
            timeshare processes into the lottery queue. If you're 
            hoping for absolutely no apparent change from the regular
            FreeBSD scheduling, and no benchmark statistics go ahead
            and stay in root. 
            If you want the lottery scheduler to do anything at all
            go ahead and EXIT from ROOT.
            
 - Run Benchmark - benchmark runs a cpu intensive loop INT_MAX/15 times, and 
                   takes a single integer argument indicating how many times
                   that loop should be run. Obviously, a higher input will
                   result in processes taking longer to execute.
                   The default is 2 if the argument is omitted.

 - Wait     - 5 processes are created, with priorities 0, 5, 10, 15, and 20.
              Each will run as many times as indicated from the command line 
              argument (or twice is an argument was omitted). Each prints
              how many iterations of the cpu intensive loop it has completed.
 - Observeve 
    results - Below many `Complete x of y` will be the expected and actual
                probability that any process was selected to run. Hopefully
                the expected and actual values are comparable.
            

