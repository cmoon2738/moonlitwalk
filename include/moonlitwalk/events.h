#ifndef _amw_events_h_
#define _amw_events_h_

#include <moonlitwalk/defines.h>

AMW_C_DECL_BEGIN

typedef enum {
    AMW_EVENT_NONE   = 0,
    AMW_EVENT_USER   = 0x1000,      /**< amw_event_user_t */

    AMW_EVENT_ENUM_PADDING = 0x7FFFFFFF,
} amw_eventtype_t;

/** Fields shared by every event */
typedef struct amw_event_common {
    uint32_t      type;             /**< The eventtype_t code indicated what event structure it is. */
    uint32_t      flags;            /**< May be used to mask a collection of bit flags. */
    uint64_t      ticks_ns;         /**< Populated using amw_ticks_ns. */
} amw_event_common_t;

/** A custom event  */
typedef struct amw_event_user {
    uint32_t      type;             /**< AMW_EVENT_USER */
    uint32_t      flags;            /**< The user may pass in his boolean bit flags for checks. */
    uint64_t      ticks_ns;         /**< Populated using amw_ticks_ns. */
    int32_t       code;             /**< User defined event code. */
    void         *data1;            /**< User defined data pointer. */
    void         *data2;            /**< User defined data pointer. */
    void         *data3;            /**< User defined data pointer. */
} amw_event_user_t;

typedef union amw_event {
    uint32_t           type;

    amw_event_common_t common;
    amw_event_user_t   user;

    /* To force ABI compatibility between different compilers. */
    uint8_t padding[48];
} amw_event_t;

/* Make sure the binary compatibility has not been broken */
amw_static_assert(event_padding, sizeof(amw_event_t) == sizeof(((amw_event_t *)NULL)->padding));

AMW_C_DECL_END

#endif /* _amw_events_h_ */
