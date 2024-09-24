#include <moonlitwalk/amw.h>
#include <moonlitwalk/hadopelagic.h>
#include <moonlitwalk/sewing.h>
#include <moonlitwalk/system.h>
#include <moonlitwalk/vk.h>

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
    fprintf(f, "\nA Moonlit Walk engine version %d.%d.%d\n", 
        AMW_VERSION_MAJOR, AMW_VERSION_MINOR, AMW_VERSION_PATCH);
    amw_vk_open_library();
    int32_t vk_version = amw_vk_version();
    fprintf(f, "\tFound Vulkan driver version %d.%d.%d\n",
        VK_VERSION_MAJOR(vk_version),
        VK_VERSION_MINOR(vk_version),
        VK_VERSION_PATCH(vk_version));
    amw_vk_close_library();
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

    amw_log_terminate();
    amw_arena_free(&lake.arena);
}

/* TODO this function will be the entry point for the fiber job system
static void main_loop(amw_sewing_t *sewing,
                      amw_thread_t *threads,
                      size_t thread_count,
                      amw_procedure_argument_t argument);
*/
static void main_loop(void)
{
    struct timespec last_update;
    struct timespec now;
    int32_t error;
    int64_t delta_time;

    do {
        error = clock_gettime(CLOCK_MONOTONIC, &last_update);
    } while (error == EINTR);
    if (error != 0) {
        amw_log_error("Main loop clock error '%d'", error);
        amw_exitcode(error);
        return;
    }

    int64_t remainder = 0;
    while (!amw_hadal_should_close(lake.window)) {
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

        amw_vk_draw_frame(&lake.vk); 
    }

    vkDeviceWaitIdle(lake.vk.device);
    return;
}

static int32_t a_moonlit_walk(void)
{
    int32_t res = AMW_SUCCESS;

    amw_log_init(&lake.arena, NULL);
    amw_log_set_level(opt_log_level);

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
        amw_log_fatal("Could not initialize Hadal, code '%d'.", res);
        cleanup();
        return res;
    }

    res = amw_lake_init_game();

    main_loop();
    cleanup();
    return AMW_SUCCESS;
}

#ifdef AMW_PLATFORM_WINDOWS
#include <windows.h>
#include <wchar.h>

static char **command_line_to_utf8_argv(LPWSTR w_command_line, int32_t *o_argc)
{
    int32_t argc = 0;
    char **argv = 0;
    char  *args;

    LPWSTR *w_argv = CommandLineToArgvW(w_command_line, &argc);
    if (w_argv == NULL) {
        amw_log_error("Win32 couldn't fetch command line arguments");
    } else {
        size_t size = wcslen(w_command_line) * 4;
        void *ptr_argv = amw_malloc(((size_t)argc + 1) * sizeof(char *) + size);
        amw_zerop(ptr_argv);
        argv = (char **)ptr_argv;
        amw_assert_release(argv);
        args = (char *)&argv[argc + 1];

        int32_t n;
        for (int32_t i = 0; i < argc; ++i) {
            n = WideCharToMultiByte(CP_UTF8, 0, w_argv[i], -1, args, (int32_t)size, NULL, NULL);
            if (n == 0) {
                amw_log_error("Win32 got a 0 length argument");
                break;
            }
            argv[i] = args;
            size -= (size_t)n;
            args += n;
        }
        LocalFree(w_argv);
    }
    *o_argc = argc;
    return argv;
}

#ifdef AMW_PLATFORM_WINDOWS_FORCE_MAIN
int32_t main(int32_t argc, char **argv)
{
    int32_t res = AMW_SUCCESS;

    parse_arguments(argc, argv);
    res = a_moonlit_walk();
    amw_exit(res);
}
#else
int32_t WINAPI WinMain(_In_ HINSTANCE hInstance, 
                       _In_opt_ HINSTANCE hPrevInstance, 
                       _In_ LPSTR lpCmdLine, 
                       _In_ int32_t nCmdShow)
{
    /* TODO */
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    int32_t res = AMW_SUCCESS;
    int32_t argc_utf8 = 0;
    char **argv_utf8 = command_line_to_utf8_argv(GetCommandLineW(), &argc_utf8);

    parse_arguments(argc, argv);
    res = a_moonlit_walk();

    amw_free(argv_utf8);
    amw_exit(res);
}
#endif /* AMW_PLATFORM_WINDOWS_FORCE_MAIN */

#elif AMW_PLATFORM_MACOSX
    /* TODO */
#elif AMW_PLATFORM_IOS
    /* TODO */

#elif AMW_PLATFORM_ANDROID
#include <android/log.h>
#include <android_native_app_glue.h>
#include <jni.h>

JNIEXPORT void ANativeActivity_onCreate(ANativeActivity* activity, 
                                        void* saved_state, 
                                        size_t saved_state_size) 
{
    /* TODO */
}

#else
int32_t main(int32_t argc, char **argv)
{
    int32_t res = AMW_SUCCESS;

    parse_arguments(argc, argv);
    res = a_moonlit_walk();
    amw_exit(res);
}
#endif /* main entry points */
