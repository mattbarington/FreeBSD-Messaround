#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TST_LGNTH 3
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
    int pid;

    int pri;
    int which;

    int proc_num;
    int st;

    proc_num = NUM_PROCS;

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

    //create 20 processes
    do {
        pid = fork();
        if(pid == 0) {
            printf("I am child: %d\n", getpid());
            proc_num--;
        } else {
            proc_num = 0;
            setpriority(PRIO_PROCESS, getpid(), proc_num);
            pri = getpriority(PRIO_PROCESS, getpid());
            printf("I am %d and my priority is %d\n", getpid(), pri);
            long_func();
            printf("waiting for pid: %d\n", pid);
            waitpid(pid, &st, 0);
        }
    } while(proc_num > 0);

    return 0;
}
