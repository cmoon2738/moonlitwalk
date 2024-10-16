#include <moonlitwalk/os.h>

#include <pthread.h>

struct amw_thread {
    amw_thread_t *next;
    pthread_t     handle;
};

static amw_thread_t *thread_list = NULL;

AMW_INLINE void setup_main_thread(amw_arena_t *a)
{
    if (thread_list != NULL)
        return;
    amw_thread_t *main_thread = (amw_thread_t *)amw_arena_alloc(a, sizeof(amw_thread_t));
    main_thread->handle = pthread_self();
    thread_list = main_thread;
}

amw_thread_t *amw_thread_create(amw_arena_t *a, void *(*procedure)(void *), void *arg)
{
    amw_assert(a);
    setup_main_thread(a);

    amw_thread_t *thread = (amw_thread_t *)amw_arena_alloc(a, sizeof(amw_thread_t));
    if (pthread_create(&thread->handle, NULL, procedure, arg) != 0) {
        amw_log_error("not enough system resources to create a pthread");
        thread = NULL;
    } else {
        thread->next = thread_list;
    }
    return thread;
}

amw_thread_t *amw_thread_current(amw_arena_t *a)
{
    if (thread_list) {
        pthread_t self = pthread_self();
        for (amw_thread_t *current = thread_list; current->next != NULL; current = current->next) {
            if (pthread_equal(self, current->handle)) {
                return current;
            }
        }
    } else {
        amw_assert(a && "To get the current thread if only the main thread exists, pass in a pointer to an arena allocator");
        setup_main_thread(a);
    }
    return thread_list;
}

uint64_t amw_thread_handle(amw_thread_t *thread)
{
    if (thread) {
        return thread->handle;
    }
    return pthread_self();
}

void amw_thread_join(amw_thread_t *thread)
{
    amw_assert(thread);
    pthread_join(thread->handle, NULL);
}

void amw_thread_detach(amw_thread_t *thread)
{
    amw_assert(thread);
    pthread_detach(thread->handle);
}

size_t amw_thread_index(amw_thread_t **threads, size_t threadpool_size)
{
    amw_assert(threads);
    pthread_t self = pthread_self();
    for (;;) {
        for (size_t i = 0; i < threadpool_size; i++) {
            if (pthread_equal(self, threads[i]->handle)) {
                return i;
            }
        }
    }
}

void amw_thread_destroy(amw_thread_t *thread)
{
    amw_assert(thread);
    pthread_cancel(thread->handle);
}
