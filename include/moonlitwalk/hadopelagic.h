#ifndef _amw_hadopelagic_h_
#define _amw_hadopelagic_h_

#include "amw.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum amw_platform_id {
    AMW_HADAL_ANY_PLATFORM = 0,
    AMW_HADAL_PLATFORM_WIN32,
    AMW_HADAL_PLATFORM_COCOA,
    AMW_HADAL_PLATFORM_IOS,
    AMW_HADAL_PLATFORM_ANDROID,
    AMW_HADAL_PLATFORM_WAYLAND,
    AMW_HADAL_PLATFORM_XCB,
    AMW_HADAL_PLATFORM_KMS,
    AMW_HADAL_PLATFORM_HEADLESS,
} amw_platform_id_t;

typedef struct amw_window amw_window_t;
typedef enum amw_window_flags {
    AMW_WINDOW_FLAG_NOTHING           = 0,
    AMW_WINDOW_FLAG_RESIZABLE         = 0x00000001,
    AMW_WINDOW_FLAG_MAXIMIZED         = 0x00000002,
    AMW_WINDOW_FLAG_TRANSPARENT       = 0x00000004,
    AMW_WINDOW_FLAG_SCALE_FRAMEBUFFER = 0x00000008,
    AMW_WINDOW_FLAG_AUTO_ICONIFY      = 0x00000010,
    AMW_WINDOW_FLAG_DECORATED         = 0x00000020,
    AMW_WINDOW_FLAG_FLOATING          = 0x00000040,
    AMW_WINDOW_FLAG_FOCUS_ON_SHOW     = 0x00000080,
    AMW_WINDOW_FLAG_MOUSE_PASSTHROUGH = 0x00000100,
} amw_window_flags_t;

typedef enum amw_keycode {
    AMW_KEY_SPACE            = 32,
    AMW_KEY_APOSTROPHE       = 39,  /* ' */
    AMW_KEY_COMMA            = 44,  /* , */
    AMW_KEY_MINUS            = 45,  /* - */
    AMW_KEY_PERIOD           = 46,  /* . */
    AMW_KEY_SLASH            = 47,  /* / */
    AMW_KEY_0                = 48,
    AMW_KEY_1                = 49,
    AMW_KEY_2                = 50,
    AMW_KEY_3                = 51,
    AMW_KEY_4                = 52,
    AMW_KEY_5                = 53,
    AMW_KEY_6                = 54,
    AMW_KEY_7                = 55,
    AMW_KEY_8                = 56,
    AMW_KEY_9                = 57,
    AMW_KEY_SEMICOLON        = 59,  /* ; */
    AMW_KEY_EQUAL            = 61,  /* = */
    AMW_KEY_A                = 65,
    AMW_KEY_B                = 66,
    AMW_KEY_C                = 67,
    AMW_KEY_D                = 68,
    AMW_KEY_E                = 69,
    AMW_KEY_F                = 70,
    AMW_KEY_G                = 71,
    AMW_KEY_H                = 72,
    AMW_KEY_I                = 73,
    AMW_KEY_J                = 74,
    AMW_KEY_K                = 75,
    AMW_KEY_L                = 76,
    AMW_KEY_M                = 77,
    AMW_KEY_N                = 78,
    AMW_KEY_O                = 79,
    AMW_KEY_P                = 80,
    AMW_KEY_Q                = 81,
    AMW_KEY_R                = 82,
    AMW_KEY_S                = 83,
    AMW_KEY_T                = 84,
    AMW_KEY_U                = 85,
    AMW_KEY_V                = 86,
    AMW_KEY_W                = 87,
    AMW_KEY_X                = 88,
    AMW_KEY_Y                = 89,
    AMW_KEY_Z                = 90,
    AMW_KEY_LEFT_BRACKET     = 91,  /* [ */
    AMW_KEY_BACKSLASH        = 92,  /* \ */
    AMW_KEY_RIGHT_BRACKET    = 93,  /* ] */
    AMW_KEY_GRAVE_ACCENT     = 96,  /* ` */
    AMW_KEY_WORLD_1          = 161, /* non-US #1 */
    AMW_KEY_WORLD_2          = 162, /* non-US #2 */
    AMW_KEY_ESCAPE           = 256,
    AMW_KEY_ENTER            = 257,
    AMW_KEY_TAB              = 258,
    AMW_KEY_BACKSPACE        = 259,
    AMW_KEY_INSERT           = 260,
    AMW_KEY_DELETE           = 261,
    AMW_KEY_RIGHT            = 262,
    AMW_KEY_LEFT             = 263,
    AMW_KEY_DOWN             = 264,
    AMW_KEY_UP               = 265,
    AMW_KEY_PAGE_UP          = 266,
    AMW_KEY_PAGE_DOWN        = 267,
    AMW_KEY_HOME             = 268,
    AMW_KEY_END              = 269,
    AMW_KEY_CAPS_LOCK        = 280,
    AMW_KEY_SCROLL_LOCK      = 281,
    AMW_KEY_NUM_LOCK         = 282,
    AMW_KEY_PRINT_SCREEN     = 283,
    AMW_KEY_PAUSE            = 284,
    AMW_KEY_F1               = 290,
    AMW_KEY_F2               = 291,
    AMW_KEY_F3               = 292,
    AMW_KEY_F4               = 293,
    AMW_KEY_F5               = 294,
    AMW_KEY_F6               = 295,
    AMW_KEY_F7               = 296,
    AMW_KEY_F8               = 297,
    AMW_KEY_F9               = 298,
    AMW_KEY_F10              = 299,
    AMW_KEY_F11              = 300,
    AMW_KEY_F12              = 301,
    AMW_KEY_F13              = 302,
    AMW_KEY_F14              = 303,
    AMW_KEY_F15              = 304,
    AMW_KEY_F16              = 305,
    AMW_KEY_F17              = 306,
    AMW_KEY_F18              = 307,
    AMW_KEY_F19              = 308,
    AMW_KEY_F20              = 309,
    AMW_KEY_F21              = 310,
    AMW_KEY_F22              = 311,
    AMW_KEY_F23              = 312,
    AMW_KEY_F24              = 313,
    AMW_KEY_F25              = 314,
    AMW_KEY_KP_0             = 320,
    AMW_KEY_KP_1             = 321,
    AMW_KEY_KP_2             = 322,
    AMW_KEY_KP_3             = 323,
    AMW_KEY_KP_4             = 324,
    AMW_KEY_KP_5             = 325,
    AMW_KEY_KP_6             = 326,
    AMW_KEY_KP_7             = 327,
    AMW_KEY_KP_8             = 328,
    AMW_KEY_KP_9             = 329,
    AMW_KEY_KP_DECIMAL       = 330,
    AMW_KEY_KP_DIVIDE        = 331,
    AMW_KEY_KP_MULTIPLY      = 332,
    AMW_KEY_KP_SUBTRACT      = 333,
    AMW_KEY_KP_ADD           = 334,
    AMW_KEY_KP_ENTER         = 335,
    AMW_KEY_KP_EQUAL         = 336,
    AMW_KEY_LEFT_SHIFT       = 340,
    AMW_KEY_LEFT_CONTROL     = 341,
    AMW_KEY_LEFT_ALT         = 342,
    AMW_KEY_LEFT_SUPER       = 343,
    AMW_KEY_RIGHT_SHIFT      = 344,
    AMW_KEY_RIGHT_CONTROL    = 345,
    AMW_KEY_RIGHT_ALT        = 346,
    AMW_KEY_RIGHT_SUPER      = 347,
    AMW_KEY_MENU             = 348,
    AMW_KEY_INVALID          = 0,
} amw_keycode_t;
#define AMW_KEY_LAST AMW_KEY_MENU

typedef enum amw_mouse_button {
    AMW_MOUSE_BUTTON_1       = 0x0001,
    AMW_MOUSE_BUTTON_2       = 0x0002,
    AMW_MOUSE_BUTTON_3       = 0x0004,
    AMW_MOUSE_BUTTON_4       = 0x0008,
    AMW_MOUSE_BUTTON_5       = 0x0010,
    AMW_MOUSE_BUTTON_6       = 0x0020,
    AMW_MOUSE_BUTTON_7       = 0x0040,
    AMW_MOUSE_BUTTON_8       = 0x0080,
    AMW_MOUSE_BUTTON_LEFT    = AMW_MOUSE_BUTTON_1,
    AMW_MOUSE_BUTTON_RIGHT   = AMW_MOUSE_BUTTON_2,
    AMW_MOUSE_BUTTON_MIDDLE  = AMW_MOUSE_BUTTON_3,
    AMW_MOUSE_BUTTON_INVALID = 0,
} amw_mouse_button_t;
#define AMW_MOUSE_BUTTON_LAST AMW_MOUSE_BUTTON_8

typedef enum amw_keymod {
    AMW_KEYMOD_SHIFT     = 0x0001, 
    AMW_KEYMOD_CTRL      = 0x0002, 
    AMW_KEYMOD_ALT       = 0x0004, 
    AMW_KEYMOD_SUPER     = 0x0008, 
    AMW_KEYMOD_CAPS_LOCK = 0x0010,
    AMW_KEYMOD_NUM_LOCK  = 0x0020,
    AMW_KEYMOD_INVALID   = 0,
} amw_keymod_t;

typedef enum amw_android_tooltype {
    AMW_ANDROID_TOOLTYPE_UNKNOWN = 0,
    AMW_ANDROID_TOOLTYPE_FINGER  = 1,
    AMW_ANDROID_TOOLTYPE_STYLUS  = 2,
    AMW_ANDROID_TOOLTYPE_MOUSE   = 3,
} amw_android_tooltype_t;

typedef struct amw_touchpoint {
    uintptr_t identifier;
    float     pos_x, pos_y;
    amw_android_tooltype_t android_tooltype; // only valid on Android
    bool      changed;
} amw_touchpoint_t;

typedef enum amw_joystick_id {
    AMW_JOYSTICK_1       = 0x0001,
    AMW_JOYSTICK_2       = 0x0002,
    AMW_JOYSTICK_3       = 0x0004,
    AMW_JOYSTICK_4       = 0x0008,
    AMW_JOYSTICK_5       = 0x0010,
    AMW_JOYSTICK_6       = 0x0020,
    AMW_JOYSTICK_7       = 0x0040,
    AMW_JOYSTICK_8       = 0x0080,
    AMW_JOYSTICK_9       = 0x0100,
    AMW_JOYSTICK_10      = 0x0200,
    AMW_JOYSTICK_11      = 0x0400,
    AMW_JOYSTICK_12      = 0x0800,
    AMW_JOYSTICK_13      = 0x1000,
    AMW_JOYSTICK_14      = 0x2000,
    AMW_JOYSTICK_15      = 0x4000,
    AMW_JOYSTICK_16      = 0x8000,
    AMW_JOYSTICK_INVALID = 0,
} amw_joystick_id_t;
#define AMW_JOYSTICK_LAST AMW_JOYSTICK_16

typedef enum amw_gamepad_button {
    AMW_GAMEPAD_BUTTON_A            = 0x0001,
    AMW_GAMEPAD_BUTTON_B            = 0x0002,
    AMW_GAMEPAD_BUTTON_X            = 0x0004,
    AMW_GAMEPAD_BUTTON_Y            = 0x0008,
    AMW_GAMEPAD_BUTTON_LEFT_BUMPER  = 0x0010,
    AMW_GAMEPAD_BUTTON_RIGHT_BUMPER = 0x0020,
    AMW_GAMEPAD_BUTTON_BACK         = 0x0040,
    AMW_GAMEPAD_BUTTON_START        = 0x0080,
    AMW_GAMEPAD_BUTTON_GUIDE        = 0x0100,
    AMW_GAMEPAD_BUTTON_LEFT_THUMB   = 0x0200,
    AMW_GAMEPAD_BUTTON_RIGHT_THUMB  = 0x0400,
    AMW_GAMEPAD_BUTTON_DPAD_UP      = 0x0800,
    AMW_GAMEPAD_BUTTON_DPAD_RIGHT   = 0x1000,
    AMW_GAMEPAD_BUTTON_DPAD_DOWN    = 0x2000,
    AMW_GAMEPAD_BUTTON_DPAD_LEFT    = 0x4000,
    AMW_GAMEPAD_BUTTON_CROSS        = AMW_GAMEPAD_BUTTON_A,
    AMW_GAMEPAD_BUTTON_CIRCLE       = AMW_GAMEPAD_BUTTON_B,
    AMW_GAMEPAD_BUTTON_SQUARE       = AMW_GAMEPAD_BUTTON_X,
    AMW_GAMEPAD_BUTTON_TRIANGLE     = AMW_GAMEPAD_BUTTON_Y,
    AMW_GAMEPAD_BUTTON_INVALID      = 0,
} amw_gamepad_button_t;
#define AMW_GAMEPAD_BUTTON_LAST AMW_GAMEPAD_BUTTON_DPAD_LEFT

typedef enum amw_gamepad_axis {
    AMW_GAMEPAD_AXIS_LEFT_X         = 0x0001,
    AMW_GAMEPAD_AXIS_LEFT_Y         = 0x0002,
    AMW_GAMEPAD_AXIS_RIGHT_X        = 0x0004,
    AMW_GAMEPAD_AXIS_RIGHT_Y        = 0x0008,
    AMW_GAMEPAD_AXIS_LEFT_TRIGGER   = 0x0010,
    AMW_GAMEPAD_AXIS_RIGHT_TRIGGER  = 0x0020,
} amw_gamepad_axis_t;
#define AMW_GAMEPAD_AXIS_LAST AMW_GAMEPAD_AXIS_RIGHT_TRIGGER

typedef enum amw_gamepad_hat {
    AMW_GAMEPAD_HAT_CENTERED    = 0,
    AMW_GAMEPAD_HAT_UP          = 0x0001,
    AMW_GAMEPAD_HAT_RIGHT       = 0x0002,
    AMW_GAMEPAD_HAT_DOWN        = 0x0004,
    AMW_GAMEPAD_HAT_LEFT        = 0x0008,
    AMW_GAMEPAD_HAT_RIGHT_UP    = (AMW_GAMEPAD_HAT_RIGHT | AMW_GAMEPAD_HAT_UP),
    AMW_GAMEPAD_HAT_RIGHT_DOWN  = (AMW_GAMEPAD_HAT_RIGHT | AMW_GAMEPAD_HAT_DOWN),
    AMW_GAMEPAD_HAT_LEFT_UP     = (AMW_GAMEPAD_HAT_LEFT | AMW_GAMEPAD_HAT_UP),
    AMW_GAMEPAD_HAT_LEFT_DOWN   = (AMW_GAMEPAD_HAT_LEFT | AMW_GAMEPAD_HAT_DOWN),
} amw_gamepad_hat_t;

typedef enum amw_input_event {
    AMW_INPUT_RELEASE           = 0x0001,
    AMW_INPUT_PRESS             = 0x0002,
    AMW_INPUT_REPEAT            = 0x0004,
    AMW_INPUT_MOUSE_DOWN        = 0x0008,
    AMW_INPUT_MOUSE_UP          = 0x0010,
    AMW_INPUT_MOUSE_SCROLL      = 0x0020,
    AMW_INPUT_MOUSE_MOVE        = 0x0040,
    AMW_INPUT_MOUSE_ENTER       = 0x0080,
    AMW_INPUT_MOUSE_LEAVE       = 0x0100,
    AMW_INPUT_TOUCH_BEGAN       = 0x0200,
    AMW_INPUT_TOUCH_MOVED       = 0x0400,
    AMW_INPUT_TOUCH_ENDED       = 0x0800,
    AMW_INPUT_TOUCH_CANCELLED   = 0x1000,
    AMW_INPUT_UNDEFINED         = 0,
} amw_input_event_t;

/* callbacks, to be set up by the input manager */
typedef void *(*PFN_hadal_key_callback)(void *data, amw_window_t *window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
typedef void *(*PFN_hadal_mousebutton_callback)(void *data, amw_window_t *window, int32_t button, int32_t action, int32_t mods);
typedef void *(*PFN_hadal_touch_callback)(void *data, amw_window_t *window, amw_touchpoint_t touchpoint, int32_t action);
typedef void *(*PFN_hadal_joystick_callback)(void *data, int32_t jid, int32_t event);
typedef struct {
    void *data; /* user defined input context */

    PFN_hadal_key_callback              key;
    //PFN_hadal_unicode_callback          unicode; /* TODO */
    //PFN_hadal_scroll_callback           scroll; /* TODO */ 
    //PFN_hadal_mousebutton_callback      mousebutton; /* TODO */
    //PFN_hadal_cursor_pos_callback       cursor_pos; /* TODO */ 
    //PFN_hadal_cursor_enter_callback     cursor_enter; /* TODO */ 
    //PFN_hadal_touch_callback            touch; /* TODO */
    //PFN_hadal_joystick_callback         joystick; /* TODO */
    //PFN_hadal_joystick_axis_callback    joystick_axis; /* TODO */
    //PFN_hadal_joystick_button_callback  joystick_button; /* TODO */
    //PFN_hadal_joystick_hat_callback     joystick_hat; /* TODO */
    //PFN_hadal_drop_callback             drop; /* TODO */
} amw_input_callbacks_t;

/* callbacks, to be set up by the renderer */
typedef void *(*PFN_hadal_framebuffer_resized_callback)(void *data, amw_window_t *window, int32_t width, int32_t height);
typedef struct {
    void *data; /* user defined rendering context */

    PFN_hadal_framebuffer_resized_callback framebuffer_resized;
} amw_renderer_callbacks_t;

/* core calls */
uint32_t      amw_hadal_platform(void);
bool          amw_hadal_platform_supported(int32_t id);

int32_t       amw_hadal_init(int32_t platform_id);
void          amw_hadal_terminate(void);

/* platform calls */
void          amw_hadal_set_clipboard_string(const char *string);
const char   *amw_hadal_get_clipboard_string(void);

/* window-specific calls */
amw_window_t *amw_hadal_create_window(const char *title, int32_t width, int32_t height, uint32_t flags);
void          amw_hadal_destroy_window(amw_window_t *window);
void          amw_hadal_show_window(amw_window_t *window);
void          amw_hadal_hide_window(amw_window_t *window);
bool          amw_hadal_should_close(amw_window_t *window);
void          amw_hadal_request_close(amw_window_t *window, bool request);

void          amw_hadal_retitle_window(amw_window_t *window, const char *title);
void          amw_hadal_resize_window(amw_window_t *window, int32_t width, int32_t height);
void          amw_hadal_window_size(amw_window_t *window, int32_t *width, int32_t *height);
void          amw_hadal_framebuffer_size(amw_window_t *window, int32_t *width, int32_t *height);
void          amw_hadal_content_scale(amw_window_t *window, float *xscale, float *yscale);

/* callbacks for other systems */
void          amw_hadal_setup_input_callbacks(amw_window_t *window, amw_input_callbacks_t callbacks);
void          amw_hadal_setup_renderer_callbacks(amw_window_t *window, amw_renderer_callbacks_t callbacks);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_hadopelagic_h_ */
