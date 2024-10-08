#include <moonlitwalk/amw.h>
#include <moonlitwalk/system.h>

#if defined(__WATCOMC__) && defined(__386__)
extern __inline int _MW_xchg_watcom(volatile int *a, int v);
#pragma aux _MW_xchg_watcom = \
    "lock xchg [ecx], eax" \
    parm [ecx] [eax] \
    value [eax] \
    modify exact [eax];
#endif /* __WATCOMC__ && __386__ */

bool amw_spinlock_try_acquire(int32_t *lock)
{
#if defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
    return _InterlockedExchange_acq(lock, 1) == 0;

#elif defined(_MSC_VER)
    return InterlockedExchange((long *)lock, 1) == 0;

#elif defined(__GNUC__) && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1)
    return __sync_bool_compare_and_swap(lock, 0, 1);

#elif defined(__WATCOMC__) && defined(__386__)
    return _MW_xchg_watcom(lock, 1) == 0;

#elif defined(__GNUC__) && defined(__arm__) && \
    (defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__) || \
     defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__) || \
     defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5TE__) || \
     defined(__ARC_ARCH_5TEJ__))

    int32_t result;
    __asm__ __volatile__(
        "swp %0, %1, [%2]\n"
        : "=&r,&r"(result)
        : "r,0"(1), "r,r"(lock)
        : "memory");
    return result == 0;

#elif defined(__GNUC__)  && defined(__arm__)
    int32_t result;
    __asm__ __volatile__(
        "ldrex %0, [%2]\nteq    %0, $0\nstrexeq %0, %1, [%2]"
        : "=&r"(result)
        : "r"(1), "r"(lock)
        : "cc", "memory");
    return result == 0;

#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    int32_t result;
    __asm__ __volatile__(
        "lock ; xchgl %0, (%1)\n"
        : "=r"(result)
        : "r"(lock), "0"(1)
        : "cc", "memory");
    return result == 0;

#else
    static amw_mutex_t *_spinlock_mutex;
    if(!_spinlock_mutex)
        _spinlock_mutex = amw_mutex_create();

    amw_lock_mutex(_spinlock_mutex);
    if (*lock == 0) {
        *lock = 1;
        amw_mutex_unlock(_spinlock_mutex);
        return AMW_TRUE;
    } else {
        amw_mutex_unlock(_spinlock_mutex);
        return AMW_FALSE;
    }
#endif
}

void amw_spinlock_acquire(int32_t *lock)
{
    int32_t iterations = 0;
    while (!amw_spinlock_try_acquire(lock)) {
        if (iterations < 32) {
            iterations++;
            amw_cpu_pause_instruction();
        } else {
            amw_sleep(0);
        }
    }
}

void amw_spinlock_release(int32_t *lock)
{
#if defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
    _InterlockedExchange_rel(lock, 0);
#elif defined(_MSC_VER)
    _ReadWriteBarrier();
    *lock = 0;
#elif defined(__WATCOMC__) && defined(__386__)
    mw_compiler_barrier();
    *lock = 0;
#elif defined(__GNUC__) && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1)
    __sync_bool_compare_and_swap(lock, 1, 0);
#else
    *lock = 0;
#endif
}
