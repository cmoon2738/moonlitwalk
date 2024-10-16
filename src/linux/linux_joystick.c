#include "linux_joystick.h"

void hadal_linux_detect_joystick_connection(void)
{

}

bool hadal_linux_init_joysticks(void)
{
    return AMW_TRUE;
}

void hadal_linux_terminate_joysticks(void)
{

}

bool hadal_linux_poll_joystick(struct amw_joystick *js, int32_t mode)
{
    (void)js;
    (void)mode;
    return AMW_TRUE;
}
