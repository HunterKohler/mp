#include <mp/bigint.h>
#include <mp/config.h>
#include <mp/errc.h>
#include <mp/memory.h>
#include <mp/mp.h>
#include "./util.h"

static inline mp_size mp_bigint_get_size(const struct mp_bigint *bigint)
{
    return bigint->_size >= 0 ? bigint->_size : -bigint->_size;
}

static inline mp_size mp_bigint_normal_size(struct mp_bigint *bigint, mp_size n)
{
    while (n > 0 && bigint->_data[n - 1]) {
        --n;
    }

    return n;
}

static enum mp_errc mp_bigint_reserve(struct mp_bigint *bigint, mp_size n)
{
    if (n > bigint->_capacity) {
        mp_uint *new_data = mp_allocate_uint(bigint->_alloc, n);

        mp_uint_copy(bigint->_data, mp_bigint_get_size(bigint), new_data);
        mp_deallocate_uint(bigint->_alloc, bigint->_data, bigint->_capacity);
        bigint->_data = new_data;
        bigint->_capacity = n;
    }
}

static enum mp_errc mp_bigint_assign_uint_positive(
    struct mp_bigint *bigint, mp_uint value)
{
    if (mp_bigint_reserve(bigint, 1)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    bigint->_size = 1;
    bigint->_data[0] = value;

    return MP_ERRC_OK;
}

static enum mp_errc mp_bigint_assign_uint_negative(
    struct mp_bigint *bigint, mp_uint value)
{
    if (mp_bigint_reserve(bigint, 1)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    bigint->_size = -1;
    bigint->_data[0] = value;

    return MP_ERRC_OK;
}

static inline void mp_bigint_assign_zero(struct mp_bigint *bigint)
{
    bigint->_size = 0;
}

static inline enum mp_errc mp_bigint_construct_with_reserved(
    struct mp_bigint *bigint, mp_size n, struct mp_allocator *alloc)
{
    bigint->_alloc = alloc;
    bigint->_capacity = n;
    bigint->_data = mp_allocate_uint(bigint->_alloc, n);
    bigint->_size = 0;

    if (!bigint->_data) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    return MP_ERRC_OK;
}

void mp_bigint_construct(struct mp_bigint *bigint, struct mp_allocator *alloc)
{
    bigint->_alloc = alloc ? alloc : mp_get_default_allocator();
    bigint->_capacity = 0;
    bigint->_data = NULL;
    bigint->_size = 0;
}

enum mp_errc mp_bigint_construct_int(
    struct mp_bigint *bigint, mp_int value, struct mp_allocator *alloc)
{
    if (value >= 0) {
        return mp_bigint_construct_uint(bigint, value, alloc);
    }

    bigint->_alloc = alloc ? alloc : mp_get_default_allocator();
    bigint->_data = mp_allocate_uint(bigint->_alloc, 1);

    if (!bigint->_data) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    bigint->_data[0] = -value;
    bigint->_size = -1;
    bigint->_capacity = 1;

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_construct_uint(
    struct mp_bigint *bigint, mp_uint value, struct mp_allocator *alloc)
{
    bigint->_alloc = alloc ? alloc : mp_get_default_allocator();

    if (value) {
        bigint->_data = mp_allocate_uint(bigint->_alloc, 1);

        if (!bigint->_data) {
            return MP_ERRC_NOT_ENOUGH_MEMORY;
        }

        bigint->_data[0] = value;
        bigint->_size = 1;
        bigint->_capacity = 1;
    } else {
        bigint->_data = NULL;
        bigint->_size = 0;
        bigint->_capacity = 0;
    }

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_construct_copy(
    struct mp_bigint *bigint, const struct mp_bigint *other,
    struct mp_allocator *alloc)
{
    mp_size size = mp_bigint_get_size(other);

    bigint->_alloc = alloc ? alloc : other->_alloc;
    bigint->_size = other->_size;
    bigint->_data = mp_allocate_uint(bigint->_alloc, size);

    if (!bigint->_data) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    mp_uint_copy(other->_data, size, bigint->_data);
    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_construct_move(
    struct mp_bigint *bigint, struct mp_bigint *other,
    struct mp_allocator *alloc)
{
    alloc = alloc ? alloc : other->_alloc;

    if (!mp_allocator_is_equal(alloc, other->_alloc)) {
        return mp_bigint_construct_copy(bigint, other, alloc);
    }

    bigint->_alloc = alloc;
    bigint->_data = other->_data;
    bigint->_size = other->_size;
    bigint->_capacity = other->_capacity;
    other->_data = NULL;
    other->_size = 0;
    other->_capacity = 0;
}

void mp_bigint_destruct(struct mp_bigint *bigint)
{
    mp_deallocate_uint(bigint->_alloc, bigint->_data, bigint->_capacity);
}

enum mp_errc mp_bigint_assign_int(struct mp_bigint *bigint, mp_int value)
{
    if (value > 0) {
        return mp_bigint_assign_uint_positive(bigint, value);
    } else if (value < 0) {
        return mp_bigint_assign_uint_negative(bigint, -value);
    } else {
        mp_bigint_assign_zero(bigint);
    }

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_assign_uint(struct mp_bigint *bigint, mp_uint value)
{
    if (value > 0) {
        return mp_bigint_assign_uint_positive(bigint, value);
    } else {
        mp_bigint_assign_zero(bigint);
    }

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_assign_copy(
    struct mp_bigint *bigint, const struct mp_bigint *other)
{
    mp_size n = mp_bigint_get_size(other);

    if (bigint == other) {
        return MP_ERRC_OK;
    } else if (mp_bigint_reserve(bigint, n)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    mp_uint_copy(other->_data, n, bigint->_data);
    bigint->_size = other->_size;

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_assign_move(
    struct mp_bigint *bigint, struct mp_bigint *other)
{
    if (bigint == other) {
        return MP_ERRC_OK;
    } else if (!mp_allocator_is_equal(bigint->_alloc, other->_alloc)) {
        return mp_bigint_assign_copy(bigint, other);
    }

    mp_deallocate_uint(
        bigint->_alloc, bigint->_data, mp_bigint_get_size(bigint));

    bigint->_size = other->_size;
    bigint->_capacity = other->_capacity;
    bigint->_data = other->_data;

    return MP_ERRC_OK;
}

struct mp_allocator *mp_bigint_get_allocator(const struct mp_bigint *bigint)
{
    return bigint->_alloc;
}

void mp_bigint_swap(struct mp_bigint *a, struct mp_bigint *b)
{
    mp_uint *data = a->_data;
    mp_ssize size = a->_size;
    mp_size capacity = a->_capacity;

    a->_data = b->_data;
    a->_size = b->_size;
    a->_capacity = b->_capacity;
    b->_data = data;
    b->_size = size;
    b->_capacity = capacity;
}

int mp_bigint_sign(const struct mp_bigint *bigint)
{
    return bigint->_size > 0 ? 1 : bigint->_size < 0 ? -1 : 0;
}

mp_bool mp_bigint_signbit(const struct mp_bigint *bigint)
{
    return bigint->_size > 0;
}

void mp_bigint_abs(struct mp_bigint *bigint)
{
    bigint->_size = mp_bigint_get_size(bigint);
}

void mp_bigint_negate(struct mp_bigint *bigint)
{
    bigint->_size = -bigint->_size;
}

static enum mp_errc mp_bigint_abs_add_ord(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r,
    mp_size an, mp_size bn)
{
    mp_size rn = an + 1;

    if (mp_bigint_reserve(r, rn)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    r->_data[an] = mp_add(a->_data, an, b->_data, bn, r->_data);
    rn = mp_bigint_normal_size(r, rn);
    r->_size = a->_size >= 0 ? rn : -rn;

    return MP_ERRC_OK;
}

static enum mp_errc mp_bigint_abs_add(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    if (!a->_size) {
        return mp_bigint_assign_copy(r, b);
    } else if (!b->_size) {
        return mp_bigint_assign_copy(r, a);
    }

    mp_size an = mp_bigint_get_size(a);
    mp_size bn = mp_bigint_get_size(b);

    if (an >= bn) {
        return mp_bigint_abs_add_ord(a, b, r, an, bn);
    } else {
        return mp_bigint_abs_add_ord(b, a, r, bn, an);
    }
}

static enum mp_errc mp_bigint_abs_add_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r)
{
    if (!a->_size) {
        return mp_bigint_assign_uint(r, b);
    } else if (!b) {
        return mp_bigint_assign_copy(r, a);
    }

    mp_size an = mp_bigint_get_size(a);
    mp_size rn = an + 1;

    if (mp_bigint_reserve(r, rn)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    r->_data[an] = mp_bigint_add_uint(a, b, r);
    rn = mp_bigint_normal_size(r, rn);
    r->_size = a->_size >= 0 ? rn : -rn;

    return MP_ERRC_OK;
}

static enum mp_errc mp_bigint_abs_sub_ord(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r,
    mp_size an, mp_size bn)
{
    mp_size rn = an;

    if (mp_bigint_reserve(r, rn)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    mp_sub(a->_data, an, b->_data, bn, r->_data);
    rn = mp_bigint_normal_size(r, rn);
    r->_size = a->_size >= 0 ? rn : -rn;

    return MP_ERRC_OK;
}

static enum mp_errc mp_bigint_abs_sub(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    if (!a->_size) {
        if (mp_bigint_assign_copy(r, b)) {
            return MP_ERRC_NOT_ENOUGH_MEMORY;
        }

        mp_bigint_negate(r);
        return MP_ERRC_OK;
    } else if (!b->_size) {
        return mp_bigint_assign_copy(r, b);
    }

    mp_size an = mp_bigint_get_size(a);
    mp_size bn = mp_bigint_get_size(b);
    int cmp = mp_cmp(a, an, b, bn);

    if (cmp > 0) {
        return mp_bigint_abs_sub_ord(a, b, r, an, bn);
    } else if (cmp < 0) {
        return mp_bigint_abs_sub_ord(b, a, r, bn, an);
    } else {
        mp_bigint_assign_zero(r);
        return MP_ERRC_OK;
    }
}

static enum mp_errc mp_bigint_abs_sub_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r)
{
    if (!a->_size) {
        return mp_bigint_assign_uint_negative(r, b);
    } else if (!b) {
        return mp_bigint_assign_copy(r, a);
    }

    mp_size an = mp_bigint_get_size(a);
    int cmp = mp_cmp_uint(a->_data, an, b);

    if (cmp > 0) {
        mp_size rn = an;

        if (mp_bigint_reserve(r, rn)) {
            return MP_ERRC_NOT_ENOUGH_MEMORY;
        }

        mp_sub_uint(a->_data, an, b, r->_data);
        rn = mp_bigint_normal_size(r, rn);
        r->_size = a->_size > 0 ? rn : -rn;
    } else if (cmp < 0) {
        if (a->_size > 0) {
            return mp_bigint_assign_uint_negative(r, b);
        } else {
            return mp_bigint_assign_uint_positive(r, b);
        }
    } else {
        mp_bigint_assign_zero(r);
    }

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_add(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    if (mp_same_sign(a->_size, b->_size)) {
        return mp_bigint_abs_add(a, b, r);
    } else {
        return mp_bigint_abs_sub(a, b, r);
    }
}

enum mp_errc mp_bigint_add_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r)
{
    if (mp_same_sign(a->_size, b)) {
        return mp_bigint_abs_add_uint(a, mp_int_abs(b), r);
    } else {
        return mp_bigint_abs_sub_uint(a, mp_int_abs(b), r);
    }
}

enum mp_errc mp_bigint_add_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r)
{
    if (mp_same_sign(a->_size, b)) {
        return mp_bigint_abs_add_uint(a, b, r);
    } else {
        return mp_bigint_abs_sub_uint(a, b, r);
    }
}

enum mp_errc mp_bigint_sub(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    if (mp_same_sign(a->_size, b->_size)) {
        return mp_bigint_abs_sub(a, b, r);
    } else {
        return mp_bigint_abs_add(a, b, r);
    }
}

enum mp_errc mp_bigint_sub_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r)
{
    if (mp_same_sign(a->_size, b)) {
        return mp_bigint_abs_sub_uint(a, mp_int_abs(b), r);
    } else {
        return mp_bigint_abs_add_uint(a, mp_int_abs(b), r);
    }
}

enum mp_errc mp_bigint_sub_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r)
{
    if (mp_same_sign(a->_size, b)) {
        return mp_bigint_abs_sub_uint(a, b, r);
    } else {
        return mp_bigint_abs_add_uint(a, b, r);
    }
}

static enum mp_errc mp_bigint_mul_ord(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r,
    mp_size an, mp_size bn)
{
    MP_EXPECTS(an >= bn);
    MP_EXPECTS(bn);

    mp_size rn = an + bn;

    if (bn == 1) {
        if (mp_bigint_reserve(r, rn)) {
            return MP_ERRC_NOT_ENOUGH_MEMORY;
        }

        a->_data[an] = mp_mul_uint(a->_data, an, b->_data, bn);
    } else {
        struct mp_bigint tmp;

        if (mp_bigint_construct_with_reserved(&tmp, rn, r->_alloc)) {
            return MP_ERRC_NOT_ENOUGH_MEMORY;
        }

        mp_mul(a->_data, an, b->_data, bn);
        mp_bigint_swap(r, tmp);
        mp_bigint_destruct(tmp);
    }

    rn = mp_bigint_normal_size(r, rn);
    r->_size = mp_same_sign(a->_size, b->_size) ? rn : -rn;

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_mul(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    if (!a->_size || !b->_size) {
        mp_bigint_assign_zero(r);
        return MP_ERRC_OK;
    }

    mp_size an = mp_bigint_get_size(a);
    mp_size bn = mp_bigint_get_size(b);

    if (an >= bn) {
        return mp_bigint_mul_ord(a, b, r, an, bn);
    } else {
        return mp_bigint_mul_ord(b, a, r, bn, an);
    }
}

enum mp_errc mp_bigint_mul_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r)
{
    if (b >= 0) {
        return mp_bigint_mul_uint(a, b, r);
    } else if (mp_bigint_mul_uint(a, -b, r)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    mp_bigint_negate(r);
    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_mul_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r)
{
    if (!a->_size || !b) {
        mp_bigint_assign_zero(r);
        return MP_ERRC_OK;
    }

    mp_size an = mp_bigint_get_size(a);
    mp_size rn = an + 1;

    if (mp_bigint_reserve(r, rn)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    r->_data[an] = mp_mul_uint(a->_data, an, b, r->_data);
    rn = mp_bigint_normal_size(r, rn);
    r->_size = a->_size >= 0 ? rn : -rn;

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_div(const struct mp_bigint *a, const struct mp_bigint *b,
                           struct mp_bigint *q, struct mp_bigint *r);

enum mp_errc mp_bigint_div_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *q, mp_uint *r)
{
    if (b >= 0) {
        return mp_bigint_div_uint(a, b, r);
    } else if (mp_bigint_div_uint(a, -b, r)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    } else {
        mp_bigint_negate(r);
        return MP_ERRC_OK;
    }
}

enum mp_errc mp_bigint_div_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *q, mp_uint *r)
{
    if (!b) {
        return MP_ERRC_DIVIDE_BY_ZERO;
    } else if (!a) {
        mp_bigint_assign_zero(q);
        *r = 0;
        return MP_ERRC_OK;
    }

    mp_size an = mp_bigint_get_size(a);

    if (mp_bigint_reserve(q, an)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    *r = mp_div_uint(a->_data, an, b, q->_data);
    q->_size = mp_bigint_normal_size(q, an);

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_mod(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r);

enum mp_errc mp_bigint_mod_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_mod_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

// ~a = -(a + 1)

enum mp_errc mp_bigint_bit_not(const struct mp_bigint *a, struct mp_bigint *r)
{
    if (mp_bigint_add_uint(a, 1, r)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    mp_bigint_negate(r);
    return MP_ERRC_OK;
}

static enum mp_errc mp_bigint_bit_and_pp(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    MP_EXPECTS(a->_size >= b->_size);
    MP_EXPECTS(b->_size > 0);

    if (mp_bigint_reserve(r, b->_size)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    mp_bit_and_n(a->_data, b->_data, b->_size, r->_data);
    r->_size = mp_bigint_normal_size(r, b->_size);

    return MP_ERRC_OK;
}

// a & -b = a & ~(b - 1)

// static enum mp_errc mp_bigint_bit_and_pn(
//     const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
// {
//     MP_EXPECTS(a->_size > 0);
//     MP_EXPECTS(b->_size < 0);

//     mp_size an = a->_size;
//     mp_size bn = -b->_size;
//     mp_uint c1 = 1;

//     if (mp_bigint_reserve(r, an)) {
//         return MP_ERRC_NOT_ENOUGH_MEMORY;
//     }
// }

// -a & -b = ~(a - 1) & ~(b - 1)
//         = ~((a - 1) | (b - 1))
//         = -(((a - 1) | (b - 1)) + 1)

static enum mp_errc mp_bigint_bit_and_nn(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    MP_EXPECTS(a->_size <= b->_size);
    MP_EXPECTS(b->_size < 0);

    mp_size an = -a->_size;
    mp_size bn = -b->_size;
    mp_size rn = an + 1;
    mp_uint c1 = 1;
    mp_uint c2 = 1;
    mp_uint c3 = 1;

    if (mp_bigint_reserve(r, rn)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    for (mp_size i = 0; i < bn; i++) {
        mp_uint av = a->_data[i];
        mp_uint bv = b->_data[i];
        mp_uint rv = ((av - c1) | (bv - c2)) + c3;

        c1 = av < c1;
        c2 = bv < c2;
        c3 = rv < c3;
        r->_data[i] = rv;
    }

    for (mp_size i = bn; i < an; i++) {
        mp_uint av = a->_data[i];
        mp_uint rv = av - c1 + c3;

        c1 = av < c1;
        c3 = rv < c3;
        r->_data[i] = rv;
    }

    r->_data[an] = c3;
    r->_size = -mp_bigint_normal_size(r, rn);

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_bit_and(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    if (!a->_size || !b->_size) {
        mp_bigint_assign_zero(r);
        return MP_ERRC_OK;
    } else if (a->_size > 0) {
        if (b->_size > 0) {
            if (a->_size >= b->_size) {
                return mp_bigint_bit_and_pp(a, b, r);
            } else {
                return mp_bigint_bit_and_pp(b, a, r);
            }
        } else {
            return mp_bigint_bit_and_pn(a, b, r);
        }
    } else if (b->_size > 0) {
        return mp_bigint_bit_and_pn(b, a, r);
    } else if (a->_size <= b->_size) {
        return mp_bigint_bit_and_nn(a, b, r);
    } else {
        return mp_bigint_bit_and_nn(a, b, r);
    }
}

enum mp_errc mp_bigint_bit_and_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_and_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

static enum mp_errc mp_bigint_bit_or_pp(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    MP_EXPECTS(a->_size >= b->_size);
    MP_EXPECTS(b->_size > 0);

    if (mp_bigint_reserve(r->_data, a->_size)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    mp_bit_or(a->_data, a->_size, b->_data, a->_size, r->_data);
    r->_size = a->_size;

    return MP_ERRC_OK;
}

// a | -b = a | ~(b - 1)
//        = ~(~a & (b - 1))
//        = -((~a & (b - 1)) + 1)

static enum mp_errc mp_bigint_bit_or_pn(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    MP_EXPECTS(a->_size > 0);
    MP_EXPECTS(b->_size < 0);

    mp_size an = a->_size;
    mp_size bn = -b->_size;

    if (mp_bigint_reserve(r, bn)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY
    }

    mp_uint c1 = 1;
    mp_uint c2 = 1;
    mp_uint min_n = an >= bn ? bn : an;

    for (mp_size i = 0; i < min_n; i++) {
        mp_uint av = a->_data[i];
        mp_uint bv = b->_data[i];
        mp_uint rv = (~av & (bv - c1)) + c2;

        c1 = bv < c1;
        c2 = rv < c2;
        r->_data[i] = rv;
    }

    for (mp_size i = min_n; i < bn; i++) {
        mp_uint bv = b->_data[i];
        mp_uint rv = bv - c1 + c2;

        c1 = bv < c1;
        c2 = rv < c2;
        r->_data[i] = rv;
    }

    return MP_ERRC_OK;
}

// -a | -b = ~(a - 1) | ~(b - 1)
//         = ~((a - 1) & (b - 1))
//         = -(((a - 1) & (b - 1)) + 1)

static enum mp_errc mp_bigint_bit_or_nn(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    MP_EXPECTS(a->_size < b->_size);
    MP_EXPECTS(b->_size < 0);

    mp_size an = -a->_size;
    mp_size bn = -b->_size;
    mp_uint c1 = 1;
    mp_uint c2 = 1;
    mp_uint c3 = 1;

    if (mp_bigint_reserve(r, bn)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    for (mp_size i = 0; i < bn; i++) {
        mp_uint av = a->_data[i];
        mp_uint bv = b->_data[i];
        mp_uint rv = ((av - c1) & (bv - c2)) + c3;

        c1 = av < c1;
        c2 = bv < c2;
        c3 = rv < c3;
        r->_data[i] = rv;
    }

    r->_size = -bn;
    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_bit_or(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    if (!a->_size) {
        return mp_bigint_assign_copy(r, b);
    } else if (!b->_size) {
        return mp_bigint_assign_copy(r, a);
    } else if (a->_size > 0) {
        if (b->_size > 0) {
            if (a->_size >= b->_size) {
                return mp_bigint_bit_or_pp(a, b, r);
            } else {
                return mp_bigint_bit_or_pp(b, a, r);
            }
        } else {
            return mp_bigint_bit_or_pn(a, b, r);
        }
    } else if (b->_size > 0) {
        return mp_bigint_bit_or_pn(b, a, r);
    } else if (a->_size <= b->_size) {
        return mp_bigint_bit_or_nn(a, b, r);
    } else {
        return mp_bigint_bit_or_nn(b, a, r);
    }
}

enum mp_errc mp_bigint_bit_or_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_bit_or_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

static enum mp_errc mp_bigint_xor_pp(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    MP_EXPECTS(a->_size >= b->_size);
    MP_EXPECTS(b->_size > 0);

    if (mp_bigint_reserve(r, a->_size)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    mp_bit_xor(a->_data, a->_size, b->_data, b->_size, r->_data);

    mp_size rn = mp_bigint_normal_size(bigint, a->_size);
    r->_data = mp_same_sign(a->_size, b->_size) ? rn : -rn;
}

// a ^ -b = a ^ ~(b - 1)
//        = ~(a ^ (b - 1))
//        = -((a ^ (b - 1)) + 1)

static enum mp_errc mp_bigint_xor_pn(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    MP_EXPECTS(a->_size > 0);
    MP_EXPECTS(b->_size < 0);

    mp_size an = a->_size;
    mp_size bn = -b->_size;
    mp_uint c1 = 1;
    mp_uint c2 = 1;

    if (an > bn) {
        if (mp_bigint_reserve(r, an)) {
            return MP_ERRC_NOT_ENOUGH_MEMORY;
        }

        for (mp_size i = 0; i < bn; i++) {
            mp_uint av = a->_data[i];
            mp_uint bv = b->_data[i];
            mp_uint rv = (a ^ (b - c1)) + c2;

            c1 = b < c1;
            c2 = rv < c2;
            r->_data[i] = rv;
        }

        mp_uint_copy(a->_data + bn, an - bn, r->_data + bn);
        r->_size = an;
    } else if (an < bn) {
        if (mp_bigint_reserve(r, bn)) {
            return MP_ERRC_NOT_ENOUGH_MEMORY;
        }

        for (mp_size i = 0; i < bn; i++) {
            mp_uint av = a->_data[i];
            mp_uint bv = b->_data[i];
            mp_uint rv = (a ^ (b - c1)) + c2;

            c1 = b < c1;
            c2 = rv < c2;
            r->_data[i] = rv;
        }

        for (mp_size i = an; i < bn; i++) {
            mp_uint bv = b->_data[i];
            mp_uint rv = bv - c1 + c2;

            c1 = bv < c1;
            c2 = rv < c2;
            r->_data[i] = rv;
        }

        r->_size = -mp_bigint_normal_size(r, bn);
    }

    return MP_ERRC_OK;
}

// -a ^ -b = ~(a - 1) ^ ~(b - 1)
//         = (a - 1) ^ (b - 1)

static enum mp_errc mp_bigint_xor_nn(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    MP_EXPECTS(a->_size <= b->_size);
    MP_EXPECTS(b->_size < 0);

    mp_size an = -a->_size;
    mp_size bn = -b->_size;
    mp_uint c1 = 1;
    mp_uint c2 = 1;

    if (mp_bigint_reserve(r, an)) {
        return MP_ERRC_NOT_ENOUGH_MEMORY;
    }

    for (mp_size i = 0; i < bn; i++) {
        mp_uint av = a->_data[i];
        mp_uint bv = b->_data[i];
        mp_uint rv = (av - c1) ^ (bv - c2);

        c1 = av < c1;
        c2 = bv < c2;
        r->_data[i] = rv;
    }

    if (an > bn) {
        mp_sub_uint(a->_data + bn, an - bn, c1);
    }

    return MP_ERRC_OK;
}

enum mp_errc mp_bigint_xor(
    const struct mp_bigint *a, const struct mp_bigint *b, struct mp_bigint *r)
{
    if (!a->_size) {
        return mp_bigint_assign_copy(r, b);
    } else if (!b->_size) {
        return mp_bigint_assign_copy(r, a);
    } else if (a->_size > 0) {
        if (b->_size > 0) {
            if (a->_size >= b->_size) {
                return mp_bigint_xor_pp(a, b, r);
            } else {
                return mp_bigint_xor_pp(b, a, r);
            }
        } else {
            return mp_bigint_xor_pn(a, b, r);
        }
    } else if (b->_size > 0) {
        return mp_bigint_xor_pn(a, b, r);
    } else if (a->_size <= b->_size) {
        return mp_bigint_xor_nn(a, b, r);
    } else {
        return mp_bigint_xor_nn(b, a, r);
    }
}

enum mp_errc mp_bigint_xor_int(
    const struct mp_bigint *a, mp_int b, struct mp_bigint *r);

enum mp_errc mp_bigint_xor_uint(
    const struct mp_bigint *a, mp_uint b, struct mp_bigint *r);

mp_bool mp_bigint_equal(const struct mp_bigint *a, const struct mp_bigint *b)
{
    return a->_size == b->_size &&
           mp_equal_n(a->_data, b->_data, a->_size < 0 ? -a->_size : a->_size);
}

mp_bool mp_bigint_equal_int(const struct mp_bigint *a, mp_int b)
{
    if (b > 0) {
        return a->_size == 1 && a->_data[0] == b;
    } else if (b < 0) {
        return a->_size == -1 && a->_data[0] == -b;
    } else {
        return a->_size == 0;
    }
}

mp_bool mp_bigint_equal_uint(const struct mp_bigint *a, mp_uint b)
{
    if (b > 0) {
        return a->_size == 1 && a->_data[0] == b;
    } else {
        return a->_size == 0;
    }
}

int mp_bigint_cmp(const struct mp_bigint *a, const struct mp_bigint *b)
{
    if (a->_size > b->_size) {
        return 1;
    } else if (a->_size < b->_size) {
        return -1;
    } else if (a->_size > 0) {
        return mp_cmp_n(a->_data, b->_data, a->_size);
    } else if (a->_size < 0) {
        return mp_cmp_n(b->_data, a->_data, -a->_size);
    } else {
        return 0;
    }
}

int mp_bigint_cmp_int(const struct mp_bigint *a, mp_int b)
{
    if (b >= 0) {
        return mp_bigint_cmp_uint(a, b);
    } else if (a->_size > -1) {
        return 1;
    } else if (a->_size < -1) {
        return -1;
    } else if (a->_data[0] < b) {
        return 1;
    } else if (a->_data[0] > b) {
        return -1;
    } else {
        return 0;
    }
}

int mp_bigint_cmp_uint(const struct mp_bigint *a, mp_uint b)
{
    if (b > 0) {
        if (a->_size > 1) {
            return 1;
        } else if (a->_size < 1) {
            return -1;
        } else if (a->_data[0] > b) {
            return 1;
        } else if (a->_data[0] < b) {
            return -1;
        } else {
            return 0;
        }
    } else if (a->_size > 0) {
        return 1;
    } else if (a->_size < 0) {
        return -1;
    } else {
        return 0;
    }
}

struct mp_bigint *mp_bigint_new(struct mp_allocator *alloc)
{
    struct mp_bigint *bigint = mp_allocate_bigint(alloc, 1);

    if (bigint) {
        mp_bigint_construct(bigint, alloc);
    }

    return bigint;
}

struct mp_bigint *mp_bigint_new_int(mp_int value, struct mp_allocator *alloc)
{
    struct mp_bigint *bigint = mp_allocate_bigint(alloc, 1);

    if (bigint) {
        mp_bigint_construct_int(bigint, value, alloc);
    }

    return bigint;
}

struct mp_bigint *mp_bigint_new_uint(mp_uint value, struct mp_allocator *alloc)
{
    struct mp_bigint *bigint = mp_allocate_bigint(alloc, 1);

    if (bigint) {
        mp_bigint_construct_uint(bigint, value, alloc);
    }

    return bigint;
}

struct mp_bigint *mp_bigint_new_copy(
    const struct mp_bigint *other, struct mp_allocator *alloc)
{
    struct mp_bigint *bigint = mp_allocate_bigint(alloc, 1);

    if (bigint) {
        mp_bigint_construct_copy(bigint, other, alloc);
    }

    return bigint;
}

struct mp_bigint *mp_bigint_new_move(
    struct mp_bigint *other, struct mp_allocator *alloc)
{
    struct mp_bigint *bigint = mp_allocate_bigint(alloc, 1);

    if (bigint) {
        mp_bigint_construct_move(bigint, other, alloc);
    }

    return bigint;
}

void mp_bigint_delete(struct mp_bigint *bigint)
{
    struct mp_allocator *alloc = bigint->_alloc;

    mp_bigint_destruct(bigint);
    mp_deallocate_bigint(alloc, bigint, 1);
}
