#include <moonlitwalk/amw.h>

#include "lake.h"

amw_lake_t  lake;
static bool initialized;

#define SQUARE_MAX_VELOCITY         AMW_PIf
#define SQUARE_ACCELERATION         AMW_PI_2f
#define SQUARE_STOP_ACCELERATION    (15.0f * AMW_PIf)

bool amw_lake_init_game(void)
{
    amw_assert(!initialized);

    return AMW_SUCCESS;
}

void amw_lake_terminate_game(void)
{
    amw_assert(initialized);

    return;
}

int32_t amw_lake_update_game(void)
{
    float fdt = (float)(TIMESTEP_NS) / (1000.f * 1000.0f * 1000.0f);

    if (lake.freeze) {
        float acceleration = SQUARE_STOP_ACCELERATION * 
            (0.0f - lake.rotation_velocity) / SQUARE_MAX_VELOCITY;
        lake.rotation_velocity += fdt * acceleration;
    } else {
        lake.rotation_velocity += fdt * SQUARE_ACCELERATION * 
            (float)lake.direction;
    }

    lake.rotation_velocity = amw_max(
        amw_min(lake.rotation_velocity, SQUARE_MAX_VELOCITY), -1.0f * SQUARE_MAX_VELOCITY);

    lake.rotation_angle += lake.rotation_velocity * fdt;

    if (lake.rotation_angle >= 2.0f * AMW_PIf) {
        lake.rotation_angle -= 2.0f * AMW_PIf;
    }
    if (lake.rotation_angle < 0.0f) {
        lake.rotation_angle += 2.0f * AMW_PIf;
    }

    return AMW_SUCCESS;
}
