#ifndef _amw_lake_h_
#define _amw_lake_h_

#include <moonlitwalk/amw.h>
#include <moonlitwalk/hadopelagic.h>
#include <moonlitwalk/sewing.h>

#include "renderer/vk.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TIMESTEP_NS (4L * 1000L * 1000L)

/* game data */
typedef struct {
    amw_window_t *window;
    amw_vulkan_t  vk;
    amw_sewing_t *sewing;

    amw_arena_t   temporary_arena;

    float   rotation_velocity;
    float   rotation_angle;
    int32_t direction;
    bool    freeze;
    bool    done;

    struct {
        int32_t threads, cores, packages;
    } cpu;
} amw_lake_t;

extern bool    AMW_CALL amw_lake_init_game(void);
extern void    AMW_CALL amw_lake_terminate_game(void);
extern int32_t AMW_CALL amw_lake_update_game(void);

extern amw_lake_t lake;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_lake_h_ */
