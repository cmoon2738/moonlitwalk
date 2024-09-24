#ifndef _amw_h_
#define _amw_h_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define AMW_VERSION_MAJOR 0
#define AMW_VERSION_MINOR 1
#define AMW_VERSION_PATCH 1

#define amw_versionnum(major, minor, patch) \
    ((major) * 1000000 + (minor) * 1000 + (patch))

#define amw_versionnum_major(version) ((version) / 1000000)
#define amw_versionnum_minor(version) (((version) / 1000) % 1000)
#define amw_versionnum_patch(version) ((version) % 1000)

#define AMW_VERSION \
    amw_versionnum(AMW_VERSION_MAJOR, AMW_VERSION_MINOR, AMW_VERSION_PATCH)

#define AMW_CONCAT_HELPER(prefix, suffix)    prefix##suffix
#define amw_concat(prefix, suffix)           AMW_CONCAT_HELPER(prefix, suffix)

#ifndef AMW_PLATFORM_WINDOWS
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
        #define AMW_PLATFORM_WINDOWS
        #ifndef VK_USE_PLATFORM_WIN32_KHR
            #define VK_USE_PLATFORM_WIN32_KHR
        #endif
    #endif
#endif /* Windows */

#ifndef AMW_PLATFORM_UNIX
    #if defined(__unix__) || defined(__unix) || defined(unix)
        #define AMW_PLATFORM_UNIX
    #endif
#endif /* Mac, Linux, Unix, etc. */

#ifndef AMW_PLATFORM_APPLE
    #if defined(__APPLE__)
        #define AMW_PLATFORM_APPLE
        #ifndef VK_USE_PLATFORM_METAL_EXT
            #define VK_USE_PLATFORM_METAL_EXT    /* both Mac & iOS */
        #endif
        #include <TargetConditionals.h>
        #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
            #ifndef AMW_PLATFORM_IOS
                #define AMW_PLATFORM_IOS
            #endif
        #else
            #define AMW_PLATFORM_MACOSX
        #endif
    #endif
#endif /* MacOSX & iOS */

#if !defined(AMW_PLATFORM_ANDROID)
    #if defined(__ANDROID__)
        #define AMW_PLATFORM_ANDROID
        #ifndef VK_USE_PLATFORM_ANDROID_KHR
            #define VK_USE_PLATFORM_ANDROID_KHR
        #endif
    #endif
#endif /* Android */

#if !defined(AMW_PLATFORM_LINUX)
    #if defined(__linux__) || defined(__gnu_linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        #define AMW_PLATFORM_LINUX
        #if defined(AMW_NATIVE_WAYLAND)
            #ifndef VK_USE_PLATFORM_WAYLAND_KHR
                #define VK_USE_PLATFORM_WAYLAND_KHR
            #endif
        #endif
        #if defined(AMW_NATIVE_XCB)
            #ifndef VK_USE_PLATFORM_XCB_KHR
                #define VK_USE_PLATFORM_XCB_KHR
            #endif
        #endif
        #if defined(AMW_NATIVE_KMS)
            #ifndef VK_USE_PLATFORM_KMS_KHR
                #define VK_USE_PLATFORM_KMS_KHR
            #endif
        #endif
    #endif
#endif /* Linux / BSD */

#ifndef AMW_PLATFORM_EMSCRIPTEN
    #if defined(__EMSCRIPTEN__)
        #define AMW_PLATFORM_EMSCRIPTEN
    #endif
#endif /* Emscripten (HTML5) */

#if defined(__clang__) || defined(__GNUC__)
    #define AMW_INLINE __attribute__((always_inline)) inline
    #define AMW_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
    #define AMW_INLINE __forceinline
    #define AMW_NOINLINE --declspec(noinline)
#else
    #define AMW_INLINE static inline
    #define AMW_NOINLINE
#endif

#if defined(__GNUC__)
    #define AMW_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
    #define AMW_NORETURN __declspec(noreturn)
#else
    #define AMW_NORETURN
#endif

#ifndef __MACH__
    #ifndef NULL
        #ifdef __cplusplus
            #define NULL 0
        #else
            #define NULL ((void *)0)
        #endif
    #endif
#endif /* ! macOS - breaks precompiled headers */
    
#ifdef __has_builtin
    #define AMW_HAS_BUILTIN(x) __has_builtin(x)
#else
    #define AMW_HAS_BUILTIN(x) 0
#endif

#if defined(__clang__) && defined(__has_attribute)
    #if __has_attribute(target)
        #define AMW_HAS_TARGET_ATTRIBS
    #endif
#elif defined(__GNUC__) && (__GNUC__ + (__GNUC_MINOR__ >= 9) > 4) /* gcc >= 4.9 */
    #define AMW_HAS_TARGET_ATTRIBS
#elif defined(__ICC) && __ICC >= 1600
    #define AMW_HAS_TARGET_ATTRIBS
#endif

#ifdef AMW_HAS_TARGET_ATTRIBS
    #define AMW_TARGETING(x) __attribute__((target(x)))
#else
    #define AMW_TARGETING(x)
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
    #include <stdalign.h>
#endif

#if defined(_MSC_VER)
    /* do not use alignment for older visual studio versions */
    #if _MSC_VER < 1913 /*  Visual Studio 2017 version 15.6  */
        #define AMW_ALL_UNALIGNED
        #define AMW_ALIGN(X) /* no alignment */
    #else
        #define AMW_ALIGN(X) __declspec(align(X))
    #endif
#else
    #define AMW_ALIGN(X) __attribute((aligned(X)))
#endif

#ifndef AMW_ALL_UNALIGNED
    #define AMW_ALIGN_IF(X) AMW_ALIGN(X)
#else
    #define AMW_ALIGN_IF(X) /* no alignment */
#endif

#ifdef __AVX__
    #define AMW_ALIGN_MAT AMW_ALIGN(32)
#else
    #define AMW_ALIGN_MAT AMW_ALIGN(16)
#endif

#ifndef AMW_HAVE_BUILTIN_ASSUME_ALIGNED
    #if defined(__has_builtin)
        #if __has_builtin(__builtin_assume_aligned)
            #define AMW_HAVE_BUILTIN_ASSUME_ALIGNED 1
        #endif
    #elif defined(__GNUC__) && defined(__GNUC_MINOR__)
        #if __GNUC__ >= 4 && __GNUC_MINOR__ >= 7
            #define AMW_HAVE_BUILTIN_ASSUME_ALIGNED 1
        #endif
    #endif
    #ifndef AMW_HAVE_BUILTIN_ASSUME_ALIGNED
        #define AMW_HAVE_BUILTIN_ASSUME_ALIGNED 0
    #endif
#endif

#if AMW_HAVE_BUILTIN_ASSUME_ALIGNED
    #define AMW_ASSUME_ALIGNED(expr, alignment) \
        __builtin_assume_aligned((expr), (alignment))
#else
    #define AMW_ASSUME_ALIGNED(expr, alignment) (expr)
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
    #define AMW_CASTPTR_ASSUME_ALIGNED(expr, type) \
        ((type*)AMW_ASSUME_ALIGNED((expr), alignof(type)))
#elif defined(_MSC_VER)
    #define AMW_CASTPTR_ASSUME_ALIGNED(expr, type) \
        ((type*)AMW_ASSUME_ALIGNED((expr), __alignof(type)))
#else
    #define AMW_CASTPTR_ASSUME_ALIGNED(expr, type) \
        ((type*)AMW_ASSUME_ALIGNED((expr), __alignof__(type)))
#endif

#if defined(_MSC_VER)
    #if (defined(_M_AMD64) || defined(_M_X64)) || _M_IX86_FP == 2
        #ifndef __SSE__
            #define __SSE__
        #endif
        #ifndef __SSE2__
            #define __SSE2__
        #endif
    #elif _M_IX86_FP == 1
        #ifndef __SSE__
            #define __SSE__
        #endif
    #endif
    /* do not use alignment for older visual studio versions */
    #if _MSC_VER < 1913     /* Visual Studio 2017 version 15.6 */
        #ifndef MW_SIMD_ALL_UNALIGNED
            #define MW_SIMD_ALL_UNALIGNED
        #endif
    #endif
#endif /* _MSC_VER */

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    #ifdef __clang__
        #ifndef __PRFCHWINTRIN_H
            #define __PRFCHWINTRIN_H
            static __inline__ void __attribute__((__always_inline__, __nodebug__))
            _m_prefetch(void *__P) {
                __builtin_prefetch(__P, 0, 3);
            }
        #endif
    #endif
    #include <intrin.h>
#elif defined(__MINGW64_VERSION_MAJOR)
    #include <intrin.h>
#endif

#ifdef __loongarch64
    #ifndef AMW_DISABLE_LSX
        #define AMW_LSX_FP 1
        #include <lsxintrin.h>
    #endif
    #ifndef AMW_DISABLE_LASX
        #define AMW_LASX_FP 1
        #include <lasxintrin.h>
    #endif
#else
    #define AMW_LSX_FP  0
    #define AMW_LASX_FP 0
#endif

#if defined(__MMX__) && !defined(AMW_DISABLE_MMX)
    #include <mmintrin.h>
    #define AMW_MMX_FP 1
    #ifndef AMW_SIMD_X86
        #define AMW_SIMD_X86
    #endif
#else
    #define AMW_MMX_FP 0
#endif

#if defined(__SSE__) && !defined(AMW_DISABLE_SSE)
    #include <xmmintrin.h>
    #define AMW_SSE_FP 1
    #ifndef AMW_SIMD_X86
        #define AMW_SIMD_X86
    #endif
#else
    #define AMW_SSE_FP 0
#endif

#if defined(__SSE2__) && !defined(AMW_DISABLE_SSE2)
    #include <emmintrin.h>
    #define AMW_SSE2_FP 1
    #ifndef AMW_SIMD_X86
        #define AMW_SIMD_X86
    #endif
#else
    #define AMW_SSE2_FP 0
#endif

#if defined(__SSE3__) && !defined(AMW_DISABLE_SSE3)
    #include <pmmintrin.h>
    #define AMW_SSE3_FP 1
    #ifndef AMW_SIMD_X86
        #define AMW_SIMD_X86
    #endif
#else
    #define AMW_SSE3_FP 0
#endif

#if defined(__SSE4_1__) && !defined(AMW_DISABLE_SSE4_1)
    #include <smmintrin.h>
    #define AMW_SSE4_1_FP 1
    #ifndef AMW_SIMD_X86
        #define AMW_SIMD_X86
    #endif
#else
    #define AMW_SSE4_1_FP 0
#endif

#if defined(__SSE4_2__) && !defined(AMW_DISABLE_SSE4_2)
    #include <nmmintrin.h>
    #define AMW_SSE4_2_FP 1
    #ifndef AMW_SIMD_X86
        #define AMW_SIMD_X86
    #endif
#else
    #define AMW_SSE4_2_FP 0
#endif

#if defined(__AVX__) && !defined(AMW_DISABLE_AVX)
    #include <immintrin.h>
    #define AMW_AVX_FP 1
    #ifndef AMW_SIMD_X86
        #define AMW_SIMD_X86
    #endif
#else
    #define AMW_AVX_FP 0
#endif

#if (defined(_WIN32) && defined(_MSC_VER)) && !defined(AMW_DISABLE_NEON)
    #if defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC)
        #include <arm64intr.h>
        #include <arm64_neon.h>
        #ifndef AMW_NEON_FP
            #define AMW_NEON_FP  1
        #endif
        #ifndef AMW_SIMD_ARM
            #define AMW_SIMD_ARM
        #endif
    #elif defined(_M_ARM)
        #include <armintr.h>
        #include <arm_neon.h>
        #ifndef AMW_NEON_FP
            #define AMW_NEON_FP 1
        #endif
        #ifndef AMW_SIMD_ARM
            #define AMW_SIMD_ARM
        #endif
    #else
        #ifndef AMW_NEON_FP
            #define AMW_NEON_FP 0
        #endif
    #endif
#elif !defined(AMW_DISABLE_NEON)
    #if defined(__ARM_NEON) || defined(__ARM_NEON__)
        #include <arm_neon.h>
        #if defined(__ARM_NEON_FP)
            #define AMW_NEON_FP 1
        #endif
        #ifndef AMW_SIMD_ARM
            #define AMW_SIMD_ARM
        #endif
    #else
        #ifndef AMW_NEON_FP
            #define AMW_NEON_FP 0
        #endif
    #endif
#endif

#if defined(__wasm__) && defined(__wasm_simd128__)
    #ifndef AMW_SIMD_WASM
        #define AMW_SIMD_WASM
    #endif
#endif

#if defined(AMW_SIMD_X86) || defined(AMW_SIMD_ARM) || defined(AMW_SIMD_WASM)
    #ifndef AMW_SIMD
        #define AMW_SIMD
    #endif
#endif

#define AMW_LIL_ENDIAN  1234
#define AMW_BIG_ENDIAN  4321

#ifndef AMW_BYTEORDER
    #ifdef AMW_PLATFORM_LINUX
        #include <endian.h>
        #define AMW_BYTEORDER  __BYTE_ORDER
    /* predefs from newer gcc and clang versions: */
    #elif defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__) && defined(__BYTE_ORDER__)
        #if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            #define AMW_BYTEORDER   AMW_LIL_ENDIAN
        #elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            #define AMW_BYTEORDER   AMW_BIG_ENDIAN
        #else
            #error Unsupported endianness
        #endif /**/
    #else
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(__powerpc__) || defined(__PPC__) || \
            defined(__sparc__) || defined(__sparc)

            #define AMW_BYTEORDER   AMW_BIG_ENDIAN
        #else
            #define AMW_BYTEORDER   AMW_LIL_ENDIAN
        #endif
    #endif
#endif

#ifndef AMW_FLOATWORDORDER
    /* predefs from newer gcc versions: */
    #if defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__) && defined(__FLOAT_WORD_ORDER__)
        #if (__FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            #define AMW_FLOATWORDORDER   AMW_LIL_ENDIAN
        #elif (__FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__)
            #define AMW_FLOATWORDORDER   AMW_BIG_ENDIAN
        #else
            #error Unsupported endianness
        #endif
    #elif defined(__MAVERICK__)
        /* For Maverick, float words are always little-endian. */
        #define AMW_FLOATWORDORDER   AMW_LIL_ENDIAN
    #elif (defined(__arm__) || defined(__thumb__)) && !defined(__VFP_FP__) && !defined(__ARM_EABI__)
        /* For FPA, float words are always big-endian. */
        #define AMW_FLOATWORDORDER   AMW_BIG_ENDIAN
    #else
        /* By default, assume that floats words follow the memory system mode. */
        #define AMW_FLOATWORDORDER   AMW_BYTEORDER
    #endif
#endif

/* various modern compilers may have builtin swap */
#if defined(__GNUC__) || defined(__clang__)
    #define AMW_HAS_BUILTIN_BSWAP16 (AMW_HAS_BUILTIN(__builtin_bswap16)) || \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
    #define AMW_HAS_BUILTIN_BSWAP32 (AMW_HAS_BUILTIN(__builtin_bswap32)) || \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
    #define AMW_HAS_BUILTIN_BSWAP64 (AMW_HAS_BUILTIN(__builtin_bswap64)) || \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))

    /* this one is broken */
    #define AMW_HAS_BROKEN_BSWAP (__GNUC__ == 2 && __GNUC_MINOR__ <= 95)
#else
    #define AMW_HAS_BUILTIN_BSWAP16 0
    #define AMW_HAS_BUILTIN_BSWAP32 0
    #define AMW_HAS_BUILTIN_BSWAP64 0
    #define AMW_HAS_BROKEN_BSWAP 0
#endif

/* Byte swap 16-bit integer. */
#if AMW_HAS_BUILTIN_BSWAP16
    #define amw_swap16(x) __builtin_bswap16(x)
#elif (defined(_MSC_VER) && (_MSC_VER >= 1400)) && !defined(__ICL)
    #pragma intrinsic(_byteswap_ushort)
    #define amw_swap16(x) _byteswap_ushort(x)
#elif defined(__i386__) && !AMW_HAS_BROKEN_BSWAP
    AMW_INLINE uint16_t amw_swap16(uint16_t x)
    {
        __asm__("xchgb %b0,%h0": "=q"(x):"0"(x));
        return x;
    }
#elif defined(__x86_64__)
    AMW_INLINE uint16_t amw_swap16(uint16_t x)
    {
        __asm__("xchgb %b0,%h0": "=Q"(x):"0"(x));
        return x;
    }
#elif (defined(__powerpc__) || defined(__ppc__))
    AMW_INLINE uint16_t amw_swap16(uint16_t x)
    {
        int32_t result;
        __asm__("rlwimi %0,%2,8,16,23": "=&r"(result):"0"(x >> 8), "r"(x));
        return (uint16_t)result;
    }
#elif (defined(__m68k__) && !defined(__mcoldfire__))
    AMW_INLINE uint16_t amw_swap16(uint16_t x)
    {
        __asm__("rorw #8,%0": "=d"(x): "0"(x):"cc");
        return x;
    }
#elif defined(__WATCOMC__) && defined(__386__)
    extern __inline uint16_t amw_swap16(uint16_t);
    #pragma aux amw_swap16 = \
        "xchg al, ah" \
        parm   [ax]   \
        modify [ax];
#else
    AMW_INLINE uint16_t amw_swap16(uint16_t x)
    {
        return (uint16_t)((x << 8) | (x >> 8)));
    }
#endif

/* Byte swap 32-bit integer. */
#if AMW_HAS_BUILTIN_BSWAP32
    #define amw_swap32(x) __builtin_bswap32(x)
#elif (defined(_MSC_VER) && (_MSC_VER >= 1400)) && !defined(__ICL)
    #pragma intrinsic(_byteswap_ulong)
    #define amw_swap32(x) _byteswap_ulong(x)
#elif defined(__i386__) && !AMW_HAS_BROKEN_BSWAP
    AMW_INLINE uint32_t amw_swap32(uint32_t x)
    {
        __asm__("bswap %0": "=r"(x):"0"(x));
        return x;
    }
#elif defined(__x86_64__)
    AMW_INLINE uint32_t amw_swap32(uint32_t x)
    {
        __asm__("bswapl %0": "=r"(x):"0"(x));
        return x;
    }
#elif (defined(__powerpc__) || defined(__ppc__))
    AMW_INLINE uint32_t amw_swap32(uint32_t x)
    {
        uint32_t result;

        __asm__("rlwimi %0,%2,24,16,23": "=&r"(result): "0" (x>>24),  "r"(x));
        __asm__("rlwimi %0,%2,8,8,15"  : "=&r"(result): "0" (result), "r"(x));
        __asm__("rlwimi %0,%2,24,0,7"  : "=&r"(result): "0" (result), "r"(x));
        return result;
    }
#elif (defined(__m68k__) && !defined(__mcoldfire__))
    AMW_INLINE uint32_t amw_swap32(uint32_t x)
    {
        __asm__("rorw #8,%0\n\tswap %0\n\trorw #8,%0": "=d"(x): "0"(x):"cc");
        return x;
    }
#elif defined(__WATCOMC__) && defined(__386__)
    extern __inline uint32_t amw_swap32(uint32_t);
    #pragma aux amw_swap32 = \
        "bswap eax"  \
        parm   [eax] \
        modify [eax];
#else
    AMW_INLINE uint32_t amw_swap32(uint32_t x)
    {
        return (uint32_t) ((x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24)));
    }
#endif

/* Byte swap 64-bit integer. */
#if AMW_HAS_BUILTIN_BSWAP64
    #define amw_swap64(x) __builtin_bswap64(x)
#elif (defined(_MSC_VER) && (_MSC_VER >= 1400)) && !defined(__ICL)
    #pragma intrinsic(_byteswap_uint64)
    #define amw_swap64(x) _byteswap_uint64(x)
#elif defined(__i386__) && !AMW_HAS_BROKEN_BSWAP
    AMW_INLINE uint64_t amw_swap64(uint64_t x)
    {
        union {
            struct {
                uint32_t a, b;
            } s;
            uint64_t u;
        } v;
        v.u = x;
        __asm__("bswapl %0 ; bswapl %1 ; xchgl %0,%1"
                : "=r"(v.s.a), "=r"(v.s.b)
                : "0" (v.s.a),  "1"(v.s.b));
        return v.u;
    }
#elif defined(__x86_64__)
    AMW_INLINE uint64_t amw_swap64(uint64_t x)
    {
        __asm__("bswapq %0": "=r"(x):"0"(x));
        return x;
    }
#elif defined(__WATCOMC__) && defined(__386__)
    extern __inline uint64_t amw_swap64(uint64_t);
    #pragma aux amw_swap64 = \
        "bswap eax"     \
        "bswap edx"     \
        "xchg eax,edx"  \
        parm [eax edx]  \
        modify [eax edx];
#else
    AMW_INLINE uint64_t amw_swap64(uint64_t x)
    {
        uint32_t hi, lo;

        /* Separate into high and low 32-bit values and swap them */
        lo = (uint32_t)(x & 0xFFFFFFFF);
        x >>= 32;
        hi = (uint32_t)(x & 0xFFFFFFFF);
        x = amw_swap32(lo);
        x <<= 32;
        x |= amw_swap32(hi);
        return (x);
    }
#endif

AMW_INLINE float amw_swap_float(float x)
{
    union {
        float f;
        uint32_t ui32;
    } swapper;
    swapper.f = x;
    swapper.ui32 = amw_swap32(swapper.ui32);
    return swapper.f;
}

/* remove extra macros */
#undef AMW_HAS_BROKEN_BSWAP
#undef AMW_HAS_BUILTIN_BSWAP16
#undef AMW_HAS_BUILTIN_BSWAP32
#undef AMW_HAS_BUILTIN_BSWAP64

#if MW_BYTEORDER == MW_LIL_ENDIAN
    #define amw_swap16LE(x)     (x)
    #define amw_swap32LE(x)     (x)
    #define amw_swap64LE(x)     (x)
    #define amw_swap_floatLE(x) (x)
    #define amw_swap16BE(x)     amw_swap16(x)
    #define amw_swap32BE(x)     amw_swap32(x)
    #define amw_swap64BE(x)     amw_swap64(x)
    #define amw_swap_floatBE(x) amw_swap_float(x)
#else
    #define amw_swap16LE(x)     amw_swap16(x)
    #define amw_swap32LE(x)     amw_swap32(x)
    #define amw_swap64LE(x)     amw_swap64(x)
    #define amw_swap_floatLE(x) amw_swap_float(x)
    #define amw_swap16BE(x)     (x)
    #define amw_swap32BE(x)     (x)
    #define amw_swap64BE(x)     (x)
    #define amw_swap_floatBE(x) (x)
#endif       

#define amw_arraysize(array) (sizeof(array)/sizeof(array[0]))
#define amw_clamp(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))
#define amw_min(x, y) (((x) < (y)) ? (x) : (y))
#define amw_max(x, y) (((x) > (y)) ? (x) : (y))
#define amw_swap(type, a, b)   \
    {                           \
        type temp = a;          \
        a = b;                  \
        b = temp;               \
    }

#define amw_malloc  malloc
#define amw_calloc  calloc
#define amw_realloc realloc
#define amw_free    free

#define amw_memset  memset
#define amw_memcpy  memcpy
#define amw_memmove memmove
#define amw_memcmp  memcmp

#define amw_zero(x)  amw_memset(&(x), 0, sizeof((x)))
#define amw_zerop(x) amw_memset((x), 0, sizeof(*(x)))
#define amw_zeroa(x) amw_memset((x), 0, sizeof((x)))

#define AMW_FALSE 0
#define AMW_TRUE 1
#ifndef __cplusplus
    typedef _Bool bool;
#endif

typedef int32_t                 ivec2_t[2];
typedef int32_t                 ivec3_t[3];
typedef int32_t                 ivec4_t[4];

typedef float                   vec2_t[2];
typedef float                   vec3_t[3];
typedef AMW_ALIGN_IF(16) float  vec4_t[4];

/* quaternion of tensor 1 */
typedef vec4_t                  versor_t;     /* |x, y, z, w| -> w is the last */

typedef AMW_ALIGN_IF(16) vec2_t mat2_t[2];
typedef vec3_t                  mat2x3_t[2];  /* [col (2), row (3)] */
typedef vec4_t                  mat2x4_t[2];  /* [col (2), row (4)] */

typedef vec3_t                  mat3_t[3];
typedef vec2_t                  mat3x2_t[3];  /* [col (3), row (2)] */
typedef vec4_t                  mat3x4_t[3];  /* [col (3), row (4)] */

typedef AMW_ALIGN_MAT vec4_t    mat4_t[4];
typedef vec2_t                  mat4x2_t[4];  /* [col (4), row (2)] */
typedef vec3_t                  mat4x3_t[4];  /* [col (4), row (3)] */

#define AMW_E         2.71828182845904523536028747135266250   /* e           */
#define AMW_LOG2E     1.44269504088896340735992468100189214   /* log2(e)     */
#define AMW_LOG10E    0.434294481903251827651128918916605082  /* log10(e)    */
#define AMW_LN2       0.693147180559945309417232121458176568  /* loge(2)     */
#define AMW_LN10      2.30258509299404568401799145468436421   /* loge(10)    */
#define AMW_PI        3.14159265358979323846264338327950288   /* pi          */
#define AMW_PI_2      1.57079632679489661923132169163975144   /* pi/2        */
#define AMW_PI_4      0.785398163397448309615660845819875721  /* pi/4        */
#define AMW_1_PI      0.318309886183790671537767526745028724  /* 1/pi        */
#define AMW_2_PI      0.636619772367581343075535053490057448  /* 2/pi        */
#define AMW_2_SQRTPI  1.12837916709551257389615890312154517   /* 2/sqrt(pi)  */
#define AMW_SQRT2     1.41421356237309504880168872420969808   /* sqrt(2)     */
#define AMW_SQRT1_2   0.707106781186547524400844362104849039  /* 1/sqrt(2)   */

#define AMW_Ef        ((float)AMW_E)
#define AMW_LOG2Ef    ((float)AMW_LOG2E)
#define AMW_LOG10Ef   ((float)AMW_LOG10E)
#define AMW_LN2f      ((float)AMW_LN2)
#define AMW_LN10f     ((float)AMW_LN10)
#define AMW_PIf       ((float)AMW_PI)
#define AMW_PI_2f     ((float)AMW_PI_2)
#define AMW_PI_4f     ((float)AMW_PI_4)
#define AMW_1_PIf     ((float)AMW_1_PI)
#define AMW_2_PIf     ((float)AMW_2_PI)
#define AMW_2_SQRTPIf ((float)AMW_2_SQRTPI)
#define AMW_SQRT2f    ((float)AMW_SQRT2)
#define AMW_SQRT1_2f  ((float)AMW_SQRT1_2)

#ifdef __GNUC__
    #define AMW_PRINTF_FORMAT(fmtargnumber) __attribute__((format(__printf__, fmtargnumber, fmtargnumber + 1)))
#else
    #define AMW_PRINTF_FORMAT(fmtargnumber)
#endif

#ifndef AMW_LOG_USE_COLOR
    #define AMW_LOG_USE_COLOR 1
#endif

#ifndef AMW_LOG_DISABLE_FUNCTION
    #define __AMW_FUNCTION__ __FUNCTION__
#else
    #define __AMW_FUNCTION__ NULL
#endif

#ifndef AMW_LOG_DISABLE_FILE
    #ifndef AMW_LOG_FULL_FILEPATH
        #ifdef AMW_PLATFORM_WINDOWS
            #define __AMW_FILE__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
        #else
            #define __AMW_FILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
        #endif
    #else
        /* if a full path should be logged */
        #define __AMW_FILENAME__ __FILE__
    #endif /* AMW_LOG_FULL_FILEPATH */
#else
    #define __AMW_FILENAME__ NULL
#endif

#ifndef AMW_LOG_DISABLE_LINE
    #define __AMW_LINE__ __LINE__
#else
    #define __AMW_LINE__ 0
#endif

typedef struct amw_log {
    va_list     ap;
    const char *fmt;
    const char *function;
    const char *file;
    struct tm  *time;
    void       *userdata;
    int32_t     line;
    int32_t     level;
} amw_log_t;

typedef enum amw_log_level {
    AMW_LOG_VERBOSE = 0,
    AMW_LOG_DEBUG,
    AMW_LOG_INFO,
    AMW_LOG_WARN,
    AMW_LOG_ERROR,
    AMW_LOG_FATAL,
} amw_log_level_t;

void    amw_log_init(void *output);
void    amw_log_terminate(void);

void    amw_log_message(int32_t level, const char *function, const char *file, int32_t line, const char *fmt, ...) AMW_PRINTF_FORMAT(5);
void    amw_log_raw(char *fmt, ...) AMW_PRINTF_FORMAT(1);
char   *amw_log_va(char *fmt, ...) AMW_PRINTF_FORMAT(1);

int32_t amw_log_level(void);
void    amw_log_set_level(int32_t level);
void    amw_log_set_quiet(bool silence);

#ifndef AMW_LOG_DISABLE_OUTPUT
#define amw_log_raw(...)     amw_log_raw(__VA_ARGS__)
#define amw_log_verbose(...) amw_log_message(AMW_LOG_VERBOSE, __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_debug(...)   amw_log_message(AMW_LOG_DEBUG,   __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_info(...)    amw_log_message(AMW_LOG_INFO,    __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_warn(...)    amw_log_message(AMW_LOG_WARN,    __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_error(...)   amw_log_message(AMW_LOG_ERROR,   __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_fatal(...)   amw_log_message(AMW_LOG_FATAL,   __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#else
#define amw_log_raw(...) 
#define amw_log_verbose(...) 
#define amw_log_debug(...) 
#define amw_log_info(...)  
#define amw_log_warn(...)  
#define amw_log_error(...) 
#define amw_log_fatal(...) 
#endif

#define AMW_MS_PER_SECOND   1000
#define AMW_US_PER_SECOND   1000000
#define AMW_NS_PER_SECOND   1000000000LL
#define AMW_NS_PER_MS       1000000
#define AMW_NS_PER_US       1000
#define AMW_SECONDS_TO_NS(S)    (((uint64_t)(S)) * AMW_NS_PER_SECOND)
#define AMW_NS_TO_SECONDS(NS)   ((NS) / AMW_NS_PER_SECOND)
#define AMW_MS_TO_NS(MS)        (((uint64_t)(MS)) * AMW_NS_PER_MS)
#define AMW_NS_TO_MS(NS)        ((NS) / AMW_NS_PER_MS)
#define AMW_US_TO_NS(US)        (((uint64_t)(US)) * AMW_NS_PER_US)
#define AMW_NS_TO_US(NS)        ((NS) / AMW_NS_PER_US)

#if defined static_assert
    #define amw_static_assert static_assert
/* it's ugly, but should work as a fallback */
#else
    /* C11 or later */
    #if defined _Static_assert
        #define amw_static_assert _Static_assert

    /* GCC 4.6 or later */
    #elif defined(__GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && defined __GNUC_MINOR__ && __GNUC_MINOR >= 6)
        /* It will work but it will throw a warning:
         * warning: ISO C90 does not support '_Static_assert' [-Wpedantic] */
        #define amw_static_assert _Static_assert
    #else
        #if defined __COUNTER__
            #define amw_static_assert(cond,msg)                      \
                struct amw_concat(__amw_static_assert_, __COUNTER__) \
                {                                                    \
                    char                                             \
                    amw_static_assertion                             \
                    [2*(cond)-1];                                    \
                } amw_concat(__amw_static_assert_, __COUNTER__)
        #else /* fallback to inscope assertion */
            #define amw_static_assert(cond,msg)                      \
                {                                                    \
                    char                                             \
                    amw_static_assertion                             \
                    [2*(cond)-1];                                    \
                    (void)amw_static_assertion;                      \
                }
        #endif
    #endif
#endif

#ifndef AMW_ASSERT_LEVEL
    #ifdef AMW_DEFAULT_ASSERT_LEVEL
        #define AMW_ASSERT_LEVEL AMW_DEFAULT_ASSERT_LEVEL
    #elif defined(AMW_DEBUG) || defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__))
        #define AMW_ASSERT_LEVEL 2
    #else
        #define AMW_ASSERT_LEVEL 1
    #endif
#endif

#if defined(_MSC_VER)
    extern void __cdecl __debugbreak(void);
    #define amw_debug_breakpoint() __debugbreak()
#elif AMW_HAS_BUILTIN(__builtin_debugtrap)
    #define amw_debug_breakpoint() __builtin_debugtrap()
#elif ( (!defined(__NACL__)) && ((defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))) )
    #define amw_debug_breakpoint() __asm__ __volatile__ ( "int $3\n\t" )
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__riscv)
    #define amw_debug_breakpoint() __asm__ __volatile__ ( "ebreak\n\t" )
#elif ( defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__)) )
    #define amw_debug_breakpoint() __asm__ __volatile__ ( "brk #22\n\t" )
#elif defined(__APPLE__) && defined(__arm__)
    #define amw_debug_breakpoint() __asm__ __volatile__ ( "bkpt #22\n\t" )
#elif defined(_WIN32) && ((defined(__GNUC__) || defined(__clang__)) && (defined(__arm64__) || defined(__aarch64__)) )
    #define amw_debug_breakpoint() __asm__ __volatile__ ( "brk #0xF000\n\t" )
#elif defined(__386__) && defined(__WATCOMC__)
    #define amw_debug_breakpoint() { _asm { int 0x03 } }
#else
    #define amw_debug_breakpoint()
#endif

#define amw_disabled_assert(condition)
#define amw_enabled_assert(condition)                            \
    do {                                                         \
        if (! (condition)) {                                     \
            amw_log_fatal("Assertion '%s' failed.", #condition); \
            amw_debug_breakpoint();                              \
        }                                                        \
    } while (AMW_FALSE)

/* This assertion is never disabled at any level. */
#define amw_assert_always(condition) amw_enabled_assert(condition)

#if AMW_ASSERT_LEVEL == 0   /* assertions disabled */
    #define amw_assert(condition)          amw_disabled_assert(condition)
    #define amw_assert_release(condition)  amw_disabled_assert(condition)
    #define amw_assert_paranoid(condition) amw_disabled_assert(condition)
#elif AMW_ASSERT_LEVEL == 1  /* release settings. */
    #define amw_assert(condition)          amw_disabled_assert(condition)
    #define amw_assert_release(condition)  amw_enabled_assert(condition)
    #define amw_assert_paranoid(condition) amw_disabled_assert(condition)
#elif AMW_ASSERT_LEVEL == 2  /* normal settings. */
    #define amw_assert(condition)          amw_enabled_assert(condition)
    #define amw_assert_release(condition)  amw_enabled_assert(condition)
    #define amw_assert_paranoid(condition) amw_disabled_assert(condition)
#elif AMW_ASSERT_LEVEL == 3  /* paranoid settings. */
    #define amw_assert(condition)          amw_enabled_assert(condition)
    #define amw_assert_release(condition)  amw_enabled_assert(condition)
    #define amw_assert_paranoid(condition) amw_enabled_assert(condition)
#else
    #error Unknown assertion level.
#endif

/**
 * A compiler barrier prevents the compiler from reordering reads and writes
 * to globally visible variables across the call.
 *
 * This macro only prevents the compiler from reordering reads and writes, it 
 * does not prevent the CPU from reordering reads and writes. However, all of
 * the atomic operations that modify memory are full memory barriers.
 */
#if defined(_MSC_VER) && (_MSC_VER > 1200) && !defined(__clang__)
    void _ReadWriteBarrier(void);
    #pragma intrinsic(_ReadWriteBarrier)
    #define amw_compiler_barrier() _ReadWriteBarrier()
#elif (defined(__GNUC__) && !defined(MW_PLATFORM_EMSCRIPTEN)) || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x5120))
    #define amw_compiler_barrier() __asm__ __volatile__ ("" : : : "memory")
#elif defined(__WATCOMC__)
    extern __inline void mw_compiler_barrier(void);
    #pragma aux amw_compiler_barrier = "" parm [] modify exact [];
#else
    /* TODO implement spinlocks
    #define amw_compiler_barrier() \
        { amw_spinlock_t _tmp = 0; amw_spinlock_lock(&_tmp); amw_spinlock_unlock(&_tmp); }
    */
#endif

/**
 * Memory barriers (aka 'fences') are designed to prevent reads and writes from being
 * reordered by the compiler and being seen out of order on multi-core CPUs.
 * They work by selectively prohibiting reordering of memory operations across the 
 * barrier. They also provide some synchronization among threads.
 *
 * A typical pattern would be for thread A to write some data and a flag, and 
 * for thread B to read the flag and get the data. In this case you would insert 
 * a release barrier between writing the data and the flag, guaranteeing that 
 * the data write completes no later than the flag is written, and you would 
 * insert an acquire barrier between reading the flag and reading the data, 
 * to ensure that all the reads associated with the flag have completed.
 *
 * In this pattern you should always see a release barrier paired with an 
 * acquire barrier and you should gate the data reads/writes with a single flag 
 * variable. For more information on these semantics, take a look at the blog post:
 * http://preshing.com/20120913/acquire-and-release-semantics
 */
#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
    #define amw_memory_barrier_release()   __asm__ __volatile__ ("lwsync" : : : "memory")
    #define amw_memory_barrier_acquire()   __asm__ __volatile__ ("lwsync" : : : "memory")
#elif defined(__GNUC__) && defined(__aarch64__)
    #define amw_memory_barrier_release()   __asm__ __volatile__ ("dmb ish" : : : "memory")
    #define amw_memory_barrier_acquire()   __asm__ __volatile__ ("dmb ish" : : : "memory")
#elif defined(__GNUC__) && defined(__arm__)
    #if defined(AMW_PLATFORM_LINUX) || defined(AMW_PLATFORM_ANDROID)
    /* Information from:
       https://chromium.googlesource.com/chromium/chromium/+/trunk/base/atomicops_internals_arm_gcc.h#19
       The Linux kernel provides a helper function which provides the right code for a memory barrier,
       hard-coded at address 0xffff0fa0 */

        typedef void (*PFN_amw_kernel_memory_barrier_func)();
        #define amw_memory_barrier_release()	((PFN_amw_kernel_memory_barrier_func)0xffff0fa0)()
        #define amw_memory_barrier_acquire()	((PFN_amw_kernel_memory_barrier_func)0xffff0fa0)()
    #else
        #if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__) || defined(__ARM_ARCH_8A__)
            #define amw_memory_barrier_release()   __asm__ __volatile__ ("dmb ish" : : : "memory")
            #define amw_memory_barrier_acquire()   __asm__ __volatile__ ("dmb ish" : : : "memory")
        #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
            #define amw_memory_barrier_release()   __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r"(0) : "memory")
            #define amw_memory_barrier_acquire()   __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r"(0) : "memory")
        #else
            #define amw_memory_barrier_release()   __asm__ __volatile__ ("" : : : "memory")
            #define amw_memory_barrier_acquire()   __asm__ __volatile__ ("" : : : "memory")
        #endif /* AMW_PLATFORM_LINUX || AMW_PLATFORM_ANDROID */
    #endif /* __GNUC__ && __arm__ */
#else
    #if (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x5120)) /* solaris */
        #include <mbarrier.h>
        #define amw_memory_barrier_release()  __machine_rel_barrier()
        #define amw_memory_barrier_acquire()  __machine_acq_barrier()
    #else
    /* This is correct for the x86 and x64 CPUs. */
        #define amw_memory_barrier_release()  amw_compiler_barrier()
        #define amw_memory_barrier_acquire()  amw_compiler_barrier()
    #endif
#endif

/**
 * A macro to insert a CPU-specific "pause" instruction into the program.
 *
 * This can be useful in busy-wait loops, as it serves as a hint to the CPU 
 * as to the program's intent. Some CPUs can use this to do more efficient 
 * processing. On some platforms it will not do anything.
 *
 * Note that if you are busy-waiting, there may be other more-efficient
 * approaches with other synchronization primitives: mutexes, semaphores, etc.
 */
#if (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    #define amw_cpu_pause_instruction() __asm__ __volatile__("pause\n")  /* some assemblers can't do REP NOP, so PAUSE. */
#elif (defined(__arm__) && defined(__ARM_ARCH) && __ARM_ARCH >= 7) || defined(__aarch64__)
    #define amw_cpu_pause_instruction() __asm__ __volatile__("yield" ::: "memory")
#elif (defined(__powerpc__) || defined(__powerpc64__))
    #define amw_cpu_pause_instruction() __asm__ __volatile__("or 27,27,27");
#elif (defined(__riscv) && __riscv_xlen == 64)
    #define amw_cpu_pause_instruction() __asm__ __volatile__(".insn i 0x0F, 0, x0, x0, 0x010");
#elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
    #define amw_cpu_pause_instruction() _mm_pause()  /* this is actually "rep nop" and not a SIMD instruction. No inline asm in MSVC x86-64! */
#elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
    #define amw_cpu_pause_instruction() __yield()
#elif defined(__WATCOMC__) && defined(__386__)
    extern __inline void amw_cpu_pause_instruction(void);
    #pragma aux amw_cpu_pause_instruction = ".686p" ".xmm2" "pause"
#else
    #define amw_cpu_pause_instruction()
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_h_ */
