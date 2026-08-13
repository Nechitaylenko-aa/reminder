#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <ctime>

#include "DateTimeCalculator.h"
#include "base-types.h"

// Ensure tests use UTC for deterministic behavior
static struct TZSetter {
    TZSetter() {
#ifdef _WIN32
        _putenv_s("TZ", "UTC");
#else
        setenv("TZ", "UTC", 1);
#endif
        tzset();
    }
} tzSetterInstance;

// Helper to build UTC timestamp
static time_t make_utc(int year, int month, int day, int hour = 0, int min = 0, int sec = 0)
{
    struct tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = -1;
    return mktime(&t);
}

// Helper to get UTC struct tm in a portable way
static struct tm get_tm_utc(time_t ts)
{
    struct tm out{};
#ifdef _WIN32
    gmtime_s(&out, &ts);
#else
    gmtime_r(&ts, &out);
#endif
    return out;
}

TEST_CASE("Date-only event triggers at local 08:00 and respects applyTrigger", "[DateEvent]") {
    // Event date: 2021-10-09 (Saturday)
    time_t ev_date = make_utc(2021, 10, 9, 0, 0, 0);
    EventEntry ev{};
    ev.event = ev_date;
    ev.period = EP_NONE; // single-date
    ev.type = EventType::ET_DATE;
    ev.trigger = ET_BEFORE; // should move to previous workday (Friday)
    ev.period_count = 0;
    ev.isEnabled = true;
    ev.is_removed = false;
    ev.was_shown = 0;

    // current time: 2021-10-09 09:00 UTC (after local 08:00)
    time_t current = make_utc(2021, 10, 9, 9, 0, 0);

    // Build local ev date at 08:00 and apply trigger
    struct tm ev_tm = get_tm_utc(ev_date);
    ev_tm.tm_hour = 8;
    ev_tm.tm_min = 0;
    ev_tm.tm_sec = 0;
    ev_tm.tm_isdst = -1;
    time_t date_trigger = mktime(&ev_tm);
    date_trigger = DateTimeCalculator::applyTrigger(date_trigger, ev.trigger);

    // Expected: ET_BEFORE moves Saturday -> Friday (2021-10-08) at 08:00
    time_t expected = make_utc(2021, 10, 8, 8, 0, 0);

    REQUIRE(date_trigger == expected);
    REQUIRE(date_trigger <= current);
}
