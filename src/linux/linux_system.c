#include <moonlitwalk/os.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

static int32_t overwrite_exitcode = 0;

static int32_t cpu_threads, cpu_cores, cpu_packages;

AMW_NORETURN void amw_exit(int32_t exitcode)
{
    if (overwrite_exitcode)
        exit(overwrite_exitcode);
    exit(exitcode);
}

void amw_exitcode(int32_t exitcode)
{
    overwrite_exitcode = exitcode;
}

void amw_sleep(int32_t ms)
{
    usleep(ms * 1000);
}

void amw_cpu_count(int32_t *threads, int32_t *cores, int32_t *packages)
{
    static bool count_cpu_init = AMW_FALSE;
    if (count_cpu_init) {
        if (threads)  *threads = cpu_threads;
        if (cores)    *cores = cpu_cores;
        if (packages) *packages = cpu_packages;
        return;
    }

    int32_t fd, len, pos, end;
    char    buf[4096];
    char    num[100];

    cpu_threads = 1;
    cpu_cores = 1;
    cpu_packages = 1;

    fd = open("/proc/cpuinfo", O_RDONLY);
    if (fd != -1) {
        len = read(fd, buf, 4096);
        close(fd);
        pos = 0;
        while (pos < len) {
            if (!strncmp(buf + pos, "cpu cores", 9)) {
                pos = strchr(buf + pos, ':') - buf + 2;
                end = strchr(buf + pos, '\n') - buf;

                if (pos < len && end < len) {
                    strncpy(num, buf + pos, sizeof(num));
                    num[100-1] = 0; /* because the compiler screams at me [-Wstringop-truncation] */
                    amw_assert((end - pos) > 0 && (end - pos) < (int32_t)sizeof(num));
                    num[end - pos] = '\0';

                    int32_t processor = atoi(num);
                    if ((processor) > cpu_cores) {
                        cpu_cores = processor;
                    }
                } else {
                    amw_log_error("failed parsing /proc/cpuinfo");
                    break;
                }
            } else if (!strncmp(buf + pos, "siblings", 8)) {
                pos = strchr(buf + pos, ':') - buf + 2;
                end = strchr(buf + pos, '\n') - buf;

                if (pos < len && end < len) {
                    strncpy(num, buf + pos, sizeof(num));
                    num[100-1] = 0; /* because the compiler screams at me [-Wstringop-truncation] */
                    amw_assert((end - pos) > 0 && (end - pos) < (int32_t)sizeof(num));
                    num[end - pos] = '\0'; /* because it keeps fucking screaming */

                    int32_t core_id = atoi(num);
                    if ((core_id) > cpu_threads) {
                        cpu_threads = core_id;
                    }
                } else {
                    amw_log_error("failed parsing /proc/cpuinfo");
                    break;
                }
            }
            pos = strchr(buf + pos, '\n') - buf + 1;
        }
    } else {
        amw_log_error("failed parsing /proc/cpuinfo");
        cpu_cores = sysconf(_SC_NPROCESSORS_CONF);
        cpu_threads = 2 * cpu_cores;
    }

    if (threads)  *threads = cpu_threads;
    if (cores)    *cores = cpu_cores;
    if (packages) *packages = cpu_packages;
}
