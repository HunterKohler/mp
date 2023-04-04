#ifndef MP_CONFIG_H_
#define MP_CONFIG_H_

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

#if defined(__GNUC__) || defined(__GNUG__)
#define MP_GCC 1
#endif

#if defined(__clang__)
#define MP_CLANG 1
#endif

#if defined(_MSC_VER)
#define MP_MSVC 1
#endif

#if defined(__has_builtin)
#define MP_HAS_BUILTIN(x) __has_builtin(x)
#else
#define MP_HAS_BUILTIN(x) 0
#endif

#if defined(__has_cpp_attribute)
#define MP_HAS_ATTR(x) __has_cpp_attribute(x)
#else
#define MP_HAS_ATTR(x) 0
#endif

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define MP_WINDOWS 1
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__bsdi__) || defined(__DragonFly__)
#define MP_BSD 1
#endif

#if defined(__linux__) || defined(__linux)
#define MP_LINUX 1
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define MP_MACH 1
#endif

#if defined(__ANDROID__)
#define MP_ANDROID
#endif

#if !defined(MP_INLINE_ASM)
#define MP_INLINE_ASM 1
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || \
    defined(__amd64) || defined(_M_AMD64)
#define MP_ARCH_X86_64 1
#elif defined(__i386__) || defined(__i386) || defined(_M_I86) || \
    defined(_M_IX86)
#define MP_ARCH_x86 1
#elif defined(__mips__) || defined(__mips)
#define MP_ARCH_MIPS 1
#elif defined(__powerpc) || defined(__powerpc__) || defined(_M_PPC)
#define MP_ARCH_POWERPC 1
#elif defined(__aarch64__)
#define MP_ARCH_ARM64 1
#elif defined(__arm__) || defined(_M_ARM)
#define MP_ARCH_ARM 1
#elif defined(__riscv)
#define MP_ARCH_RISCV 1
#elif defined(__ia64__) || defined(__ia64) || defined(_IA64) || \
    defined(__IA64__) || defined(__itanium__) || defined(_M_IA64)
#define MP_ARCH_IA64 1
#elif defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#define MP_ARCH_ALPHA 1
#endif

#if MP_ARCH_X86_64
#define MP_INT_TYPE int64_t
#define MP_INT_WIDTH 64
#define MP_INT_MAX INT64_MAX
#define MP_INT_MIN INT64_MIN
#define MP_UINT_TYPE uint64_t
#define MP_UINT_WIDTH 64
#define MP_UINT_MAX UINT64_MAX
#elif MP_ARCH_X86
#define MP_INT_TYPE int64_t
#define MP_INT_WIDTH 64
#define MP_INT_MAX INT64_MAX
#define MP_INT_MIN INT64_MIN
#define MP_UINT_TYPE uint64_t
#define MP_UINT_WIDTH 64
#define MP_UINT_MAX UINT64_MAX
#elif MP_ARCH_MIPS
#define MP_INT_TYPE int32_t
#define MP_INT_WIDTH 32
#define MP_INT_MAX INT32_MAX
#define MP_INT_MIN INT32_MIN
#define MP_UINT_TYPE uint32_t
#define MP_UINT_WIDTH 32
#define MP_UINT_MAX UINT32_MAX
#elif LONG_WIDTH && ULONG_WIDTH
#define MP_INT_TYPE long
#define MP_INT_WIDTH LONG_WIDTH
#define MP_INT_MAX LONG_MAX
#define MP_INT_MIN LONG_MIN
#define MP_UINT_TYPE unsigned long
#define MP_UINT_WIDTH ULONG_WIDTH
#define MP_UINT_MAX ULONG_MAX
#else
#error "mp: compiler and architecture unsupported"
#endif

#define MP_SIZE_TYPE size_t
#define MP_SSIZE_TYPE ssize_t
#define MP_FLOAT_TYPE long double
#define MP_BOOL_TYPE _Bool

#define MP_EXPECTS(x) assert((x))
#define MP_ENSURES(x) assert((x))

#endif
