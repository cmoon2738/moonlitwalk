#ifndef _amw_os_h_
#define _amw_os_h_

#include <moonlitwalk/defines.h>

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
    #include <stdatomic.h>
#else 
    #error "Needed C11 standard for atomic operations"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* To enable analysis, set these environment variables before running meson:
 *     export CC=clang
 *     export CFLAGS="-DAMW_THREAD_SAFETY_ANALYSIS -Wthread-safety"
 */
#if defined(AMW_THREAD_SAFETY_ANALYSIS) && defined(__clang__) && (!defined(SWIG))
    #define _AMW_THREAD_ATTRIBUTE(x) __attribute__((x))
#else
    #define _AMW_THREAD_ATTRIBUTE(x)
#endif

#define AMW_CAPABILITY(x)               _AMW_THREAD_ATTRIBUTE(capability(x))
#define AMW_SCOPED_CAPABILITY           _AMW_THREAD_ATTRIBUTE(scoped_lockable)
#define AMW_GUARDED_BY(x)               _AMW_THREAD_ATTRIBUTE(guarded_by(x))
#define AMW_PT_GUARDED_BY(x)            _AMW_THREAD_ATTRIBUTE(pt_guarded_by(x))
#define AMW_ACQUIRED_BEFORE(x)          _AMW_THREAD_ATTRIBUTE(acquired_before(x))
#define AMW_ACQUIRED_AFTER(x)           _AMW_THREAD_ATTRIBUTE(acquired_after(x))
#define AMW_REQUIRES(x)                 _AMW_THREAD_ATTRIBUTE(requires_capability(x))
#define AMW_REQUIRES_SHARED(x)          _AMW_THREAD_ATTRIBUTE(requires_shared_capability(x))
#define AMW_ACQUIRE(x)                  _AMW_THREAD_ATTRIBUTE(acquire_capability(x))
#define AMW_ACQUIRE_SHARED(x)           _AMW_THREAD_ATTRIBUTE(acquire_shared_capability(x))
#define AMW_RELEASE(x)                  _AMW_THREAD_ATTRIBUTE(release_capability(x))
#define AMW_RELEASE_SHARED(x)           _AMW_THREAD_ATTRIBUTE(release_shared_capability(x))
#define AMW_RELEASE_GENERIC(x)          _AMW_THREAD_ATTRIBUTE(release_generic_capability(x))
#define AMW_TRY_ACQUIRE(x)              _AMW_THREAD_ATTRIBUTE(try_acquire_capability(x))
#define AMW_TRY_ACQUIRE_SHARED(x)       _AMW_THREAD_ATTRIBUTE(try_acquire_shared_capability(x))
#define AMW_EXCLUDES(x)                 _AMW_THREAD_ATTRIBUTE(locks_excluded(x))
#define AMW_ASSERT_CAPABILITY(x)        _AMW_THREAD_ATTRIBUTE(assert_capability(x))
#define AMW_ASSERT_SHARED_CAPABILITY(x) _AMW_THREAD_ATTRIBUTE(assert_shared_capability(x))
#define AMW_RETURN_CAPABILITY(x)        _AMW_THREAD_ATTRIBUTE(lock_returned(x))
#define AMW_NO_THREAD_SAFETY_ANALYSIS   _AMW_THREAD_ATTRIBUTE(no_thread_safety_analysis)

/** Exits the game process with a return code. */
AMW_NORETURN AMW_API void AMW_CALL amw_exit(int32_t exitcode);

/** 
 * Overwrites the return exit code of the game process 
 * this code will be returned when amw_exit() is called.
 */
AMW_API void        AMW_CALL amw_exitcode(int32_t exitcode);

/** Puts current thread to sleep. */
AMW_API void        AMW_CALL amw_sleep(int32_t ns);

/** Loads a shared library into memory, returns an opaque handle. */
AMW_API void *      AMW_CALL amw_load_dll(const char *libname);

/** Closes the loaded shared library. */
AMW_API void        AMW_CALL amw_close_dll(void *handle);

/** Returns a memory address for a DLL exported function. */
AMW_API void *      AMW_CALL amw_get_proc_address(void *handle, const char *procname);

/** Reads information about the cpu, saves them to the arguments. */
AMW_API void        AMW_CALL amw_cpu_count(int32_t *threads, int32_t *cores, int32_t *packages);

/** Get the current value of the high resolution counter. */
AMW_API uint64_t    AMW_CALL amw_systime_counter(void);

/** Get the count per second of the high resolution counter. */
AMW_API uint64_t    AMW_CALL amw_systime_frequency(void);

/**
 * Provides a locking mechanism, ensuring that only one thread 
 * will access a shared resource at a time, so, critical sections.
 */
typedef struct amw_mutex amw_mutex_t;

AMW_API amw_mutex_t * AMW_CALL amw_mutex_create(void);
AMW_API void          AMW_CALL amw_mutex_lock(amw_mutex_t *mutex) AMW_ACQUIRE(mutex);
AMW_API bool          AMW_CALL amw_mutex_try_lock(amw_mutex_t *mutex) AMW_TRY_ACQUIRE(mutex); 
AMW_API void          AMW_CALL amw_mutex_unlock(amw_mutex_t *mutex) AMW_RELEASE(mutex);
AMW_API void          AMW_CALL amw_mutex_destroy(amw_mutex_t *mutex);

/**
 * A means to manage access to a resource, by count, between threads.
 *
 * Semaphores (specifically, "counting semaphores"), let X number of threads 
 * request access at the same time, each thread granted access decrementing a 
 * counter. When the counter reaches zero, future requests block until a prior 
 * thread releases their request, incrementing the counter again.
 */
typedef struct amw_semaphore amw_semaphore_t;

AMW_API amw_semaphore_t * AMW_CALL amw_semaphore_create(uint32_t initvalue);
AMW_API void              AMW_CALL amw_semaphore_wait(amw_semaphore_t *sem);
AMW_API bool              AMW_CALL amw_semaphore_wait_timeout(amw_semaphore_t *sem, int32_t ms);
AMW_API bool              AMW_CALL amw_semaphore_wait_timeout_ns(amw_semaphore_t *sem, int32_t ns);
AMW_API bool              AMW_CALL amw_semaphore_try_wait(amw_semaphore_t *sem);
AMW_API void              AMW_CALL amw_semaphore_signal(amw_semaphore_t *sem);
AMW_API uint32_t          AMW_CALL amw_semaphore_value(amw_semaphore_t *sem);
AMW_API void              AMW_CALL amw_semaphore_destroy(amw_semaphore_t *sem);

/**
 * An opaque data type holding a system thread id.
 */
typedef struct amw_thread amw_thread_t;

AMW_API amw_thread_t * AMW_CALL amw_thread_create(amw_arena_t *a, void *(*procedure)(void *), void *arg);
AMW_API amw_thread_t * AMW_CALL amw_thread_current(amw_arena_t *a);
AMW_API uint64_t       AMW_CALL amw_thread_handle(amw_thread_t *thread);
AMW_API void           AMW_CALL amw_thread_join(amw_thread_t *thread);
AMW_API void           AMW_CALL amw_thread_detach(amw_thread_t *thread);
AMW_API size_t         AMW_CALL amw_thread_index(amw_thread_t **threads, size_t threadpool_size);
AMW_API void           AMW_CALL amw_thread_destroy(amw_thread_t *thread);

/**
 * Spinlocks are efficient atomic locks using CPU instructions, but are 
 * vulnerable to starvation and can spin forever if a thread holding 
 * a lock has been terminated. Not save to lock recursively.
 */
typedef uint32_t amw_spinlock_t;

AMW_API void AMW_CALL amw_spinlock_acquire(amw_spinlock_t *lock) AMW_ACQUIRE(lock);
AMW_API bool AMW_CALL amw_spinlock_try_acquire(amw_spinlock_t *lock) AMW_TRY_ACQUIRE(lock);
AMW_API void AMW_CALL amw_spinlock_release(amw_spinlock_t *lock) AMW_RELEASE(lock);

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
#elif (defined(__GNUC__) && !defined(AMW_PLATFORM_EMSCRIPTEN)) || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x5120))
    #define amw_compiler_barrier() __asm__ __volatile__ ("" : : : "memory")
#elif defined(AMW_PLATFORM_EMSCRIPTEN) || defined(AMW_ARCH_WASM)
    #include <emscripten/em_asm.h>
    #define amw_compiler_barrier() /* TODO */
#elif defined(__WATCOMC__)
    extern __inline void amw_compiler_barrier(void);
    #pragma aux amw_compiler_barrier = "" parm [] modify exact [];
#else
    #define amw_compiler_barrier()          \
    {                                       \
        uint32_t lock_temp = 0;              \
        amw_spinlock_acquire(&lock_temp);   \
        amw_spinlock_release(&lock_temp);   \
    }
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

#endif /* _amw_os_h_ */
