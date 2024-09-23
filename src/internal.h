#ifndef _amw_internal_h_
#define _amw_internal_h_

#include "amw.h"

#include "linux/wayland.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct amw_window {
    int32_t     width, height;
    const char *title;
    AMW_WINDOW_WAYLAND_STATE;
};

typedef struct hadal_api {
    uint32_t id; 


} hadal_api_t;

typedef struct hadopelagic {
    hadal_api_t api;
} hadopelagic_t;

/* global platform abstraction, display/input state */
extern hadopelagic_t hadal;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_hadopelagic_h_ */
