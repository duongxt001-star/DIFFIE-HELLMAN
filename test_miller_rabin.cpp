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

void test_rand_range(prng_ctx *ctx) {
  uint512_t min = u512_from_u64(2);
  uint512_t max = u512_from_u64(10);
  for (int i = 0; i < 100; i++) {
    uint512_t val = rand_range_u512(ctx, min, max);
    assert(val >= min);
    assert(val <= max);
  }
  uint512_t same = rand_range_u512(ctx, min, min);
  assert(same == min);
  printf("[PASS] test_rand_range\n");
}

int main() {
  prng_ctx ctx;
  uint8_t seed[32];
  memset(seed, 0x42, sizeof(seed));
  prng_seed(&ctx, seed, sizeof(seed));

  test_rand_range(&ctx);
  return 0;
}
