#include <moonlitwalk/amw.h>
#include <moonlitwalk/system.h>

#include <stdio.h>
#include <time.h>

static struct {
    void           *userdata;
    amw_mutex_t    *lock;
    int32_t         level;
    bool            quiet;
    bool            initialized;
} logger = {0};

static const char *level_strings[] = {
    "V", 
    "D",
    "I",
    "W",
    "E",
    "F"
};

static const char *level_colors[] = {
    "\033[38;5;180m",
    "\033[38;5;215m",
    "\033[38;5;209m", 
    "\033[38;5;167m", 
    "\033[38;5;160m",
    "\033[38;5;68m",
};

static void default_callback(amw_log_t *log)
{
    char timestamp[22];
    timestamp[strftime(timestamp, sizeof(timestamp), "%H:%M:%S", log->time)] = '\0';

#if AMW_LOG_USE_COLOR
    fprintf(log->userdata, "%s %s%s \x1b[0m", timestamp, 
            level_colors[log->level], level_strings[log->level]);
    if (log->file != NULL && log->function != NULL) {
        fprintf(log->userdata, "\033[38;5;8m%s %s:%d\x1b[0m ", log->file, log->function, log->line);
    } else if (log->file != NULL) {
        fprintf(log->userdata, "\033[38;5;8m%26s:%-5d\x1b[0m ", log->file, log->line);
    } else if (log->function != NULL) {
        fprintf(log->userdata, "\033[38;5;8m%s:%d\x1b[0m ", log->function, log->line);
    }
#else
    fprintf(log->userdata, "%s %s ", timestamp, level_strings[log->level]);
    if (log->file != NULL && log->function != NULL) {
        fprintf(log->userdata, "(%s %s:%d) ", log->file, log->function, log->line);
    } else if (log->file != NULL) {
        fprintf(log->userdata, "(%s:%d) ", log->file, log->line);
    } else if (log->function != NULL) {
        fprintf(log->userdata, "(%s:%d)", log->function, log->line);
    }
#endif
    vfprintf(log->userdata, log->fmt, log->ap);
    fprintf(log->userdata, "\n");
    fflush(log->userdata);
}

void amw_log_init(void *output)
{
    if (logger.initialized)
        return;

    if (output) 
        logger.userdata = output;
    else
        logger.userdata = stderr;

    logger.lock = amw_mutex_create();
    if (!logger.lock) {
        fprintf(stderr, "DEBUG PRINT failed to create a logger mutex lock !!!");
    }
    logger.initialized = AMW_TRUE;
}

void amw_log_terminate(void)
{
    if (logger.initialized) {
        amw_mutex_destroy(logger.lock);
        amw_zero(logger);
    }
}

static void process_message(amw_log_t *log)
{
    amw_mutex_lock(logger.lock);
    if (!logger.quiet && (log->level >= logger.level)) {
        if (!log->time) {
            time_t t = time(NULL);
            log->time = localtime(&t);
        }
        log->userdata = logger.userdata;
        default_callback(log);
    }
    amw_mutex_unlock(logger.lock);
}

void amw_log_message(int32_t level, 
                     const char *function, 
                     const char *file, 
                     int32_t line, 
                     const char *fmt, ...) 
{
    if (!logger.initialized)
        return;

    amw_assert_release(fmt != NULL);

    amw_log_t log = {
        .fmt = fmt,
        .function = function,
        .file = file,
        .line = line,
        .level = level,
    };

    va_start(log.ap, fmt);
    process_message(&log);
    va_end(log.ap);
}

void amw_log_raw(char *fmt, ...)
{
    if (!logger.initialized || logger.quiet)
        return;

    amw_assert_release(fmt != NULL);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(logger.userdata, fmt, ap);
    va_end(ap);
}

int32_t amw_log_level(void)
{
    return logger.level;
}

void amw_log_set_level(int32_t level)
{
    amw_assert(level <= AMW_LOG_FATAL && level >= 0);
    logger.level = level;
}

void amw_log_set_quiet(bool silence)
{
    logger.quiet = silence;
}
