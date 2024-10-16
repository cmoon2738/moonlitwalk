#ifndef _amw_linux_joystick_h_
#define _amw_linux_joystick_h_

#include <moonlitwalk/defines.h>

AMW_C_DECL_BEGIN
#ifdef AMW_PLATFORM_LINUX

#include <linux/input.h>
#include <linux/limits.h>
#include <regex.h>

typedef struct amw_joystick_linux amw_joystick_linux_t;
#define AMW_HADAL_JOYSTICK_LINUX_STATE amw_joystick_linux_t linux_js;

typedef struct hadopelagic_joystick_linux hadopelagic_joystick_linux_t;
#define AMW_HADAL_HADOPELAGIC_JOYSTICK_LINUX_STATE hadopelagic_joystick_linux_t linux_js;

struct amw_joystick_linux {
    int32_t                 fd;
    char                    path[PATH_MAX];
    int32_t                 key_map[KEY_CNT - BTN_MISC];
    int32_t                 abs_map[ABS_CNT];
    struct input_absinfo    abs_info[ABS_CNT];
    int32_t                 hats[4][2];
};

struct hadopelagic_joystick_linux {
    int32_t     inotify;
    int32_t     watch;
    regex_t     regex;
    bool        regex_compiled;
    bool        dropped;
};

struct amw_joystick;

extern void         AMW_CALL hadal_linux_detect_joystick_connection(void);
extern bool         AMW_CALL hadal_linux_init_joysticks(void);
extern void         AMW_CALL hadal_linux_terminate_joysticks(void);
extern bool         AMW_CALL hadal_linux_poll_joystick(struct amw_joystick *js, int32_t mode);

#else
    /* null the linux joystick structure */
    #define AMW_HADAL_JOYSTICK_LINUX_STATE
    #define AMW_HADAL_HADOPELAGIC_JOYSTICK_LINUX_STATE 
#endif /* AMW_PLATFORM_LINUX */
AMW_C_DECL_END

#endif /* _amw_linux_joystick_h_ */
