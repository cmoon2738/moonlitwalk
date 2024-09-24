#include <moonlitwalk/system.h>

#include <pthread.h>

amw_thread_t *amw_thread_create(void *(*procedure)(void *), void *arg)
{
    amw_thread_t *thread;
    if (pthread_create((pthread_t *)thread, NULL, procedure, arg) != 0) {
        amw_log_error("not enough system resources to create a pthread");
        thread = NULL;
    }
    return thread;
}

amw_thread_t amw_thread_current(void)
{
    return (amw_thread_t)pthread_self();
}

void amw_thread_join(amw_thread_t *thread)
{
    pthread_join((pthread_t)*thread, NULL);
}

void amw_thread_detach(amw_thread_t *thread)
{
    pthread_detach((pthread_t)*thread);
}

size_t amw_thread_index(amw_thread_t *threads, size_t threadpool_size)
{
    pthread_t self = pthread_self();
    for (;;) {
        for (size_t i = 0; i < threadpool_size; i++) {
            if (pthread_equal(self, (pthread_t)threads[i])) {
                return i;
            }
        }
    }
}

void amw_thread_destroy(amw_thread_t thread)
{
    pthread_cancel(*(pthread_t*)thread);
}
