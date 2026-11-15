#ifndef DH_H
#define DH_H

#include "prng.h"
#include "u512.h"
#include <stdint.h>

int compare_u512(const uint512_t &a, const uint512_t &b);
uint512_t mod_u512(const uint512_t &x, const uint512_t &y);
uint512_t add_mod_u512(const uint512_t &x, const uint512_t &y,
                       const uint512_t &p);
uint512_t mult_mod_u512(const uint512_t &x, const uint512_t &y,
                        const uint512_t &p);
uint512_t exp_mod_u512(const uint512_t &base, const uint512_t &exp,
                       const uint512_t &mod);

uint512_t modular_exponentiation(const uint512_t &base, const uint512_t &exp,
                                 const uint512_t &mod);
uint512_t rand_range_u512(prng_ctx *ctx, const uint512_t &min,
                          const uint512_t &max);
bool miller_rabin_test(prng_ctx *ctx, const uint512_t &n, size_t rounds = 20);
uint512_t generate_safe_prime(prng_ctx *ctx, size_t bitsize = 512);
uint512_t generate_private_key(prng_ctx *ctx, const uint512_t &p);

#endif
