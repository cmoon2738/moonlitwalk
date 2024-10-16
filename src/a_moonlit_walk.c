#include <moonlitwalk/amw.h>

static amw_t amw = {0};

static int32_t default_app_init(void *d)                  { (void)d; return 0; }
static int32_t default_app_frame(void *d, double dt)      { (void)d; (void)dt; return 0; }
static void    default_app_event(void *d, amw_event_t *e) { (void)d; (void)e; return; }
static void    default_app_clean(void *d)                 { (void)d; return; }

static void main_loop(amw_sewing_t *sewing,
                      amw_thread_t **threads,
                      size_t thread_count,
                      amw_procedure_argument_t argument)
{
    /* unused for now */
    (void)sewing;
    (void)threads;
    (void)thread_count;
    (void)argument;

    uint64_t time_now = amw_systime_counter();
    uint64_t time_last = 0;

    while (!amw_read_bits(amw.flags, AMW_FLAG_SHOULD_QUIT)) {
        time_last = time_now;
        time_now = amw_systime_counter();
        amw.delta_time = (double)((time_now - time_last)*1000 / (double)amw_systime_frequency());

        amw_event_t *event = NULL;
        /* TODO poll from the event loop */
        while (event) {
            /* TODO handle events */
            switch (event->type) {
            default:
                break;
            }
            amw.app->event(amw.app->userdata, event);
        }

        amw_moony_begin_frame(amw.moony);

        amw.app->frame(amw.app->userdata, amw.delta_time);

        amw_moony_end_frame(amw.moony);

        /* TODO remove this later */
        amw_window_should_close(amw.window, AMW_TRUE);
        if (amw_read_bits(amw_window_flags(amw.window), AMW_WINDOW_FLAG_SHOULD_CLOSE)) {
            amw_set_bits(amw.flags, AMW_FLAG_SHOULD_QUIT);
        }
    }
    /* when the main loop returns, the sewing system 
     * will destroy all threads (except the main one)
     * and 'amw_sew_it' will return, proceeds to cleanup. */
}

static void cleanup(void)
{
    amw.app->clean(amw.app->userdata);

    amw_moony_destroy(amw.moony);

    amw_window_destroy(amw.window);
    amw_hadal_terminate();

    if (amw.sewing)
        amw_free(amw.sewing);

    amw_log_terminate();
    amw_ticks_quit();
    amw_zero(amw);
}

static void check_result(int32_t res, const char *msg)
{
    if (res != AMW_SUCCESS) {
        amw_log_fatal("%s, exit code '%d'.", msg, res);
        cleanup();
        amw_exit(res);
    }
}

const amw_t *a_moonlit_walk(amw_app_t *app)
{
    int32_t res = AMW_SUCCESS;

    if (app == NULL || amw_read_bits(amw.flags, AMW_FLAG_INITIALIZED)) {
        return &amw;
    }
    amw_set_bits(amw.flags, AMW_FLAG_INITIALIZED);

    amw_ticks_init();
    amw_log_init(NULL);

    amw_cpu_count(&amw.hardware.cpu_threads, &amw.hardware.cpu_cores, &amw.hardware.cpu_packages);

    amw.app = app;
    if (!amw.app->init)  amw.app->init  = default_app_init;
    if (!amw.app->frame) amw.app->frame = default_app_frame;
    if (!amw.app->event) amw.app->event = default_app_event;
    if (!amw.app->clean) amw.app->clean = default_app_clean;

    res = amw_hadal_init(0);
    check_result(res, "Could not initialize Hadal");

    const size_t sewing_bytes = amw_sew_it(
            NULL,                     /* set to null to get the required memory stack */
            32 * 1024,                /* 32kb stack per fiber */
            amw.hardware.cpu_threads, /* how many threads to run (including the main thread) */
            10,                       /* job queue will be (1 << 10) entries large */
            128,                      /* we can have up to 128 fibers at once */
            main_loop,                /* entry point for the sewing system */
            NULL);                    /* argument for the entry point */
    amw.sewing = (amw_sewing_t *)amw_malloc(sewing_bytes);
    if (!amw.sewing) {
        amw_log_debug("FAILURE: %lu sewing_bytes", sewing_bytes);
        check_result(-1, "Failed to allocate a memory stack for Sewing");
    }

    amw.window = amw_window_create(amw.app->name, 800, 600, 0);
    if (!amw.window)
        check_result(-1, "Failed to create a window");

    amw.moony = amw_moony_create(AMW_MOONY_ANY_BACKEND, amw.sewing, amw.window);
    if (!amw.moony)
        check_result(-1, "Failed to create a rendering pipeline");

    amw_window_visible(amw.window, AMW_TRUE);

    amw.app->init(amw.app->userdata);

    /* run the main game loop */
    amw_sew_it(
            amw.sewing,
            32 * 1024,
            amw.hardware.cpu_threads,
            10,
            128,
            main_loop,
            NULL);

    cleanup();
    return NULL;
}
