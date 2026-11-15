#ifndef DH_H
#define DH_H

#include <stdint.h>
#include "prng.h"
#include "u512.h"

// Modular arithmetic functions (cài đặt theo pseudocode-modular-operation)
int compare_u512(const uint512_t &a, const uint512_t &b);
uint512_t mod_u512(const uint512_t &x, const uint512_t &y);
uint512_t add_mod_u512(const uint512_t &x, const uint512_t &y, const uint512_t &p);
uint512_t mult_mod_u512(const uint512_t &x, const uint512_t &y, const uint512_t &p);
uint512_t exp_mod_u512(const uint512_t &base, const uint512_t &exp, const uint512_t &mod);

// Các hàm nghiệp vụ của bài tập lớn Lab01
uint512_t modular_exponentiation(const uint512_t &base, const uint512_t &exp, const uint512_t &mod);
uint512_t generate_safe_prime(prng_ctx *ctx, size_t bitsize = 512);
uint512_t generate_private_key(prng_ctx *ctx, const uint512_t &p);

#endif
