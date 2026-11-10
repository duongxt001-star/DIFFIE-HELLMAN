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
    for(size_t i = 0; i < 8; i++)
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
    for(size_t i = 0; i < 8; i++)
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
    for(size_t i = 0; i < 8; i++)
    {
        if(a.digits[i] >= b.digits[i])
            return true;
        else if(a.digits[i] < b.digits[i])
            return false;
    }
    return true; // Nếu tất cả các chữ số bằng nhau, a lớn hơn hoặc bằng b
}
bool operator<=(const uint512_t &a, const uint512_t &b)
{
    for(size_t i = 0; i < 8; i++)
    {
        if(a.digits[i] <= b.digits[i])
            return true;
        else if(a.digits[i] > b.digits[i])
            return false;
    }
    return true; // Nếu tất cả các chữ số bằng nhau, a nhỏ hơn hoặc bằng b
}