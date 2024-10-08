#ifndef _amw_system_h_
#define _amw_system_h_

#include "amw.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

AMW_NORETURN AMW_API void AMW_CALL amw_exit(int32_t exitcode);

/* overwrites the return exit code of the game process 
 * this code will be returned when amw_exit() is called */
AMW_API void AMW_CALL amw_exitcode(int32_t exitcode);

/* puts current thread to sleep */
AMW_API void AMW_CALL amw_sleep(int32_t ms);

/* loads a shared library into memory, returns an opaque handle */
AMW_API void * AMW_CALL amw_load_dll(const char *libname);

/* returns a memory address for an exported function */
AMW_API void * AMW_CALL amw_get_proc_address(void *handle, const char *procname);

/* closes the loaded shared library */
AMW_API void AMW_CALL amw_close_dll(void *handle);

/* reads multicore information about the cpu, saves them to the arguments */
AMW_API void AMW_CALL amw_cpu_count(int32_t *threads, int32_t *cores, int32_t *packages);

/**
 * A high resolution timer.
 */
typedef struct amw_timer amw_timer_t;

AMW_API void     AMW_CALL amw_timer_init(amw_timer_t *timer);
AMW_API uint64_t AMW_CALL amw_timer_value(amw_timer_t *timer);
AMW_API uint64_t AMW_CALL amw_timer_frequency(amw_timer_t *timer);

/**
 * Provides a locking mechanism, ensuring that only one thread 
 * will access a shared resource at a time, so, critical sections.
 */
typedef struct amw_mutex amw_mutex_t;

AMW_API amw_mutex_t * AMW_CALL amw_mutex_create(void);
AMW_API void          AMW_CALL amw_mutex_lock(amw_mutex_t *mutex);     /* will block */
AMW_API bool          AMW_CALL amw_mutex_try_lock(amw_mutex_t *mutex); /* no blocking */
AMW_API void          AMW_CALL amw_mutex_unlock(amw_mutex_t *mutex);
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_system_h_ */
