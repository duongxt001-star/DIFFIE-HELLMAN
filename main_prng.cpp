// main_dh.cpp - Trao đổi khóa Diffie-Hellman (Lab01)
//
// Cách dùng:
//   ./dh                 -> p 512 bit, seed ngẫu nhiên từ hệ điều hành
//   ./dh 1024            -> p 1024 bit (cần uint1024_t nếu bạn đổi kiểu số lớn)
//   ./dh 512 --fixed     -> dùng seed cố định để kết quả lặp lại được (debug)
//
// File này phụ thuộc vào:
//   - u512_sample.h  : kiểu uint512_t + các toán tử (cần có file .cpp cài đặt)
//   - dh_sample.h    : modular_exponentiation, generate_safe_prime, generate_private_key
//   - prng.h         : PRNG dùng SHAKE128

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "prng.h"
#include "u512.h"
#include "dh_sample.h"

// ---------------------------------------------------------------------------
// Hàm tiện ích (không thuộc phần bạn phải làm ở bước 3, 4, 5)
// ---------------------------------------------------------------------------

// In số lớn dạng hex (digits[0] là chunk thấp nhất, digits[7] là chunk cao nhất).
static void print_u512(const char *label, const uint512_t &x)
{
    printf("%s0x", label);
    for (int i = 7; i >= 0; i--)
    {
        printf("%016" PRIx64, x.digits[i]);
    }
    printf("\n");
}

// Tạo uint512_t từ một số nhỏ.
static uint512_t u512_from_u64(uint64_t v)
{
    uint512_t r; // constructor đã khởi tạo toàn 0
    r.digits[0] = v;
    return r;
}

// Khởi tạo PRNG: seed ngẫu nhiên từ OS, hoặc seed cố định khi debug.
static void init_prng(prng_ctx *ctx, bool fixed_seed)
{
    uint8_t seed[32];
    memset(seed, 0, sizeof(seed));

    if (fixed_seed)
    {
        memcpy(seed, "seed123", sizeof("seed123"));
    }
    else
    {
        randombytes(seed, sizeof(seed)); // getrandom() của Linux/macOS
    }
    prng_seed(ctx, seed, sizeof(seed));
}

// ---------------------------------------------------------------------------
// main: logic trao đổi khóa Diffie-Hellman
// ---------------------------------------------------------------------------
int main(int argc, char **argv)
{
    size_t bit_size = 512; // tối thiểu 512 bit theo đề
    bool fixed_seed = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--fixed") == 0)
        {
            fixed_seed = true;
        }
        else
        {
            long v = atol(argv[i]);
            if (v >= 512)
            {
                bit_size = (size_t)v;
            }
            else
            {
                fprintf(stderr, "Kich thuoc bit khong hop le (toi thieu 512): %s\n", argv[i]);
                return 1;
            }
        }
    }

    prng_ctx ctx;
    init_prng(&ctx, fixed_seed);

    // 1. Sinh số nguyên tố an toàn p và phần tử sinh g
    printf("Dang sinh so nguyen to an toan %zu bit (co the mat vai giay)...\n", bit_size);
    uint512_t p = generate_safe_prime(&ctx, bit_size);

    // Chọn g = 4 (= 2^2). Với p = 2q + 1 là safe prime, 4 luôn là số chính phương
    // và có bậc đúng bằng q (nguyên tố, rất lớn) trong Z_p^*. Nhờ đó tránh được
    // các nhóm con nhỏ (bậc 2) và không phụ thuộc p mod 8 như khi chọn g = 2.
    uint512_t g = u512_from_u64(4);

    // 2. Sinh khóa riêng của Alice và Bob trong [2, p-2]
    uint512_t a = generate_private_key(&ctx, p); // khóa riêng Alice
    uint512_t b = generate_private_key(&ctx, p); // khóa riêng Bob

    // 3. Tính giá trị công khai
    uint512_t A = modular_exponentiation(g, a, p); // Alice: A = g^a mod p
    uint512_t B = modular_exponentiation(g, b, p); // Bob:   B = g^b mod p

    // 4. Trao đổi A, B rồi tính bí mật chung
    uint512_t alice_shared_secret = modular_exponentiation(B, a, p); // s = B^a mod p
    uint512_t bob_shared_secret = modular_exponentiation(A, b, p);   // s = A^b mod p

    // 5. Hiển thị kết quả và kiểm tra
    printf("\n===== Tham so cong khai =====\n");
    print_u512("p = ", p);
    print_u512("g = ", g);

    printf("\n===== Alice =====\n");
    print_u512("Khoa rieng a = ", a);
    print_u512("Gia tri cong khai A = ", A);

    printf("\n===== Bob =====\n");
    print_u512("Khoa rieng b = ", b);
    print_u512("Gia tri cong khai B = ", B);

    printf("\n===== Bi mat chung =====\n");
    print_u512("Alice nhan duoc: ", alice_shared_secret);
    print_u512("Bob nhan duoc:   ", bob_shared_secret);

    bool ok = (alice_shared_secret == bob_shared_secret);
    printf("\nQua trinh tinh toan dung khong? %s\n", ok ? "DUNG (trung khop)" : "SAI (khong trung khop)");

    return ok ? 0 : 1;
}

// #include "prng.h"
// #include <inttypes.h>
// #include <stdio.h>
// #include <string.h>

// int main()
// {
//     uint8_t seed[32];

//     // you can use a fixed seed for this lab and for learning purposes.
//     memcpy(seed, "seed123", sizeof("seed123"));

//     prng_ctx ctx;
//     prng_seed(&ctx, seed, sizeof(seed));

//     uint16_t r1 = prng_next_u16(&ctx);
//     uint32_t r2 = prng_next_u32(&ctx);
//     uint64_t r3 = prng_next_u64(&ctx);

//     printf("Random 16-bit number: %u\n", r1);
//     printf("Random 32-bit number: %u\n", r2);
//     printf("Random 64-bit number: %" PRIu64 "\n", r3);
//     return 0;
// }
