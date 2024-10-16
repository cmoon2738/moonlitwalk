#ifndef _amw_simd_h_
#define _amw_simd_h_

#include <moonlitwalk/defines.h>

#if defined(AMW_ARCH_ARM) || defined(AMW_ARCH_AARCH64)
    #include <moonlitwalk/simd/arm.h>
#endif

#if defined(AMW_ARCH_AVX)
    #include <moonlitwalk/simd/avx.h>
#endif

#if defined(AMW_ARCH_ARM_NEON)
    #include <moonlitwalk/simd/neon.h>
#endif

#if defined(AMW_ARCH_SSE2)
    #include <moonlitwalk/simd/sse2.h>
#endif

#if defined(AMW_ARCH_WASM)
    #include <moonlitwalk/simd/wasm.h>
#endif

#if defined(AMW_ARCH_X86) || defined(AMW_ARCH_AMD64) || defined(AMW_ARCH_E2K)
    #include <moonlitwalk/simd/x86.h>
#endif

#endif /* _amw_simd_h_ */
