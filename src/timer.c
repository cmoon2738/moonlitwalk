#include "moonlitwalk/os.h"

static uint64_t tick_start = 0;
static uint32_t tick_numerator_ms;
static uint32_t tick_denominator_ms;
static uint32_t tick_numerator_ns;
static uint32_t tick_denominator_ns;

static uint32_t calculate_gcd(uint32_t a, uint32_t b)
{
    if (b == 0)
        return a;
    return calculate_gcd(b, (a % b));
}

void amw_ticks_init(void)
{
    uint64_t tick_freq;
    uint32_t gcd;

    if (tick_start)
        return;

    tick_freq = amw_systime_frequency();
    amw_assert(tick_freq > 0 && tick_freq <= (uint64_t)UINT32_MAX);

    gcd = calculate_gcd(AMW_MS_PER_SECOND, (uint32_t)tick_freq);
    tick_numerator_ms = (AMW_MS_PER_SECOND / gcd);
    tick_denominator_ms = (uint32_t)(tick_freq / gcd);

    gcd = calculate_gcd(AMW_NS_PER_SECOND, (uint32_t)tick_freq);
    tick_numerator_ns = (AMW_NS_PER_SECOND / gcd);
    tick_denominator_ns = (uint32_t)(tick_freq / gcd);

    tick_start = amw_systime_counter();

    if (!tick_start)
        --tick_start;
}

void amw_ticks_quit(void)
{
    tick_start = 0;
}

uint64_t amw_ticks_ms(void)
{
    uint64_t starting_value, value;

    if (!tick_start)
        amw_ticks_init();

    starting_value = (amw_systime_counter() - tick_start);
    value = (starting_value * tick_numerator_ms);
    amw_assert(value >= starting_value);
    value /= tick_denominator_ms;
    return value;
}

uint64_t amw_ticks_ns(void)
{
    uint64_t starting_value, value;

    if (!tick_start)
        amw_ticks_init();

    starting_value = (amw_systime_counter() - tick_start);
    value = (starting_value * tick_numerator_ns);
    amw_assert(value >= starting_value);
    value /= tick_denominator_ns;
    return value;
}
