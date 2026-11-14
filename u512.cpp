#include <u512.h>
uint512_t operator+(const uint512_t &a, const uint512_t &b)
{
    uint512_t result;
    unsigned __int128 carry = 0;

    for(size_t i = 0; i < 8; i++)
    {
        unsigned __int128 sum =(unsigned __int128)a.digits[i] + b.digits[i] + carry;

        result.digits[i] = (uint64_t)sum;
        carry = sum >> 64;
    }

    return result;
}
uint512_t operator-(const uint512_t &a, const uint512_t &b)
{
    //a - b
    uint512_t result;
    uint64_t borrow = 0;

    for(size_t i = 0; i < 8; i++)
    {
        unsigned __int128 sub = (unsigned __int128)b.digits[i] + borrow;

        result.digits[i] = (uint64_t)((unsigned __int128)a.digits[i] - sub);

        borrow = ((unsigned __int128)a.digits[i] < sub) ? 1 : 0;
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