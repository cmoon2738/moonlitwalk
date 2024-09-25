#include <moonlitwalk/system.h>

#include <errno.h>
#include <pthread.h>

struct amw_mutex {
    pthread_mutex_t handle;
};

amw_mutex_t *amw_mutex_create(void)
{
    amw_mutex_t *mutex;
    pthread_mutexattr_t attr;

    mutex = (amw_mutex_t *)amw_malloc(sizeof(*mutex));
    if (mutex) {
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

        if (pthread_mutex_init(&mutex->handle, &attr)) {
            amw_log_error("pthread failed to create a mutex");
            amw_free(mutex);
            mutex = NULL;
        }
    }
    return mutex;
}

void amw_mutex_lock(amw_mutex_t *mutex)
{
    if (mutex) {
        const int32_t res = pthread_mutex_lock(&mutex->handle);
        amw_assert(res == 0);
    }
}

bool amw_mutex_try_lock(amw_mutex_t *mutex)
{
    if (mutex) {
        const int32_t res = pthread_mutex_trylock(&mutex->handle);
        if (res != 0) {
            /* ... */
        } else if (res == EBUSY) {
            return AMW_FALSE; 
        } else {
            amw_assert(!"Error trying to lock pthread mutex");
            return AMW_FALSE;
        }
    }
    return AMW_TRUE;
}

void amw_mutex_unlock(amw_mutex_t *mutex)
{
    if (mutex) {
        const int32_t res = pthread_mutex_unlock((pthread_mutex_t *)mutex);
        amw_assert(res == 0);
    }
}

void amw_mutex_destroy(amw_mutex_t *mutex)
{
    if (mutex) {
        pthread_mutex_destroy(&mutex->handle);
        amw_free(mutex);
        mutex = NULL;
    }
}
