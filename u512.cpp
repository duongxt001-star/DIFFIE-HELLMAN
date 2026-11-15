#include <u512.h>
uint512_t operator+(const uint512_t &a, const uint512_t &b)
{
    uint512_t result;
    uint64_t carry = 0;

    for (size_t i = 0; i < 8; i++)
    {
        uint64_t s = a.digits[i] + b.digits[i];
        uint64_t c1 = (s < a.digits[i]) ? 1 : 0;   // tràn khi cộng a + b
        uint64_t s2 = s + carry;
        uint64_t c2 = (s2 < s) ? 1 : 0;            // tràn khi cộng carry

        result.digits[i] = s2;
        carry = c1 | c2;
    }
    return result;
}

uint512_t operator-(const uint512_t &a, const uint512_t &b)
{
    uint512_t result;
    uint64_t borrow = 0;

    for (size_t i = 0; i < 8; i++)
    {
        uint64_t d = a.digits[i] - b.digits[i];
        uint64_t b1 = (a.digits[i] < b.digits[i]) ? 1 : 0;
        uint64_t d2 = d - borrow;
        uint64_t b2 = (d < borrow) ? 1 : 0;

        result.digits[i] = d2;
        borrow = b1 | b2;
    }
    return result;
}
bool operator==(const uint512_t &a, const uint512_t &b)
{
    for(size_t i = 0; i < 8; i++)
    {
        if(a.digits[i] != b.digits[i])
            return false;
    }
    return true;
}
bool operator<(const uint512_t &a, const uint512_t &b)
{
    for(int i = 7; i >= 0; i--)
    {
        if(a.digits[i] < b.digits[i])
            return true;
        else if(a.digits[i] > b.digits[i])
            return false;
    }
    return false; // Nếu tất cả các chữ số bằng nhau, a không nhỏ hơn b
}
bool operator>(const uint512_t &a, const uint512_t &b)
{
    for(int i = 7; i >= 0; i--)
    {
        if(a.digits[i] > b.digits[i])
            return true;
        else if(a.digits[i] < b.digits[i])
            return false;
    }
    return false; // Nếu tất cả các chữ số bằng nhau, a không lớn hơn b
}
bool operator>=(const uint512_t &a, const uint512_t &b)
{
    for(int i = 7; i >= 0; i--)
    {
        if(a.digits[i] > b.digits[i])
            return true;
        else if(a.digits[i] < b.digits[i])
            return false;
    }
    return true; // Nếu tất cả các chữ số bằng nhau, a lớn hơn hoặc bằng b
}
bool operator<=(const uint512_t &a, const uint512_t &b)
{
    for(int i = 7; i >= 0; i--)
    {
        if(a.digits[i] < b.digits[i])
            return true;
        else if(a.digits[i] > b.digits[i])
            return false;
    }
    return true; // Nếu tất cả các chữ số bằng nhau, a nhỏ hơn hoặc bằng b
}
bool get_bit(const uint512_t &x, size_t i)
{
    if (i >= 512) return false;
    return ((x.digits[i / 64] >> (i % 64)) & 1ULL) != 0;
}
void set_bit(uint512_t &x, size_t i, bool v)
{
    if (i >= 512) return;
    uint64_t mask = 1ULL << (i % 64);
    if (v) x.digits[i / 64] |= mask;    // bật bit lên 1
    else   x.digits[i / 64] &= ~mask;   // tắt bit về 0
}
uint512_t operator<<(const uint512_t &a, size_t n)
{
    uint512_t result;
    size_t shift = n % 64;
    size_t offset = n / 64;

    for (size_t i = 0; i < 8; i++)
    {
        if (i + offset < 8)
        {
            result.digits[i + offset] |= a.digits[i] << shift;
        }
        if (shift > 0 && i + offset + 1 < 8)
        {
            result.digits[i + offset + 1] |= a.digits[i] >> (64 - shift);
        }
    }

    return result;
}
uint512_t operator>>(const uint512_t &a, size_t n)
{
    uint512_t result;
    size_t shift = n % 64;
    size_t offset = n / 64;

    for (size_t i = 0; i < 8; i++)
    {
        if (i >= offset)
        {
            result.digits[i - offset] |= a.digits[i] >> shift;
        }
        if (shift > 0 && i > offset)
        {
            result.digits[i - offset - 1] |= a.digits[i] << (64 - shift);
        }
    }

    return result;
}