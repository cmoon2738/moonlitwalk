#include <moonlitwalk/amw.h>
#include <moonlitwalk/hadopelagic.h>
#include <moonlitwalk/sewing.h>
#include <moonlitwalk/system.h>

#include "lake.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static int32_t opt_log_level = AMW_LOG_VERBOSE;
static int32_t opt_platform_id = AMW_HADAL_ANY_PLATFORM;

static bool log_level_from_string(const char *str, int32_t *level)
{
    if ((strcmp(str, "verbose")) == 0) {
        *level = AMW_LOG_VERBOSE;
    } else if (strcmp(str, "debug") == 0) {
        *level = AMW_LOG_DEBUG;
    } else if (strcmp(str, "info") == 0) {
        *level = AMW_LOG_INFO;
    } else if (strcmp(str, "warn") == 0) {
        *level = AMW_LOG_WARN;
    } else if (strcmp(str, "error") == 0) {
        *level = AMW_LOG_ERROR;
    } else if (strcmp(str, "fatal") == 0) {
        *level = AMW_LOG_FATAL;
    } else {
        return AMW_FALSE;
    }
    return AMW_TRUE;
}

static bool platform_id_from_string(const char *str, int32_t *id) 
{
    if ((strcmp(str, "auto")) == 0) {
        *id = AMW_HADAL_ANY_PLATFORM;
#ifdef AMW_NATIVE_WAYLAND
    } else if (strcmp(str, "wayland") == 0) {
        *id = AMW_HADAL_PLATFORM_WAYLAND;
#endif
#ifdef AMW_NATIVE_XCB
    } else if (strcmp(str, "xcb") == 0) {
        *id = AMW_HADAL_PLATFORM_XCB;
#endif
#ifdef AMW_NATIVE_KMS
    } else if (strcmp(str, "kms") == 0) {
        *id = AMW_HADAL_PLATFORM_KMS;
#endif
    } else if (strcmp(str, "headless") == 0) {
        *id = AMW_HADAL_PLATFORM_HEADLESS;
    } else {
        return AMW_FALSE;
    }
    return AMW_TRUE;
}

AMW_NORETURN static void print_version(FILE *f) 
{
    fprintf(f, "Lake in the Lungs, A MoonlitWalk Engine ver. %d.%d.%d", 
            AMW_VERSION_MAJOR, AMW_VERSION_MINOR, AMW_VERSION_REVISION);
    amw_exit(0);
}

AMW_NORETURN static void print_usage(FILE *f) 
{
    const char *usage = 
        "usage: moonlitwalk [-v/-h] [-l <log_level>] [-d <display_mode>]\n"
        "\n"
        "   -l <log_level>      Set a log scope, where <log_level> is one of:\n"
        "                       'verbose' (default), 'debug', 'info', 'warn', 'error', 'fatal'.\n"
        "\n"
        "   -d <display_mode>   Choose a display backend, where <display_mode> is one of:\n"
        "                       'auto' (native platform), 'headless'"
#ifdef AMW_NATIVE_WAYLAND
        ", 'wayland'"
#endif
#ifdef AMW_NATIVE_XCB
        ", 'xcb'"
#endif
#ifdef AMW_NATIVE_KMS
        ", 'kms'"
#endif
        ".\n\n"
        "   -h                  Displays this help message.\n"
        "\n"
        "   -v                  Prints the version.\n"
        ;
    fprintf(f, "\n%s", usage);
    amw_exit(0);
}

static void parse_arguments(int32_t argc, char **argv)
{
    /* Setting '+' in the optstring is the same as setting POSIXLY_CORRECT in
     * the enviroment. It tells getopt to stop parsing argv when it encounters
     * the first non-option argument; it also prevents getopt from permuting
     * argv during parsing.
     *
     * The initial ':' in the optstring makes getopt return ':' when an option
     * is missing a required argument.
     */
    static const char *optstring = "+:hvl:d:";
    int32_t opt;

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
        case 'h':   print_usage(stderr);
        case 'v':   print_version(stderr);
        case 'l':
            if (!log_level_from_string(optarg, &opt_log_level)) {
                fprintf(stderr, "Option '-%c' given bad log level.\n", optopt);
                print_usage(stderr);
            }
            break;
        case 'd':
            if (!platform_id_from_string(optarg, &opt_platform_id)) {
                fprintf(stderr, "Option '-%c' given bad display mode.\n", optopt);
                print_usage(stderr);
            }
            break;
        case '?':
            fprintf(stderr, "Unknown option '-%C'.\n", optopt);
            print_usage(stderr);
        case ':':
            fprintf(stderr, "Option '-%c' requires an argument.\n", optopt);
            print_usage(stderr);
        default:
            amw_exit(0);
        }
    }
    if (optind != argc) {
        fprintf(stderr, "Error trailing arguments.\n");
        print_usage(stderr);
    }
}

static int64_t timespec_diff(struct timespec *end, struct timespec *start) {
    int64_t sec = (int64_t)end->tv_sec - (int64_t)start->tv_sec;
    int64_t sec_diff = sec * 1000L * 1000L * 1000L;
    int64_t nsec_diff = (int64_t)end->tv_nsec - (int64_t)start->tv_nsec;
    return sec_diff + nsec_diff;
}

static void cleanup(void)
{
    amw_vk_terminate(&lake.vk);

    amw_hadal_destroy_window(lake.window);
    amw_hadal_terminate();

    if (lake.sewing)
        amw_free(lake.sewing);

    amw_log_terminate();
    amw_arena_free(&lake.temporary_arena);
}

static void jobtest(amw_procedure_argument_t argument)
{
    uint32_t i = (uint32_t)(size_t)argument;
    amw_log_debug("%d", i);
}

static void main_loop(amw_sewing_t *sewing,
                      amw_thread_t **threads,
                      size_t thread_count,
                      amw_procedure_argument_t argument)
{
    /* unused */
    (void)threads;
    (void)thread_count;
    (void)argument;

    struct timespec last_update;
    struct timespec now;
    int32_t error;
    int64_t delta_time;

    /* FIXME remove this later */
    amw_stitch_t jobs[10];
    for (uint32_t i = 0; i < 10; i++) {
        jobs[i].procedure = jobtest;
        jobs[i].argument = (amw_procedure_argument_t)(size_t)i;
        jobs[i].name = "jobtest";
    }
    amw_sew_stitches_and_wait(sewing, jobs, 10);

    do {
        error = clock_gettime(CLOCK_MONOTONIC, &last_update);
    } while (error == EINTR);
    if (error != 0) {
        amw_log_error("Main loop initial clock error '%d'", error);
        amw_exitcode(error);
        return;
    }

    /* TODO now i can update the clock here :3 but im lazy so the future me will do it */

    int64_t remainder = 0;
    while (!amw_hadal_read(lake.window, AMW_WINDOW_FLAG_SHOULD_CLOSE)) {
        do {
            error = clock_gettime(CLOCK_MONOTONIC, &now);
        } while (error == EINTR);
        if (error != 0) {
            amw_log_error("Main loop clock error '%d'", error);
            amw_exitcode(error);
            return;
        }

        delta_time = timespec_diff(&now, &last_update) + remainder;
        while (delta_time >= TIMESTEP_NS) {
            delta_time -= TIMESTEP_NS;
            amw_lake_update_game(); /* FIXME for now it will work until it's just a spinning cube model, but i'll have to find a different way to do the delta_time calculations */
        }
        last_update = now;
        remainder = delta_time;

        //amw_vk_draw_frame(&lake.vk); 
    
        /* FIXME remove this later */
        amw_hadal_should_close(lake.window, AMW_TRUE);
    }

    //vkDeviceWaitIdle(lake.vk.device);

    /* when the main loop returns, the sewing system 
     * will destroy all threads (except the main one)
     * and 'sew_it' called from 'a_moonlit_walk' 
     * will return, proceding to cleanup. */
}

static int32_t a_moonlit_walk(void)
{
    int32_t res = AMW_SUCCESS;

    amw_log_init(NULL);
    amw_log_set_level(opt_log_level);

    amw_cpu_count(&lake.cpu.threads, &lake.cpu.cores, &lake.cpu.packages);

    const size_t sewing_bytes = amw_sew_it(
            NULL,               /* set to null to get the required memory stack */
            32 * 1024,          /* 32kb stack per fiber */
            lake.cpu.threads,   /* how many threads to run (including the main thread) */
            10,                 /* job queue will be (1 << 10) entries large */
            128,                /* we can have up to 128 fibers at once */
            main_loop,          /* entry point for the sewing system */
            NULL);              /* argument for the entry point */
    lake.sewing = (amw_sewing_t *)amw_malloc(sewing_bytes);

    res = amw_hadal_init(opt_platform_id);
    if (res != AMW_SUCCESS) {
        amw_log_fatal("Could not initialize Hadal, code '%d'.", res);
        cleanup();
        return res;
    }

    lake.window = amw_hadal_create_window("moonlitwalk", 800, 600, 0);
    if (lake.window == NULL) {
        amw_log_fatal("Failed to create a window, received a null pointer.");
        cleanup();
        return -1; // TODO error code
    }

    res = amw_vk_init(&lake.vk, lake.window);
    if (res != AMW_SUCCESS) {
        amw_log_fatal("Could not initialize Vulkan backend, code '%d'.", res);
        cleanup();
        return res;
    }
    amw_hadal_visible(lake.window, AMW_TRUE);

    res = amw_lake_init_game();
    if (res != AMW_SUCCESS) {
        amw_log_fatal("Game error while initializing it's resources, code '%d'.", res);
        return res;
    }

    /* run the main game loop */
    amw_sew_it(
            lake.sewing,
            32 * 1024,
            lake.cpu.threads,
            10,
            128,
            main_loop,
            NULL);

    cleanup();
    return AMW_SUCCESS;
}
