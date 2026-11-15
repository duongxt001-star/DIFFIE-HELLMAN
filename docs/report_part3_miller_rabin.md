# BÁO CÁO PHẦN 3: THUẬT TOÁN KIỂM TRA SỐ NGUYÊN TỐ MILLER-RABIN

**Môn học:** Nhập môn Mã hóa Mật mã (Lab 01)  
**Phần đảm nhiệm:** Phần 3 — Cài đặt Miller-Rabin và kiểm thử với các số nguyên tố/hợp số đã biết  
**Họ và tên:** Nguyễn Ngọc Quang (MSSV: 24120127)  
**Ngày hoàn thành:** 15/11/2026  

---

## 1. Cơ sở toán học của thuật toán Miller-Rabin

Thuật toán Miller-Rabin là thuật toán kiểm tra tính nguyên tố xác suất (probabilistic primality test) hiệu quả nhất và được sử dụng rộng rãi nhất trong mật mã học hiện đại (RSA, Diffie-Hellman, DSA). Thuật toán dựa trên hai nguyên lý số học nền tảng:

### 1.1. Định lý Fermat nhỏ (Fermat's Little Theorem)
Nếu $p$ là số nguyên tố và $a$ là số nguyên không chia hết cho $p$, thì:
$$a^{p-1} \equiv 1 \pmod p$$

Tuy nhiên, nếu chỉ kiểm tra $a^{n-1} \equiv 1 \pmod n$ (phương pháp Fermat test), ta sẽ gặp phải các **hợp số giả nguyên tố (Fermat pseudoprimes)**, đặc biệt là các **số Carmichael** (như $561, 1105, 1729 \dots$). Với mọi $a$ nguyên tố cùng nhau với số Carmichael $n$, ta đều có $a^{n-1} \equiv 1 \pmod n$, khiến phương pháp Fermat bị lừa 100%.

### 1.2. Tính chất căn bậc hai không tầm thường của 1 (Non-trivial Square Roots of 1)
Trong trường hữu hạn $\mathbb{Z}_p$ với $p$ nguyên tố, phương trình $x^2 \equiv 1 \pmod p$ chỉ có đúng hai nghiệm tầm thường là:
$$x \equiv 1 \pmod p \quad \text{hoặc} \quad x \equiv -1 \equiv p - 1 \pmod p$$
Nếu tồn tại một số $x \not\equiv \pm 1 \pmod n$ sao cho $x^2 \equiv 1 \pmod n$, thì $n$ **chắc chắn là hợp số**.

### 1.3. Khai triển lũy thừa của 2
Với số lẻ $n > 2$, số $n - 1$ là số chẵn. Ta luôn có thể phân tích duy nhất dưới dạng:
$$n - 1 = 2^s \cdot d \quad (d \text{ lẻ}, s \ge 1)$$

Xét dãy lũy thừa liên tiếp với cơ số ngẫu nhiên $a \in [2, n - 2]$:
$$a^d, \quad a^{2d}, \quad a^{4d}, \quad \dots, \quad a^{2^{s-1}d}, \quad a^{2^s d} = a^{n-1} \pmod n$$

Nếu $n$ là số nguyên tố, thì:
- Hoặc phần tử đầu tiên $a^d \equiv 1 \pmod n$ (hoặc $a^d \equiv n - 1 \pmod n$).
- Hoặc tồn tại một chỉ số $r \in \{0, 1, \dots, s - 1\}$ sao cho $a^{2^r d} \equiv n - 1 \pmod n$ (bình phương tiếp theo sẽ đạt 1).

Nếu không thỏa mãn một trong hai điều kiện trên, hoặc xuất hiện $x^2 \equiv 1$ khi $x \not\equiv n - 1$, thì $a$ được gọi là **nhân chứng chứng minh $n$ là hợp số (witness for compositeness)**.

---

## 2. Thiết kế và Triển khai Thuật toán

### 2.1. Phân chia module và Nguyên mẫu hàm
Mã nguồn được tích hợp trực tiếp vào thư viện mật mã `dh_sample.h` và `dh_sample.cpp`, tuân thủ chuẩn định dạng Clang-Format 2 spaces:

```cpp
// Sinh số ngẫu nhiên 512-bit đều trong đoạn [min, max] dùng CSPRNG SHAKE128
uint512_t rand_range_u512(prng_ctx *ctx, const uint512_t &min, const uint512_t &max);

// Kiểm tra số nguyên tố Miller-Rabin với tham số k vòng thử
bool miller_rabin_test(prng_ctx *ctx, const uint512_t &n, size_t rounds = 20);
```

### 2.2. Chi tiết các bước thực hiện trong mã nguồn
1. **Lọc trường hợp biên (Edge Cases)**:
   - Nếu $n < 2$: trả về `false`.
   - Nếu $n = 2$ hoặc $n = 3$: trả về `true`.
   - Nếu $n > 2$ và chẵn (`!get_bit(n, 0)`): trả về `false` ngay lập tức mà không cần tốn chi phí tính toán lũy thừa.
2. **Phân tích $n - 1 = 2^s \cdot d$**:
   - Khởi tạo $d = n - 1, s = 0$.
   - Dùng toán tử dịch phải bit `operator>>` của `u512` kết hợp `!get_bit(d, 0)` để chia $d$ cho 2 cho đến khi $d$ trở thành số lẻ.
3. **Vòng lặp nhân chứng (Witness Loop - lặp $k = 20$ lần)**:
   - Chọn ngẫu nhiên cơ số $a \in [2, n - 2]$ bằng hàm `rand_range_u512(ctx, 2, n - 2)`.
   - Tính $x = a^d \pmod n$ bằng hàm `exp_mod_u512(a, d, n)`.
   - Nếu $x = 1$ hoặc $x = n - 1$: đạt điều kiện, chuyển sang vòng thử $a$ tiếp theo.
   - Ngược lại, thực hiện lặp bình phương $s - 1$ lần: $x \leftarrow (x \times x) \pmod n$ (dùng `mult_mod_u512` để tối ưu thời gian).
     - Nếu $x = n - 1$: đạt điều kiện, thoát vòng lặp con để sang cơ số $a$ tiếp theo.
     - Nếu $x = 1$: phát hiện căn bậc hai không tầm thường của 1, kết luận ngay $n$ là **HỢP SỐ** (`false`).
   - Nếu duyệt hết $s - 1$ lần bình phương mà $x$ không bao giờ bằng $n - 1$: kết luận $n$ là **HỢP SỐ** (`false`).
4. **Kết luận**: Nếu vượt qua cả $k$ vòng thử độc lập, $n$ được xác nhận là **SỐ NGUYÊN TỐ XÁC SUẤT CAO** (`true`).

---

## 3. Phân tích Xác suất Sai số và Độ phức tạp

### 3.1. Xác suất sai số
Với mỗi hợp số lẻ $n$, số lượng cơ số $a \in [2, n-2]$ là nhân chứng hợp số chiếm ít nhất $\frac{3}{4}$ tổng số cơ số (Định lý Monier-Rabin). Do đó, xác suất để một hợp số vượt qua ngẫu nhiên 1 vòng thử Miller-Rabin là không quá $\frac{1}{4}$.

Sau $k$ vòng thử độc lập với các cơ số ngẫu nhiên được sinh từ CSPRNG SHAKE128:
$$P(\text{Hợp số bị nhận nhầm là số nguyên tố}) \le \left(\frac{1}{4}\right)^k = 4^{-k}$$

Với $k = 20$:
$$P(\text{Sai số}) \le 4^{-20} = 2^{-40} \approx 9.09 \times 10^{-13}$$
Mức xác suất lỗi này thấp hơn nhiều so với xác suất xảy ra lỗi phần cứng (hardware bit-flip do bức xạ vũ trụ), hoàn toàn đáp ứng các tiêu chuẩn an ninh mật mã quốc tế (NIST FIPS 186-4).

### 3.2. Độ phức tạp thời gian
- Mỗi vòng thử bao gồm: 1 phép lũy thừa mô-đun $a^d \pmod n$ và tối đa $s - 1$ phép nhân mô-đun $x^2 \pmod n$.
- Với thuật toán Double-and-Add hiện tại, một phép nhân mô-đun 512-bit có độ phức tạp $O(B)$, phép lũy thừa có độ phức tạp $O(B^2)$ (với $B = 512$).
- Tổng độ phức tạp của kiểm tra Miller-Rabin là $O(k \cdot B^2)$.

---

## 4. Kết quả Thực nghiệm và Kiểm chứng

Bộ kiểm thử toàn diện được thực thi tự động qua file `test_miller_rabin.cpp` với 6 nhóm kiểm tra:

### 4.1. Bảng kết quả kiểm thử thực tế

| Nhóm kiểm tra | Tập dữ liệu thử nghiệm | Kết quả thực tế | Đánh giá |
| :--- | :--- | :---: | :---: |
| **1. Hàm sinh ngẫu nhiên** | `rand_range_u512(ctx, 2, 10)` (100 lần) | $100\%$ nằm trong $[2, 10]$ | **PASS** |
| **2. Số nguyên tố nhỏ** | $2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 97, 541, 10007, 65537, 10^9+7$ | $16/16$ nhận diện là Prime | **PASS** |
| **3. Hợp số thông thường** | $0, 1, 4, 6, 8, 9, 15, 21, 25, 27, 33, 35, 100, 500, 10^9+5$ | $15/15$ nhận diện là Composite | **PASS** |
| **4. Số Carmichael** | $561, 1105, 1729, 2821, 6601$ | $5/5$ phát hiện là Composite | **PASS** |
| **5. Số nguyên tố lớn** | Mersenne $2^{31}-1$, Mersenne $2^{61}-1$, Prime 128-bit ($2^{128}-159$), Prime 256-bit (NIST P-256), Prime 512-bit ($2^{512}-569$) | $5/5$ nhận diện là Prime | **PASS** |
| **6. Hợp số lớn 512-bit** | $2^{512} - 571$ | Phát hiện là Composite | **PASS** |

### 4.2. Nghiên cứu trường hợp số Carmichael (Điểm nhấn học thuật)
Số Carmichael $n = 561 = 3 \times 11 \times 17$ là hợp số giả nguyên tố Fermat nhỏ nhất:
- Nếu dùng Fermat test: Với mọi cơ số $a$ thỏa $\gcd(a, 561) = 1$, ta luôn có $a^{560} \equiv 1 \pmod{561}$. Do đó Fermat test **thất bại hoàn toàn**.
- Với Miller-Rabin test: 
  Ta có $560 = 2^4 \times 35 \implies s = 4, d = 35$.
  Chọn ngẫu nhiên cơ số $a = 2$:
  $$2^{35} \equiv 263 \pmod{561}$$
  $$263^2 \equiv 166 \pmod{561}$$
  $$166^2 \equiv 67 \pmod{561}$$
  $$67^2 \equiv 1 \pmod{561}$$
  Dãy giá trị là: $263 \to 166 \to 67 \to 1$. Số 67 bình phương lên bằng $1$ nhưng $67 \not\equiv -1 \pmod{561}$. Miller-Rabin phát hiện 67 là căn bậc hai không tầm thường của 1 và lập tức bác bỏ tính nguyên tố của 561!

### 4.3. Đo đạc hiệu năng thực tế (Benchmark)
Thực hiện đo đạc trên số nguyên tố 512-bit ($2^{512} - 569$) với $k = 20$ vòng lặp:
- Tổng thời gian thực thi 5 lần kiểm tra: **2.251,00 ms**
- Thời gian trung bình cho 1 lần kiểm tra Miller-Rabin 512-bit đầy đủ ($k = 20$): **450,20 ms (~0,45 giây)**.

---

## 5. Đánh giá tính sẵn sàng cho Phần 4 (Sinh Safe Prime)
Với tốc độ trung bình **~450 ms cho một số nguyên tố 512-bit**, và **dưới 10 ms cho một hợp số** (vì hợp số thường bị phát hiện ngay ở cơ số đầu tiên $a$ mà không cần chạy đủ $k$ vòng), thuật toán Miller-Rabin đã hoàn toàn sẵn sàng để tích hợp vào Phần 4. 

Để tối ưu hóa hơn nữa cho Phần 4, ta sẽ kết hợp thêm kỹ thuật **sàng số nguyên tố nhỏ (trial division với 100 số nguyên tố đầu tiên)** trước khi gọi Miller-Rabin, giúp loại bỏ $>85\%$ ứng viên hợp số chỉ trong vài micro-giây.
