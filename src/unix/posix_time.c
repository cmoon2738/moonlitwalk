#include <moonlitwalk/os.h>

#include <unistd.h>
#include <sys/time.h>

/* TODO check for <time.h> 'clock_gettime' in meson */
#define HAVE_CLOCK_GETTIME 1

#ifdef HAVE_CLOCK_GETTIME
#include <time.h>
#endif

#ifdef AMW_PLATFORM_APPLE
#include <mach/mach_time.h>
#endif

static bool checked_monotonic = AMW_FALSE;
static bool has_monotonic = AMW_FALSE;

#if !defined(HAVE_CLOCK_GETTIME) && defined(AMW_PLATFORM_APPLE)
static mach_timebase_info_data_t mach_base_info;
#endif

static void check_monotonic(void)
{
#ifdef HAVE_CLOCK_GETTIME
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) == 0) {
        has_monotonic = AMW_TRUE;
    }
#elif defined(AMW_PLATFORM_APPLE)
    if (mach_timebase_info(&mach_base_info) == 0) {
        has_monotonic = AMW_TRUE;
    }
#endif
    checked_monotonic = AMW_TRUE;
}

uint64_t amw_systime_counter(void)
{
    uint64_t ticks;

    if (!checked_monotonic)
        check_monotonic();

    if (has_monotonic) {
#ifdef HAVE_CLOCK_GETTIME
        struct timespec now;

        clock_gettime(CLOCK_MONOTONIC, &now);
        ticks = now.tv_sec;
        ticks *= AMW_NS_PER_SECOND;
        ticks += now.tv_nsec;
#elif defined(AMW_PLATFORM_APPLE)
        ticks = mach_absolute_time();
#else
        amw_assert(AMW_FALSE);
        ticks = 0;
#endif
    } else {
        struct timeval now;

        gettimeofday(&now, NULL);
        ticks = now.tv_sec;
        ticks *= AMW_US_PER_SECOND;
        ticks += now.tv_usec;
    }
    return ticks;
}

uint64_t amw_systime_frequency(void)
{
    if (!checked_monotonic)
        check_monotonic();

    if (has_monotonic) {
#ifdef HAVE_CLOCK_GETTIME
        return AMW_NS_PER_SECOND;
#elif defined(AMW_PLATFORM_APPLE)
        uint64_t freq = mach_base_info.denom;
        freq *= AMW_NS_PER_SECOND;
        freq /= mach_base_info.numer;
        return freq;
#endif
    } 
    return AMW_US_PER_SECOND;
}
