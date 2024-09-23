#include "amw.h"
#include "system.h"
#include "linux/wayland.h"
#include "renderer/vulkan.h"

typedef struct amw_window amw_window_t;

amw_window_t *amw_window_create(const char *title, int32_t width, int32_t height, uint32_t flags)
{
}

static void parse_arguments(int32_t argc, const char **argv)
{
    (void)argc;
    (void)argv;
}

int32_t main(int32_t argc, const char **argv)
{
    parse_arguments(argc, argv);

    amw_window_t window = amw_window_create("moonlitwalk", 800, 600, 0);
}
