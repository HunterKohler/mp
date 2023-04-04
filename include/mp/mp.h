#ifndef MP_MP_H_
#define MP_MP_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <mp/config.h>
#include <mp/errc.h>

typedef MP_INT_TYPE mp_int;
typedef MP_UINT_TYPE mp_uint;
typedef MP_SIZE_TYPE mp_size;
typedef MP_SSIZE_TYPE mp_ssize;
typedef MP_FLOAT_TYPE mp_float;
typedef MP_BOOL_TYPE mp_bool;
typedef unsigned char mp_byte;

#define mp_false ((mp_bool)0)
#define mp_true ((mp_bool)1)

enum mp_endian {
    MP_ENDIAN_NATIVE = __BYTE_ORDER__,
    MP_ENDIAN_LITTLE = __ORDER_LITTLE_ENDIAN__,
    MP_ENDIAN_BIG = __ORDER_BIG_ENDIAN__
};

struct mp_to_string_result {
    enum mp_errc ec;
    char *ptr;
};

struct mp_from_string_result {
    enum mp_errc ec;
    const char *ptr;
    mp_size size;
};

mp_uint mp_add_uint(const mp_uint *ap, mp_size an, mp_uint b, mp_uint *rp);

mp_uint mp_add_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp);

mp_uint mp_add(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
               mp_uint *rp);

mp_uint mp_sub_uint(const mp_uint *ap, mp_size an, mp_uint b, mp_uint *rp);

mp_uint mp_sub_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp);

mp_uint mp_sub(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
               mp_uint *rp);

mp_uint mp_mul_uint(const mp_uint *ap, mp_size an, mp_uint b, mp_uint *rp);

mp_uint mp_mul(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
               mp_uint *rp);

mp_uint mp_div_uint(const mp_uint *np, mp_size nn, mp_uint d, mp_uint *qp);

void mp_div(const mp_uint *np, mp_uint nn, const mp_uint *dp, mp_size dn,
            mp_uint *qp, mp_uint *rp);

void mp_mod_uint(const mp_uint *np, mp_uint nn, mp_uint d, mp_uint *rp);

void mp_mod(const mp_uint *np, mp_uint nn, const mp_uint *dp, mp_size dn,
            mp_uint *rp);

mp_uint mp_left_shift(const mp_uint *ap, mp_size an, mp_size bits, mp_uint *rp);

mp_uint mp_right_shift(
    const mp_uint *ap, mp_size an, mp_size bits, mp_uint *rp);

void mp_bit_and_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp);

void mp_bit_and(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
                mp_uint *rp);

void mp_bit_or_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp);

void mp_bit_or(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
               mp_uint *rp);

void mp_bit_xor_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp);

void mp_bit_xor(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
                mp_uint *rp);

void mp_bit_not(const mp_uint *ap, mp_size an, mp_uint *rp);

// -a = ~(a - 1)

mp_uint mp_negate(const mp_uint *ap, mp_size an, mp_uint *rp);

mp_bool mp_equal_n(const mp_uint *ap, const mp_uint *bp, mp_size n);

mp_bool mp_equal(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn);

int mp_cmp_n(const mp_uint *ap, const mp_uint *bp, mp_size n);

int mp_cmp(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn);

int mp_cmp_uint(const mp_uint *ap, mp_size an, mp_uint b);

mp_size single_bit(const mp_uint *ap, mp_size an);

mp_size mp_countl_zero(const mp_uint *ap, mp_size an);

mp_size mp_countl_one(const mp_uint *ap, mp_size an);

mp_size mp_countr_zero(const mp_uint *ap, mp_size an);

mp_size mp_countr_one(const mp_uint *ap, mp_size an);

mp_size mp_bit_width(const mp_uint *ap, mp_size an);

mp_size mp_popcount(const mp_uint *ap, mp_size an);

mp_size mp_to_bytes(const mp_uint *ap, mp_size an, mp_byte *bytes,
                    enum mp_endian endian);
mp_size mp_from_bytes(
    mp_byte *bytes, mp_size byte_count, mp_uint *rp, enum mp_endian endian);

struct mp_to_string_result mp_to_string(
    char *first, char *last, const mp_uint *ap, mp_size an, int base);

struct mp_from_string_result mp_from_string(
    const char *first, const char *last, mp_uint *ap, mp_size an, int base);

#endif
