#include <moonlitwalk/system.h>
#include <moonlitwalk/system.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>

#if defined(MW_PLATFORM_MACOSX) || defined(MW_PLATFORM_IOS)
    error('macOS doesn't support sem_getvalue() as of version 10.4, fix this)
#endif

struct amw_semaphore {
    sem_t handle;
};


amw_semaphore_t *amw_semaphore_create(uint32_t initvalue)
{
    amw_semaphore_t *sem = (amw_semaphore_t *)amw_malloc(sizeof(amw_semaphore_t));
    if (sem) {
        if (sem_init(&sem->handle, 0, initvalue) < 0) {
            amw_log_error("pthread failed to create a semaphore");
            amw_free(sem);
            sem = NULL;
        }
    }
    return sem;
}

void amw_semaphore_wait(amw_semaphore_t *sem)
{
    amw_semaphore_wait_timeout_ns(sem, -1);
}

bool amw_semaphore_wait_timeout(amw_semaphore_t *sem, int32_t ms)
{
    int64_t timeout_ns;

    if (ms >= 0) {
        timeout_ns = AMW_MS_TO_NS(ms);
    } else {
        timeout_ns = -1;
    }
    return amw_semaphore_wait_timeout_ns(sem, timeout_ns);
}

bool amw_semaphore_wait_timeout_ns(amw_semaphore_t *sem, int32_t ns)
{
    struct timespec abstime;
    int32_t res;

    if (!sem)
        return AMW_TRUE; 
    if (ns == 0)
        return (sem_trywait(&sem->handle) == 0);
    if (ns < 0) {
        do {
            res = sem_wait(&sem->handle);
        } while (res < 0 && errno == EINTR);
        return (res == 0);
    }

    clock_gettime(CLOCK_REALTIME, &abstime);

    abstime.tv_sec += (ns / AMW_NS_PER_SECOND);
    abstime.tv_nsec += (ns % AMW_NS_PER_SECOND);

    while (abstime.tv_nsec >= 1000000000) {
        abstime.tv_sec += 1;
        abstime.tv_nsec -= 1000000000;
    }

    do {
        res = sem_timedwait(&sem->handle, &abstime);
    } while (res < 0 && errno == EINTR);

    return (res == 0);
}

bool amw_semaphore_try_wait(amw_semaphore_t *sem)
{
    return amw_semaphore_wait_timeout_ns(sem, 0);
}

void amw_semaphore_signal(amw_semaphore_t *sem)
{
    if (sem)
        sem_post(&sem->handle);
}

uint32_t amw_semaphore_value(amw_semaphore_t *sem)
{
    int32_t res = 0;

    if (sem) {
        sem_getvalue(&sem->handle, &res);
        if (res < 0) res = 0;
    }
    return (uint32_t)res;
}

void amw_semaphore_destroy(amw_semaphore_t *sem)
{
    if (sem) {
        sem_destroy(&sem->handle);
        amw_free(sem);
    }
}
