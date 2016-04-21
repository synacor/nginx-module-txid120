#include "ngx_http_txid120_logic.h"

static const char b64_chars[] = "0123456789:@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void ngx_http_txid120_logic(FILE* urandom, uint8_t* txid120) {
  uint8_t txid[15];
  unsigned int i;

  struct timeval time;
  gettimeofday(&time, NULL);
  uint64_t high56 = time.tv_sec * 1000000 + time.tv_usec;

  // fill first 7 bytes with usec-since-epoch timestamp
  for (i=0; i<=6; i++) {
    txid[i] = (high56 >> (8*(6-i))) & 0xff;
  }

  // fill remaining 8 bytes with random data
  fread(&txid[7], 8, 1, urandom);

  // encode as base64
  for (i=0; i<5; i++) {
    uint32_t block =
        (txid[i*3+0] << 8*2)
      | (txid[i*3+1] << 8*1)
      | (txid[i*3+2] << 8*0);

    txid120[i*4+0] = b64_chars[(block >> (6*3)) & 0x3f];
    txid120[i*4+1] = b64_chars[(block >> (6*2)) & 0x3f];
    txid120[i*4+2] = b64_chars[(block >> (6*1)) & 0x3f];
    txid120[i*4+3] = b64_chars[(block >> (6*0)) & 0x3f];
  }
}
