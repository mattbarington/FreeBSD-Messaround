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
    int pid, pid_start;

    int pri;
    int which;

    int proc_num;
    int st;

    struct timeval stop, start;

    /*
       FILE* syslog;
       int file_pos;

       char rec1[100];
       char rec2[100];

       char msg1[] = "This is the first message\n";
       char msg2[] = "This is the second message\n";

       printf("Attempting to open syslog...\n");
       syslog = fopen("/var/log/console.log", "r");
       if(syslog) {
       printf("Success\n");
       fseek(syslog, 0, SEEK_END);
       file_pos = ftell(syslog);
       printf("file_pos: %d\n", file_pos);
       fclose(syslog);
       } else {
       printf("Failed to open system log\n");
       return 1;
       }

       syslog = fopen("/var/log/console.log", "a");
       fputs(msg1, syslog);
       fputs(msg2, syslog);
       fclose(syslog);

       syslog = fopen("/var/log/console.log", "r");
       fseek(syslog, file_pos, SEEK_SET);
       fgets(rec1, sizeof(rec1), (FILE*)syslog);
       fgets(rec2, sizeof(rec2), (FILE*)syslog);
       fclose(syslog);

       printf("msg1: %s\n", rec1);
       printf("msg2: %s\n", rec2);

       return 0;
     */


    proc_num = NUM_PROCS;

    /*

    parent_pid = getpid();
    printf("My pid is %d\n", parent_pid);

    printf("Attempting to get my priority...\n");
    pri = getpriority(PRIO_PROCESS, parent_pid);

    printf("My priority is %d\n", pri);

    printf("Attempting to set my priority...\n");
    if(setpriority(PRIO_PROCESS, parent_pid, 5)) {
        printf("Priority Set was unsuccessful\n");
    } else {
        printf("Success. Attempting to get my priority...\n");
        pri = getpriority(PRIO_PROCESS, parent_pid);

        printf("My priority is %d\n", pri);
    }
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
                setpriority(PRIO_PROCESS, getpid(), proc_num);
                pri = getpriority(PRIO_PROCESS, getpid());
                proc_num = 0;
                //printf("I am %d and my priority is %d\n", getpid(), pri);
                waitpid(pid_start, &st, 0);
                gettimeofday(&start, NULL);
                long_func();
                gettimeofday(&stop, NULL);
                printf("Process: %d Priority: %d Total time: %lu\n", getpid(), pri, stop.tv_usec - start.tv_usec);
                //printf("waiting for pid: %d\n", pid);
                waitpid(pid, &st, 0);
            }
        } while(proc_num > 0);
    }

    return 0;
}
