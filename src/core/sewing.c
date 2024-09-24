#include <moonlitwalk/sewing.h>

#include "sew_context.h"

#if !defined(SEWING_STACK_GUARD)
    #define SEWING_STACK_GUARD 0
#endif
#define A16(x) (((x) + 15) & ~15)

#define SEW_INVALID SIZE_MAX

typedef struct thread_local tls_t;

amw_static_assert(sizeof(intptr_t) == sizeof(amw_sewing_t *), \
    "context argument size assumption failed, sew_jump_context will crash.");
amw_static_assert(sizeof(size_t) >= sizeof(void *), "size_t cannot hold pointers");

struct amw_sewing {
    amw_thread_t     *threads;
    amw_sew_stitch_t *ends;

    size_t  thread_count;
    size_t  fiber_count;
};

void amw_sew_stitches_and_wait(amw_sewing_t *sewing, 
                               amw_sew_stitch_t *stitches, 
                               size_t stitch_count)
{

}

void amw_sew_stitches(amw_sewing_t *sewing, 
                      amw_sew_stitch_t *stitches, 
                      size_t stitch_count, 
                      amw_sew_chain_t *chain)
{

}

void amw_sew_wait(amw_sewing_t *sewing, amw_sew_chain_t chain)
{

}

void amw_sew_external(amw_sewing_t *sewing, amw_sew_chain_t *chain)
{

}

void amw_sew_external_finished(amw_sew_chain_t chain)
{

}

size_t amw_sew_it(void *sewing_memory, 
                  size_t stack_bytes, 
                  size_t thread_count,
                  size_t log_2_job_count,
                  size_t fiber_count,
                  PFN_amw_main_procedure main_procedure,
                  amw_procedure_argument_t main_procedure_argument)
{
    return 0;
}
