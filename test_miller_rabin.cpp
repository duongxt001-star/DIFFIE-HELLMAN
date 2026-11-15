#include "dh_sample.h"
#include "prng.h"
#include "u512.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <vector>

static uint512_t u512_from_u64(uint64_t v) {
  uint512_t r;
  r.digits[0] = v;
  return r;
}

void test_rand_range(prng_ctx *ctx) {
  printf("--- 1. Testing rand_range_u512 ---\n");
  uint512_t min = u512_from_u64(2);
  uint512_t max = u512_from_u64(10);
  for (int i = 0; i < 100; i++) {
    uint512_t val = rand_range_u512(ctx, min, max);
    assert(val >= min);
    assert(val <= max);
  }
  uint512_t same = rand_range_u512(ctx, min, min);
  assert(same == min);
  printf("[PASS] rand_range_u512 satisfies range constraints.\n\n");
}

void test_small_primes(prng_ctx *ctx) {
  printf("--- 2. Testing Known Small Primes ---\n");
  std::vector<uint64_t> primes = {2,  3,  5,  7,   11,  13,        17,
                                  19, 23, 29, 31,  97,  541,       10007,
                                  65537,      1000000007ULL};

  for (uint64_t p : primes) {
    bool res = miller_rabin_test(ctx, u512_from_u64(p), 20);
    assert(res == true);
    printf("  Prime %10llu -> PASS\n", (unsigned long long)p);
  }
  printf("[PASS] All small primes correctly identified!\n\n");
}

void test_small_composites(prng_ctx *ctx) {
  printf("--- 3. Testing Known Small Composites ---\n");
  std::vector<uint64_t> composites = {0,  1,  4,  6,   8,  9,         15,
                                      21, 25, 27, 33,  35, 100,       500,
                                      1000000005ULL};

  for (uint64_t c : composites) {
    bool res = miller_rabin_test(ctx, u512_from_u64(c), 20);
    assert(res == false);
    printf("  Composite %10llu -> PASS (detected as composite)\n",
           (unsigned long long)c);
  }
  printf("[PASS] All small composites correctly identified!\n\n");
}

void test_carmichael_numbers(prng_ctx *ctx) {
  printf("--- 4. Testing Carmichael Numbers (Fermat Pseudoprimes) ---\n");
  // Carmichael numbers fool the naive Fermat test a^(n-1) == 1 mod n
  // but Miller-Rabin detects non-trivial square roots of 1 and rejects them!
  std::vector<uint64_t> carmichaels = {561, 1105, 1729, 2821, 6601};

  for (uint64_t c : carmichaels) {
    bool res = miller_rabin_test(ctx, u512_from_u64(c), 20);
    assert(res == false);
    printf("  Carmichael %6llu -> PASS (Miller-Rabin successfully identified as COMPOSITE)\n",
           (unsigned long long)c);
  }
  printf("[PASS] All Carmichael numbers successfully rejected by Miller-Rabin!\n\n");
}

void test_large_known_primes(prng_ctx *ctx) {
  printf("--- 5. Testing Large Known Primes ---\n");

  // Mersenne 31: 2^31 - 1
  uint512_t m31 = u512_from_u64(2147483647ULL);
  assert(miller_rabin_test(ctx, m31, 20) == true);
  printf("  Mersenne 31 (2^31 - 1 = 2147483647) -> PASS\n");

  // Mersenne 61: 2^61 - 1
  uint512_t m61 = u512_from_u64(2305843009213693951ULL);
  assert(miller_rabin_test(ctx, m61, 20) == true);
  printf("  Mersenne 61 (2^61 - 1) -> PASS\n");

  // 128-bit prime: 2^128 - 159
  uint512_t p128;
  p128.digits[0] = 0xffffffffffffff61ULL;
  p128.digits[1] = 0xffffffffffffffffULL;
  assert(miller_rabin_test(ctx, p128, 20) == true);
  printf("  128-bit Prime (2^128 - 159) -> PASS\n");

  // 256-bit prime: NIST P-256
  // 0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF
  uint512_t p256;
  p256.digits[0] = 0xFFFFFFFFFFFFFFFFULL;
  p256.digits[1] = 0x00000000FFFFFFFFULL;
  p256.digits[2] = 0x0000000000000000ULL;
  p256.digits[3] = 0xFFFFFFFF00000001ULL;
  assert(miller_rabin_test(ctx, p256, 20) == true);
  printf("  256-bit Prime (NIST P-256) -> PASS\n");

  // 512-bit prime: 2^512 - 569
  uint512_t p512;
  p512.digits[0] = 0xfffffffffffffdc7ULL;
  for (size_t i = 1; i < 8; i++) {
    p512.digits[i] = 0xffffffffffffffffULL;
  }
  assert(miller_rabin_test(ctx, p512, 20) == true);
  printf("  512-bit Prime (2^512 - 569) -> PASS\n");

  // 512-bit composite: 2^512 - 571
  uint512_t c512;
  c512.digits[0] = 0xfffffffffffffdc5ULL;
  for (size_t i = 1; i < 8; i++) {
    c512.digits[i] = 0xffffffffffffffffULL;
  }
  assert(miller_rabin_test(ctx, c512, 20) == false);
  printf("  512-bit Composite (2^512 - 571) -> PASS (detected as composite)\n");

  printf("[PASS] All large known primes and composites verified!\n\n");
}

void test_timing_benchmark(prng_ctx *ctx) {
  printf("--- 6. Performance Benchmark (512-bit Prime, k = 20 rounds) ---\n");
  uint512_t p512;
  p512.digits[0] = 0xfffffffffffffdc7ULL;
  for (size_t i = 1; i < 8; i++) {
    p512.digits[i] = 0xffffffffffffffffULL;
  }

  const int NUM_RUNS = 5;
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_RUNS; i++) {
    bool ok = miller_rabin_test(ctx, p512, 20);
    assert(ok == true);
  }
  auto end = std::chrono::high_resolution_clock::now();
  double total_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  double avg_ms = total_ms / NUM_RUNS;

  printf("  Total time for %d runs: %.2f ms\n", NUM_RUNS, total_ms);
  printf("  Average time per 512-bit Miller-Rabin test (20 rounds): %.2f ms\n",
         avg_ms);
  printf("[PASS] Benchmark completed successfully!\n\n");
}

int main() {
  printf("=================================================================\n");
  printf("        MILLER-RABIN PRIMALITY TEST VERIFICATION SUITE           \n");
  printf("=================================================================\n\n");

  prng_ctx ctx;
  uint8_t seed[32];
  memset(seed, 0xA5, sizeof(seed));
  prng_seed(&ctx, seed, sizeof(seed));

  test_rand_range(&ctx);
  test_small_primes(&ctx);
  test_small_composites(&ctx);
  test_carmichael_numbers(&ctx);
  test_large_known_primes(&ctx);
  test_timing_benchmark(&ctx);

  printf("=================================================================\n");
  printf("        ALL TEST SUITES PASSED WITH 100%% SUCCESS RATE!          \n");
  printf("=================================================================\n");
  return 0;
}
