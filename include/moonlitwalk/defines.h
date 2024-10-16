#ifndef _amw_defines_h_
#define _amw_defines_h_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>

#ifdef __cplusplus
    #define AMW_C_DECL_BEGIN extern "C" {
    #define AMW_C_DECL_END }
    #define AMW_C_DECL extern "C"
#else
    #define AMW_C_DECL_BEGIN 
    #define AMW_C_DECL_END
    #define AMW_C_DECL
#endif

AMW_C_DECL_BEGIN

#define AMW_VERSION_MAJOR 0
#define AMW_VERSION_MINOR 1
#define AMW_VERSION_REVISION 3

#define amw_version(major, minor, revision) \
    ((major) * 1000000 + (minor) * 1000 + (revision))

#define AMW_VERSION amw_version(AMW_VERSION_MAJOR, AMW_VERSION_MINOR, AMW_VERSION_REVISION)

#define amw_version_major(version) ((version) / 1000000)
#define amw_version_minor(version) (((version) / 1000) % 1000)
#define amw_version_revision(version) ((version) % 1000)

#define AMW_STRINGIFY_HELPER(x)     #x
#define amw_stringify(x)            AMW_STRINGIFY_HELPER(x)

#define AMW_CONCAT_HELPER(a, b)     a##b
#define amw_concat(a, b)            AMW_CONCAT_HELPER(a, b)

#define AMW_CONCAT3_HELPER(a, b, c) a##b##c
#define amw_concat3(a, b, c)        AMW_CONCAT3_HELPER(a, b, c)

#ifndef AMW_PLATFORM_WINDOWS
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
        #define AMW_PLATFORM_WINDOWS
        #ifndef VK_USE_PLATFORM_WIN32_KHR
            #define VK_USE_PLATFORM_WIN32_KHR
        #endif
    #endif
#endif
#ifndef AMW_PLATFORM_UNIX
    #if defined(__unix__) || defined(__unix) || defined(unix)
        #define AMW_PLATFORM_UNIX
    #endif
#endif
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
#endif
#ifndef AMW_PLATFORM_ANDROID
    #if defined(__ANDROID__)
        #define AMW_PLATFORM_ANDROID
        #ifndef VK_USE_PLATFORM_ANDROID_KHR
            #define VK_USE_PLATFORM_ANDROID_KHR
        #endif
    #endif
#endif
#ifndef AMW_PLATFORM_EMSCRIPTEN
    #if defined(__EMSCRIPTEN__)
        #define AMW_PLATFORM_EMSCRIPTEN
    #endif
#endif 
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
#endif

#if defined(__GNUC__) && defined(__GNUC_PATCHLEVEL__)
    #define AMW_GNUC_VERSION amw_version(__GNUC__, __GNUC__MINOR__, __GNUC_PATCHLEVEL__)
#elif defined(__GNUC__)
    #define AMW_GNUC_VERSION amw_version(__GNUC__, __GNUC_MINOR__, 0)
#endif
#if defined(AMW_GNUC_VERSION)
    #define AMW_GNUC_VERSION_CHECK(ma,mi,p) (AMW_GNUC_VERSION >= amw_version(ma,mi,p))
#else
    #define AMW_GNUC_VERSION_CHECK(ma,mi,p) (0)
#endif

#if defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 140000000) && !defined(__ICL)
    #define AMW_MSVC_VERSION amw_version(_MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, (_MSC_FULL_VER % 100000) / 100)
#elif defined(_MSC_FULL_VER) && !defined(__ICL)
    #define AMW_MSVC_VERSION amw_version(_MSC_FULL_VER / 1000000, (_MSC_FULL_VER % 1000000) / 10000, (_MSC_FULL_VER % 10000) / 100)
#elif defined(_MSC_VER) && !defined(__ICL)
    #define AMW_MSVC_VERSION amw_version(_MSC_VER / 100, _MSC_VER % 100, 0)
#endif
#if !defined(AMW_MSVC_VERSION)
    #define AMW_MSVC_VERSION_CHECK(ma,mi,p) (0)
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
    #define AMW_MSVC_VERSION_CHECK(ma,mi,p) (_MSC_FULL_VER >= ((ma * 10000000) + (mi * 100000) + (p)))
#elif defined(_MSC_VER) && (_MSC_VER >= 1200)
    #define AMW_MSVC_VERSION_CHECK(ma,mi,p) (_MSC_FULL_VER >= ((ma * 1000000) + (mi * 10000) + (p)))
#else
    #define AMW_MSVC_VERSION_CHECK(ma,mi,p) (_MSC_VER >= ((ma * 100) + (mi)))
#endif

/* DLL export */
#ifndef AMW_API
    #ifdef AMW_DLL_EXPORT
        #if defined(AMW_PLATFORM_WINDOWS) || defined(__CYGWIN__)
            #define AMW_API extern __declspec(dllexport)
        #elif defined(__GNUC__) && __GNUC__ >= 4
            #define AMW_API extern __attribute__ ((visibility("default")))
        #endif
    #else
        #define AMW_API extern
    #endif
#endif

/* By default use the C calling convention */
#ifndef AMW_CALL
    #if defined(AMW_PLATFORM_WINDOWS) && !defined(__GNUC__)
        #define AMW_CALL __cdecl
    #else
        #define AMW_CALL
    #endif
#endif

#if !defined(AMW_INLINE) && !defined(AMW_NOINLINE)
    #if defined(__clang__) || defined(__GNUC__)
        #define AMW_INLINE __attribute__((always_inline)) inline
        #define AMW_NOINLINE __attribute__((noinline))
    #elif defined(_MSC_VER)
        #define AMW_INLINE __forceinline
        #define AMW_NOINLINE __declspec(noinline)
    #else
        #define AMW_INLINE static inline
        #define AMW_NOINLINE
    #endif
#endif

#ifndef AMW_NORETURN
    #if defined(__GNUC__)
        #define AMW_NORETURN __attribute__((noreturn))
    #elif defined(_MSC_VER)
        #define AMW_NORETURN __declspec(noreturn)
    #else
        #define AMW_NORETURN
    #endif
#endif

#ifndef AMW_UNUSED
    #ifdef __GNUC__
        #define AMW_UNUSED __attribute((unused))
    #else
        #define AMW_UNUSED
    #endif
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

#ifdef __has_feature
    #define AMW_HAS_FEATURE(x) __has_feature(x)
#else
    #define AMW_HAS_FEATURE(x) 0
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

#if !defined(AMW_ALIGN_MAXIMUM)
    #if defined(AMW_MSVC_VERSION)
        #if AMW_MSVC_VERSION_CHECK(19, 16, 0)
            /* vs 2017 or newer dont need this */
        #else
            #if defined(_M_IX86) || defined(_M_AMD64)
                #if AMW_MSVC_VERSION_CHECK(19, 14, 0)
                    #define AMW_ALIGN_PLATFORM_MAXIMUM 64
                #elif AMW_MSVC_VERSION_CHECK(16, 0, 0)
                    #define AMW_ALIGN_PLATFORM_MAXIMUM 32
                #else
                    #define AMW_ALIGN_PLATFORM_MAXIMUM 16
                #endif
            #elif defined(_M_ARM) || defined(_M_ARM64)
                #define AMW_ALIGN_PLATFORM_MAXIMUM 8
            #endif
        #endif
    #elif defined(AMW_IBM_VERSION)
        #define AMW_ALIGN_PLATFORM_MAXIMUM 16
    #endif
#endif

#if defined(AMW_ALIGN_PLATFORM_MAXIMUM)
    #if AMW_ALIGN_PLATFORM_MAXIMUM >= 64
        #define AMW_ALIGN_64_ 64
        #define AMW_ALIGN_32_ 32
        #define AMW_ALIGN_16_ 16
        #define AMW_ALIGN_8_  8
    #elif AMW_ALIGN_PLATFORM_MAXIMUM >= 32
        #define AMW_ALIGN_64_ 32
        #define AMW_ALIGN_32_ 32
        #define AMW_ALIGN_16_ 16
        #define AMW_ALIGN_8_  8
    #elif AMW_ALIGN_PLATFORM_MAXIMUM >= 16
        #define AMW_ALIGN_64_ 16
        #define AMW_ALIGN_32_ 16
        #define AMW_ALIGN_16_ 16
        #define AMW_ALIGN_8_  8
    #elif AMW_ALIGN_PLATFORM_MAXIMUM >= 8
        #define AMW_ALIGN_64_ 8
        #define AMW_ALIGN_32_ 8
        #define AMW_ALIGN_16_ 8
        #define AMW_ALIGN_8_  8
    #endif
#else
    #define AMW_ALIGN_64_ 64
    #define AMW_ALIGN_32_ 32
    #define AMW_ALIGN_16_ 16
    #define AMW_ALIGN_8_  8
#endif

#if defined(AMW_ALIGN_MAXIMUM)
    #define AMW_ALIGN_CAP(Alignment) (((Alignment) < (AMW_ALIGN_PLATFORM_MAXIMUM)) ? (Alignment) : (AMW_ALIGN_PLATFORM_MAXIMUM))
#else
    #define AMW_ALIGN_CAP(Alignment) (Alignment)
#endif

#if __has_attribute(__aligned__) 
    #define AMW_ALIGN_TO(Alignment) __attribute__((__aligned__(AMW_ALIGN_CAP(Alignment))))
#elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
    #define AMW_ALIGN_TO(Alignment) _Alignas(AMW_ALIGN_CAP(Alignment))
#elif (defined(__cplusplus) && (__cplusplus >= 201103L))
    #define AMW_ALIGN_TO(Alignment) alignas(AMW_ALIGN_CAP(Alignment))
#elif defined(AMW_MSVC_VERSION)
    #define AMW_ALIGN_TO(Alignment) __declspec(align(Alignment))
#endif
#define AMW_ALIGN_TO_64 AMW_ALIGN_TO(AMW_ALIGN_64_)
#define AMW_ALIGN_TO_32 AMW_ALIGN_TO(AMW_ALIGN_32_)
#define AMW_ALIGN_TO_16 AMW_ALIGN_TO(AMW_ALIGN_16_)
#define AMW_ALIGN_TO_8  AMW_ALIGN_TO(AMW_ALIGN_8_)

#ifdef __AVX__
    #define AMW_ALIGN_MAT AMW_ALIGN_TO_32
#else
    #define AMW_ALIGN_MAT AMW_ALIGN_TO_16
#endif

#ifndef AMW_HAVE_BUILTIN_ASSUME_ALIGNED
    #if AMW_HAS_BUILTIN(__builtin_assume_aligned)
        #define AMW_HAVE_BUILTIN_ASSUME_ALIGNED 1
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

#ifdef __GNUC__
    #define AMW_PRINTF_FORMAT(fmtargnumber) __attribute__((format(__printf__, fmtargnumber, fmtargnumber + 1)))
#else
    #define AMW_PRINTF_FORMAT(fmtargnumber)
#endif

#ifdef __cplusplus
    #define amw_cast_ptr(ptr) (decltype(ptr))
#else
    #define amw_cast_ptr(...)
#endif

#define AMW_FALSE 0
#define AMW_TRUE 1
#ifndef __cplusplus
typedef _Bool bool;
#endif

/* Alpha
   <https://en.wikipedia.org/wiki/DEC_Alpha> */
#if defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
    #if defined(__alpha_ev6__)
        #define AMW_ARCH_ALPHA 6
    #elif defined(__alpha_ev5__)
        #define AMW_ARCH_ALPHA 5
    #elif defined(__alpha_ev4__)
        #define AMW_ARCH_ALPHA 4
    #else
        #define AMW_ARCH_ALPHA 1
    #endif
#endif
#if defined(AMW_ARCH_ALPHA)
    #define AMW_ARCH_ALPHA_CHECK(version) ((version) <= AMW_ARCH_ALPHA)
#else
    #define AMW_ARCH_ALPHA_CHECK(version) (0)
#endif

/* Atmel AVR
   <https://en.wikipedia.org/wiki/Atmel_AVR> */
#if defined(__AVR_ARCH__)
    #define AMW_ARCH_AVR __AVR_ARCH__
#endif

/* AMD64 / x86_64
   <https://en.wikipedia.org/wiki/X86-64> */
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
    #if !defined(_M_ARM64EC)
        #define AMW_ARCH_AMD64 1000
    #endif
#endif

/* ARM
   <https://en.wikipedia.org/wiki/ARM_architecture> */
#if defined(__ARM_ARCH)
    #if __ARM_ARCH > 100
        #define AMW_ARCH_ARM (__ARM_ARCH)
    #else
        #define AMW_ARCH_ARM (__ARM_ARCH * 100)
    #endif
#elif defined(_M_ARM)
    #if _M_ARM > 100
        #define AMW_ARCH_ARM (_M_ARM)
    #else
        #define AMW_ARCH_ARM (_M_ARM * 100)
    #endif
#elif defined(_M_ARM64) || defined(_M_ARM64EC)
    #define AMW_ARCH_ARM 800
#elif defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(_ARM) || defined(_M_ARM) || defined(_M_ARM)
    #define AMW_ARCH_ARM 1
    #if defined(AMW_MSVC_VERSION)
        #include <armintr.h>
    #endif
#endif
#if defined(AMW_ARCH_ARM)
    #define AMW_ARCH_ARM_CHECK(major, minor) (((major * 100) + (minor)) <= AMW_ARCH_ARM)
#else
    #define AMW_ARCH_ARM_CHECK(major, minor) (0)
#endif

/* AArch64
   <https://en.wikipedia.org/wiki/ARM_architecture> */
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    #define AMW_ARCH_AARCH64 1000
    #if defined(AMW_MSVC_VERSION)
        #include <arm64intr.h>
    #endif
#endif
#if defined(AMW_ARCH_AARCH64)
    #define AMW_ARCH_AARCH64_CHECK(version) ((version) <= AMW_ARCH_AARCH64)
#else
    #define AMW_ARCH_AARCH64_CHECK(version) (0)
#endif

/* ARM SIMD ISA extensions */
#if defined(__ARM_NEON) || defined(AMW_ARCH_AARCH64)
    #if defined(AMW_ARCH_AARCH64)
        #define AMW_ARCH_ARM_NEON AMW_ARCH_AARCH64
        #include <arm64_neon.h>
    #elif defined(AMW_ARCH_ARM)
        #define AMW_ARCH_ARM_NEON AMW_ARCH_ARM
        #include <arm_neon.h>
    #endif
#endif
#if defined(__ARM_FEATURE_AES) && __ARM_FEATURE_AES
    #define AMW_ARCH_ARM_AES
#endif
#if defined(__ARM_FEATURE_COMPLEX) && __ARM_FEATURE_COMPLEX
    #define AMW_ARCH_ARM_COMPLEX
#endif
#if defined(__ARM_FEATURE_CRYPTO) && __ARM_FEATURE_CRYPTO
    #define AMW_ARCH_ARM_CRYPTO
#endif
#if defined(__ARM_FEATURE_CRC32) && __ARM_FEATURE_CRC32
    #define AMW_ARCH_ARM_CRC32
#endif
#if defined(__ARM_FEATURE_DOTPROD) && __ARM_FEATURE_DOTPROD
    #define AMW_ARCH_ARM_DOTPROD
#endif
#if defined(__ARM_FEATURE_FMA) && __ARM_FEATURE_FMA
    #define AMW_ARCH_ARM_FMA
#endif
#if defined(__ARM_FEATURE_FP16_FML) && __ARM_FEATURE_FP16_FML
    #define AMW_ARCH_ARM_FP16_FML
#endif
#if defined(__ARM_FEATURE_FRINT) && __ARM_FEATURE_FRINT
    #define AMW_ARCH_ARM_FRINT
#endif
#if defined(__ARM_FEATURE_MATMUL_INT8) && __ARM_FEATURE_MATMUL_INT8
    #define AMW_ARCH_ARM_MATMUL_INT8
#endif
#if defined(__ARM_FEATURE_SHA2) && __ARM_FEATURE_SHA2 && !defined(__APPLE_CC__)
    #define AMW_ARCH_ARM_SHA2
#endif
#if defined(__ARM_FEATURE_SHA3) && __ARM_FEATURE_SHA3
    #define AMW_ARCH_ARM_SHA3
#endif
#if defined(__ARM_FEATURE_SHA512) && __ARM_FEATURE_SHA512
    #define AMW_ARCH_ARM_SHA512
#endif
#if defined(__ARM_FEATURE_SM3) && __ARM_FEATURE_SM3
    #define AMW_ARCH_ARM_SM3
#endif
#if defined(__ARM_FEATURE_SM4) && __ARM_FEATURE_SM4
    #define AMW_ARCH_ARM_SM4
#endif
#if defined(__ARM_FEATURE_SVE) && __ARM_FEATURE_SVE
    #define AMW_ARCH_ARM_SVE
#endif
#if defined(__ARM_FEATURE_QRDMX) && __ARM_FEATURE_QRDMX
    #define AMW_ARCH_ARM_QRDMX
#endif

/* Blackfin
   <https://en.wikipedia.org/wiki/Blackfin> */
#if defined(__bfin) || defined(__BFIN__) || defined(__bfin__)
    #define AMW_ARCH_BLACKFIN 1
#endif

/* CRIS
   <https://en.wikipedia.org/wiki/ETRAX_CRIS> */
#if defined(__CRIS_arch_version)
    #define AMW_ARCH_CRIS __CRIS_arch_version
#elif defined(__cris__) || defined(__cris) || defined(__CRIS) || defined(__CRIS__)
    #define AMW_ARCH_CRIS 1
#endif

/* Convex
   <https://en.wikipedia.org/wiki/Convex_Computer> */
#if defined(__convex_c38__)
    #define AMW_ARCH_CONVEX 38
#elif defined(__convex_c34__)
    #define AMW_ARCH_CONVEX 34
#elif defined(__convex_c32__)
    #define AMW_ARCH_CONVEX 32
#elif defined(__convex_c2__)
    #define AMW_ARCH_CONVEX 2
#elif defined(__convex__)
    #define AMW_ARCH_CONVEX 1
#endif
#if defined(AMW_ARCH_CONVEX)
    #define AMW_ARCH_CONVEX_CHECK(version) ((version) <= AMW_ARCH_CONVEX)
#else
    #define AMW_ARCH_CONVEX_CHECK(version) (0)
#endif

/* Adapteva Epiphany
   <https://en.wikipedia.org/wiki/Adapteva_Epiphany> */
#if defined(__epiphany__)
    #define AMW_ARCH_EPIPHANY 1
#endif

/* Fujitsu FR-V
   <https://en.wikipedia.org/wiki/FR-V_(microprocessor)> */
#if defined(__frv__)
    #define AMW_ARCH_FRV 1
#endif

/* H8/300
   <https://en.wikipedia.org/wiki/H8_Family> */
#if defined(__H8300__)
    #define AMW_ARCH_H8300
#endif

/* Elbrus (8S, 8SV and successors)
   <https://en.wikipedia.org/wiki/Elbrus-8S> */
#if defined(__e2k__)
    #define AMW_ARCH_E2K
#endif

/* HP/PA / PA-RISC
   <https://en.wikipedia.org/wiki/PA-RISC> */
#if defined(__PA8000__) || defined(__HPPA20__) || defined(__RISC2_0__) || defined(_PA_RISC2_0)
    #define AMW_ARCH_HPPA 20
#elif defined(__PA7100__) || defined(__HPPA11__) || defined(_PA_RISC1_1)
    #define AMW_ARCH_HPPA 11
#elif defined(_PA_RISC1_0)
    #define AMW_ARCH_HPPA 10
#elif defined(__hppa__) || defined(__HPPA__) || defined(__hppa)
    #define AMW_ARCH_HPPA 1
#endif
#if defined(AMW_ARCH_HPPA)
    #define AMW_ARCH_HPPA_CHECK(version) ((version) <= AMW_ARCH_HPPA)
#else
    #define AMW_ARCH_HPPA_CHECK(version) (0)
#endif

/* x86
   <https://en.wikipedia.org/wiki/X86> */
#if defined(_M_IX86)
    #define AMW_ARCH_X86 (_M_IX86 / 100)
#elif defined(__I86__)
    #define AMW_ARCH_X86 __I86__
#elif defined(i686) || defined(__i686) || defined(__i686__)
    #define AMW_ARCH_X86 6
#elif defined(i586) || defined(__i586) || defined(__i586__)
    #define AMW_ARCH_X86 5
#elif defined(i486) || defined(__i486) || defined(__i486__)
    #define AMW_ARCH_X86 4
#elif defined(i386) || defined(__i386) || defined(__i386__)
    #define AMW_ARCH_X86 3
#elif defined(_X86_) || defined(__X86__) || defined(__THW_INTEL__)
    #define AMW_ARCH_X86 3
#endif
#if defined(AMW_ARCH_X86)
    #define AMW_ARCH_X86_CHECK(version) ((version) <= SIMDE_ARCH_X86)
#else
    #define AMW_ARCH_X86_CHECK(version) (0)
#endif

/* SIMD ISA extensions for x86/x86_64 and Elbrus */
#if defined(AMW_ARCH_X86) || defined(AMW_ARCH_AMD64) || defined(AMW_ARCH_E2K)
    #if defined(_M_IX86_FP)
        #define AMW_ARCH_X86_MMX 1
        #include <mmintrin.h>
        #if (_M_IX86_FP >= 1)
            #define AMW_ARCH_X86_SSE 1
            #include <xmmintrin.h>
        #endif
        #if (_M_IX86_FP >= 2)
            #define AMW_ARCH_X86_SSE2 1
            #include <emmintrin.h>
        #endif
    #elif defined(_M_X64)
        #define AMW_ARCH_X86_SSE 1
        #define AMW_ARCH_X86_SSE2 1
        #include <xmmintrin.h>
        #include <emmintrin.h>
    #else
        #if defined(__MMX__)
            #define AMW_ARCH_X86_MMX 1
            #include <mmintrin.h>
        #endif
        #if defined(__SSE__)
            #define AMW_ARCH_X86_SSE 1
            #include <xmmintrin.h>
        #endif
        #if defined(__SSE2__)
            #define AMW_ARCH_X86_SSE2 1
            #include <emmintrin.h>
        #endif
    #endif
    #if defined(__SSE3__)
        #define AMW_ARCH_X86_SSE3 1
        #include <pmmintrin.h>
    #endif
    #if defined(__SSSE3__)
        #define AMW_ARCH_X86_SSSE3 1
        #include <tmmintrin.h>
    #endif
    #if defined(__SSE4_1__)
        #define AMW_ARCH_X86_SSE4_1 1
        #include <smmintrin.h>
    #endif
    #if defined(__SSE4_2__)
        #define AMW_ARCH_X86_SSE4_2 1
        #include <nmmintrin.h>
    #endif
    #if defined(__SSE4A__)
        #define AMW_ARCH_X86_SSE4A 1
        #include <ammintrin.h>
    #endif
    #if defined(__XOP__)
        #define AMW_ARCH_X86_XOP 1
        #include <xopintrin.h>
    #endif
    #if defined(__AVX__)
        #define AMW_ARCH_X86_AVX 1
        #if !defined(AMW_ARCH_X86_SSE3)
            #define AMW_ARCH_X86_SSE3 1
            #include <pmmintrin.h>
        #endif
        #if !defined(AMW_ARCH_X86_SSE4_1)
            #define AMW_ARCH_X86_SSE4_1 1
            #include <smmintrin.h>
        #endif
        #if !defined(AMW_ARCH_X86_SSE4_2)
            #define AMW_ARCH_X86_SSE4_2 1
            #include <nmmintrin.h>
        #endif
        #include <immintrin.h>
    #endif
    #if defined(__AVX2__)
        #define AMW_ARCH_X86_AVX2 1
        #if defined(_MSC_VER)
            #define AMW_ARCH_X86_FMA 1
        #endif
        #include <immintrin.h>
    #endif
    #if defined(__FMA__)
        #define AMW_ARCH_X86_FMA 1
        #if !defined(AMW_ARCH_X86_AVX)
            #define AMW_ARCH_X86_AVX 1
        #endif
        #include <immintrin.h>
    #endif
    #if defined(__AVX512VP2INTERSECT__)
        #define AMW_ARCH_X86_AVX512VP2INTERSECT 1
    #endif
    #if defined(__AVX512BITALG__)
        #define AMW_ARCH_X86_AVX512BITALG 1
    #endif
    #if defined(__AVX512VPOPCNTDQ__)
        #define AMW_ARCH_X86_AVX512VPOPCNTDQ 1
    #endif
    #if defined(__AVX512VBMI__)
        #define AMW_ARCH_X86_AVX512VBMI 1
    #endif
    #if defined(__AVX512VBMI2__)
        #define AMW_ARCH_X86_AVX512VBMI2 1
    #endif
    #if defined(__AVX512VNNI__)
        #define AMW_ARCH_X86_AVX512VNNI 1
    #endif
    #if defined(__AVX5124VNNIW__)
        #define AMW_ARCH_X86_AVX5124VNNIW 1
    #endif
    #if defined(__AVX512BW__)
        #define AMW_ARCH_X86_AVX512BW 1
    #endif
    #if defined(__AVX512BF16__)
        #define AMW_ARCH_X86_AVX512BF16 1
    #endif
    #if defined(__AVX512CD__)
        #define AMW_ARCH_X86_AVX512CD 1
    #endif
    #if defined(__AVX512DQ__)
        #define AMW_ARCH_X86_AVX512DQ 1
    #endif
    #if defined(__AVX512F__)
        #define AMW_ARCH_X86_AVX512F 1
    #endif
    #if defined(__AVX512VL__)
        #define AMW_ARCH_X86_AVX512VL 1
    #endif
    #if defined(__AVX512FP16__)
        #define AMW_ARCH_X86_AVX512FP16 1
    #endif
    #if defined(__GFNI__)
        #define AMW_ARCH_X86_GFNI 1
    #endif
    #if defined(__PCLMUL__)
        #define AMW_ARCH_X86_PCLMUL 1
    #endif
    #if defined(__VPCLMULQDQ__)
        #define AMW_ARCH_X86_VPCLMULQDQ 1
    #endif
    #if (defined(__F16C__) || AMW_MSVC_VERSION_CHECK(19,30,0)) && defined(SIMDE_ARCH_X86_AVX2)
        #define AMW_ARCH_X86_F16C 1
    #endif
    #if defined(__AES__)
        #define AMW_ARCH_X86_AES 1
    #endif
#endif

/* Itanium
   <https://en.wikipedia.org/wiki/Itanium> */
#if defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(__ia64) || defined(_M_IA64) || defined(__itanium__)
    #define AMW_ARCH_IA64 1
#endif

/* Renesas M32R
   <https://en.wikipedia.org/wiki/M32R> */
#if defined(__m32r__) || defined(__M32R__)
    #define AMW_ARCH_M32R
#endif

/* Motorola 68000
   <https://en.wikipedia.org/wiki/Motorola_68000> */
#if defined(__mc68060__) || defined(__MC68060__)
    #define AMW_ARCH_M68K 68060
#elif defined(__mc68040__) || defined(__MC68040__)
    #define AMW_ARCH_M68K 68040
#elif defined(__mc68030__) || defined(__MC68030__)
    #define AMW_ARCH_M68K 68030
#elif defined(__mc68020__) || defined(__MC68020__)
    #define AMW_ARCH_M68K 68020
#elif defined(__mc68010__) || defined(__MC68010__)
    #define AMW_ARCH_M68K 68010
#elif defined(__mc68000__) || defined(__MC68000__)
    #define AMW_ARCH_M68K 68000
#endif
#if defined(MDE_ARCH_M68K)
    #define AMW_ARCH_M68K_CHECK(version) ((version) <= AMW_ARCH_M68K)
#else
    #define AMW_ARCH_M68K_CHECK(version) (0)
#endif

/* Xilinx MicroBlaze
   <https://en.wikipedia.org/wiki/MicroBlaze> */
#if defined(__MICROBLAZE__) || defined(__microblaze__)
    #define AMW_ARCH_MICROBLAZE
#endif

/* MIPS
   <https://en.wikipedia.org/wiki/MIPS_architecture> */
#if defined(_MIPS_ISA_MIPS64R2)
    #define AMW_ARCH_MIPS 642
#elif defined(_MIPS_ISA_MIPS64)
    #define AMW_ARCH_MIPS 640
#elif defined(_MIPS_ISA_MIPS32R2)
    #define AMW_ARCH_MIPS 322
#elif defined(_MIPS_ISA_MIPS32)
    #define AMW_ARCH_MIPS 320
#elif defined(_MIPS_ISA_MIPS4)
    #define AMW_ARCH_MIPS 4
#elif defined(_MIPS_ISA_MIPS3)
    #define AMW_ARCH_MIPS 3
#elif defined(_MIPS_ISA_MIPS2)
    #define AMW_ARCH_MIPS 2
#elif defined(_MIPS_ISA_MIPS1)
    #define AMW_ARCH_MIPS 1
#elif defined(_MIPS_ISA_MIPS) || defined(__mips) || defined(__MIPS__)
    #define AMW_ARCH_MIPS 1
#endif
#if defined(AMW_ARCH_MIPS)
    #define AMW_ARCH_MIPS_CHECK(version) ((version) <= AMW_ARCH_MIPS)
#else
    #define AMW_ARCH_MIPS_CHECK(version) (0)
#endif
#if defined(__mips_loongson_mmi)
    #define AMW_ARCH_MIPS_LOONGSON_MMI 1
#endif
#if defined(__mips_msa)
    #define AMW_ARCH_MIPS_MSA 1
#endif

/* Matsushita MN10300
   <https://en.wikipedia.org/wiki/MN103> */
#if defined(__MN10300__) || defined(__mn10300__)
    #define AMW_ARCH_MN10300 1
#endif

/* POWER
   <https://en.wikipedia.org/wiki/IBM_POWER_Instruction_Set_Architecture> */
#if defined(_M_PPC)
    #  define AMW_ARCH_POWER _M_PPC
#elif defined(_ARCH_PWR9)
    #  define AMW_ARCH_POWER 900
#elif defined(_ARCH_PWR8)
    #  define AMW_ARCH_POWER 800
#elif defined(_ARCH_PWR7)
    #  define AMW_ARCH_POWER 700
#elif defined(_ARCH_PWR6)
    #  define AMW_ARCH_POWER 600
#elif defined(_ARCH_PWR5)
    #  define AMW_ARCH_POWER 500
#elif defined(_ARCH_PWR4)
    #  define AMW_ARCH_POWER 400
#elif defined(_ARCH_440) || defined(__ppc440__)
    #  define AMW_ARCH_POWER 440
#elif defined(_ARCH_450) || defined(__ppc450__)
    #  define AMW_ARCH_POWER 450
#elif defined(_ARCH_601) || defined(__ppc601__)
    #  define AMW_ARCH_POWER 601
#elif defined(_ARCH_603) || defined(__ppc603__)
    #  define AMW_ARCH_POWER 603
#elif defined(_ARCH_604) || defined(__ppc604__)
    #  define AMW_ARCH_POWER 604
#elif defined(_ARCH_605) || defined(__ppc605__)
    #  define AMW_ARCH_POWER 605
#elif defined(_ARCH_620) || defined(__ppc620__)
    #  define AMW_ARCH_POWER 620
#elif defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC) || defined(__ppc)
    #  define AMW_ARCH_POWER 1
#endif
#if defined(AMW_ARCH_POWER)
    #define AMW_ARCH_POWER_CHECK(version) ((version) <= AMW_ARCH_POWER)
#else
    #define AMW_ARCH_POWER_CHECK(version) (0)
#endif
#if defined(__ALTIVEC__)
    #define AMW_ARCH_POWER_ALTIVEC SIMDE_ARCH_POWER
    #define AMW_ARCH_POWER_ALTIVEC_CHECK(version) ((version) <= AMW_ARCH_POWER)
#else
    #define AMW_ARCH_POWER_ALTIVEC_CHECK(version) (0)
#endif

/* RISC-V
   <https://en.wikipedia.org/wiki/RISC-V> */
#if defined(__riscv) || defined(__riscv__)
    #if __riscv_xlen == 64
        #define AMW_ARCH_RISCV64
    #elif __riscv_xlen == 32
        #define AMW_ARCH_RISCV32
    #endif
#endif

/* RISC-V SIMD ISA extensions */
#if defined(__riscv_zve32x)
    #define AMW_ARCH_RISCV_ZVE32X 1
#endif
#if defined(__riscv_zve32f)
    #define AMW_ARCH_RISCV_ZVE32F 1
#endif
#if defined(__riscv_zve64x)
    #define AMW_ARCH_RISCV_ZVE64X 1
#endif
#if defined(__riscv_zve64f)
    #define AMW_ARCH_RISCV_ZVE64F 1
#endif
#if defined(__riscv_zve64d)
    #define AMW_ARCH_RISCV_ZVE64D 1
#endif
#if defined(__riscv_v)
    #define AMW_ARCH_RISCV_V 1
#endif
#if defined(__riscv_zvfh)
    #define AMW_ARCH_RISCV_ZVFH 1
#endif
#if defined(__riscv_zvfhmin)
    #define AMW_ARCH_RISCV_ZVFHMIN 1
#endif

/* SPARC
   <https://en.wikipedia.org/wiki/SPARC> */
#if defined(__sparc_v9__) || defined(__sparcv9)
    #define AMW_ARCH_SPARC 9
#elif defined(__sparc_v8__) || defined(__sparcv8)
    #define AMW_ARCH_SPARC 8
#elif defined(__sparc_v7__) || defined(__sparcv7)
    #define AMW_ARCH_SPARC 7
#elif defined(__sparc_v6__) || defined(__sparcv6)
    #define AMW_ARCH_SPARC 6
#elif defined(__sparc_v5__) || defined(__sparcv5)
    #define AMW_ARCH_SPARC 5
#elif defined(__sparc_v4__) || defined(__sparcv4)
    #define AMW_ARCH_SPARC 4
#elif defined(__sparc_v3__) || defined(__sparcv3)
    #define AMW_ARCH_SPARC 3
#elif defined(__sparc_v2__) || defined(__sparcv2)
    #define AMW_ARCH_SPARC 2
#elif defined(__sparc_v1__) || defined(__sparcv1)
    #define AMW_ARCH_SPARC 1
#elif defined(__sparc__) || defined(__sparc)
    #define AMW_ARCH_SPARC 1
#endif
#if defined(AMW_ARCH_SPARC)
    #define AMW_ARCH_SPARC_CHECK(version) ((version) <= AMW_ARCH_SPARC)
#else
    #define AMW_ARCH_SPARC_CHECK(version) (0)
#endif

/* SuperH
   <https://en.wikipedia.org/wiki/SuperH> */
#if defined(__sh5__) || defined(__SH5__)
    #define AMW_ARCH_SUPERH 5
#elif defined(__sh4__) || defined(__SH4__)
    #define AMW_ARCH_SUPERH 4
#elif defined(__sh3__) || defined(__SH3__)
    #define AMW_ARCH_SUPERH 3
#elif defined(__sh2__) || defined(__SH2__)
    #define AMW_ARCH_SUPERH 2
#elif defined(__sh1__) || defined(__SH1__)
    #define AMW_ARCH_SUPERH 1
#elif defined(__sh__) || defined(__SH__)
    #define AMW_ARCH_SUPERH 1
#endif

/* IBM System z
   <https://en.wikipedia.org/wiki/IBM_System_z> */
#if defined(__370__) || defined(__THW_370__) || defined(__s390__) || defined(__s390x__) || defined(__zarch__) || defined(__SYSC_ZARCH__)
    #define AMW_ARCH_ZARCH __ARCH__
#endif
#if defined(AMW_ARCH_ZARCH)
    #define AMW_ARCH_ZARCH_CHECK(version) ((version) <= AMW_ARCH_ZARCH)
#else
    #define AMW_ARCH_ZARCH_CHECK(version) (0)
#endif

#if defined(AMW_ARCH_ZARCH) && defined(__VEC__)
  #define AMW_ARCH_ZARCH_ZVECTOR AMW_ARCH_ZARCH
#endif

/* TMS320 DSP
   <https://en.wikipedia.org/wiki/Texas_Instruments_TMS320> */
#if defined(_TMS320C6740) || defined(__TMS320C6740__)
    #define AMW_ARCH_TMS320 6740
#elif defined(_TMS320C6700_PLUS) || defined(__TMS320C6700_PLUS__)
    #define AMW_ARCH_TMS320 6701
#elif defined(_TMS320C6700) || defined(__TMS320C6700__)
    #define AMW_ARCH_TMS320 6700
#elif defined(_TMS320C6600) || defined(__TMS320C6600__)
    #define AMW_ARCH_TMS320 6600
#elif defined(_TMS320C6400_PLUS) || defined(__TMS320C6400_PLUS__)
    #define AMW_ARCH_TMS320 6401
#elif defined(_TMS320C6400) || defined(__TMS320C6400__)
    #define AMW_ARCH_TMS320 6400
#elif defined(_TMS320C6200) || defined(__TMS320C6200__)
    #define AMW_ARCH_TMS320 6200
#elif defined(_TMS320C55X) || defined(__TMS320C55X__)
    #define AMW_ARCH_TMS320 550
#elif defined(_TMS320C54X) || defined(__TMS320C54X__)
    #define AMW_ARCH_TMS320 540
#elif defined(_TMS320C28X) || defined(__TMS320C28X__)
    #define AMW_ARCH_TMS320 280
#endif
#if defined(AMW_ARCH_TMS320)
  #define AMW_ARCH_TMS320_CHECK(version) ((version) <= AMW_ARCH_TMS320)
#else
  #define AMW_ARCH_TMS320_CHECK(version) (0)
#endif

/* WebAssembly */
#if defined(__wasm__)
    #define AMW_ARCH_WASM 1
#endif
#if defined(AMW_ARCH_WASM) && defined(__wasm_simd128__)
    #define AMW_ARCH_WASM_SIMD128
    #include <wasm_simd128.h>
#endif
#if defined(AMW_ARCH_WASM) && defined(__wasm_relaxed_simd__)
    #define AMW_ARCH_WASM_RELAXED_SIMD
#endif

/* Xtensa
   <https://en.wikipedia.org/wiki/> */
#if defined(__xtensa__) || defined(__XTENSA__)
    #define AMW_ARCH_XTENSA 1
#endif
/* Availability of 16-bit floating-point arithmetic intrinsics */
#if defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)
    #define AMW_ARCH_ARM_NEON_FP16
#endif
/* Availability of 16-bit brain floating-point arithmetic intrinsics */
#if defined(__ARM_FEATURE_BF16_VECTOR_ARITHMETIC)
    #define AMW_ARCH_ARM_NEON_BF16
#endif

/* LoongArch
   <https://en.wikipedia.org/wiki/Loongson#LoongArch> */
#if defined(__loongarch32)
    #define AMW_ARCH_LOONGARCH 1
#elif defined(__loongarch64)
    #define AMW_ARCH_LOONGARCH 2
#endif

/* LSX: LoongArch 128-bits SIMD extension */
#if defined(__loongarch_sx)
    #define AMW_ARCH_LOONGARCH_LSX 1
    #include <lsxintrin.h>
#endif

/* LASX: LoongArch 256-bits SIMD extension */
#if defined(__loongarch_asx)
    #define AMW_ARCH_LOONGARCH_LASX 2
    #include <lasxintrin.h>
#endif

#if AMW_MSVC_VERSION_CHECK(14,0,0)
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

#ifndef AMW_LOG_DISABLE_COLOR
    #ifndef AMW_LOG_USE_COLOR
        #define AMW_LOG_USE_COLOR 1
    #endif
#else 
    #ifndef AMW_LOG_USE_COLOR
        #define AMW_LOG_USE_COLOR 0
    #endif
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
        #define __AMW_FILE__ __FILE__
    #endif /* AMW_LOG_FULL_FILEPATH */
#else
    #define __AMW_FILE__ NULL
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

enum amw_log_level {
    AMW_LOG_VERBOSE = 0,
    AMW_LOG_DEBUG,
    AMW_LOG_INFO,
    AMW_LOG_WARN,
    AMW_LOG_ERROR,
    AMW_LOG_FATAL,
};

AMW_API void    AMW_CALL amw_log_init(void *output);
AMW_API void    AMW_CALL amw_log_terminate(void);

AMW_API void    AMW_CALL amw_log_raw(char *fmt, ...) AMW_PRINTF_FORMAT(1);
AMW_API void    AMW_CALL amw_log_message(int32_t level, const char *function, const char *file, int32_t line, const char *fmt, ...) AMW_PRINTF_FORMAT(5);

AMW_API int32_t AMW_CALL amw_log_level(void);
AMW_API void    AMW_CALL amw_log_set_level(int32_t level);
AMW_API void    AMW_CALL amw_log_set_quiet(bool silence);

#ifndef AMW_LOG_DISABLE_OUTPUT
#define amw_log_verbose(...) amw_log_message(AMW_LOG_VERBOSE, __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_debug(...)   amw_log_message(AMW_LOG_DEBUG,   __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_info(...)    amw_log_message(AMW_LOG_INFO,    __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_warn(...)    amw_log_message(AMW_LOG_WARN,    __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_error(...)   amw_log_message(AMW_LOG_ERROR,   __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#define amw_log_fatal(...)   amw_log_message(AMW_LOG_FATAL,   __AMW_FUNCTION__, __AMW_FILE__, __AMW_LINE__, __VA_ARGS__)
#else
#undef  amw_log_raw
#define amw_log_raw(...)
#define amw_log_verbose(...) 
#define amw_log_debug(...) 
#define amw_log_info(...)  
#define amw_log_warn(...)  
#define amw_log_error(...) 
#define amw_log_fatal(...) 
#endif

#if defined(__cplusplus)
    #if (__cplusplus >= 201103L)
        #define amw_static_assert(name, x) static_assert(x, #x)
    #endif
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
    #define amw_static_assert(name, x) static_assert(x, #x)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
    #define amw_static_assert(name, x) _Static_assert(x, #x)
/* GCC 4.6 or later */
#elif defined(__GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && defined __GNUC_MINOR__ && __GNUC_MINOR >= 6)
    /* It will work but it may throw a warning:
     * warning: ISO C99 does not support '_Static_assert' [-Wpedantic] */
    #define amw_static_assert(name, x) _Static_assert(x, #x)
#endif
#ifndef amw_static_assert
    #define amw_static_assert(name, x) \
        typedef int amw_static_assert_ ## name[(x) * 2 - 1]
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
    #error Unknown assertion level. Use: 0-disabled, 1-release, 2-debug, 3-paranoid.
#endif


#if AMW_HAS_FEATURE(address_sanitizer)
    #define __SANITIZE_ADDRESS__ 1
#endif
#if AMW_HAS_FEATURE(memory_sanitizer)
    #include <sanitizer/msan_interface.h>
    #ifndef AMW_HAVE_VALGRIND
        #define AMW_HAVE_VALGRIND
    #endif
    #define AMW_MEM_UNDEFINED(a,len) __msan_allocated_memory(a,len)
    #define AMW_MEM_MAKE_ADDRESSABLE(a,len) MEM_UNDEFINED(a,len)
    #define AMW_MEM_MAKE_DEFINED(a,len) __msan_unpoison(a,len)
    #define AMW_MEM_NOACCESS(a,len) ((void) 0)
    #define AMW_MEM_CHECK_ADDRESSABLE(a,len) ((void) 0)
    #define AMW_MEM_CHECK_DEFINED(a,len) __msan_check_mem_is_initialized(a,len)
    #define AMW_MEM_GET_VBITS(a,b,len) __msan_copy_shadow(b,a,len)
    #define AMW_MEM_SET_VBITS(a,b,len) __msan_copy_shadow(a,b,len)
    #define AMW_REDZONE_SIZE 8
#elif defined(AMW_HAVE_VALGRIND)
    #include <valgrind/memcheck.h>
    #define AMW_MEM_UNDEFINED(a,len) VALGRIND_MAKE_MEM_UNDEFINED(a,len)
    #define AMW_MEM_MAKE_ADDRESSABLE(a,len) MEM_UNDEFINED(a,len)
    #define AMW_MEM_MAKE_DEFINED(a,len) VALGRIND_MAKE_MEM_DEFINED(a,len)
    #define AMW_MEM_NOACCESS(a,len) VALGRIND_MAKE_MEM_NOACCESS(a,len)
    #define AMW_MEM_CHECK_ADDRESSABLE(a,len) VALGRIND_CHECK_MEM_IS_ADDRESSABLE(a,len)
    #define AMW_MEM_CHECK_DEFINED(a,len) VALGRIND_CHECK_MEM_IS_DEFINED(a,len)
    #define AMW_MEM_GET_VBITS(a,b,len) VALGRIND_GET_VBITS(a,b,len)
    #define AMW_MEM_SET_VBITS(a,b,len) VALGRIND_SET_VBITS(a,b,len)
    #define AMW_REDZONE_SIZE 8
#elif defined(__SANITIZE_ADDRESS__)
    #include <sanitizer/asan_interface.h>
    /* How to do manual poisoning:
     * https://github.com/google/sanitizers/wiki/AddressSanitizerManualPoisoning */
    #define AMW_MEM_UNDEFINED(a,len) ((void) 0)
    #define AMW_MEM_MAKE_ADDRESSABLE(a,len) ASAN_UNPOISON_MEMORY_REGION(a,len)
    #define AMW_MEM_MAKE_DEFINED(a,len) ((void) 0)
    #define AMW_MEM_NOACCESS(a,len) ASAN_POISON_MEMORY_REGION(a,len)
    #define AMW_MEM_CHECK_ADDRESSABLE(a,len) \
        amw_assert_always(!__asan_region_is_poisoned((void*) a,len))
    #define AMW_MEM_CHECK_DEFINED(a,len) ((void) 0)
    #define AMW_MEM_GET_VBITS(a,b,len) ((void) 0)
    #define AMW_MEM_SET_VBITS(a,b,len) ((void) 0)
    #define AMW_REDZONE_SIZE 8
#else
    #define AMW_MEM_UNDEFINED(a,len) ((void) 0)
    #define AMW_MEM_MAKE_ADDRESSABLE(a,len) ((void) 0)
    #define AMW_MEM_MAKE_DEFINED(a,len) ((void) 0)
    #define AMW_MEM_NOACCESS(a,len) ((void) 0)
    #define AMW_MEM_CHECK_ADDRESSABLE(a,len) ((void) 0)
    #define AMW_MEM_CHECK_DEFINED(a,len) ((void) 0)
    #define AMW_MEM_GET_VBITS(a,b,len) ((void) 0)
    #define AMW_MEM_SET_VBITS(a,b,len) ((void) 0)
    #define AMW_REDZONE_SIZE 0
#endif /* valgrind */

#ifdef AMW_HAVE_VALGRIND
    #define AMW_IF_VALGRIND(A,B) A
#else
    #define AMW_IF_VALGRIND(A,B) B
#endif

#ifdef AMW_TRASH_FREED_MEMORY
   /* _AMW_TRASH_FILL() has to call AMW_MEM_MAKE_ADDRESSABLE() to cancel any effect of
    * AMW_TRASH_FREE().
    * This can happen in the case one does
    * AMW_TRASH_ALLOC(A,B) ; AMW_TRASH_FREE(A,B) ; AMW_TRASH_ALLOC(A,B)
    * to reuse the same memory in an internal memory allocator like MEM_ROOT.
    * _AMW_TRASH_FILL() is an internal function and should not be used externally. */
    #define _AMW_TRASH_FILL(A,B,C) \
        do { const size_t trash_tmp= (B); AMW_MEM_MAKE_ADDRESSABLE(A, trash_tmp); amw_memset(A, C, trash_tmp); } while (0)
#else
    #define _AMW_TRASH_FILL(A,B,C) \
        do { AMW_MEM_UNDEFINED((A), (B)); } while (0)
#endif
/** Note that some memory became allocated and/or uninitialized. */
#define AMW_TRASH_ALLOC(A,B) do { _AMW_TRASH_FILL(A,B,0xA5); AMW_MEM_MAKE_ADDRESSABLE(A,B); } while(0)
/** Note that some memory became freed. (Prohibit further access to it.) */
#define AMW_TRASH_FREE(A,B) do { _AMW_TRASH_FILL(A,B,0x8F); AMW_MEM_NOACCESS(A,B); } while(0)

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
            #error "Unsupported endianness"
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

AMW_INLINE float AMW_CALL amw_swap_float(float x) {
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

/* a block of memory */
typedef struct amw_slice amw_slice_t;

struct amw_slice {
    amw_slice_t *next;
    size_t       count;
    size_t       capacity;
    uintptr_t    data[];
};

typedef struct amw_arena {
    amw_slice_t *begin;
    amw_slice_t *end;
} amw_arena_t;

#define AMW_SLICE_DEFAULT_CAPACITY (8*1024)

AMW_API amw_slice_t * AMW_CALL amw_slice_new(size_t capacity);
AMW_API void          AMW_CALL amw_slice_free(amw_slice_t *slice);

AMW_API void *        AMW_CALL amw_arena_alloc(amw_arena_t *a, size_t size_bytes);
AMW_API void *        AMW_CALL amw_arena_realloc(amw_arena_t *a, void *oldptr, size_t old_size, size_t new_size);
AMW_API char *        AMW_CALL amw_arena_strdup(amw_arena_t *a, const char *str);
AMW_API void *        AMW_CALL amw_arena_memdup(amw_arena_t *a, void *data, size_t size_bytes);
AMW_API char *        AMW_CALL amw_arena_sprintf(amw_arena_t *a, const char *fmt, ...) AMW_PRINTF_FORMAT(2);

AMW_API void          AMW_CALL amw_arena_reset(amw_arena_t *a);
AMW_API void          AMW_CALL amw_arena_free(amw_arena_t *a);

#define AMW_MS_PER_SECOND       1000
#define AMW_US_PER_SECOND       1000000
#define AMW_NS_PER_SECOND       1000000000LL
#define AMW_NS_PER_MS           1000000
#define AMW_NS_PER_US           1000
#define AMW_SECONDS_TO_NS(S)    (((uint64_t)(S)) * AMW_NS_PER_SECOND)
#define AMW_NS_TO_SECONDS(NS)   ((NS) / AMW_NS_PER_SECOND)
#define AMW_MS_TO_NS(MS)        (((uint64_t)(MS)) * AMW_NS_PER_MS)
#define AMW_NS_TO_MS(NS)        ((NS) / AMW_NS_PER_MS)
#define AMW_US_TO_NS(US)        (((uint64_t)(US)) * AMW_NS_PER_US)
#define AMW_NS_TO_US(NS)        ((NS) / AMW_NS_PER_US)

/** A global timer that counts from the moment it's initialized. */
AMW_API void     AMW_CALL amw_ticks_init(void);
AMW_API void     AMW_CALL amw_ticks_quit(void);

/** Get the time passed since initializing the 'ticks' timer, in ms or ns. */
AMW_API uint64_t AMW_CALL amw_ticks_ms(void);
AMW_API uint64_t AMW_CALL amw_ticks_ns(void);

/* TODO atomic operations */

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

typedef int32_t                 ivec2_t[2];
typedef int32_t                 ivec3_t[3];
typedef int32_t                 ivec4_t[4];

typedef float                   vec2_t[2];
typedef float                   vec3_t[3];
typedef AMW_ALIGN_TO_16 float   vec4_t[4];

typedef vec4_t                  quat_t;       /* |x, y, z, w| -> w is the last */

typedef AMW_ALIGN_TO_16 vec2_t  mat2_t[2];
typedef vec3_t                  mat2x3_t[2];  /* [col (2), row (3)] */
typedef vec4_t                  mat2x4_t[2];  /* [col (2), row (4)] */

typedef vec3_t                  mat3_t[3];
typedef vec2_t                  mat3x2_t[3];  /* [col (3), row (2)] */
typedef vec4_t                  mat3x4_t[3];  /* [col (3), row (4)] */

typedef AMW_ALIGN_MAT vec4_t    mat4_t[4];
typedef vec2_t                  mat4x2_t[4];  /* [col (4), row (2)] */
typedef vec3_t                  mat4x3_t[4];  /* [col (4), row (3)] */

#define AMW_FLT_EPSILON 1e-5f

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

#define amw_arraysize(array) (sizeof(array)/sizeof(array[0]))
#define amw_clamp(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))
#define amw_clamp_zo(x) (amw_clamp(x, 0, 1)) 
#define amw_min(x, y) (((x) < (y)) ? (x) : (y))
#define amw_max(x, y) (((x) > (y)) ? (x) : (y))
#define amw_swap(type, a, b) \
    {                        \
        type temp = a;       \
        a = b;               \
        b = temp;            \
    }

/** returns +1, -1 or 0 */
AMW_INLINE int32_t AMW_CALL amw_sign(int32_t val) {
    return ((val >> 31) - (-val << 31));
}

/** returns +1, -1 or 0 */
AMW_INLINE float AMW_CALL amw_signf(float val) {
    return (float)((val > 0.0f) - (val < 0.0f));
}

/** convert degrees to radians */
AMW_INLINE float AMW_CALL amw_rad(float deg) {
    return deg * AMW_PIf / 180.0f;
}

/** convert radians to degrees */
AMW_INLINE float AMW_CALL amw_deg(float rad) {
    return rad * 180.0f / AMW_PIf;
}

/** convert existing degrees to radians, override degrees value */
AMW_INLINE void AMW_CALL amw_make_rad(float *deg) {
    *deg = *deg * AMW_PIf / 180.0f;
}

/** convert existing radians to degrees, override radians value */
AMW_INLINE void AMW_CALL amw_make_deg(float *rad) {
    *rad = *rad * 180.0f / AMW_PIf;
}

/** multiplies a parameter with itself */
AMW_INLINE float AMW_CALL amw_pow2(float x) {
    return x * x;
}

/** 
 * linear interpolation between two numbers
 * t    - interpolant (amount) 
 */
AMW_INLINE float AMW_CALL amw_lerp(float from, float to, float t) {
    return from + t * (to - from);
}

/** 
 * clamped linear interpolation between two numbers
 * t    - interpolant (amount, clamped between 0 and 1) 
 */
AMW_INLINE float AMW_CALL amw_lerpc(float from, float to, float t) {
    return amw_lerp(from, to, amw_clamp_zo(t));
}

/** 
 * threshold function
 * edge - threshold
 * x    - value to test against threshold 
 */
AMW_INLINE float AMW_CALL amw_step(float edge, float x) {
    /* branching - no type conversion */
    return (x < edge) ? 0.0f : 1.0f;
}

/** 
 * smooth Hermite interpolation
 * t    - interpolant (amount)
 */
AMW_INLINE float AMW_CALL amw_smooth(float t) {
    return t * t * (3.0f - 2.0f * t);
}

/** 
 * threshold function with a smooth transition 
 * edge0 - low threshold
 * edge1 - high threshold
 * t     - interpolant (amount)
 */
AMW_INLINE float AMW_CALL amw_smoothstep(float edge0, float edge1, float t) {
    float x;
    x = amw_clamp_zo((t - edge0) / (edge1 - edge0));
    return amw_smooth(x);
}

/**
 * smoothstep interpolation between two numbers
 * t    - interpolant (amount)
 */
AMW_INLINE float AMW_CALL amw_smoothinterp(float from, float to, float t) {
    return from + amw_smooth(t) * (to - from);
}

/**
 * clamped smoothstep interpolation between two numbers
 * t    - interpolant (amount, clamped between 0 and 1)
 */
AMW_INLINE float AMW_CALL amw_smoothinterpc(float from, float to, float t) {
    return from + amw_smoothinterp(from, to, amw_clamp_zo(t));
}

/** check if two floats are equal using EPSILON */
AMW_INLINE bool AMW_CALL amw_equalf(float a, float b) {
    return fabsf(a - b) <= AMW_FLT_EPSILON;
}

/** percentage of current value between start and end value */
AMW_INLINE float AMW_CALL amw_percent(float from, float to, float current) {
    float t;
    if ((t = to - from) == 0.0f)
        return 1.0f;
    return (current - from) / t;
}

/** clamped (0-1) percentage of current value between start and end value */
AMW_INLINE float AMW_CALL amw_percentc(float from, float to, float current) {
    return amw_clamp_zo(amw_percent(from, to, current));
}

typedef enum {
    AMW_SUCCESS = 0,
    AMW_ERROR_UNKNOWN = -1,
    AMW_ERROR_FEATURE_NOT_SUPPORTED = -100,
    AMW_ERROR_PLATFORM_NOT_SUPPORTED = -101,
    AMW_ERROR_THREAD_FAILED = -200,
    AMW_ERROR_MUTEX_FAILED = -201,
    /* TODO */
} amw_result_t;

#define AMW_FLAGS_WORD_BIT_SIZE (sizeof(unsigned int) * CHAR_BIT)
#define AMW_FLAGS_WORD_MASK(bit) ((unsigned int)1 << ((bit) % AMW_FLAGS_WORD_BIT_SIZE))

#define amw_read_bits(bits, mask)   ((bits) & (mask))   /* check for bits */
#define amw_set_bits(bits, mask)    ((bits) |= (mask))  /* set the specified bits to 1 */
#define amw_unset_bits(bits, mask)  ((bits) &= ~(mask)) /* set the specified bits to 0 */
#define amw_toggle_bits(bits, mask) ((bits) ^= (mask))  /* toggle the specified bits */

#define amw_read_bit(bits, bit)     (bits & AMW_FLAGS_WORD_MASK(bit))   /* check for bit */
#define amw_set_bit(bits, bit)      (bits |= AMW_FLAGS_WORD_MASK(bit))  /* set the specified bits to 1 */
#define amw_unset_bit(bits, bit)    (bits &= ~AMW_FLAGS_WORD_MASK(bit)) /* set the specified bits to 0 */
#define amw_toggle_bit(bits, bit)   (bits ^= AMW_FLAGS_WORD_MASK(bit))  /* toggle the specified bits */

AMW_C_DECL_END

#endif /* _amw_defines_h_ */
