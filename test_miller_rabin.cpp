#include "dh_sample.h"
#include "prng.h"
#include "u512.h"
#include <cassert>
#include <cstdio>
#include <cstring>

static uint512_t u512_from_u64(uint64_t v) {
  uint512_t r;
  r.digits[0] = v;
  return r;
}

int main() {
  printf("Khung kiem thu Miller-Rabin da san sang.\n");
  return 0;
}
