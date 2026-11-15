#include "dh_sample.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstdlib>

static size_t bit_length(const uint512_t &x) {
  for (int i = 511; i >= 0; --i)
    if (get_bit(x, static_cast<size_t>(i)))
      return static_cast<size_t>(i + 1);
  return 0;
}

static uint512_t make_one() {
  uint512_t one;
  one.digits[0] = 1;
  return one;
}

static void print_hex(const uint512_t &x) {
  bool started = false;
  for (int i = 7; i >= 0; --i) {
    if (!started) {
      if (x.digits[i] == 0) continue;
      std::printf("%llx", (unsigned long long)x.digits[i]);
      started = true;
    } else {
      std::printf("%016llx", (unsigned long long)x.digits[i]);
    }
  }
  if (!started) std::printf("0");
}

static void run_test(prng_ctx *ctx, size_t bits) {
  std::printf("\nGenerating %zu-bit safe prime...\n", bits);
  auto start = std::chrono::steady_clock::now();
  uint512_t p = generate_safe_prime(ctx, bits);
  auto stop = std::chrono::steady_clock::now();
  uint512_t one = make_one();
  uint512_t q = (p - one) >> 1;

  bool correct_size = bit_length(p) == bits;
  bool p_prime = miller_rabin_test(ctx, p, 20);
  bool q_prime = miller_rabin_test(ctx, q, 20);
  bool relation = ((q << 1) + one) == p;

  std::printf("p bits: %zu; q bits: %zu\n", bit_length(p), bit_length(q));
  std::printf("p = "); print_hex(p); std::printf("\n");
  std::printf("q = "); print_hex(q); std::printf("\n");
  std::printf("p prime: %s\n", p_prime ? "PASS" : "FAIL");
  std::printf("q prime: %s\n", q_prime ? "PASS" : "FAIL");
  std::printf("p = 2q + 1: %s\n", relation ? "PASS" : "FAIL");
  std::printf("exact bit length: %s\n", correct_size ? "PASS" : "FAIL");
  std::printf("elapsed: %.3f seconds\n",
      std::chrono::duration<double>(stop - start).count());

  if (!(correct_size && p_prime && q_prime && relation))
    std::abort();
}

int main(int argc, char **argv) {
  prng_ctx ctx;
  uint8_t seed[32];
  std::memset(seed, 0x37, sizeof(seed));
  prng_seed(&ctx, seed, sizeof(seed));

  if (argc == 1) {
    run_test(&ctx, 64);
    run_test(&ctx, 128);
  } else {
    for (int i = 1; i < argc; ++i) {
      size_t bits = static_cast<size_t>(std::strtoul(argv[i], nullptr, 10));
      run_test(&ctx, bits);
    }
  }
  return 0;
}
