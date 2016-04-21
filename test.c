#include "ngx_http_txid120_logic.h"
#include <stdio.h>

// test harness to generate a txid120. run and verify with test.pl.

int main(int argc, char **argv) {
  FILE* urandom = fopen("/dev/urandom", "r");
  if (urandom == NULL) {
    printf("could not open /dev/urandom\n");
    return 1;
  }

  uint8_t txid120[20];
  ngx_http_txid120_logic(urandom, txid120);
  printf("%.20s\n", txid120);

  return 0;
}
