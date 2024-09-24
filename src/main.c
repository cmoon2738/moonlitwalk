#include "amw.h"
#include "system.h"
#include "hadopelagic.h"
#include "renderer/vulkan.h"

static void parse_arguments(int32_t argc, const char **argv)
{
    (void)argc;
    (void)argv;
}

int32_t main(int32_t argc, const char **argv)
{
    parse_arguments(argc, argv);

    amw_log_init(NULL);
    amw_hadal_init(0);

    amw_window_t *window = amw_window_create("moonlitwalk", 800, 600, 0);

    amw_hadal_terminate();
    amw_log_terminate();
}
