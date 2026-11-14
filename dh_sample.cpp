#include "dh_sample.h"

// So sanh hai so nguyen 512 bit
static int compare_u512(const uint512_t &a, const uint512_t &b)
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

// Lay bit thu i cua so nguyen
static bool get_bit(const uint512_t &x, int i)
{
    return (x.digits[i / 64] >> (i % 64)) & 1ULL;
}

// 1. Mod: Tinh x % y
static uint512_t Mod(const uint512_t &x, const uint512_t &y)
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

// 2. AddMod: Tinh (x + y) % p

static uint512_t AddMod(
    const uint512_t &x,
    const uint512_t &y,
    const uint512_t &p)
{
    // Dieu kien: x < p va y < p

    uint512_t remaining = p - y;

    if (compare_u512(x, remaining) >= 0)
    {
        return x - remaining;
    }

    return x + y;
}


// 3. MultMod: Tinh (x * y) % p
static uint512_t MultMod(
    const uint512_t &x,
    const uint512_t &y,
    const uint512_t &p)
{
    uint512_t ret;
    uint512_t base = Mod(x, p);

    for (int i = 0; i < 512; i++)
    {
        if (get_bit(y, i))
        {
            ret = AddMod(ret, base, p);
        }

        base = AddMod(base, base, p);
    }

    return ret;
}

// 4. ExpMod: Tinh (x^y) % p
uint512_t modular_exponentiation(
    const uint512_t &base,
    const uint512_t &exp,
    const uint512_t &mod)
{
    uint512_t ret;
    ret.digits[0] = 1;

    if (compare_u512(mod, uint512_t()) == 0)
        return uint512_t();

    ret = Mod(ret, mod);

    uint512_t current_base = Mod(base, mod);

    for (int i = 0; i < 512; i++)
    {
        if (get_bit(exp, i))
        {
            ret = MultMod(ret, current_base, mod);
        }

        current_base = MultMod(
            current_base,
            current_base,
            mod
        );
    }

    return ret;
}
