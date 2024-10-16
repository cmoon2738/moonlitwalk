#ifndef _amw_h_
#define _amw_h_

#include <moonlitwalk/defines.h>

#include <moonlitwalk/events.h>
#include <moonlitwalk/hadal.h>
#include <moonlitwalk/math.h>
#include <moonlitwalk/moony.h>
#include <moonlitwalk/os.h>
#include <moonlitwalk/sewing.h>
#include <moonlitwalk/simd.h>

AMW_C_DECL_BEGIN

/**
 * Application defined callbacks, used from 'a_moonlit_walk'.
 * - 'init' is called before entering the main loop, all engine subsystems are initialized.
 * - 'frame' is called after most of the updates, before drawing.
 * - 'event' is called at begin of the mainloop, while polling events.
 * - 'clean' is called after leaving the main loop, before any cleaning in the engine.
 */
typedef int32_t (AMW_CALL *PFN_amw_init)(void *userdata);
typedef int32_t (AMW_CALL *PFN_amw_frame)(void *userdata, double delta_time);
typedef void    (AMW_CALL *PFN_amw_event)(void *userdata, amw_event_t *event);
typedef void    (AMW_CALL *PFN_amw_clean)(void *userdata);

/**
 * Describes the applications metadata and callbacks.
 *
 * If you choose to use the framework instead of handling the engine yourself,
 * you should define this structure and return it from your 'amw_main()' function.
 * Or pass it directly to 'a_moonlit_walk()', if you define your own main entry points.
 *
 * You can define the '*data' member as a structure holding the game state,
 * then allocate its memory and pass a pointer to it along with the app description.
 * Or pass NULL to the data, if you handle your game state in another way.
 */
typedef struct amw_app {
    PFN_amw_init    init;
    PFN_amw_frame   frame;
    PFN_amw_event   event;
    PFN_amw_clean   clean;
    void           *userdata;

    const char     *name;
    uint32_t        version;
} amw_app_t;

enum amw_framework_flags {
    AMW_FLAG_INITIALIZED = 0x1,
    AMW_FLAG_SHOULD_QUIT = 0x2,
};

typedef struct amw {
    uint32_t flags;
    double   delta_time;

    amw_app_t          *app;
    amw_sewing_t       *sewing;
    amw_window_t       *window;
    amw_moony_t        *moony;

    /* hardware specs */
    struct {
        int32_t cpu_threads, 
                cpu_cores, 
                cpu_packages;
    } hardware;
} amw_t;

/**
 * Runs the framework: inits all engine subsystems and setups the game application
 * via the given app description and any initialization configuration. 
 * It handles the main loop and shuts down subsystems after the app should close.
 *
 * If called with a NULL argument, or if the framework is already running,
 * it will return a pointer to the framework structure. It's members can be 
 * used for specific engine subsystem calls from within the application.
 * This state is read only, its internal structures can be passed to other systems.
 */
AMW_API const amw_t * AMW_CALL a_moonlit_walk(amw_app_t *app_desc);

/* Define this only once in your main file. */
#ifdef A_MOONLIT_WALK_MAIN
#undef A_MOONLIT_WALK_MAIN

/**
 * User defined main entry point. Called from platform-specific entry points,
 * like WinMain, main, ANativeActivity, etc.. It should parse the commandline 
 * arguments, construct an 'amw_app_t' structure and pass it as a return value.
 * Here do any setup that is needed before the engine and game state inits.
 */
extern amw_app_t AMW_CALL amw_main(int32_t argc, const char **argv);

/* WINDOWS */
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
    amw_app_t app = amw_main(argc, argv);
    a_moonlit_walk(&app);
    amw_exit(0);
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

    int32_t argc_utf8 = 0;
    char **argv_utf8 = command_line_to_utf8_argv(GetCommandLineW(), &argc_utf8);

    amw_app_t app = amw_main(argc_utf8, argv_utf8);
    a_moonlit_walk(&app);

    amw_free(argv_utf8);
    amw_exit(0);
}
#endif /* AMW_PLATFORM_WINDOWS_FORCE_MAIN */

/* APPLE MAC OSX */
#elif AMW_PLATFORM_MACOSX
    /* TODO */

/* APPLE IOS */
#elif AMW_PLATFORM_IOS
    /* TODO */

/* ANDROID */
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

/* EMSCRIPTEN WASM */
#elif AMW_PLATFORM_EMSCRIPTEN
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
    /* TODO */

/* LINUX BSD UNIX */
#else
int32_t main(int32_t argc, char **argv)
{
    amw_app_t app = amw_main(argc, argv);
    a_moonlit_walk(&app);
    amw_exit(0);
}
#endif /* main entry points */
#endif /* A_MOONLIT_WALK_MAIN */

AMW_C_DECL_END

#endif /* _amw_h_ */
