/* File: benchmark.c
 * Author(s): Ryan Blelloch, Matt Ovenden
 *
 * Description: This single source file compiles to a test program
 * for process priority. Takes 0 or 1 arguments. The argument must
 * be a valid positive integer to define the test length. Forty
 * processes will be run simultaneously, each with a unique priority
 * value. Results of the extremes are printed at the end.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCS 5

//Result structure for a single process
struct result {
    int proc;
    int pri;
    unsigned long time_s;
    unsigned long time_ms;
    double per_exp;
    double per_act;
};

//Comparison function for struct result in qsort
int compare(const void *s1, const void *s2) {

    struct result *r1 = (struct result *)s1;
    struct result *r2 = (struct result *)s2;
    if(r1->time_s < r2->time_s) {
        return 1;
    } else if (r2->time_s < r1->time_s) {
        return -1;
    } else {
        if(r1->time_ms < r2->time_ms) {
            return 1;
        } else if (r2->time_ms < r1->time_ms) {
            return -1;
        } else {
            return 0;
        }
    }
}

//prints result array (only [-20:-10] and [10:20]
void print_results(struct result all[], int size) {

    int iter;

    printf("--------------------------------\n");
    printf("---pro---pri---tim---\n");
    for(iter = 0; iter < size; ++iter) {
      if(iter >=0) {
            printf("%6d%6d %5lu.%lu\n", all[iter].proc, all[iter].pri, all[iter].time_s, all[iter].time_ms);
        }
    }
    printf("--------------------------------\n");
}

//long function to test process run at some priority
void long_func(const int len) {
    int tmp, j, k;
    for(j = 0; j < len; ++j) {
        for(k = 0; k < INT_MAX/15; ++k) {
            if(j % 2 == 0) {
                tmp++;
            } else {
                tmp--;
            }
        }
        printf("%d: Completed %d of %d\n", getpid(), j+1, len);
        fflush(stdout);
    }
}

int main(int argc, char* argv[]) {

    int parent_pid;
    int child_pid;
    int pid, my_pid, pid_start;

    int pri;
    int which;

    int proc_num;
    int st;

    struct timeval stop, start;

    FILE* test_results;
    char filename[] = "test_results";
    char filename2[100];

    char result_msg[400];
    unsigned long time_delta_ms, time_delta_s;

    //default test length is 2
    int tst_length = 2;

    //handle args
    if(argc > 2) {
        printf("Too many arguments. Single positive interger argument to define test length.\n");
        return 1;
    }
    if(argc == 2) {
        tst_length = atoi(argv[1]);
        if(tst_length == 0) {
            printf("%s is not a valid positive integer argument.\n", argv[1]);
            return 1;
        }
    }

    //get number of procs
    proc_num = 20;

    //fork and wait for child tests to build, run, and finish
    printf("Creating processes: ");
    fflush(stdout);
    
    pid_start = fork();
    if(pid_start == 0) {
        //create 20 processes
        do {
            pid = fork();
            if(pid == 0) {
                printf("%d, ", getpid());
                fflush(stdout);
                proc_num -= 5;
            } else {
                //get process ID for printing
                my_pid = getpid();
                //set priority of process and get to ensure it worked
                setpriority(PRIO_PROCESS, getpid(), proc_num);
                pri = getpriority(PRIO_PROCESS, getpid());
                //start timer
                gettimeofday(&start, NULL);
                //run variable time test based on test_length
                long_func(tst_length);
                //stop timer
                gettimeofday(&stop, NULL);
                //get time of test in seconds
                time_delta_s = stop.tv_sec - start.tv_sec;
                if(stop.tv_usec < start.tv_usec) {
                    stop.tv_usec += 1e6;
                    time_delta_s--;
                }
                time_delta_ms =  round((stop.tv_usec - start.tv_usec) / 1.0e3);
                //prepare result message
                sprintf(result_msg, "%d %d %lu %lu\n", my_pid, pri, time_delta_s, time_delta_ms);
                //prepare temp result file
                sprintf(filename2, "%s%d", filename, proc_num/5);
                //open temp result file to print results
                test_results = fopen(filename2, "w");
                if(!test_results) {
                    printf("Error %d: Could not open/create %s\n", my_pid, filename);
                } else {
                    //print results and close temp file
                    fprintf(test_results, "%s", result_msg);
                    fclose(test_results); 
                }
                waitpid(pid, &st, 0);
                exit(0);
                //printf("waiting for pid: %d\n", pid);
            }
        } while(proc_num >= 0);
        printf("\n");
        exit(0);

    } else {
        int wpid;
        //wait for all child processes to finish before moving on
        while((wpid = wait(&st)) > 0);
    }

    //process the results
    int i;
    struct result all_res[NUM_PROCS];

    //read all result files, store results in array, and delete result files
    for(i = 0; i < NUM_PROCS; ++i) {
        sprintf(filename2, "%s%d", filename, i);
        test_results = fopen(filename2, "r");
        if(test_results) {
            fscanf(test_results, "%d %d %lu %lu", &(all_res[i].proc), &(all_res[i].pri), &(all_res[i].time_s), &(all_res[i].time_ms));
            fclose(test_results);
            remove(filename2);
        } else {
            printf("Can't open result file %d\n", i);
        }
    }

    //sort result array by time
    qsort((void*)all_res, NUM_PROCS, sizeof(all_res[0]), compare);

    //print results to screen
    print_results(all_res, NUM_PROCS);

    return 0;
}
