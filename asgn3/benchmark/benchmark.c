/* File: benchmark.c
 * Author(s): Ryan Blelloch
 *
 * Description: boi
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MEM_PER 0.90
#define SEAL_PER 0.98
#define SEAL_NUM 16 

/* Function: swap_has_occured
 *
 * Description: runs dmesg to check system log for swap
 * messages. If messages found, returns 1, otherwise 0.
 * If file cannot be opened, returns 1 to kill loop.
*/
int swap_has_occurred() {

    int swap;

    //get all useful syslog lines into file
    char command[] = "dmesg | grep \"swap_pager_getswapspace\" > swap.dat";
    int error = system(command);

    //attempt to open file
    FILE* swap_file;

    swap_file = fopen("swap.dat", "r");
    if(!swap_file) {
        printf("Can't open swap file.\n");
        return 1;
    } else {
        fseek(swap_file, 0, SEEK_END);
        if(ftell(swap_file)) {
            swap = 1;
        } else { swap = 0; }
    }
    fclose(swap_file);
    return swap;
}

//link list
typedef struct lnklst {
    char* memblock;
    struct lnklst* next;
} lnk;

int main(int argc, char* argv[]) {

    //system values
    long    page_size, page_num, total_ram;

    long    ram_to_alloc; //size of initial memory block

    char*   init_arr; //character array for initial memory block
    long    seal_size; //size of memory per seal block

    long    iter; //iterator
    long    pg_jmp; //iterator increment size

    lnk*    head; //head to seal link list

    //get user arguments

    //Initialize system values for test
    page_size = sysconf(_SC_PAGESIZE); //bytes
    page_num =  sysconf(_SC_PHYS_PAGES);
    total_ram = page_size * page_num;

    //print system specs:
    printf("Total ram: %lu\n", total_ram);
    printf("Page size: %ld\n", page_size); 

    //calculate percent of memory to use
    ram_to_alloc = (long)(total_ram * MEM_PER);
    printf("Target memory allocation: %ld\n", ram_to_alloc);

    //begin by allocating base percent of memory
    init_arr = malloc(ram_to_alloc * sizeof(char));

    printf("Initial target memory allocated.\n");

    //reference all pages in init_arr
    printf("Referencing all pages in initial target memory.\n");
    pg_jmp = page_size / 4;
    for(iter = 0; iter < ram_to_alloc; iter += pg_jmp) {
        init_arr[iter] = 'd';
    }

    //check to make sure swapping hasn't already occured
    if(swap_has_occurred()) {
        printf("ERROR: Initial target memory allocation has triggered swap.\n");
        printf("       Consider lowering initial target memory percent.\n");
        return 1;
    }

    //initialize size of memory per lnk in link list
    seal_size = (long)(page_size * SEAL_PER) * SEAL_NUM;
    head = NULL;
    iter = 0;

    //continually allocate more memory until pages need to be swapped
    while(1) {

        //used to periodically print progress (number of seal blocks allocated)
        ++iter;
        if(iter % 80 == 0) {printf("Allocating seal pages iteration: %ld\n", iter);}
        
        //allocate the next seal block
        lnk* nxt = malloc(sizeof(lnk));
        nxt->memblock = malloc(seal_size * sizeof(char));
        nxt->next = head;
        head = nxt;

        //check to see if pages are beginning to be swapped
        if(swap_has_occurred())
            break;
    }

    //free initial memory
    printf("CLEANUP: freeing initial target memory...\n");
    free(init_arr);
    
    //free lnk memory
    printf("CLEANUP: freeing seal memory...\n");
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


