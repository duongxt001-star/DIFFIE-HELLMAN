#include "dh_sample.h"

int compare_u512(const uint512_t &a, const uint512_t &b)
{
    for (int i = 7; i >= 0; i--)
    {
        if (a.digits[i] > b.digits[i])
            return 1;

        if (a.digits[i] < b.digits[i])
            return -1;
    }

    return 0;
}

bool get_bit(const uint512_t &x, int i)
{
    return (x.digits[i / 64] >> (i % 64)) & 1ULL;
}

uint512_t mod_u512(const uint512_t &x, const uint512_t &y)
{
    uint512_t rem;

    if (compare_u512(y, uint512_t()) == 0)
        return rem;

    for (int i = 511; i >= 0; i--)
    {
        bool overflow = (rem.digits[7] >> 63) & 1ULL;
        uint64_t carry = get_bit(x, i);

        for (int j = 0; j < 8; j++)
        {
            uint64_t next_carry = rem.digits[j] >> 63;
            rem.digits[j] = (rem.digits[j] << 1) | carry;
            carry = next_carry;
        }

        if (overflow || compare_u512(rem, y) >= 0)
        {
            rem = rem - y;
        }
    }

    return rem;
}

uint512_t add_mod_u512(
    const uint512_t &x,
    const uint512_t &y,
    const uint512_t &p)
{
    if (compare_u512(p, uint512_t()) == 0)
        return uint512_t();

    uint512_t a = mod_u512(x, p);
    uint512_t b = mod_u512(y, p);

    uint512_t remaining = p - b;

    if (compare_u512(a, remaining) >= 0)
    {
        return a - remaining;
    }

    return a + b;
}

uint512_t mult_mod_u512(
    const uint512_t &x,
    const uint512_t &y,
    const uint512_t &p)
{
    uint512_t ret;
    uint512_t current_base;

    if (compare_u512(p, uint512_t()) == 0)
        return ret;

    current_base = mod_u512(x, p);

    for (int i = 0; i < 512; i++)
    {
        if (get_bit(y, i))
        {
            ret = add_mod_u512(ret, current_base, p);
        }

        current_base = add_mod_u512(
            current_base,
            current_base,
            p
        );
    }

    return ret;
}

uint512_t exp_mod_u512(
    const uint512_t &base,
    const uint512_t &exp,
    const uint512_t &mod)
{
    uint512_t one;
    one.digits[0] = 1;

    if (compare_u512(mod, uint512_t()) == 0)
        return uint512_t();

    uint512_t result = mod_u512(one, mod);
    uint512_t current_base = mod_u512(base, mod);

    for (int i = 0; i < 512; i++)
    {
        if (get_bit(exp, i))
        {
            result = mult_mod_u512(
                result,
                current_base,
                mod
            );
        }

        current_base = mult_mod_u512(
            current_base,
            current_base,
            mod
        );
    }

    return result;
}

uint512_t modular_exponentiation(
    const uint512_t &base,
    const uint512_t &exp,
    const uint512_t &mod)
{
    return exp_mod_u512(base, exp, mod);
}
