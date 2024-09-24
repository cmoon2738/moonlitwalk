#ifndef _amw_lake_h_
#define _amw_lake_h_

#include "amw.h"
#include "hadopelagic.h"
#include "renderer/vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TIMESTEP_NS (4L * 1000L * 1000L)

/* game data */
typedef struct {
    amw_window_t *window;
    amw_vulkan_t vk;

    amw_arena_t  arena;

    float rotation_velocity;
    float rotation_angle;
    int32_t direction;
    bool freeze;
    bool done;
} amw_lake_t;

bool    amw_lake_init_game(amw_lake_t *state);
void    amw_lake_terminate_game(void);
int32_t amw_lake_update_game(void);

// TODO implement events
//int32_t amw_lake_event(lake_t *lake);

amw_lake_t *amw_lake_gamedata(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_lake_h_ */
