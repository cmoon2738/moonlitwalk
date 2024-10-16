#ifndef _amw_sewing_h_
#define _amw_sewing_h_

#include <moonlitwalk/defines.h>
#include <moonlitwalk/os.h>

AMW_C_DECL_BEGIN

/** Opaque handle for the fiber job system */
typedef struct amw_sewing amw_sewing_t;

/** 
 * The sew chain is bind to a job (or an array of jobs) and will block 
 * any wait calls to the sewing system, until the binded jobs of a chain 
 * are all finished. So it's used as a synchronization primitive.
 */
typedef void *amw_sew_chain_t;

typedef void *amw_procedure_argument_t;
typedef void (AMW_CALL *PFN_amw_procedure)(amw_procedure_argument_t);

typedef void (AMW_CALL *PFN_amw_main_procedure)(amw_sewing_t *sewing,
                                                amw_thread_t **threads,
                                                size_t thread_count,
                                                amw_procedure_argument_t argument);

/** A job run by a fiber context */
typedef struct amw_stitch {
    PFN_amw_procedure         procedure;
    amw_procedure_argument_t  argument;
    const char               *name;
} amw_stitch_t;

/**
 * Run 'stitch_count' amount of jobs (passed in the flat array 'stitches'),
 * and wait for all of them to finish.
 *
 * This will block until all items have been queued in the job queue.
 *
 * 'amw_sewing_t' is the opaque pointer passed into the call to sew it's
 * 'main_procedure' procedure.
 */
AMW_API void AMW_CALL amw_sew_stitches_and_wait(amw_sewing_t *sewing, 
                                                amw_stitch_t *stitches, 
                                                size_t stitch_count);

/**
 * Run 'stitch_count' amount of jobs (passed in the flat array 'stitches'),
 * and if chain is non-null, then set chain to a value that can be used to wait 
 * for the jobs in a call to 'mw_sew_wait'.
 *
 * This will block until all items have been queued in the job queue.
 */
AMW_API void AMW_CALL amw_sew_stitches(amw_sewing_t *sewing, 
                                       amw_stitch_t *stitches, 
                                       size_t stitch_count, 
                                       amw_sew_chain_t *chain);

/**
 * If chain is not NULL, then wait for all the jobs attached to chain
 * to finish running. If chain is null, then the call may or may not be 
 * yield to the job system before returning.
 */
AMW_API void AMW_CALL amw_sew_wait(amw_sewing_t *sewing, amw_sew_chain_t chain);

/**
 * Used to get a chain that's not tied to any stitches.
 *
 * The chain is markes as "in progress", and will block 
 * any call to 'amw_sew_wait()' until it is marked as finished.
 */
AMW_API void AMW_CALL amw_sew_external(amw_sewing_t *sewing, amw_sew_chain_t *chain);

/**
 * Used to mark a chain taken from 'amw_sew_external()' as finished.
 *
 * Once called, the chain is now invalid and calling again with
 * this chain is probably not a good idea... fiber leaks etc.
 */
AMW_API void AMW_CALL amw_sew_external_finished(amw_sew_chain_t chain);

/**
 * 'amw_sew_it()' serves two purposes:
 * 1) Entry point to the sewing system.
 * 2) Get the amount of memory required for 'sewing_memory' (in bytes)
 *
 * When passed with a non-NULL value of 'sewing_memory', then it will initialize
 * its internal state using the memory pointed to by 'sewing_memory'. It will create
 * 'fiber_count' of fibers, using a stack of 'stack_bytes'. It will create 
 * a threadsafe job queue with a size of (1 << log_2_job_count) jobs. It will then
 * create and initialize 'thread_count' -1 (main) threads before calling 'main_procedure'
 * with the arguments of 'sewing', 'threads', 'thread_count' and 'main_procedure_argument'.
 * Where 'sewing' is an opaque pointer to the sewing system required by any call to the 
 * sewing system. When the function returns then 'sewing_memory' may be deallocated.
 *
 * If 'sewing_memory' is NULL, then 'amw_sew_it' will return the amount of bytes needed
 * for its internal state using 'thread_count' threads, 'fiber_count' fibers using a 
 * stack size of 'stack_bytes', and a job queue of (1 << log_2_job_count) jobs.
 * Create a buffer of that size and pass that into 'mw_sew_it' to start the sewing system.
 */
AMW_API size_t AMW_CALL amw_sew_it(void *sewing_memory, 
                                   size_t fiber_stack_bytes, 
                                   size_t thread_count,
                                   size_t log_2_job_count,
                                   size_t fiber_count,
                                   PFN_amw_main_procedure main_procedure,
                                   amw_procedure_argument_t main_procedure_argument);

AMW_C_DECL_END

#endif /* _amw_sewing_h_ */
