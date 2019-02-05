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

#define TST_LGNTH 5
#define NUM_PROCS 40

struct result {
    int proc;
    int pri;
    unsigned long time;
};

int compare(const void *s1, const void *s2) {
    
    struct result *r1 = (struct result *)s1;
    struct result *r2 = (struct result *)s2;
    if(r1->time < r2->time) {
        return 1;
    } else if (r2->time < r1->time) {
        return -1;
    } else {
        return 0;
    }
}

void print_results(struct result all[], int size) {

    int iter;

    printf("-----------------------------------------\n");
    printf("---Process--------Priority----Time(s)----\n");
    for(iter = 0; iter < size; ++iter) {
        printf("%10d", all[iter].proc);
        printf("%10d", all[iter].pri);
        printf("%14lu\n", all[iter].time);
    }
    printf("-----------------------------------------\n");
}

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
         //create 20 processes
        do {
            pid = fork();
            if(pid == 0) {
                printf("I am child: %d\n", getpid());
                proc_num--;
            } else {
                my_pid = getpid();
                setpriority(PRIO_PROCESS, getpid(), proc_num-20);
                pri = getpriority(PRIO_PROCESS, getpid());
                //printf("I am %d and my priority is %d\n", getpid(), pri);
                //waitpid(pid_start, &st, 0);
                gettimeofday(&start, NULL);
                long_func();
                gettimeofday(&stop, NULL);
                time_delta =  stop.tv_sec - start.tv_sec;
                sprintf(result_msg, "%d %d %lu\n", my_pid, pri, time_delta);
                sprintf(filename2, "%s%d", filename, proc_num);
                test_results = fopen(filename2, "w");
                if(!test_results) {
                    printf("Error %d: Could not open/create %s\n", my_pid, filename);
                } else {
                    //printf("%s", result_msg);
                    fprintf(test_results, "%s", result_msg);
                    fclose(test_results); 
                }
                waitpid(pid, &st, 0);
                exit(0);
                //printf("waiting for pid: %d\n", pid);
            }
        } while(proc_num > 0);
        exit(0);
       
    } else {
        int wpid;
        while((wpid = wait(&st)) > 0);
    }

    //process the results
    int i;
    struct result all_res[NUM_PROCS];

    for(i = 1; i <= NUM_PROCS; ++i) {
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

    /*
    for(i = 0; i < NUM_PROCS; ++i) {
        printf("p: %d p: %d t: %lu\n", all_res[i].proc, all_res[i].pri, all_res[i].time);
    }*/

    qsort((void*)all_res, NUM_PROCS, sizeof(all_res[0]), compare);
    /*for(i = 0; i < NUM_PROCS; ++i) {
        printf("p: %d p: %d t: %lu\n", all_res[i].proc, all_res[i].pri, all_res[i].time);
    }*/

    print_results(all_res, NUM_PROCS);

    return 0;
}
