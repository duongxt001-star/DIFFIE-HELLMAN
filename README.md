## Lab01: Số nguyên tố & Trao đổi khóa Diffie-Hellman

Repo cung cấp thư viện sinh số giả ngẫu nhiên (PRNG) dùng SHAKE128, một XOF
(Extendable-Output Function).

```
make run-prng
```

```
./bin/prng <số bit nguyên tố> <kiểu random seed>
```
Có 2 kiểu random seed: ngẫu nhiên và cố định

vd:muốn tạo số nguyên tố seed cố định với 512 bit nguyên tố

```
./bin/prng 512 --fixed

```

vd:muốn tạo số nguyên tố seed ngẫu nhiên với 512 bit nguyên tố

```
./bin/prng 512

```
Dùng các hàm trong thư viện để tự xây dựng số lớn, 512 bit, 1024 bit, etc. (uint512_t, uint1024_t uint2048_t).