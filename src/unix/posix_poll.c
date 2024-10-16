#include <moonlitwalk/os.h>

#include "../hadopelagic.h"

#include <errno.h>

bool hadal_poll_posix(struct pollfd *fds, nfds_t count, double *timeout)
{
    for (;;) {
        if (timeout) {
            const uint64_t base = amw_systime_counter();
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__CYGWIN__)
            const time_t seconds = (time_t) *timeout;
            const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
            const struct timespec ts = { seconds, nanoseconds };
            const int result = ppoll(fds, count, &ts, NULL);
#elif defined(__NetBSD__)
            const time_t seconds = (time_t) *timeout;
            const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
            const struct timespec ts = { seconds, nanoseconds };
            const int result = pollts(fds, count, &ts, NULL);
#else
            const int milliseconds = (int) (*timeout * 1e3);
            const int result = poll(fds, count, milliseconds);
#endif
            const int error = errno; // clock_gettime may overwrite our error

            *timeout -= (amw_systime_counter() - base) /
                (double)amw_systime_frequency();

            if (result > 0)
                return AMW_TRUE;
            else if (result == -1 && error != EINTR && error != EAGAIN)
                return AMW_FALSE;
            else if (*timeout <= 0.0)
                return AMW_FALSE;
        }
        else
        {
            const int result = poll(fds, count, -1);
            if (result > 0)
                return AMW_TRUE;
            else if (result == -1 && errno != EINTR && errno != EAGAIN)
                return AMW_FALSE;
        }
    }
}
