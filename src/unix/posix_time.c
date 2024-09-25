#include <moonlitwalk/system.h>

#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#ifndef AMW_POSIX_MONOTONIC_CLOCK
#define AMW_POSIX_MONOTONIC_CLOCK 1
#endif

struct amw_timer {
    clockid_t clock;
    uint64_t  frequency;
};

void amw_platform_timer_init(amw_timer_t *timer)
{
    timer->clock = CLOCK_REALTIME;
    timer->frequency = 1000000000;

#if AMW_POSIX_MONOTONIC_CLOCK
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        timer->clock = CLOCK_MONOTONIC;
#endif
}

uint64_t amw_platform_timer_value(amw_timer_t *timer)
{
    struct timespec ts;
    clock_gettime(timer->clock, &ts);
    return (uint64_t)ts.tv_sec * timer->frequency + (uint64_t)ts.tv_nsec;
}

uint64_t amw_platform_timer_frequency(amw_timer_t *timer)
{
    return timer->frequency;
}
