#include <stdio.h>
#include <openssl/rand.h> //supposedly kernel level random number generator

// other candidates for randomness: random(4), dev/random, dev/urandom

int main() {
  unsigned char* buf = malloc(100*sizeof(char));
  RAND_bytes(buf, 100);
  printf("r = %s\n", buf);
  return 0;
}
