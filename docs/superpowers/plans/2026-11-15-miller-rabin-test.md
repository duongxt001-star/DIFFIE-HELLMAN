# Miller-Rabin Primality Test Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Triển khai thuật toán kiểm tra số nguyên tố Miller-Rabin và hàm sinh số ngẫu nhiên trong đoạn xác định cho số lớn 512-bit, được kiểm thử tự động với các số nguyên tố, hợp số, số Carmichael và số nguyên tố lớn.

**Architecture:** Tích hợp trực tiếp các hàm `rand_range_u512` và `miller_rabin_test` vào `dh_sample.h` và `dh_sample.cpp` trên nhánh Git độc lập `feature/miller-rabin-test`. Xây dựng bộ kiểm thử riêng trong `test_miller_rabin.cpp` sử dụng CSPRNG SHAKE128 (`prng.h`) và số học mô-đun tối ưu (`u512.h`, `dh_sample.h`).

**Tech Stack:** C++14, GCC 16, Clang-Format (2 spaces), Make, POSIX.

**Spec:** `docs/superpowers/specs/2026-11-15-miller-rabin-design.md`

## Global Constraints

- Ngôn ngữ: C++14 trở xuống (`-std=c++14`).
- Cờ biên dịch: `-O3 -Wall -Wextra -Wpedantic`.
- Định dạng mã nguồn: Clang-Format 2 spaces (đồng bộ với `dh_sample.cpp`).
- Bộ sinh số ngẫu nhiên: Bắt buộc dùng `prng_ctx` (SHAKE128 CSPRNG từ `prng.h`), không dùng `rand()` hay `<random>`.
- Quản lý mã nguồn: Phát triển trên nhánh `feature/miller-rabin-test`, commit nguyên tử cho từng task.

## Review Focus

- Điều kiện biên $n \in \{0, 1\}$ (phải trả về `false`), $n \in \{2, 3\}$ (phải trả về `true`), $n$ chẵn $> 2$ (phải trả về `false` ngay lập tức).
- Số Carmichael: 561, 1105, 1729, 2821 (hợp số đặc biệt đánh lừa được kiểm tra Fermat nhưng Miller-Rabin phải phát hiện ra là hợp số).
- Trường hợp $s = 1$ (khi $(n-1)/2$ là số lẻ, vòng lặp bình phương $r = 1 \dots s-1$ không được lặp quá giới hạn).
- Phát hiện căn bậc hai không tầm thường của 1: nếu trong quá trình bình phương $x \leftarrow x^2 \pmod n$ mà $x$ trở thành 1 trước khi đạt $n-1$, thuật toán phải kết luận ngay là hợp số.
- Phạm vi cơ số ngẫu nhiên $a$: phải đảm bảo luôn nằm chặt chẽ trong $[2, n-2]$.

---

### Task 1: Thiết lập nhánh Git và khung kiểm thử ban đầu

**Files:**
- Create: `test_miller_rabin.cpp`
- Create: `Makefile.local`

**Interfaces:**
- Consumes: `u512.h`, `dh_sample.h`, `prng.h`
- Produces: Nhánh `feature/miller-rabin-test` và target biên dịch kiểm thử `make -f Makefile.local test-mr`

- [ ] **Step 1: Khởi tạo nhánh Git `feature/miller-rabin-test`**

```bash
git checkout -b feature/miller-rabin-test
```

- [ ] **Step 2: Tạo `Makefile.local` để biên dịch runner kiểm thử**

Viết `Makefile.local` liên kết `u512.o`, `dh_sample.o`, `prng.o`, `fips202.o` cùng `test_miller_rabin.cpp`:
```makefile
CXX ?= g++
CC ?= gcc
CXXFLAGS ?= -std=c++14 -O3 -Wall -Wextra -Wpedantic -I.
CFLAGS ?= -O3 -Wall -Wextra -Wpedantic -I.

BUILD := bin/build
BIN := bin

TEST_MR_BIN := $(BIN)/test_miller_rabin

.PHONY: all clean run-test-mr

all: $(TEST_MR_BIN)

$(BUILD)/fips202.o: fips202.c fips202.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/prng.o: prng.cpp prng.h fips202.h
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/u512.o: u512.cpp u512.h
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/dh_sample.o: dh_sample.cpp dh_sample.h u512.h
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/test_miller_rabin.o: test_miller_rabin.cpp dh_sample.h u512.h prng.h
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_MR_BIN): $(BUILD)/test_miller_rabin.o $(BUILD)/dh_sample.o $(BUILD)/u512.o $(BUILD)/prng.o $(BUILD)/fips202.o
	mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

run-test-mr: $(TEST_MR_BIN)
	./$(TEST_MR_BIN)

clean:
	rm -rf $(BUILD)/test_miller_rabin.o $(TEST_MR_BIN)
```

- [ ] **Step 3: Tạo khung file `test_miller_rabin.cpp`**

```cpp
#include "dh_sample.h"
#include "prng.h"
#include "u512.h"
#include <cassert>
#include <cstdio>
#include <cstring>

static uint512_t u512_from_u64(uint64_t v) {
  uint512_t r;
  r.digits[0] = v;
  return r;
}

int main() {
  printf("Khung kiem thu Miller-Rabin da san sang.\n");
  return 0;
}
```

- [ ] **Step 4: Biên dịch và chạy thử kiểm tra khung làm việc**

Run: `make -f Makefile.local run-test-mr`
Expected: Biên dịch thành công và in "Khung kiem thu Miller-Rabin da san sang."

- [ ] **Step 5: Commit scaffolding**

```bash
git add Makefile.local test_miller_rabin.cpp
git commit -m "chore: setup test scaffold and Makefile.local for Miller-Rabin tests"
```

---

### Task 2: Cài đặt hàm sinh số ngẫu nhiên `rand_range_u512`

**Files:**
- Modify: `dh_sample.h`
- Modify: `dh_sample.cpp`
- Modify: `test_miller_rabin.cpp`

**Interfaces:**
- Consumes: `prng_next_u64(ctx)`, `mod_u512`, `operator+`, `operator-`
- Produces: `uint512_t rand_range_u512(prng_ctx *ctx, const uint512_t &min, const uint512_t &max)`

- [ ] **Step 1: Viết test kiểm tra `rand_range_u512` trong `test_miller_rabin.cpp`**

Thêm hàm `test_rand_range()` kiểm tra:
1. Giá trị sinh ra luôn $\ge min$ và $\le max$.
2. Với $min = 2, max = 10$, sinh 100 lần, tất cả các kết quả đều nằm trong $[2, 10]$.
3. Với $min = max$, trả về đúng $min$.

```cpp
void test_rand_range(prng_ctx *ctx) {
  uint512_t min = u512_from_u64(2);
  uint512_t max = u512_from_u64(10);
  for (int i = 0; i < 100; i++) {
    uint512_t val = rand_range_u512(ctx, min, max);
    assert(val >= min);
    assert(val <= max);
  }
  uint512_t same = rand_range_u512(ctx, min, min);
  assert(same == min);
  printf("[PASS] test_rand_range\n");
}
```

- [ ] **Step 2: Chạy kiểm thử để xác nhận lỗi biên dịch (do hàm chưa được khai báo)**

Run: `make -f Makefile.local run-test-mr`
Expected: Biên dịch lỗi với thông báo `rand_range_u512 was not declared in this scope`.

- [ ] **Step 3: Khai báo `rand_range_u512` trong `dh_sample.h`**

```cpp
uint512_t rand_range_u512(prng_ctx *ctx, const uint512_t &min,
                          const uint512_t &max);
```

- [ ] **Step 4: Cài đặt `rand_range_u512` trong `dh_sample.cpp`**

Cài đặt theo đúng thiết kế Clang-Format 2 spaces:
```cpp
uint512_t rand_range_u512(prng_ctx *ctx, const uint512_t &min,
                          const uint512_t &max) {
  if (min >= max)
    return min;

  uint512_t one;
  one.digits[0] = 1;
  uint512_t range = (max - min) + one;

  uint512_t r;
  for (size_t i = 0; i < 8; i++) {
    r.digits[i] = prng_next_u64(ctx);
  }

  uint512_t offset = mod_u512(r, range);
  return min + offset;
}
```

- [ ] **Step 5: Biên dịch và chạy kiểm thử**

Run: `make -f Makefile.local run-test-mr`
Expected: PASS `[PASS] test_rand_range`

- [ ] **Step 6: Commit**

```bash
git add dh_sample.h dh_sample.cpp test_miller_rabin.cpp
git commit -m "feat: implement rand_range_u512 for uniform random sampling"
```

---

### Task 3: Cài đặt thuật toán Miller-Rabin `miller_rabin_test`

**Files:**
- Modify: `dh_sample.h`
- Modify: `dh_sample.cpp`
- Modify: `test_miller_rabin.cpp`

**Interfaces:**
- Consumes: `rand_range_u512`, `exp_mod_u512`, `mult_mod_u512`, `get_bit`, `operator>>`
- Produces: `bool miller_rabin_test(prng_ctx *ctx, const uint512_t &n, size_t rounds = 20)`

- [ ] **Step 1: Viết test kiểm tra cơ bản cho `miller_rabin_test` trong `test_miller_rabin.cpp`**

```cpp
void test_basic_miller_rabin(prng_ctx *ctx) {
  assert(!miller_rabin_test(ctx, u512_from_u64(0)));
  assert(!miller_rabin_test(ctx, u512_from_u64(1)));
  assert(miller_rabin_test(ctx, u512_from_u64(2)));
  assert(miller_rabin_test(ctx, u512_from_u64(3)));
  assert(!miller_rabin_test(ctx, u512_from_u64(4)));
  assert(miller_rabin_test(ctx, u512_from_u64(5)));
  assert(!miller_rabin_test(ctx, u512_from_u64(6)));
  assert(miller_rabin_test(ctx, u512_from_u64(7)));
  printf("[PASS] test_basic_miller_rabin\n");
}
```

- [ ] **Step 2: Chạy kiểm thử để xác nhận lỗi biên dịch**

Run: `make -f Makefile.local run-test-mr`
Expected: Biên dịch lỗi với thông báo `miller_rabin_test was not declared`.

- [ ] **Step 3: Khai báo `miller_rabin_test` trong `dh_sample.h`**

```cpp
bool miller_rabin_test(prng_ctx *ctx, const uint512_t &n, size_t rounds = 20);
```

- [ ] **Step 4: Cài đặt `miller_rabin_test` trong `dh_sample.cpp`**

Cài đặt chi tiết với đầy đủ các bước kiểm tra biên, phân tích $n-1 = 2^s \cdot d$, và vòng lặp witness kiểm tra căn bậc hai không tầm thường:
```cpp
bool miller_rabin_test(prng_ctx *ctx, const uint512_t &n, size_t rounds) {
  uint512_t two = u512_from_u64(2);
  uint512_t three = u512_from_u64(3);

  if (n < two)
    return false;
  if (n == two || n == three)
    return true;
  if (!get_bit(n, 0))
    return false; // Số chẵn > 2

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
        // Phat hien can bac hai khong tam thuong cua 1
        return false;
      }
    }

    if (composite)
      return false;
  }

  return true;
}
```

- [ ] **Step 5: Biên dịch và chạy kiểm thử**

Run: `make -f Makefile.local run-test-mr`
Expected: PASS `[PASS] test_basic_miller_rabin`

- [ ] **Step 6: Commit**

```bash
git add dh_sample.h dh_sample.cpp test_miller_rabin.cpp
git commit -m "feat: implement miller_rabin_test algorithm with witness loop"
```

---

### Task 4: Xây dựng bộ kiểm thử hoàn chỉnh (Primes, Composites, Carmichael, Large Primes)

**Files:**
- Modify: `test_miller_rabin.cpp`

**Interfaces:**
- Consumes: `miller_rabin_test`, `u512_from_u64`
- Produces: Test suite tự động in báo cáo chi tiết cho toàn bộ các nhóm test

- [ ] **Step 1: Mở rộng `test_miller_rabin.cpp` với đầy đủ 4 nhóm kiểm thử**

Viết các hàm kiểm thử chi tiết:
1. `test_small_primes`: 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 97, 541, 1000000007.
2. `test_small_composites`: 4, 6, 8, 9, 15, 21, 25, 27, 33, 35, 100, 1000000005.
3. `test_carmichael_numbers`: 561, 1105, 1729, 2821 (khẳng định Miller-Rabin phát hiện chính xác là hợp số).
4. `test_large_known_primes`:
   - $M_{31} = 2^{31} - 1 = 2147483647$
   - $M_{61} = 2^{61} - 1 = 2305843009213693951$
   - Số nguyên tố 128-bit chuẩn: `0xffffffffffffffffffffffffffffff61` ($2^{128} - 159$)
   - Số nguyên tố 512-bit chuẩn (RFC 3526 MODP Group 512-bit prime hoặc test vector tương đương)
5. `test_timing_benchmark`: Đo thời gian thực thi trung bình cho 1 lần kiểm tra Miller-Rabin 512-bit ($k = 20$).

- [ ] **Step 2: Biên dịch và thực thi toàn bộ kịch bản kiểm thử**

Run: `make -f Makefile.local run-test-mr`
Expected: Toàn bộ 5 nhóm kiểm thử đều in `[PASS]`, thời gian chạy 512-bit hiển thị rõ ràng (dưới 200 ms).

- [ ] **Step 3: Commit**

```bash
git add test_miller_rabin.cpp
git commit -m "test: add comprehensive test suite for Miller-Rabin including Carmichael and large primes"
```

---

### Task 5: Thu thập số liệu và soạn thảo báo cáo Markdown Phần 3

**Files:**
- Create: `docs/report_part3_miller_rabin.md`

**Interfaces:**
- Consumes: Kết quả chạy thực tế từ Task 4
- Produces: Bản thảo báo cáo markdown chi tiết phục vụ Mục 2.5 của bài tập lớn

- [ ] **Step 1: Chạy test và ghi nhận số liệu đầu ra**

Run: `./bin/test_miller_rabin > test_output.log && cat test_output.log`
Expected: Thu thập đầy đủ bảng log kết quả kiểm thử.

- [ ] **Step 2: Soạn thảo `docs/report_part3_miller_rabin.md`**

Trình bày cấu trúc khoa học:
1. Cơ sở toán học của Miller-Rabin (Định lý Fermat nhỏ, Căn bậc hai không tầm thường của 1).
2. Phân tích độ phức tạp thời gian và xác suất sai số $4^{-k}$.
3. Phân tích trường hợp đặc biệt: Tại sao kiểm tra Fermat thất bại trước số Carmichael (561, 1105) trong khi Miller-Rabin phát hiện thành công 100%.
4. Bảng số liệu kiểm thử thực tế và thời gian đo đạc trên số 64-bit, 128-bit, 512-bit.
5. Mã nguồn cài đặt và phân tích ưu điểm tối ưu tốc độ.

- [ ] **Step 3: Commit báo cáo markdown**

```bash
git add docs/report_part3_miller_rabin.md
git commit -m "docs: generate comprehensive markdown report for Miller-Rabin test (Task 3)"
```
