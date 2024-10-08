#include <moonlitwalk/sewing.h>
#include <stdatomic.h>

#include "sew_context.h"

#if !defined(SEWING_STACK_GUARD)
    #define SEWING_STACK_GUARD 0
#endif
#define SEW_INVALID SIZE_MAX

#define A16(x) (((x) + 15) & ~15)

/* for protecting memory */
#if defined(AMW_PLATFORM_UNIX)
    #include <unistd.h>
    #include <sys/mman.h>
#elif defined(AMW_PLATFORM_WINDOWS)
    #include <process.h>
    #include <intrin.h>
#elif defined(AMW_PLATFORM_EMSCRIPTEN)
    // TODO wasm
    #error "Figure out how to protect memory for this platform."
#else
    #if SEWING_STACK_GUARD
        #error "Figure out how to protect memory for this platform."
    #endif
#endif

amw_static_assert(sizeof(intptr_t) == sizeof(amw_sewing_t *), \
    "context argument size assumption failed, sew_jump_context will crash.");
amw_static_assert(sizeof(size_t) >= sizeof(void *), "size_t cannot hold pointers");

/* a local thread */
typedef struct tls tls_t;

static void make_context(sew_context_t *context, void (*procedure)(intptr_t), void *stack, size_t stack_bytes)
{
    *context = sew_make_context(stack, stack_bytes, procedure);
}

static intptr_t jump_context(tls_t *tls, sew_context_t *from, sew_context_t *to)
{
    amw_assert(*from != *to);
    return sew_jump_context(from, *to, (intptr_t)tls, 1);
}

static void stack_memory_guard(void *mem, size_t bytes)
{
#if SEWING_STACK_GUARD
#if defined(AMW_PLATFORM_UNIX)
    int32_t result = mprotect(mem, bytes, PROT_NONE);
    amw_assert(!result);
#elif defined(AMW_PLATFORM_WINDOWS)
    DWORD ignored;
    BOOL result = VirtualProtect(mem, bytes, PAGE_NOACCESS, &ignored);
    amw_assert(result);
#elif defined(AMW_PLATFORM_EMSCRIPTEN)
    // TODO wasm
#endif
#else
    (void)mem;
    (void)bytes;
    return;
#endif
}

static void stack_memory_stop(void *mem, size_t bytes)
{
#if SEWING_STACK_GUARD
#if defined(AMW_PLATFORM_UNIX)
    int32_t result = mprotect(mem, bytes, PROT_READ | PROT_WRITE);
    amw_assert(!result);
#elif defined(AMW_PLATFORM_WINDOWS)
    DWORD ignored;
    BOOL result = VirtualProtect(mem, bytes, PAGE_READWRITE, &ignored);
    amw_assert(result);
#elif defined(AMW_PLATFORM_EMSCRIPTEN)
    // TODO wasm
#endif
#else
    (void)mem;
    (void)bytes;
    return;
#endif
}

typedef struct stitch {
    amw_stitch_t   job;
    atomic_size_t *stitches_left;
} stitch_t;

#define CACHELINE_SIZE 64
typedef struct mpmc_cell {
    atomic_size_t sequence;
    stitch_t      stitch;
} mpmc_cell_t;

typedef struct mpmc_queue {
    char            pad0[CACHELINE_SIZE];

    mpmc_cell_t    *buffer;
    size_t          buffer_mask;
    char            pad1[CACHELINE_SIZE];

    atomic_size_t   enqueue_pos;
    char            pad2[CACHELINE_SIZE];
    
    atomic_size_t   dequeue_pos;
    char            pad3[CACHELINE_SIZE];
} mpmc_queue_t;

static void mpmc_queue_init(mpmc_queue_t *q, mpmc_cell_t *cells, size_t cell_count)
{
    if (!(cell_count && !(cell_count & (cell_count - 1)))) {
        amw_assert(!"cell_count is not a power of 2");
        q->buffer = NULL;
        return;
    }
    q->buffer      = cells;
    q->buffer_mask = cell_count - 1;

    for (size_t i = 0; i != (q->buffer_mask + 1); i++) {
        atomic_store_explicit(&q->buffer[i].sequence, i, memory_order_relaxed);
    }
    atomic_store_explicit(&q->enqueue_pos, (size_t)0, memory_order_relaxed);
    atomic_store_explicit(&q->dequeue_pos, (size_t)0, memory_order_relaxed);
}

typedef struct mpmc_work {
    mpmc_cell_t *cell;
    size_t       position;
} mpmc_work_t;

static mpmc_work_t mpmc_queue_work(mpmc_queue_t *q, atomic_size_t *in_or_out, const unsigned pos_delta)
{
    mpmc_cell_t *cell;
    size_t position = atomic_load_explicit(in_or_out, memory_order_relaxed);

    for (;;) {
        cell = &q->buffer[position & q->buffer_mask];
        size_t sequence = atomic_load_explicit(&cell->sequence, memory_order_acquire);
        intptr_t difference = (intptr_t)sequence - (intptr_t)(position + pos_delta);

        if (!difference) {
            if (atomic_compare_exchange_weak_explicit(in_or_out, &position, position + 1, 
                                                      memory_order_relaxed, memory_order_relaxed)) 
            {
                mpmc_work_t result = { .cell = cell, .position = position };
                return result;
            }
        } else {
            if (difference < 0) {
                mpmc_work_t result = { .cell = NULL, .position = 0 };
                return result;
            } else {
                position = atomic_load_explicit(in_or_out, memory_order_relaxed);
                /* a deadlock happens here, if the job queue is full and running from one, main thread */
            }
        }
    }
}

static int32_t mpmc_enqueue(mpmc_queue_t *q, stitch_t *stitch)
{
    mpmc_work_t work = mpmc_queue_work(q, &q->enqueue_pos, 0);

    if (work.cell) {
        work.cell->stitch = *stitch;
        atomic_store_explicit(&work.cell->sequence, work.position + 1, memory_order_release);
        return 1;
    }
    return 0;
}

static int32_t mpmc_dequeue(mpmc_queue_t *q, stitch_t *stitch)
{
    mpmc_work_t work = mpmc_queue_work(q, &q->dequeue_pos, 1);

    if (work.cell) {
        *stitch = work.cell->stitch;
        atomic_store_explicit(&work.cell->sequence, work.position + q->buffer_mask + 1, memory_order_release);
        return 1;
    }
    return 0;
}

typedef struct fiber {
    stitch_t       stitch;
    sew_context_t  context;
    atomic_size_t *wait_counter;
} fiber_t;

typedef enum tls_flags {
    TLS_IN_USE  = 0,
    TLS_TO_FREE = 0x40000000, 
    TLS_TO_WAIT = 0x80000000, 
    TLS_MASK    = ~(TLS_TO_FREE | TLS_TO_WAIT),
} tls_flags_t;

struct tls {
    amw_sewing_t    *sewing;
    sew_context_t    home_context;
    uint32_t         fiber_in_use;
    uint32_t         fiber_old;
};

typedef struct main_data {
    amw_sewing_t           *sewing;
    PFN_amw_main_procedure  procedure;
    void                   *data;
} main_data_t;

struct amw_sewing {
    mpmc_queue_t   jobs;
    mpmc_cell_t   *cells;

    tls_t         *tls;
    atomic_size_t  tls_sync;

    fiber_t       *fibers;
    atomic_size_t *waiting;
    atomic_size_t *free;
    atomic_size_t *locks;

    amw_arena_t    thread_arena;
    amw_thread_t **threads;
    amw_stitch_t  *ends;

    char          *stack;
    size_t         stack_bytes;

    size_t         thread_count;
    size_t         fiber_count;
};

static tls_t *get_thread_local(amw_sewing_t *sew) 
{
    size_t index = amw_thread_index(sew->threads, sew->thread_count);
    return &sew->tls[index];
}

static size_t get_free_fiber(amw_sewing_t *sew)
{
    for (size_t i = 0; i < sew->fiber_count; i++) {
        size_t fiber = atomic_load_explicit(&sew->free[i], memory_order_relaxed);
        
        if (fiber == SEW_INVALID)
            continue;

        fiber = atomic_load_explicit(&sew->free[i], memory_order_acquire);
        if (fiber == SEW_INVALID)
            continue;

        size_t expected = fiber;

        if (atomic_compare_exchange_weak_explicit(&sew->free[i], &expected, SEW_INVALID,
                                                  memory_order_release, memory_order_relaxed))
        {
            return fiber;
        }
    }
    return SEW_INVALID;
}

static void thread_update_free_and_waiting(tls_t *tls)
{
    amw_sewing_t *sew = tls->sewing;

    if (tls->fiber_old == (uint32_t) SEW_INVALID) {
        return;
    }

    const size_t fiber_index = tls->fiber_old & TLS_MASK;

    if (tls->fiber_old & TLS_TO_FREE) {
        /* thread that added fiber to free list is the same as the one freeing it */
        atomic_store_explicit(&sew->free[fiber_index], (size_t)fiber_index, memory_order_relaxed);
    }

    if (tls->fiber_old & TLS_TO_WAIT) {
        /* wait threshold needs to be thread synced, hence the need for release */
        atomic_store_explicit(&sew->waiting[fiber_index], (size_t)fiber_index, memory_order_release);
    }

    tls->fiber_old = (uint32_t)SEW_INVALID;
}

static void weave(intptr_t raw_tls);

static size_t get_next_fiber(amw_sewing_t *sew)
{
    size_t fiber_index = SEW_INVALID;

    for (size_t i = 0; i < sew->fiber_count; i++) {

        /* double lock helps cpus that have a weak memory model
         * arm should go 2x to 3x faster using double lock when its the bottleneck. */
        size_t fiber_waiting = atomic_load_explicit(&sew->waiting[i], memory_order_relaxed);

        if (fiber_waiting == SEW_INVALID)
            continue;

        fiber_waiting = atomic_load_explicit(&sew->waiting[i], memory_order_acquire);

        if (fiber_waiting == SEW_INVALID)
            continue;

        fiber_t       *fiber    = &sew->fibers[fiber_waiting];
        size_t         finished = 1;
        atomic_size_t *counter  = fiber->wait_counter;
        
        if (counter) {
            size_t left = atomic_load_explicit(counter, memory_order_relaxed);
            finished = (!left);
        }
        if (!finished)
            continue;

        size_t expected = fiber_waiting;

        if (atomic_compare_exchange_weak_explicit(&sew->waiting[i], &expected, SEW_INVALID,
                                                  memory_order_release, memory_order_relaxed))
        {
            fiber_index = i;
            break;
        }
    }

    if (fiber_index == SEW_INVALID) {
        stitch_t job;

        if (mpmc_dequeue(&sew->jobs, &job)) {

            while (fiber_index == SEW_INVALID)
                fiber_index = get_free_fiber(sew);

            fiber_t *fiber = &sew->fibers[fiber_index];
            fiber->stitch  = job;

            const size_t bytes = sew->stack_bytes + SEWING_STACK_GUARD;
            char        *stack = &sew->stack[(fiber_index * bytes) + bytes];

            make_context(&fiber->context, weave, stack, sew->stack_bytes);
        }
    }
    return fiber_index;
}

static tls_t *next_fiber(amw_sewing_t *sew, tls_t *tls, sew_context_t *context)
{
    atomic_size_t *wait_counter = NULL;

    if ((tls->fiber_old != (uint32_t)SEW_INVALID) && (tls->fiber_old & TLS_TO_WAIT)) {
        const size_t fiber_index = tls->fiber_old & TLS_MASK;
        fiber_t     *fiber = &sew->fibers[fiber_index];

        wait_counter = fiber->wait_counter;
    }

    for (;;) {
        size_t fiber_index = get_next_fiber(sew);

        if (fiber_index != SEW_INVALID) {
            fiber_t *fiber = &sew->fibers[fiber_index];
            tls->fiber_in_use = (uint32_t)fiber_index;

            return (tls_t *)jump_context(tls, context, &fiber->context);
        }

        /* Race condition fix.
         * 
         * Context needs to wait until a set of jobs are done, so starts
         * swapping to do a new job.
         *
         * The jobs finish *before* a new context to swap to is found.
         * There are no new jobs.
         *
         * The context swap code deadlocks looking for a new job to swap to,
         * but none will arrive. Meanwhile the 'to be swapped' context is
         * waiting to be run, but cannot as it hasn't been swapped out yet
         * (in order to be picked up by the wait list).
         */
        if (wait_counter) {
            size_t count = atomic_load_explicit(wait_counter, memory_order_relaxed);
            
            if (!count) {
                /* tls->fiber_in_use still points to the 'to waitlist' fiber */
                tls->fiber_old = (uint32_t)SEW_INVALID;
                return tls;
            }
        }
    }
}

static void weave(intptr_t raw_tls)
{
    tls_t        *tls = (tls_t *)raw_tls;
    amw_sewing_t *sew = tls->sewing;

    thread_update_free_and_waiting(tls);

    /* do the job */
    {
        fiber_t *fiber = &sew->fibers[tls->fiber_in_use];
        fiber->stitch.job.procedure(fiber->stitch.job.argument);

        if (fiber->stitch.stitches_left) {
            size_t last = atomic_fetch_sub_explicit(fiber->stitch.stitches_left, (size_t)1u, memory_order_relaxed);
            if (!last) {
                amw_assert(last > 0);
            }
        }
    }

    tls = get_thread_local(sew);
    fiber_t *old_fiber = &sew->fibers[tls->fiber_in_use];
    tls->fiber_old = tls->fiber_in_use | TLS_TO_FREE;

    next_fiber(sew, tls, &old_fiber->context);
    amw_assert_always(!"Unreachable code, shouldn't get here");
}

static atomic_size_t *get_lock(amw_sewing_t *sew, size_t initial_value)
{
    for (;;) {
        for (size_t i = 0; i < sew->fiber_count; i++) {
            atomic_size_t *lock = &sew->locks[i];

            if (atomic_load_explicit(lock, memory_order_relaxed) == SEW_INVALID) {
                size_t expected = SEW_INVALID;

                if (atomic_compare_exchange_weak_explicit(lock, &expected, initial_value, 
                                                          memory_order_relaxed, memory_order_relaxed))
                {
                    return lock;
                }
            }
        }
    }
}

static void *start_job(void *raw_tls) 
{
    tls_t        *tls = (tls_t *) raw_tls;
    amw_sewing_t *sew = tls->sewing;

    while (!atomic_load_explicit(&sew->tls_sync, memory_order_acquire)) { /* spin */ }

    tls->fiber_old = (uint32_t)SEW_INVALID;
    tls = next_fiber(sew, tls, &tls->home_context);
    thread_update_free_and_waiting(tls);

    return NULL;
}

static void quit_job(void *raw_sew) 
{
    amw_sewing_t *sew = (amw_sewing_t *)raw_sew;
    tls_t        *tls = get_thread_local(sew);
    fiber_t      *old_fiber = &sew->fibers[tls->fiber_in_use];

    if (tls == &sew->tls[0]) {
        for (size_t i = 1; i < sew->thread_count; i++) {
            amw_thread_join(sew->threads[i]);
        }
    }
    tls->fiber_old = tls->fiber_in_use | TLS_TO_FREE;

    jump_context(tls, &old_fiber->context, &tls->home_context);
    amw_assert_always(!"Unreachable code, shouldn't get here");
}

static void main_sewing_procedure(void *raw_main_data) {
    main_data_t  *main_data = (main_data_t *)raw_main_data;
    amw_sewing_t *sew = main_data->sewing;
    const size_t  threads = sew->thread_count;

    main_data->procedure(sew, sew->threads, threads, main_data->data);

    /* returned from main procedure, tell everyone to quit */
    for (size_t i = 0; i < threads; ++i) {
        sew->ends[i].procedure = quit_job;
        sew->ends[i].argument  = (void *)sew;
        sew->ends[i].name      = "sewing:quit_job";
    }
    amw_sew_stitches_and_wait(sew, sew->ends, threads);
    amw_assert_always(!"Unreachable code, shouldn't get here");
}

void amw_sew_stitches_and_wait(amw_sewing_t *sewing, 
                               amw_stitch_t *stitches, 
                               size_t stitch_count)
{
    amw_sew_chain_t chain;

    amw_sew_stitches(sewing, stitches, stitch_count, &chain);
    amw_sew_wait(sewing, chain);
}

void amw_sew_stitches(amw_sewing_t *sewing, 
                      amw_stitch_t *stitches, 
                      size_t stitch_count, 
                      amw_sew_chain_t *chain)
{
    atomic_size_t **counter        = (atomic_size_t **)chain;
    atomic_size_t  *counter_to_use = NULL;

    if (counter) {
        *counter       = get_lock(sewing, stitch_count);
        counter_to_use = *counter;
    }

    for (size_t i = 0; i < stitch_count; ++i) {
        stitch_t stitch = { .job = stitches[i], .stitches_left = counter_to_use };
        while (!mpmc_enqueue(&sewing->jobs, &stitch)) { /* wziuuuum */ }
    }
}

void amw_sew_wait(amw_sewing_t *sewing, amw_sew_chain_t chain)
{
    atomic_size_t *counter = (atomic_size_t *)chain;
    size_t         wait_value = 0;

    if (counter) {
        wait_value = atomic_load_explicit(counter, memory_order_relaxed);
        amw_assert(wait_value != SEW_INVALID);
    }

    if (wait_value) {
        tls_t   *tls = get_thread_local(sewing);
        fiber_t *old_fiber = &sewing->fibers[tls->fiber_in_use];

        old_fiber->wait_counter = counter;
        tls->fiber_old = tls->fiber_in_use | TLS_TO_WAIT;
        tls = next_fiber(sewing, tls, &old_fiber->context);

        thread_update_free_and_waiting(tls);
    }
    if (counter) {
        atomic_store_explicit(counter, SEW_INVALID, memory_order_release);
    }
}

void amw_sew_external(amw_sewing_t *sewing, amw_sew_chain_t *chain)
{
    atomic_size_t **counter = (atomic_size_t **)chain;
    *counter = get_lock(sewing, 1);
}

void amw_sew_external_finished(amw_sew_chain_t chain)
{
    atomic_size_t *counter = (atomic_size_t *)chain;
    atomic_store_explicit(counter, 0ul, memory_order_release);
}

size_t amw_sew_it(void *sewing_memory, 
                  size_t fiber_stack_bytes, 
                  size_t thread_count,
                  size_t log_2_job_count,
                  size_t fiber_count,
                  PFN_amw_main_procedure main_procedure,
                  amw_procedure_argument_t main_procedure_argument)
{
    amw_sewing_t *sewing = NULL;
    {
        size_t sewing_bytes = A16(sizeof(amw_sewing_t));

        size_t cells_count      = 1ull << log_2_job_count;
        size_t cells_bytes      = A16(sizeof(mpmc_cell_t) * cells_count);
        size_t tls_bytes            = A16(sizeof(tls_t)) * thread_count;
        size_t fibers_bytes     = A16(sizeof(fiber_t) * fiber_count);
        size_t waiting_bytes    = A16(sizeof(atomic_size_t) * fiber_count);
        size_t free_bytes       = A16(sizeof(atomic_size_t) * fiber_count);
        size_t lock_bytes       = A16(sizeof(atomic_size_t) * fiber_count);
        size_t thread_bytes     = A16(sizeof(amw_thread_t *) * thread_count);
        size_t end_bytes        = A16(sizeof(amw_stitch_t) * thread_count);
        size_t stack_bytes      = A16(fiber_stack_bytes);
        size_t stack_heap_bytes = stack_bytes * fiber_count;
        size_t bytes_alignment  = 15 + (SEWING_STACK_GUARD);

        stack_heap_bytes += SEWING_STACK_GUARD * (fiber_count + 1);

        size_t total_bytes = 
            sewing_bytes +
            cells_bytes +
            tls_bytes +
            fibers_bytes +
            waiting_bytes +
            free_bytes +
            lock_bytes +
            thread_bytes +
            end_bytes +
            stack_heap_bytes +
            bytes_alignment;

        if (!sewing_memory)
            return total_bytes;

        sewing = (amw_sewing_t *)A16((intptr_t)sewing_memory);

        {
            uint8_t *z = (uint8_t *)sewing;
            for (size_t i = 0; i < total_bytes; i++) {
                z[i] = 0;
            }
        }

        size_t o = sewing_bytes;
        char  *raw = (char *)sewing;

        sewing->cells   = (mpmc_cell_t *)    &raw[o]; o += cells_bytes;
        sewing->tls     = (tls_t *)          &raw[o]; o += tls_bytes;
        sewing->fibers  = (fiber_t *)        &raw[o]; o += fibers_bytes;
        sewing->waiting = (atomic_size_t *)  &raw[o]; o += waiting_bytes;
        sewing->free    = (atomic_size_t *)  &raw[o]; o += free_bytes;
        sewing->locks   = (atomic_size_t *)  &raw[o]; o += lock_bytes;
        sewing->threads = (amw_thread_t **)  &raw[o]; o += thread_bytes;
        sewing->ends    = (amw_stitch_t *)   &raw[o]; o += end_bytes;

#if (SEW_STACK_GUARD > 0)
        sewing->stack = (char *)((((intptr_t) &raw[0]) + (SEWING_STACK_GUARD - 1)) & ~(SEWING_STACK_GUARD - 1));
#else
        sewing->stack = (char *) &raw[0];
#endif
        sewing->stack_bytes  = stack_bytes;
        sewing->thread_count = thread_count;
        sewing->fiber_count  = fiber_count;

        amw_assert(!(((intptr_t) sewing->cells  ) & 15));
        amw_assert(!(((intptr_t) sewing->tls    ) & 15));
        amw_assert(!(((intptr_t) sewing->fibers ) & 15));
        amw_assert(!(((intptr_t) sewing->waiting) & 15));
        amw_assert(!(((intptr_t) sewing->free   ) & 15));
        amw_assert(!(((intptr_t) sewing->locks  ) & 15));
        amw_assert(!(((intptr_t) sewing->threads) & 15));
        amw_assert(!(((intptr_t) sewing->ends   ) & 15));
        amw_assert(!(((intptr_t) sewing->stack  ) & 15));
    }

    mpmc_queue_init(&sewing->jobs, sewing->cells, 1ull << log_2_job_count);

    for (size_t i = 0; i <= sewing->fiber_count; i++) {
        stack_memory_guard(&sewing->stack[i * (sewing->stack_bytes + SEWING_STACK_GUARD)], SEWING_STACK_GUARD);
    }

    for (size_t i = 0; i < sewing->fiber_count; i++) {
        atomic_store_explicit(&sewing->free[i], (size_t)i, memory_order_release);
        atomic_store_explicit(&sewing->waiting[i], SEW_INVALID, memory_order_release);
        atomic_store_explicit(&sewing->locks[i], SEW_INVALID, memory_order_release);
    }

    /* setup main thread */
    {
        sewing->tls[0].sewing       = sewing;
        sewing->tls[0].fiber_in_use = (uint32_t) get_free_fiber(sewing);
        sewing->threads[0]          = amw_thread_current(&sewing->thread_arena);
    }

    atomic_store_explicit(&sewing->tls_sync, (size_t) 0, memory_order_release);

    for (size_t i = 1; i < thread_count; i++) {
        tls_t *tls = &sewing->tls[i];

        tls->sewing       = sewing;
        tls->fiber_in_use = (uint32_t)SEW_INVALID;

        sewing->threads[i] = amw_thread_create(&sewing->thread_arena, start_job, (void *)tls);
    }
    atomic_store_explicit(&sewing->tls_sync, (size_t)1, memory_order_release);

    main_data_t  main_data = { 
        .sewing = sewing, 
        .procedure = main_procedure, 
        .data = main_procedure_argument,
    };
    amw_stitch_t main_job = { 
        .procedure = main_sewing_procedure, 
        .argument = (void *)&main_data, 
        .name = "main_sewing_procedure",
    };

    amw_sew_stitches(sewing, &main_job, 1, NULL);
    start_job((void *)&sewing->tls[0]);

    for (size_t i = 0; i <= sewing->fiber_count; i++) {
        stack_memory_stop(&sewing->stack[i * (sewing->stack_bytes + SEWING_STACK_GUARD)], SEWING_STACK_GUARD);
    }

    amw_arena_free(&sewing->thread_arena);
    return 0;
}
