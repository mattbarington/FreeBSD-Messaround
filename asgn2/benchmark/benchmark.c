#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TST_LGNTH 1
#define NUM_PROCS 20

struct result {
    int proc;
    int pri;
    unsigned long time;
};

void long_func() {
    int tmp, j, k;
    for(j = 0; j < TST_LGNTH; ++j) {
        for(k = 0; k < INT_MAX; ++k) {
            if(j % 2 == 0) {
                tmp++;
            } else {
                tmp--;
            }
        }
    }
}

int main() {

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
    unsigned long time_delta;
    
    proc_num = NUM_PROCS;

    //create a test file
    /*test_results = open(filename, O_RDWR | O_CREAT);
    if(!test_results) {
        printf("Error: Could not open/create %s\n", filename);
        return 1;
    } else { close(test_results); }
*/

    pid_start = fork();
    if(pid_start == 0) {
        sleep(1);
        printf("ready...\n");
        sleep(1);
        printf("set...\n");
        sleep(1);
        printf("GO!\n");
        exit(0);
    } else {
        //create 20 processes
        do {
            pid = fork();
            if(pid == 0) {
                printf("I am child: %d\n", getpid());
                proc_num--;
            } else {
                my_pid = getpid();
                setpriority(PRIO_PROCESS, getpid(), proc_num);
                pri = getpriority(PRIO_PROCESS, getpid());
                //printf("I am %d and my priority is %d\n", getpid(), pri);
                //waitpid(pid_start, &st, 0);
                gettimeofday(&start, NULL);
                long_func();
                gettimeofday(&stop, NULL);
                time_delta =  stop.tv_usec - start.tv_usec;
                sprintf(result_msg, "%d %d %lu\n", my_pid, pri, time_delta);
                sprintf(filename2, "%s%d", filename, proc_num);
                test_results = fopen(filename2, "w");
                if(!test_results) {
                    printf("Error %d: Could not open/create %s\n", my_pid, filename);
                } else {
                    printf("%s", result_msg);
                    fprintf(test_results, "%s", result_msg);
                    fclose(test_results); 
                }

                proc_num = 0;
                //printf("waiting for pid: %d\n", pid);
                waitpid(pid, &st, 0);
            }
        } while(proc_num > 0);
    }

    //process the results
    int i;
    struct result all_res[NUM_PROCS];

    for(i = 1; i <= 20; ++i) {
        sprintf(filename2, "%s%d", filename, i);
        test_results = fopen(filename2, "r");
        if(test_results) {
            fscanf(test_results, "%d %d %lu", &(all_res[i-1].proc), &(all_res[i-1].pri), &(all_res[i-1].time));
            fclose(test_results);
            remove(filename2);
            
        } else {
            printf("Can't open result file %d\n", i);
        }
    }

    for(i = 0; i < 20; ++i) {
        printf("p: %d p: %d t: %lu\n", all_res[i].proc, all_res[i].pri, all_res[i].time);
    }

    return 0;
}
