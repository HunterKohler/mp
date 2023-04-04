#include <mp/config.h>
#include <mp/mp.h>
#include "./util.h"

mp_uint mp_add_uint(const mp_uint *ap, mp_size an, mp_uint b, mp_uint *rp)
{
    MP_EXPECTS(an);

    do {
        mp_uint a = *ap++;
        mp_uint r = a + b;

        b = r < b;
        *rp++ = r;
    } while (--an);

    return b;
}

mp_uint mp_add_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp)
{
    MP_EXPECTS(n);

    mp_uint c = 0;
    do {
        mp_uint a = *ap++;
        mp_uint b = *bp++ + c;
        mp_uint r = a + b;

        c = (b < c) + (r < b);
        *rp++ = r;
    } while (--n);

    return c;
}

mp_uint mp_add(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
               mp_uint *rp)
{
    MP_EXPECTS(an >= bn);
    MP_EXPECTS(bn);

    mp_uint c = mp_add_n(ap, bp, bn, rp);

    if (an > bn) {
        c = mp_add_uint(ap + bn, an - bn, c, rp + bn);
    }

    return c;
}

mp_uint mp_sub_uint(const mp_uint *ap, mp_size an, mp_uint b, mp_uint *rp)
{
    MP_EXPECTS(an);

    do {
        mp_uint a = *ap++;
        mp_uint r = a - b;

        b = a < b;
        *rp++ = r;
    } while (--an);

    return b;
}

mp_uint mp_sub_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp)
{
    MP_EXPECTS(n);

    mp_uint c = 0;
    do {
        mp_uint a = *ap++;
        mp_uint b = *bp++ + c;
        mp_uint r = a - b;

        c = b < c;
        *rp++ = r;
    } while (--n);

    return c;
}

mp_uint mp_sub(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
               mp_uint *rp)
{
    MP_EXPECTS(an >= bn);
    MP_EXPECTS(bn);

    mp_uint c = mp_sub_n(ap, bp, bn, rp);

    if (an > bn) {
        c = mp_sub_uint(ap + bn, an - bn, c, rp + bn);
    }

    return c;
}

// r += a * b

static mp_uint mp_addmul_uint(
    const mp_uint *ap, mp_size an, mp_uint b, mp_uint *rp)
{
    MP_EXPECTS(an);

    mp_uint c = 0;
    do {
        mp_uint a = *ap++;
        mp_uint r = *rp;
        mp_uint hi, lo = mp_uint_mul(a, b, &hi);

        lo += c;
        r += lo;
        c = hi + (lo < c) + (r < lo);
        *rp++ = r;
    } while (--an);

    return c;
}

// static mp_uint mp_submul_uint(
//     const mp_uint *ap, mp_size an, mp_uint b, mp_uint *rp)
// {
//     MP_EXPECTS(an);

//     mp_uint c = 0;
//     do {
//         mp_uint a = *ap++;
//         mp_uint r = *rp;
//         mp_uint hi, lo = mp_uint_mul(a, b, &hi);

//         lo += c;
//         c = hi + (lo < c) + (r < lo);
//         r -= lo;
//         *rp++ = r;
//     } while (--an);

//     return c;
// }

mp_uint mp_mul_uint(const mp_uint *ap, mp_size an, mp_uint b, mp_uint *rp)
{
    MP_EXPECTS(an);

    mp_uint c = 0;
    do {
        mp_uint a = *ap++;
        mp_uint hi, lo = mp_uint_mul(a, b, &hi);

        lo += c;
        c = hi + (lo < c);
        *rp++ = lo;
    } while (--an);

    return c;
}

mp_uint mp_mul(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
               mp_uint *rp)
{
    MP_EXPECTS(an >= bn);
    MP_EXPECTS(bn);

    mp_uint c = mp_mul_uint(ap, an, *bp, rp);

    while (--bn) {
        rp[an] = c;
        c = mp_addmul_uint(++ap, --an, *++bp, ++rp);
    }

    return c;
}

mp_uint mp_div_uint(const mp_uint *np, mp_size nn, mp_uint d, mp_uint *qp)
{
    MP_EXPECTS(nn);
    MP_EXPECTS(d);

    if (d >> (MP_UINT_WIDTH - 1)) {
        mp_uint v = mp_uint_inv(d);
        mp_uint r = 0;
        mp_uint q;

        do {
            q = np[--nn];
            qp[nn] = mp_uint_div_inv(r, q, d, v, &r);
        } while (nn);

        return r;
    } else {
        mp_uint lshift = mp_uint_countl_zero(d);
        mp_uint rshift = MP_UINT_WIDTH - lshift;
        mp_uint v = mp_uint_inv(d <<= lshift);
        mp_uint q = 0;
        mp_uint r = 0;

        do {
            mp_uint next = np[--nn];

            q = q << lshift | next >> rshift;
            qp[nn] = mp_uint_div_inv(r, q, d, v, &r);
            q = next;
        } while (--nn);

        *qp = mp_uint_div_inv(r, q << lshift, d, v, &r);
        return r >> rshift;
    }
}

void mp_div(const mp_uint *np, mp_uint nn, const mp_uint *dp, mp_size dn,
            mp_uint *qp, mp_uint *rp);

void mp_mod_uint(const mp_uint *np, mp_uint nn, mp_uint d, mp_uint *rp);

void mp_mod(const mp_uint *np, mp_uint nn, const mp_uint *dp, mp_size dn,
            mp_uint *rp);

mp_uint mp_left_shift(const mp_uint *ap, mp_size an, mp_size bits, mp_uint *rp)
{
    MP_EXPECTS(an);
    MP_EXPECTS(bits < MP_UINT_WIDTH);
    MP_EXPECTS(bits);

    mp_uint rbits = MP_UINT_WIDTH - bits;
    mp_uint next = ap[--an];
    mp_uint ret = next >> rbits;

    while (an) {
        mp_uint prev = ap[--an];
        rp[an + 1] = (next << bits) | (prev >> rbits);
    }

    *rp = next << bits;
    return ret;
}

mp_uint mp_right_shift(const mp_uint *ap, mp_size an, mp_size bits, mp_uint *rp)
{
    MP_EXPECTS(an);
    MP_EXPECTS(bits < MP_UINT_WIDTH);
    MP_EXPECTS(bits);

    mp_uint lbits = MP_UINT_WIDTH - bits;
    mp_uint prev = *ap;

    while (--an) {
        mp_uint next = *++ap;
        *rp++ = (prev >> bits) | (next << lbits);
        prev = next;
    }

    return prev << lbits;
}

void mp_bit_and_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp)
{
    MP_EXPECTS(n);

    do {
        *rp++ = *ap++ & *bp++;
    } while (--n);
}

void mp_bit_and(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
                mp_uint *rp)
{
    MP_EXPECTS(an >= bn);
    MP_EXPECTS(bn);

    mp_bit_and_n(ap, bp, bn, rp);
    mp_uint_zero(rp + bn, an - bn);
}

void mp_bit_or_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp)
{
    MP_EXPECTS(n);

    do {
        *rp++ = *ap++ | *bp++;
    } while (--n);
}

void mp_bit_or(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
               mp_uint *rp)
{
    MP_EXPECTS(an >= bn);
    MP_EXPECTS(bn);

    mp_bit_or_n(ap, bp, bn, rp);
    mp_uint_move(ap + bn, an - bn, rp + bn);
}

void mp_bit_xor_n(const mp_uint *ap, const mp_uint *bp, mp_size n, mp_uint *rp)
{
    MP_EXPECTS(n);

    do {
        *rp++ = *ap++ ^ *bp++;
    } while (--n);
}

void mp_bit_xor(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn,
                mp_uint *rp)
{
    MP_EXPECTS(an >= bn);
    MP_EXPECTS(bn);

    mp_bit_xor_n(ap, bp, bn, rp);
    mp_uint_move(ap + bn, an - bn, rp + bn);
}

void mp_bit_not(const mp_uint *ap, mp_size an, mp_uint *rp)
{
    MP_EXPECTS(an);

    do {
        *rp++ = ~*ap++;
    } while (--an);
}

// -a = ~(a - 1)

mp_uint mp_negate(const mp_uint *ap, mp_size an, mp_uint *rp)
{
    MP_EXPECTS(an);

    mp_uint c = 1;
    do {
        mp_uint a = *ap++;
        mp_uint r = ~(a - c);

        c = a < c;
        *rp++ = r;
    } while (--an);
    return c;
}

mp_bool mp_equal_n(const mp_uint *ap, const mp_uint *bp, mp_size n)
{
    MP_EXPECTS(n);

    return !memcmp(ap, bp, sizeof(mp_uint) * n);
}

mp_bool mp_equal(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn)
{
    MP_EXPECTS(an);
    MP_EXPECTS(bn);

    return an == bn && mp_equal_n(ap, bp, bn);
}

int mp_cmp_n(const mp_uint *ap, const mp_uint *bp, mp_size n)
{
    MP_EXPECTS(n);

    do {
        mp_uint a = ap[--n];
        mp_uint b = bp[n];

        if (a > b) {
            return 1;
        } else if (a < b) {
            return -1;
        }
    } while (n);

    return 0;
}

int mp_cmp(const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn)
{
    MP_EXPECTS(an);
    MP_EXPECTS(bn);

    return an == bn ? mp_cmp_n(ap, bp, an) : an > bn ? 1 : an < bn ? -1 : 0;
}

int mp_cmp_uint(const mp_uint *ap, mp_size an, mp_uint b)
{
    MP_EXPECTS(an);

    return mp_cmp(ap, an, &b, 1);
}

mp_size mp_has_single_bit(const mp_uint *ap, mp_size an)
{
    MP_EXPECTS(an);

    do {
        mp_uint a = ap[--an];

        if (a) {
            if (!mp_uint_has_single_bit(a)) {
                return mp_false;
            }

            while (an) {
                if (ap[--an]) {
                    return mp_false;
                }
            }

            return mp_true;
        }
    } while (an);

    return mp_false;
}

mp_size mp_countl_zero(const mp_uint *ap, mp_size an)
{
    MP_EXPECTS(an);

    mp_size n = an;
    do {
        mp_uint a = ap[--n];

        if (a) {
            return mp_uint_countl_zero(a) + MP_UINT_WIDTH + (an - n + 1);
        }
    } while (n);
    return MP_UINT_WIDTH * an;
}

mp_size mp_countl_one(const mp_uint *ap, mp_size an)
{
    MP_EXPECTS(an);

    mp_size n = an;
    do {
        mp_uint a = ap[--n];

        if (~a) {
            return mp_uint_countl_one(a) + MP_UINT_WIDTH + (an - n + 1);
        }
    } while (n);
    return MP_UINT_WIDTH * an;
}

mp_size mp_countr_zero(const mp_uint *ap, mp_size an)
{
    MP_EXPECTS(an);

    mp_size n = 0;
    do {
        mp_uint a = ap[n];

        if (a) {
            return mp_uint_countr_zero(a) + MP_UINT_WIDTH * n;
        }
    } while (++n != an);
    return MP_UINT_WIDTH * an;
}

mp_size mp_countr_one(const mp_uint *ap, mp_size an)
{
    MP_EXPECTS(an);

    mp_size n = 0;
    do {
        mp_uint a = ap[n];

        if (~a) {
            return mp_uint_countr_one(a) + MP_UINT_WIDTH * n;
        }
    } while (++n != an);
    return MP_UINT_WIDTH * an;
}

mp_size mp_bit_width(const mp_uint *ap, mp_size an)
{
    MP_EXPECTS(an);

    return an * MP_UINT_WIDTH - mp_countl_zero(ap, an);
}

mp_size mp_popcount(const mp_uint *ap, mp_size an)
{
    MP_EXPECTS(an);

    mp_size count = 0;
    do {
        count += mp_uint_popcount(ap[--an]);
    } while (an);
    return count;
}

static mp_size mp_to_bytes_le(const mp_uint *ap, mp_size an, mp_byte *bytes);

static mp_size mp_to_bytes_be(const mp_uint *ap, mp_size an, mp_byte *bytes);

mp_size mp_to_bytes(const mp_uint *ap, mp_size an, mp_byte *bytes,
                    enum mp_endian endian)
{
    if (endian == MP_ENDIAN_LITTLE) {
        return mp_to_bytes_le(ap, an, bytes);
    } else {
        return mp_to_bytes_be(ap, an, bytes);
    }
}

mp_size mp_from_bytes(
    mp_byte *bytes, mp_size byte_count, mp_uint *rp, enum mp_endian endian);

static struct mp_to_string_result mp_to_string_zero(char *first, char *last)
{
    if (last - first < 1) {
        return mp_to_string_no_mem(first);
    }

    *first++ = '0';
    return mp_to_string_ok(first);
}

static struct mp_to_string_result mp_to_string_2(
    char *first, char *last, const mp_uint *ap, mp_size an)
{
    mp_size bit_width = mp_bit_width(ap, an);

    if (!bit_width) {
        return mp_to_string_zero(first, last);
    } else if (last - first < bit_width) {
        return mp_to_string_no_mem(first);
    }

    mp_size bit_count = bit_width % MP_UINT_WIDTH;
    mp_size uint_count = bit_width / MP_UINT_WIDTH;

    while (bit_count) {
        *first++ = '0' + ((ap[uint_count] >> --bit_count) & 1);
    }

    while (uint_count) {
        mp_size pos = MP_UINT_WIDTH;

        while (pos) {
            *first++ = '0' + ((ap[--uint_count] >> --pos) & 1);
        }
    }

    return mp_to_string_ok(first);
}

static struct mp_to_string_result mp_to_string_8(
    char *first, char *last, const mp_uint *ap, mp_size an);

static struct mp_to_string_result mp_to_string_10(
    char *first, char *last, const mp_uint *ap, mp_size an);

static struct mp_to_string_result mp_to_string_16(
    char *first, char *last, const mp_uint *ap, mp_size an);

static struct mp_to_string_result mp_to_string_n(
    char *first, char *last, const mp_uint *ap, mp_size an, int base);

struct mp_to_string_result mp_to_string(
    char *first, char *last, const mp_uint *ap, mp_size an, int base)
{
    MP_EXPECTS(base >= 2 && base <= 36);

    if (!an) {
        return mp_to_string_zero(first, last);
    }

    switch (base) {
    case 2:
        return mp_to_string_2(first, last, ap, an);
    case 8:
        return mp_to_string_8(first, last, ap, an);
    case 10:
        return mp_to_string_10(first, last, ap, an);
    case 16:
        return mp_to_string_16(first, last, ap, an);
    default:
        return mp_to_string_n(first, last, ap, an, base);
    }
}

static struct mp_from_string_result mp_from_string_2(
    const char *first, const char *last, mp_uint *ap, mp_size an);

static struct mp_from_string_result mp_from_string_8(
    const char *first, const char *last, mp_uint *ap, mp_size an);

static struct mp_from_string_result mp_from_string_10(
    const char *first, const char *last, mp_uint *ap, mp_size an);

static struct mp_from_string_result mp_from_string_16(
    const char *first, const char *last, mp_uint *ap, mp_size an);

static struct mp_from_string_result mp_from_string_n(
    const char *first, const char *last, mp_uint *ap, mp_size an, int base);

struct mp_from_string_result mp_from_string(
    const char *first, const char *last, mp_uint *ap, mp_size an, int base)
{
    MP_EXPECTS(base >= 2 && base <= 36);

    switch (base) {
    case 2:
        return mp_from_string_2(first, last, ap, an);
    case 8:
        return mp_from_string_8(first, last, ap, an);
    case 10:
        return mp_from_string_10(first, last, ap, an);
    case 16:
        return mp_from_string_16(first, last, ap, an);
    default:
        return mp_from_string_n(first, last, ap, an, base);
    }
}
