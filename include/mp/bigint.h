#ifndef MP_BIGINT_H_
#define MP_BIGINT_H_

#include <mp/memory.h>
#include <mp/mp.h>

struct mp_bigint {
    mp_ssize _size;
    mp_size _capacity;
    mp_uint *_data;
    struct mp_allocator *_alloc;
};

void mp_bigint_construct(struct mp_bigint *bigint, struct mp_allocator *alloc);

enum mp_errc mp_bigint_construct_int(
    struct mp_bigint *bigint, mp_int value, struct mp_allocator *alloc);

enum mp_errc mp_bigint_construct_uint(
    struct mp_bigint *bigint, mp_uint value, struct mp_allocator *alloc);

enum mp_errc mp_bigint_construct_copy(
    struct mp_bigint *bigint, const struct mp_bigint *other,
    struct mp_allocator *alloc);

enum mp_errc mp_bigint_construct_move(
    struct mp_bigint *bigint, struct mp_bigint *other,
    struct mp_allocator *alloc);

void mp_bigint_destruct(struct mp_bigint *bigint);

enum mp_errc mp_bigint_assign_int(struct mp_bigint *bigint, mp_int value);

enum mp_errc mp_bigint_assign_uint(struct mp_bigint *bigint, mp_uint value);

enum mp_errc mp_bigint_assign_copy(
    struct mp_bigint *bigint, const struct mp_bigint *other);

enum mp_errc mp_bigint_assign_move(
    struct mp_bigint *bigint, struct mp_bigint *other);

struct mp_allocator *mp_bigint_get_allocator(const struct mp_bigint *bigint);

void mp_bigint_swap(struct mp_bigint *a, struct mp_bigint *b);

int mp_bigint_sign(const struct mp_bigint *bigint);

mp_bool mp_bigint_signbit(const struct mp_bigint *bigint);

void mp_bigint_abs(struct mp_bigint *a);

void mp_bigint_negate(struct mp_bigint *a);

enum mp_errc mp_bigint_add(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r);

enum mp_errc mp_bigint_add_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_add_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

enum mp_errc mp_bigint_sub(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r);

enum mp_errc mp_bigint_sub_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_sub_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

enum mp_errc mp_bigint_mul(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r);

enum mp_errc mp_bigint_mul_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_mul_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

enum mp_errc mp_bigint_div(const struct mp_bigint *a, const struct mp_bigint *b,
                           struct mp_bigint *q, struct mp_bigint *r);

enum mp_errc mp_bigint_div_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *q, mp_uint *r);

enum mp_errc mp_bigint_div_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *q, mp_uint *r);

enum mp_errc mp_bigint_mod(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r);

enum mp_errc mp_bigint_mod_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_mod_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_not(const struct mp_bigint *a, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_and(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_and_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_and_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_or(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_or_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_or_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

enum mp_errc mp_bigint_xor(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r);

enum mp_errc mp_bigint_xor_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_xor_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

mp_bool mp_bigint_equal(const struct mp_bigint *a, const struct mp_bigint *b);

mp_bool mp_bigint_equal_int(const struct mp_bigint *a, mp_int b);

mp_bool mp_bigint_equal_uint(const struct mp_bigint *a, mp_uint b);

int mp_bigint_cmp(const struct mp_bigint *a, const struct mp_bigint *b);

int mp_bigint_cmp_int(const struct mp_bigint *a, mp_int b);

int mp_bigint_cmp_uint(const struct mp_bigint *a, mp_uint b);

struct mp_bigint *mp_bigint_new(struct mp_allocator *alloc);

struct mp_bigint *mp_bigint_new_int(mp_int value, struct mp_allocator *alloc);

struct mp_bigint *mp_bigint_new_uint(mp_uint value, struct mp_allocator *alloc);

struct mp_bigint *mp_bigint_new_copy(
    const struct mp_bigint *other, struct mp_allocator *alloc);

struct mp_bigint *mp_bigint_new_move(
    struct mp_bigint *other, struct mp_allocator *alloc);

void mp_bigint_delete(struct mp_bigint *bigint);

#endif
