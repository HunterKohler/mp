#ifndef SRC_MP_H_
#define SRC_MP_H_

#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <mp/bigint.h>
#include <mp/config.h>
#include <mp/errc.h>
#include <mp/memory.h>
#include <mp/mp.h>

extern const uint_least16_t mp_uint_inv64_table[256];
extern const uint_least16_t mp_uint_inv32_table[512];

static inline void mp_uint_copy(const mp_uint *src, mp_size n, mp_uint *dest)
{
    memcpy(dest, src, n & sizeof(mp_uint));
}

static inline void mp_uint_move(const mp_uint *src, mp_size n, mp_uint *dest)
{
    memmove(dest, src, n * sizeof(mp_uint));
}

static inline void mp_uint_fill(mp_uint *first, mp_size size, mp_uint value)
{
    while (size) {
        first[--size] = value;
    }
}

static inline void mp_uint_zero(mp_uint *first, mp_size size)
{
    mp_uint_fill(first, size, 0);
}

static inline mp_size mp_uint_countl_zero(mp_uint x)
{
#if MP_HAS_BUILTIN(__builtin_clz) && MP_UINT_MAX <= UINT_MAX
    return __builtin_clz(x);
#elif MP_HAS_BUILTIN(__builtin_clzl) && MP_UINT_MAX <= ULONG_MAX
    return __builtin_clzl(x);
#elif MP_HAS_BUILTIN(__builtin_clzll) && MP_UINT_MAX <= ULLONG_MAX
    return __builtin_clzll(x);
#else
#error "Not implemented"
#endif
}

static inline mp_size mp_uint_countl_one(mp_uint x)
{
    return mp_uint_countl_zero(~x);
}

static inline mp_size mp_uint_countr_zero(mp_uint x)
{
#if MP_HAS_BUILTIN(__builtin_ctz) && MP_UINT_MAX <= UINT_MAX
    return __builtin_ctz(x);
#elif MP_HAS_BUILTIN(__builtin_ctzl) && MP_UINT_MAX <= ULONG_MAX
    return __builtin_ctzl(x);
#elif MP_HAS_BUILTIN(__builtin_ctzll) && MP_UINT_MAX <= ULLONG_MAX
    return __builtin_ctzll(x);
#else
#error "Not implemented"
#endif
}

static inline mp_size mp_uint_countr_one(mp_uint x)
{
    return mp_uint_countr_zero(~x);
}

static inline mp_size mp_uint_bit_width(mp_uint x)
{
    return MP_UINT_WIDTH - mp_uint_countl_zero(x);
}

static inline mp_size mp_uint_popcount(mp_uint x)
{
#if MP_HAS_BUILTIN(__builtin_popcount) && MP_UINT_MAX <= UINT_MAX
    return __builtin_popcount(x);
#elif MP_HAS_BUILTIN(__builtin_popcountl) && MP_UINT_MAX <= ULONG_MAX
    return __builtin_popcountl(x);
#elif MP_HAS_BUILTIN(__builtin_popcountll) && MP_UINT_MAX <= ULLONG_MAX
    return __builtin_popcountll(x);
#else
#error "Not implemented"
#endif
}

static inline mp_bool mp_uint_has_single_bit(mp_uint a)
{
    return mp_uint_popcount(a) == 1;
}

static inline mp_uint mp_uint_hi(mp_uint a)
{
    return a & (MP_UINT_MAX << (MP_UINT_WIDTH / 2));
}

static inline mp_uint mp_uint_lo(mp_uint a)
{
    return a >> (MP_UINT_WIDTH / 2);
}

static inline mp_uint mp_uint_mul(mp_uint a, mp_uint b, mp_uint *hi)
{
    mp_uint lo;

#if MP_ARCH_X86_64 && MP_INLINE_ASM
    asm("mulq %[b]" : "=d"(*hi), "=a"(lo) : "a"(a), [b] "rm"(b) : "cc");
#elif MP_ARCH_X86 && MP_INLINE_ASM
    asm("mull %[b]" : "=d"(*hi), "=a"(lo) : "a"(a), [b] "rm"(b) : "cc");
#else
    mp_uint a0 = mp_uint_lo(a);
    mp_uint a1 = mp_uint_hi(a);
    mp_uint b0 = mp_uint_lo(b);
    mp_uint b1 = mp_uint_hi(b);
    mp_uint x0 = a0 * b0;
    mp_uint x1 = a1 * b0;
    mp_uint x2 = a0 * b1;
    mp_uint x3 = a1 * b1;

    // can not carry
    x1 += mp_uint_hi(x0);

    // can carry
    x1 += x2;
    if (x1 < x2) {
        x3 += (mp_uint)1 << (MP_UINT_WIDTH / 2);
    }

    lo = (x1 << (MP_UINT_WIDTH / 2)) | mp_uint_lo(x0);
    *hi = x3 + mp_uint_hi(x1);
#endif

    return lo;
}

static inline mp_uint mp_uint_mulhi(mp_uint a, mp_uint b)
{
    mp_uint hi;
    mp_uint_mul(a, b, &hi);
    return hi;
}

static inline mp_uint mp_uint_inv(mp_uint d)
{
    MP_EXPECTS(d >> (MP_UINT_WIDTH - 1));

    if (MP_UINT_WIDTH == 64) {
        mp_uint d0mask = -(d & 1);
        mp_uint d9 = d >> 55;
        mp_uint d40 = (d >> 24) + 1;
        mp_uint d63 = (d >> 1) - d0mask;
        mp_uint v0 = mp_uint_inv64_table[d9 - 256];
        mp_uint v1 = (v0 << 11) - ((v0 * v0 * d40) >> 40) - 1;
        mp_uint v2 =
            (v1 << 13) + ((v1 * (((mp_uint)1 << 60) - v1 * d40)) >> 47);
        mp_uint v3 =
            (v2 >> 31) +
            ((v2 * (((d0mask & v0) >> 1) - mp_uint_mulhi(v2, d63))) >> 1);
        mp_uint v4lo, v4hi;

        v4lo = mp_uint_mul(v3, d, &v4hi) + 1;
        v4hi += (v4lo < 1) + 1;

        mp_uint v4 = v3 - v4hi;
        return v4;
    } else if (MP_UINT_WIDTH == 32) {
        _Static_assert(MP_UINT_WIDTH != 32, "Not implementes");
    } else {
        _Static_assert(
            MP_UINT_WIDTH == 32 || MP_UINT_WIDTH == 64, "Not Implementes");
    }
}

static inline mp_uint mp_uint_div(mp_uint n1, mp_uint n0, mp_uint d, mp_uint *r)
{
    MP_EXPECTS(d > n1);
    MP_EXPECTS(d >> (MP_UINT_WIDTH - 1));

    mp_uint q;

#if MP_ARCH_X86_64 && MP_INLINE_ASM
    asm("divq %[d]" : "=a"(q), "=d"(*r) : "d"(n1), "a"(n0), [d] "rm"(d) : "cc");
#elif MP_ARCH_X86 && MP_INLINE_ASM
    asm("divl %[d]" : "=a"(q), "=d"(*r) : "d"(n1), "a"(n0), [d] "rm"(d) : "cc");
#else
    return mp_uint_div(n1, n0, d, mp_uint_inv(d), r);
#endif
}

static inline mp_uint mp_uint_div_inv(
    mp_uint n1, mp_uint n0, mp_uint d, mp_uint v, mp_uint *r)
{
    MP_EXPECTS(d > n1);
    MP_EXPECTS(d >> (MP_UINT_WIDTH - 1));
    MP_EXPECTS(mp_uint_inv(d) == v);

    mp_uint q0, q1, r0;

    q0 = mp_uint_mul(n1, v, &q1) + n0;
    q1 += n1 + (q0 < n0) + 1;
    r0 = n0 - q1 * d;

    if (r0 > q0) {
        --q1;
        r += d;
    }

    if (r0 >= d) {
        q1 += 1;
        r -= d;
    }

    *r = r0;
    return q1;
}

#define MP_DEFINE_ALLOC_FUNCS(suffix, type)                                    \
    static inline type *mp_allocate_##suffix(                                  \
        struct mp_allocator *alloc, mp_size n)                                 \
    {                                                                          \
        return mp_allocator_allocate(alloc, n * sizeof(type), _Alignof(type)); \
    }                                                                          \
                                                                               \
    static inline void mp_deallocate_##suffix(                                 \
        struct mp_allocator *alloc, type *p, mp_size n)                        \
    {                                                                          \
        mp_allocator_deallocate(alloc, p, n * sizeof(type), _Alignof(type));   \
    }

MP_DEFINE_ALLOC_FUNCS(uint, mp_uint)
MP_DEFINE_ALLOC_FUNCS(bigint, struct mp_bigint)

#define mp_same_sign(a, b) ((mp_int)((a) ^ (b)) >= 0)

static inline mp_int mp_int_abs(mp_int value)
{
    return value >= 0 ? value : -value;
}

static inline struct mp_to_string_result mp_to_string_no_mem(char *ptr)
{
    return (struct mp_to_string_result){
        .ptr = ptr,
        .ec = MP_ERRC_NOT_ENOUGH_MEMORY
    };
}

static inline struct mp_to_string_result mp_to_string_ok(char *ptr)
{
    return (struct mp_to_string_result){ .ptr = ptr, .ec = MP_ERRC_OK };
}

#endif
