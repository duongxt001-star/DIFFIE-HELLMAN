# Design Document: Miller-Rabin Primality Test (Phần 3)

**Tài liệu thiết kế kỹ thuật - Triển khai thuật toán kiểm tra số nguyên tố Miller-Rabin**  
**Dự án**: Trao đổi khóa Diffie-Hellman 512-bit (Lab01 - Nhập môn Mã hóa Mật mã)  
**Ngày**: 15/11/2026  
**Nhánh Git**: `feature/miller-rabin-test`

---

## 1. Intent and Why (Mục đích và Ý nghĩa)
Triển khai thuật toán kiểm tra tính nguyên tố Miller-Rabin (`miller_rabin_test`) và hàm sinh số ngẫu nhiên trong đoạn xác định (`rand_range_u512`) trên kiểu số lớn `uint512_t`. 

Đây là mắt xích cốt lõi của Phần 3 trong đồ án nhóm, làm tiền đề trực tiếp cho Phần 4 (sinh số nguyên tố an toàn $p = 2q + 1$ 512-bit) và cung cấp số liệu thực nghiệm để viết Mục 2.5 trong báo cáo (đánh giá Miller-Rabin, xử lý số Carmichael, đo đạc thời gian và xác suất sai số).

---

## 2. Goals, Non-Goals, and Anti-Goals

### Goals
1. Cài đặt hàm `rand_range_u512(prng_ctx *ctx, const uint512_t &min, const uint512_t &max)` dùng bộ sinh số giả ngẫu nhiên mật mã CSPRNG SHAKE128 (`prng.h`).
2. Cài đặt thuật toán Miller-Rabin `miller_rabin_test(prng_ctx *ctx, const uint512_t &n, size_t rounds = 20)`:
   - Xử lý các trường hợp biên đặc biệt: $n < 2$ (hợp số), $n = 2, 3$ (nguyên tố), $n$ chẵn $> 2$ (hợp số).
   - Phân tích $n - 1 = 2^s \cdot d$ ($d$ lẻ) bằng phép dịch phải `>> 1` từ module `u512`.
   - Lặp $k$ vòng thử (witness loop): chọn cơ số ngẫu nhiên $a \in [2, n - 2]$, tính $x = a^d \pmod n$ (`exp_mod_u512`), và lặp bình phương $x \leftarrow x^2 \pmod n$ (`mult_mod_u512`).
   - Xác suất sai số tối đa $4^{-k} \le 4^{-20} \approx 9 \times 10^{-13}$.
3. Tạo file kiểm thử độc lập `test_miller_rabin.cpp` kiểm tra:
   - Các số nguyên tố nhỏ đã biết: 2, 3, 5, 7, 13, 17, 97, 1000000007.
   - Các hợp số thông thường: 4, 6, 9, 15, 21, 100, 1000000005.
   - Các số Carmichael: 561 ($3 \times 11 \times 17$) và 1105 ($5 \times 13 \times 17$) để chứng minh khả năng phát hiện hợp số vượt trội so với Fermat test.
   - Các số nguyên tố lớn đã biết (64-bit, 128-bit, 256-bit, 512-bit chuẩn).
4. Tuân thủ tiêu chuẩn Clang-Format 2 spaces, tương thích C++14.
5. Thực hiện trên nhánh `feature/miller-rabin-test` với các commit nhỏ, an toàn và có kiểm chứng.

### Non-Goals
- Chưa triển khai thuật toán sinh số nguyên tố an toàn $p = 2q + 1$ (thuộc phạm vi Phần 4).
- Chưa chỉnh sửa luồng chính `main_prng.cpp` (thuộc phạm vi Phần 5).

### Anti-Goals
- Không dùng hàm ngẫu nhiên tiêu chuẩn của C `rand()` hay C++11 `<random>` (phải dùng `prng_ctx` theo đúng yêu cầu đề bài).
- Không sửa đổi API cơ sở của `u512` làm ảnh hưởng đến các thành viên khác.

---

## 3. Constraints (Ràng buộc kỹ thuật)
- Chuẩn ngôn ngữ: C++14 trở xuống (`-std=c++14`).
- Biên dịch không lỗi với: `-O3 -Wall -Wextra -Wpedantic`.
- Định dạng mã nguồn: Clang-Format 2 spaces (đồng bộ với `dh_sample.cpp`).

---

## 4. Approaches (Các hướng tiếp cận đã xem xét)

### Phương án A: Tích hợp trực tiếp vào `dh_sample.h` và `dh_sample.cpp` (**Được chọn**)
- **Mô tả**: Khai báo và cài đặt `rand_range_u512` cùng `miller_rabin_test` trực tiếp trong cặp file `dh_sample`.
- **Ưu điểm**: Khớp hoàn toàn với cấu trúc khung mẫu mà đề bài [`Lab01.pdf`](file:///home/quang/Workspace/24120127/NM_MaHoa_MatMa/project_1_DIFFIE_HELLMAN/docs/Lab01.pdf) và file `main_prng.cpp` quy định; không làm phát sinh file mới trong bài nộp chính của nhóm; không cần thay đổi quy tắc link `Makefile` nộp bài.
- **Đánh đổi**: File `dh_sample.cpp` tăng thêm dung lượng code.

### Phương án B: Tách module riêng `prime.h` / `prime.cpp` (**Bị loại**)
- **Mô tả**: Tạo file riêng cho toàn bộ các hàm kiểm tra và sinh số nguyên tố.
- **Ưu điểm**: Độc lập tuyệt đối về mặt file.
- **Lý do loại**: Phải sửa đổi `Makefile` chính thức của nhóm và có nguy cơ gây lỗi liên kết (link error) khi ráp nối chung.

---

## 5. Technical Specification (Quy cách kỹ thuật chi tiết)

### 5.1. Khai báo API trong `dh_sample.h`
```cpp
// Sinh số ngẫu nhiên 512-bit nằm trong đoạn [min, max]
uint512_t rand_range_u512(prng_ctx *ctx, const uint512_t &min, const uint512_t &max);

// Kiểm tra số nguyên tố Miller-Rabin với số vòng lặp k (mặc định rounds = 20)
bool miller_rabin_test(prng_ctx *ctx, const uint512_t &n, size_t rounds = 20);
```

### 5.2. Thuật toán `rand_range_u512(ctx, min, max)`
1. Nếu `min >= max`, trả về `min`.
2. Tính khoảng cách `range = max - min + 1`.
3. Tạo số ngẫu nhiên 512-bit `r`:
   - Lặp 8 lần `r.digits[i] = prng_next_u64(ctx)`.
4. Trả về `min + mod_u512(r, range)`.

### 5.3. Thuật toán `miller_rabin_test(ctx, n, rounds)`
1. Kiểm tra trường hợp biên:
   - Nếu $n < 2$: trả về `false`.
   - Nếu $n = 2$ hoặc $n = 3$: trả về `true`.
   - Nếu `get_bit(n, 0) == 0` (số chẵn $> 2$): trả về `false`.
2. Phân tách $n - 1 = 2^s \cdot d$ ($d$ lẻ):
   - $d = n - 1$.
   - $s = 0$.
   - Trong khi `get_bit(d, 0) == 0`:
     - $d = d \gg 1$.
     - $s \leftarrow s + 1$.
3. Vòng lặp kiểm tra `rounds` lần:
   - Chọn $a = \text{rand\_range\_u512}(ctx, 2, n - 2)$.
   - Tính $x = \text{exp\_mod\_u512}(a, d, n)$.
   - Nếu $x = 1$ hoặc $x = n - 1$: tiếp tục vòng thử tiếp theo.
   - Lặp $r = 1 \dots s - 1$:
     - $x \leftarrow \text{mult\_mod\_u512}(x, x, n)$.
     - Nếu $x = n - 1$: đạt yêu cầu, break sang vòng thử $a$ tiếp theo.
     - Nếu $x = 1$: chắc chắn là hợp số (vì căn bậc hai không tầm thường của 1), trả về `false`.
   - Nếu duyệt hết $r$ mà không tìm thấy $n - 1$: trả về `false` (hợp số).
4. Trả về `true` (xác suất là số nguyên tố $\ge 1 - 4^{-\text{rounds}}$).

---

## 6. Verification and Testing Plan (Kế hoạch kiểm thử)
Xây dựng file `test_miller_rabin.cpp` chạy tự động và in kết quả chi tiết:
- **Test Group 1: Known Small Primes**: Các số 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 97, 1000000007 $\implies$ mong đợi: `PASS (Prime)`.
- **Test Group 2: Known Small Composites**: Các số 0, 1, 4, 6, 8, 9, 15, 21, 25, 27, 33, 35, 100, 1000000005 $\implies$ mong đợi: `PASS (Composite)`.
- **Test Group 3: Carmichael Numbers**: Các số 561, 1105, 1729, 2821 $\implies$ mong đợi: `PASS (Composite)`.
- **Test Group 4: Known Large Primes**: 
  - $2^{31} - 1 = 2147483647$ (Mersenne prime 31-bit).
  - $2^{61} - 1 = 2305843009213693951$ (Mersenne prime 61-bit).
  - 128-bit RFC prime: $2^{127} - 1$.
  - 512-bit safe prime / prime chuẩn $\implies$ mong đợi: `PASS (Prime)`.
- **Test Group 5: Execution Speed**: Đo đạc thời gian chạy trung bình cho 1 test Miller-Rabin trên số 512-bit.

---

## 7. What's Left to the Builder (Những chi tiết dành cho người lập trình)
- Thiết lập seed cho `prng_ctx` trong `test_miller_rabin.cpp`.
- Tạo target `run-test-mr` trong `Makefile.local` để chạy test tiện lợi.
