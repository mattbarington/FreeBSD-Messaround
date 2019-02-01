#include <stdio.h>
#include <openssl/rand.h>

int main() {
  unsigned char* buf = malloc(100*sizeof(char));
  RAND_bytes(buf, 100);
  printf("r = %s\n", buf);
  return 0;
}
