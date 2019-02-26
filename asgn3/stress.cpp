/* File: stress.cpp
 * File Type: C++ source file
 * Author(s): Matt Ovenden, Ryan Blelloch
 *
 * Description: Benchmark for assignment 3. Compiled with GNU C++.
 * Builds a vector larger than memory, then accesses it contiguously,
 * cache ideally, and finally by regularly thrashing. The page fault
 * messages printed by the system are read and recorded, and the 
 * number of consecutive vs. nonconsecutive are displayed. A FIFO 
 * queue should have almost all be consecutive (outliers are due to
 * special pages (held, etc.)
*/

#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//accesses random pages, but only the first ones
//no page faults should occur
void contiguous(long *A, long lenA) {
  int r;
  int h = lenA / 10000;
  for (int i = 0; i < h; i++) {
    r = rand() % 500;
    A[r] = 'a';
    r = rand() % 500;
    A[r] = 'b';
    r = rand() % 500;
    A[r] = 'c';
  }
}

//accesses the same page every time
//no page faults should occur
void cacheIdeal(long *A, long lenA) {
  int r;
  int h = lenA / 10000;
  for (int i = 0; i < h; i++) {
    r = rand() % lenA;
    A[0] = 'a';
    r = rand() % lenA;
    A[0] = 'b';
    r = rand() % lenA;
    A[0] = 'c';
  }
}

//accesses random pages throughout
//should cause regular page faults if A is large enough
void thrashEnds(long *A, long lenA) {
  int r;
  int h = lenA / 10000;
  for (int i = 0; i < h; i++) {
    r = rand() % lenA;
    A[r] = 'a';
    r = rand() % lenA;
    A[r] = 'b';
    r = rand() % lenA;
    A[r] = 'c';
  }
}

void printTime(struct timeval tv) {
  printf ("%ld.%06ld\n", tv.tv_sec, tv.tv_usec);
}

typedef struct result {
  long f, b, n;
} rst;

int main() {
  
  //size of memory to allocate
  unsigned long   size;
  //user initialized number of megabytes to allocate
  int             MBs;
  //number of repetitions to run tests
  int             repetitions = 1;
  //iterator
  int             i; 

  //dmesg results file pointer
  FILE*           fdmesg;
  //error from system call
  int             error;
  //read and ignore strings in dmesg.dat
  char            buf1[30], buf2[30];
  //stores the page value identifier
  unsigned long   page_val;

  //used to store previous page value to compare with
  long   prev;
  //stores the actual sample size
  int             ss;

  //stores the start, end, and time delta for printing
  struct timeval start, end, diff;

  //timestamp for beginning of test  
  char startstamp[25];

  char command[400];
  char buffer[200];
  char buf[20];
  int linenum;
  std::vector<rst> vec;
  std::vector<rst> dscrp;
  rst rstv;
  FILE* syscall;
 
  //prompt user for number of MBs to allocate
  std::cout << "how many MBs do you want your vector to span? (ENTER 0 for default): ";
  std::cin >> MBs;
  
  //write start timestamp to file
  srand(time(0));
  int unique_id = (rand() % 999999) + 1000000;
  FILE* slog = fopen("/var/log/messages", "a");
  if(!slog) {
    printf("ERROR: Could not open system log\n");
    return 1;
  } else {
    sprintf(startstamp, "Starting Benchmark %d", unique_id);
    fprintf(slog, "%s\n", startstamp);
    fclose(slog); 
  }

  //convert megabytes to bytes
  //size = 125000 * MBs;
  long memsize = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE) * 0.95;
  printf("Memory size (bytes): %ld\n", memsize);
  if(MBs == 0) {
    size = memsize / sizeof(long); 
  } else {
    size = MBs * 131072;
  }
  printf("size: %lu bytes\n",size*8);

  //allocate memory
  {
  printf("Setting up\n");
  std::vector<long> V(size);
  printf("Set up finished\n");

  //test memory with similar pages (no page faults)
  gettimeofday(&start, 0);
  for (i = 0; i < repetitions; i++)
    cacheIdeal(V.data(), V.size());
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printf("Similar pages execution time: ");
  printTime(diff);

  //test memory at cache ideal positions (no page faults)
  gettimeofday(&start, 0);
  for (i = 0; i < repetitions; i++)
    contiguous(V.data(), V.size());
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printf("Cache ideal pages execution time: ");
  printTime(diff);

  //test memory at random pages across memory (lots of page faults)
  gettimeofday(&start, 0);
  for (i = 0; i < repetitions; i++)
    thrashEnds(V.data(), V.size());
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printf("Random pages execution time: ");
  printTime(diff);
  }
  //grep system log for starting line number
 
  //find line number of startstamp
  sprintf(command, 
    "grep -n \"%s\" /var/log/messages | sed 's/:/ /' | awk '{print $1;}'", 
    startstamp);
  syscall = popen(command, "r");
  if(syscall) {
    fgets(buffer, sizeof(buffer), syscall);
    sscanf(buffer, "%d", &linenum);
    pclose(syscall);
  } else {
    printf("ERROR: Couldn't find startstamp. Reading entire log.\n");
    linenum = 1;
  }
  
  //search for all NEWPAGE lines after starting line number
  sprintf(command, "sed -n \"%d,$ p\" /var/log/messages | grep -o \"NEWPAGE.*\"", linenum);
  syscall = popen(command, "r");
  if(!syscall) {
    printf("ERROR: Couldn't grep NEWPAGE lines.\n");
  } else {
  
    //read all NEWPAGE lines, storing num, front, and back values
    while(fgets(buffer, sizeof(buffer), syscall) != NULL) {
      sscanf(buffer, "%s %s %ld %s %ld %s %ld", 
		      buf, buf, &rstv.n, buf, &rstv.f, buf, &rstv.b);
      vec.push_back(rstv);
    }
    pclose(syscall);
  }
  
  std::vector<std::string> dscrp2;
  char h_msg[200];
  prev = -1;
  sscanf(h_msg, "%ld > %ld", 69, 68);
  dscrp2.push_back(std::string(h_msg));
  //find any times the queue front was newer than the queue back
  for(auto iter = vec.begin(); iter != vec.end(); ++iter) {
    if(prev > (*iter).f) {
      //printf("Observed head discrepency:\n before=%d after=%d\n", 
        //prev, (*iter).f);
      sscanf(h_msg, "%ld > %ld", prev, (*iter).f);
      dscrp2.push_back(std::string(h_msg));
    }
    if((*iter).f > (*iter).b) {
      dscrp.push_back((*iter));
    }
    prev = (*iter).f;
  }

  //print number of descrepencies
  printf("Discrepancies between head and tail: %d\n\n", dscrp.size());
  for(auto iter = dscrp.begin(); iter != dscrp.end(); ++iter) {
    printf("Front: %ld\nBack: %ld\nSize: %ld\n", (*iter).f, (*iter).b, (*iter).n);
  }
  printf("Discrepancies between head and previous head: %d\n\n", dscrp2.size());
  for(auto iter = dscrp2.begin(); iter != dscrp2.end(); ++iter) {
    printf("%s\n", (*iter));
  }


  return 0;
}


