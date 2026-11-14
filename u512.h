#ifndef U512_H
#define U512_H

#define uint128_t __uint128_t
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <iostream>

using namespace std;

struct uint512_t
{
    uint64_t digits[8];

    // Constructor mặc định khởi tạo mảng toàn số 0
    uint512_t()
    {
        for (size_t i = 0; i < 8; i++)
        {
            digits[i] = 0;
        }
    }
};

uint512_t operator+(const uint512_t &a, const uint512_t &b);
uint512_t operator-(const uint512_t &a, const uint512_t &b);
bool operator==(const uint512_t &a, const uint512_t &b);
bool operator<(const uint512_t &a, const uint512_t &b);
bool operator>(const uint512_t &a, const uint512_t &b);
bool operator>=(const uint512_t &a, const uint512_t &b);
bool operator<=(const uint512_t &a, const uint512_t &b);
bool get_bit(const uint512_t &x, size_t i);
void set_bit(uint512_t &x, size_t i, bool v);
uint512_t operator<<(const uint512_t &a, size_t n);
uint512_t operator>>(const uint512_t &a, size_t n);
#endif