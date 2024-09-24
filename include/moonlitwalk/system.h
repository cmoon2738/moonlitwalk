#ifndef _amw_system_h_
#define _amw_system_h_

#include "amw.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

AMW_NORETURN 
void amw_exit(int32_t exitcode);

/* overwrites the return exit code of the game process 
 * this code will be returned when amw_exit() is called */
void amw_exitcode(int32_t exitcode);

/* puts current thread to sleep */
void amw_sleep(int32_t ms);

/* loads a shared library into memory, returns an opaque handle */
void *amw_load_dll(const char *libname);

/* returns a memory address for an exported function */
void *amw_get_proc_address(void *handle, const char *procname);

/* closes the loaded shared library */
void amw_close_dll(void *handle);

/* reads multicore information about the cpu, saves them to the arguments */
void amw_cpu_count(int32_t *threads, int32_t *cores, int32_t *packages);

/**
 * Provides a locking mechanism, ensuring that only one thread 
 * will access a shared resource at a time, so, critical sections.
 */
typedef struct amw_mutex amw_mutex_t;

amw_mutex_t *amw_mutex_create(amw_arena_t *arena);
void         amw_mutex_lock(amw_mutex_t *mutex);     /* will block */
bool         amw_mutex_try_lock(amw_mutex_t *mutex); /* no blocking */
void         amw_mutex_unlock(amw_mutex_t *mutex);
void         amw_mutex_destroy(amw_mutex_t *mutex);

/**
 * A means to manage access to a resource, by count, between threads.
 *
 * Semaphores (specifically, "counting semaphores"), let X number of threads 
 * request access at the same time, each thread granted access decrementing a 
 * counter. When the counter reaches zero, future requests block until a prior 
 * thread releases their request, incrementing the counter again.
 */
typedef struct amw_semaphore amw_semaphore_t;

amw_semaphore_t *amw_semaphore_create(amw_arena_t *arena, uint32_t initvalue);
void             amw_semaphore_wait(amw_semaphore_t *sem);
bool             amw_semaphore_wait_timeout(amw_semaphore_t *sem, int32_t ms);
bool             amw_semaphore_wait_timeout_ns(amw_semaphore_t *sem, int32_t ns);
bool             amw_semaphore_try_wait(amw_semaphore_t *sem);
void             amw_semaphore_signal(amw_semaphore_t *sem);
uint32_t         amw_semaphore_value(amw_semaphore_t *sem);
void             amw_semaphore_destroy(amw_semaphore_t *sem);

/**
 * An opaque data type holding a system thread id.
 */
typedef uint64_t amw_thread_t;

amw_thread_t *amw_thread_create(void *(*procedure)(void *), void *arg);
amw_thread_t  amw_thread_current(void);
void          amw_thread_join(amw_thread_t *thread);
void          amw_thread_detach(amw_thread_t *thread);
size_t        amw_thread_index(amw_thread_t *threads, size_t threadpool_size);
void          amw_thread_destroy(amw_thread_t thread);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_system_h_ */
