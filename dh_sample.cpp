#include "dh_sample.h"

int compare_u512(const uint512_t &a, const uint512_t &b) {
  for (int i = 7; i >= 0; i--) {
    if (a.digits[i] > b.digits[i])
      return 1;

    if (a.digits[i] < b.digits[i])
      return -1;
  }

  return 0;
}

uint512_t mod_u512(const uint512_t &x, const uint512_t &y) {
  uint512_t rem;

  if (compare_u512(y, uint512_t()) == 0)
    return rem;

  for (int i = 511; i >= 0; i--) {
    bool overflow = (rem.digits[7] >> 63) & 1ULL;
    uint64_t carry = get_bit(x, (size_t)i);

    for (int j = 0; j < 8; j++) {
      uint64_t next_carry = rem.digits[j] >> 63;
      rem.digits[j] = (rem.digits[j] << 1) | carry;
      carry = next_carry;
    }

    if (overflow || compare_u512(rem, y) >= 0) {
      rem = rem - y;
    }
  }

  return rem;
}

uint512_t add_mod_u512(const uint512_t &x, const uint512_t &y,
                       const uint512_t &p) {
  if (compare_u512(p, uint512_t()) == 0)
    return uint512_t();

  uint512_t a = (compare_u512(x, p) >= 0) ? mod_u512(x, p) : x;
  uint512_t b = (compare_u512(y, p) >= 0) ? mod_u512(y, p) : y;

  uint512_t remaining = p - b;

  if (compare_u512(a, remaining) >= 0) {
    return a - remaining;
  }

  return a + b;
}

uint512_t mult_mod_u512(const uint512_t &x, const uint512_t &y,
                        const uint512_t &p) {
  uint512_t ret;
  uint512_t current_base;

  if (compare_u512(p, uint512_t()) == 0)
    return ret;

  current_base = (compare_u512(x, p) >= 0) ? mod_u512(x, p) : x;

  for (int i = 0; i < 512; i++) {
    if (get_bit(y, (size_t)i)) {
      ret = add_mod_u512(ret, current_base, p);
    }

    current_base = add_mod_u512(current_base, current_base, p);
  }

  return ret;
}

uint512_t exp_mod_u512(const uint512_t &base, const uint512_t &exp,
                       const uint512_t &mod) {
  uint512_t one;
  one.digits[0] = 1;

  if (compare_u512(mod, uint512_t()) == 0)
    return uint512_t();

  uint512_t result = mod_u512(one, mod);
  uint512_t current_base = mod_u512(base, mod);

  for (int i = 0; i < 512; i++) {
    if (get_bit(exp, (size_t)i)) {
      result = mult_mod_u512(result, current_base, mod);
    }

    current_base = mult_mod_u512(current_base, current_base, mod);
  }

  return result;
}

uint512_t modular_exponentiation(const uint512_t &base, const uint512_t &exp,
                                 const uint512_t &mod) {
  return exp_mod_u512(base, exp, mod);
}

uint512_t rand_range_u512(prng_ctx *ctx, const uint512_t &min,
                          const uint512_t &max) {
  if (min >= max)
    return min;

  uint512_t diff = max - min;
  bool is_full_range = true;
  for (size_t i = 0; i < 8; i++) {
    if (diff.digits[i] != 0xFFFFFFFFFFFFFFFFULL) {
      is_full_range = false;
      break;
    }
  }

  uint512_t r;
  for (size_t i = 0; i < 8; i++) {
    r.digits[i] = prng_next_u64(ctx);
  }

  if (is_full_range)
    return r;

  uint512_t one;
  one.digits[0] = 1;
  uint512_t range = diff + one;

  uint512_t offset = mod_u512(r, range);
  return min + offset;
}

bool miller_rabin_test(prng_ctx *ctx, const uint512_t &n, size_t rounds) {
  if (rounds == 0)
    return false;

  uint512_t two;
  two.digits[0] = 2;
  uint512_t three;
  three.digits[0] = 3;

  if (n < two)
    return false;
  if (n == two || n == three)
    return true;
  if (!get_bit(n, 0))
    return false;

  uint512_t one;
  one.digits[0] = 1;
  uint512_t n_minus_1 = n - one;
  uint512_t d = n_minus_1;
  size_t s = 0;

  while (!get_bit(d, 0)) {
    d = d >> 1;
    s++;
  }

  uint512_t n_minus_2 = n - two;

  for (size_t i = 0; i < rounds; i++) {
    uint512_t a = rand_range_u512(ctx, two, n_minus_2);
    uint512_t x = exp_mod_u512(a, d, n);

    if (x == one || x == n_minus_1)
      continue;

    bool composite = true;
    for (size_t r = 1; r < s; r++) {
      x = mult_mod_u512(x, x, n);
      if (x == n_minus_1) {
        composite = false;
        break;
      }
      if (x == one) {
        return false;
      }
    }

    if (composite)
      return false;
  }

  return true;
}


static bool passes_small_prime_filter(const uint512_t &n) {
  static const uint32_t small_primes[] = {
      3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
      53, 59, 61, 67, 71, 73, 79, 83, 89, 97
  };

  for (uint32_t prime : small_primes) {
    uint64_t remainder = 0;
    for (int i = 7; i >= 0; --i) {
      unsigned __int128 value =
          (static_cast<unsigned __int128>(remainder) << 64) | n.digits[i];
      remainder = static_cast<uint64_t>(value % prime);
    }
    if (remainder == 0) {
      bool equal_to_prime = (n.digits[0] == prime);
      for (size_t i = 1; i < 8; ++i)
        equal_to_prime = equal_to_prime && (n.digits[i] == 0);
      if (!equal_to_prime)
        return false;
    }
  }
  return true;
}

static uint512_t random_odd_with_bits(prng_ctx *ctx, size_t bits) {
  uint512_t candidate;
  const size_t words = (bits + 63) / 64;
  for (size_t i = 0; i < words; ++i)
    candidate.digits[i] = prng_next_u64(ctx);

  const size_t used_bits = bits % 64;
  if (used_bits != 0)
    candidate.digits[words - 1] &= ((1ULL << used_bits) - 1ULL);

  set_bit(candidate, bits - 1, true);
  set_bit(candidate, 0, true);
  return candidate;
}

uint512_t generate_safe_prime(prng_ctx *ctx, size_t bitsize) {
  if (ctx == nullptr || bitsize < 4 || bitsize > 512)
    return uint512_t();

  const size_t q_bits = bitsize - 1;
  uint512_t one;
  one.digits[0] = 1;

  while (true) {
    uint512_t q = random_odd_with_bits(ctx, q_bits);

    if (!passes_small_prime_filter(q))
      continue;
    if (!miller_rabin_test(ctx, q, 20))
      continue;

    uint512_t p = (q << 1) + one;
    if (!passes_small_prime_filter(p))
      continue;
    if (!miller_rabin_test(ctx, p, 20))
      continue;

    return p;
  }
}
