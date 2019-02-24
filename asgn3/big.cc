
#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#define SAMPLE_SIZE 10000

void start(long* A, long lenA) {
  A[0] ++;
}

void end(long *A, long lenA) {
  A[lenA - 1] ++;
}

void middle(long *A, long lenA) {
  A[lenA/2] ++;
}

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

int main() {
  unsigned long size = 1139600000;
  int MBs;
  int repetitions = 1;
  double div;
  std::cout << "how many MBs do you want your vector to span? ";
  std::cin >> MBs;
  //  std::cout << "How many times would you like to run the thing?: ";
  //  std::cin >> repetitions;
  size = 125000 * MBs;
  printf("size: %lu bytes\n",size*8);
  //  const unsigned long size = 1139600000;
  //  const int repetitions = 100;
  printf("Setting up\n");
  std::vector<long> V(size);
  srand(0);
  struct timeval start, end, diff;
  printf("Set up finished\n");

  gettimeofday(&start, 0);
  for (int i = 0; i < repetitions; i++)
    cacheIdeal(V.data(), V.size());
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printTime(diff);

  gettimeofday(&start, 0);
  for (int i = 0; i < repetitions; i++)
    contiguous(V.data(), V.size());
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printTime(diff);


  gettimeofday(&start, 0);
  for (int i = 0; i < repetitions; i++)
    thrashEnds(V.data(), V.size());
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printTime(diff);

  FILE* fdmesg;
  int error, iter;
  char read_dmesg[] = "dmesg | grep \"freeing pages\" > dmesg.dat";
  char buf1[30];
  char buf2[30];
  unsigned long page_val;

  int cnt1 = 0, cnt2 = 0;
  
  //write results to file
  error = system(read_dmesg);
  unsigned long sample[SAMPLE_SIZE];

  //attempt to open file
  fdmesg = fopen("dmesg.dat", "r");
  iter = 0;
  int ss;
  unsigned long prev;
  if(fdmesg) {
    while((iter < SAMPLE_SIZE) && fscanf(fdmesg, "%s %s %lu", buf1, buf2, &page_val) != EOF) {
      sample[iter] = page_val;
      ++iter;
    }
  } else {
    printf("ERROR: dmesg.dat could not be opened.\n");
  }
  ss = iter;

  prev = sample[0];
  for(iter = 1; iter < ss; ++iter) {
    if(prev == (sample[iter] - 1)) {
      cnt1++;
    } else {
      cnt2++;
    }
    prev = sample[iter];
  }

  printf("consecutive: %d, other: %d\n", cnt1, cnt2);

  fclose(fdmesg);
  remove("dmesg.dat");

}
