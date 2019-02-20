#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int swap_has_occurred() {
    return 0;
}

typedef struct lnklst {
    char* memblock;
    struct lnklst* next;
} lnk;

int main(int argc, char* argv[]) {

    int no_swap;

    long page_size, page_num, total_ram;
    long ram_to_alloc;
    double mem_percent = 0.85;

    char* init_arr;
    char* seal_arr;
    long seal_size;

    long iter, pg_jmp;

    lnk* head;

    //get user arguments

    //Initialize system values for test
    page_size = sysconf(_SC_PAGESIZE); //bytes
    page_num =  sysconf(_SC_PHYS_PAGES);
    total_ram = page_size * page_num;

    //print system specs:
    printf("Total ram: %lu\n", total_ram);
    printf("Page size: %ld\n", page_size); 

    //calculate percent of memory to use
    ram_to_alloc = (long)(total_ram * mem_percent);
    printf("Target memory allocation: %ld\n", ram_to_alloc);

    //begin by allocating base percent of memory
    init_arr = malloc(ram_to_alloc * sizeof(char));

    printf("Initial target memory allocated.\n");

    printf("Referencing all pages in initial target memory.\n");
    pg_jmp = page_size / 4;
    for(iter = 0; iter < ram_to_alloc; iter += pg_jmp) {
        init_arr[iter] = 'd';
    }

    printf("Sleeping 4 seconds... see memory in htop.\n");
    sleep(4);

    //check to make sure swapping hasn't already occured
    if(swap_has_occurred()) {
        printf("Initial target memory allocation has triggered swap.\n");
        printf("Consider lowering initial target memory percent.\n");
        return 1;
    }

    no_swap = 1;
    head = NULL;

    //continually allocate more memory until pages need to be swapped
    while(no_swap) {
        
        lnk* nxt = malloc(sizeof(lnk));
        nxt->memblock = malloc(seal_size * sizeof(char));
        nxt->next = head;
        head = nxt;

        if(swap_has_occurred())
            no_swap = 0;
    
        break;
    }

    //free initial memory
    free(init_arr);
    
    //free lnk memory
    lnk* nxt = head;
    while(nxt != NULL) {
        nxt = head->next;
        free(head->memblock);
        head->next = NULL;
        free(head);
        head = nxt;
    }

    return 0;
}


