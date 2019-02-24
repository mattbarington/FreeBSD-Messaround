
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

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
  int r, i;
  int h = lenA / 10000;
  for (i = 0; i < h; i++) {
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
  int MBs, i;
  int repetitions = 1;
  double div;
  printf("how many MBs do you want your vector to span? ");
  scanf("%d", &MBs);
  //  std::cout << "How many times would you like to run the thing?: ";
  //  std::cin >> repetitions;
  size = 125000 * MBs;
  printf("size: %lu bytes\n",size*8);
  //  const unsigned long size = 1139600000;
  //  const int repetitions = 100;
  printf("Setting up\n");
  //std::vector<long> V(size);
  long* V = malloc(size*sizeof(long));
  srand(0);
  struct timeval start, end, diff;
  printf("Set up finished\n");

  gettimeofday(&start, 0);
  for (i = 0; i < repetitions; i++)
    cacheIdeal(V, size);
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printTime(diff);

  gettimeofday(&start, 0);
  for (i = 0; i < repetitions; i++)
    contiguous(V, size);
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printTime(diff);


  gettimeofday(&start, 0);
  for (int i = 0; i < repetitions; i++)
    thrashEnds(V, size);
  gettimeofday(&end, 0);
  timersub(&end, &start, &diff);
  printTime(diff);

  free(V);
}
