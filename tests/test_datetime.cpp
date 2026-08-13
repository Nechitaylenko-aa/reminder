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

// Простая проверка для минутных повторений
TEST_CASE("calculateNextMinute advances by period_count minutes", "[DateTimeCalculator]") {
    EventEntry ev{};
    ev.event = 1000;            // базовый timestamp
    ev.period = EP_MINUTE;
    ev.period_count = 2;       // каждые 2 минуты -> 120 сек

    time_t current = ev.event + 60; // прошло 1 минута
    time_t next = DateTimeCalculator::calculateNextMinute(ev, current);
    REQUIRE(next == ev.event + 120);
}

// Проверка на часы
TEST_CASE("calculateNextHour advances by period_count hours", "[DateTimeCalculator]") {
    EventEntry ev{};
    ev.event = 1000;
    ev.period = EP_HOURLY;
    ev.period_count = 1; // 3600 s

    time_t current = ev.event + 3600 * 3 + 10; // спустя >3 часов
    time_t next = DateTimeCalculator::calculateNextHour(ev, current);
    REQUIRE(next == ev.event + 3600 * 4);
}

// День/неделя — аналогично
TEST_CASE("calculateNextDay and Week basic", "[DateTimeCalculator]") {
    EventEntry ev{};
    ev.event = 1000;
    ev.period = EP_DAILY;
    ev.period_count = 1;
    time_t current = ev.event + 86400 * 2 + 5;
    REQUIRE(DateTimeCalculator::calculateNextDay(ev, current) == ev.event + 86400 * 3);

    ev.period = EP_WEEKLY;
    ev.period_count = 1;
    current = ev.event + 604800 * 5 + 1;
    REQUIRE(DateTimeCalculator::calculateNextWeek(ev, current) == ev.event + 604800 * 6);
}

TEST_CASE("calculateNextMonth and Year basic properties", "[DateTimeCalculator]") {
    // Event on Jan 31, 2021 10:00 UTC
    time_t ev_ts = make_utc(2021, 1, 31, 10, 0, 0);
    EventEntry ev{};
    ev.event = ev_ts;
    ev.period = EP_MONTH;
    ev.period_count = 1;

    // Current time: 2021-02-01 00:00 UTC
    time_t current = make_utc(2021, 2, 1, 0, 0, 0);
    time_t next = DateTimeCalculator::calculateNextMonth(ev, current);
    REQUIRE(next > current);
    // stepping back from next should give value strictly less than next
    time_t stepped = DateTimeCalculator::calculateStepBack(ev, next);
    REQUIRE(stepped < next);

    // Yearly: event on 2020-02-29 (leap day) - ensure next annual occurrence after 2021 is > current
    time_t ev_leap = make_utc(2020, 2, 29, 12, 0, 0);
    EventEntry evy{};
    evy.event = ev_leap;
    evy.period = EP_YEAR;
    evy.period_count = 1;

    time_t cur2 = make_utc(2021, 3, 1, 0, 0, 0);
    time_t nexty = DateTimeCalculator::calculateNextYear(evy, cur2);
    REQUIRE(nexty > cur2);
    time_t stepy = DateTimeCalculator::calculateStepBack(evy, nexty);
    REQUIRE(stepy < nexty);
}

TEST_CASE("applyTrigger moves weekend to nearest workday before or after", "[DateTimeCalculator]") {
    // 2021-10-09 is Saturday
    time_t saturday = make_utc(2021, 10, 9, 10, 0, 0);
    // ET_BEFORE should move to Friday (2021-10-08)
    time_t before = DateTimeCalculator::applyTrigger(saturday, ET_BEFORE);
    struct tm tb = get_tm_utc(before);
    REQUIRE(tb.tm_wday == 5); // Friday (tm_wday: 0=Sun,1=Mon,...,5=Fri)

    // ET_AFTER should move to Monday (2021-10-11)
    time_t after = DateTimeCalculator::applyTrigger(saturday, ET_AFTER);
    struct tm ta = get_tm_utc(after);
    REQUIRE(ta.tm_wday == 1); // Monday

    // ET_STRICTLY on weekend should return same timestamp
    time_t strict = DateTimeCalculator::applyTrigger(saturday, ET_STRICTLY);
    REQUIRE(strict == saturday);

    // 2021-10-10 is Sunday
    time_t sunday = make_utc(2021, 10, 10, 9, 0, 0);
    time_t before_sun = DateTimeCalculator::applyTrigger(sunday, ET_BEFORE);
    struct tm tbs = get_tm_utc(before_sun);
    REQUIRE(tbs.tm_wday == 5); // Friday

    time_t after_sun = DateTimeCalculator::applyTrigger(sunday, ET_AFTER);
    struct tm tas = get_tm_utc(after_sun);
    REQUIRE(tas.tm_wday == 1); // Monday
}

TEST_CASE("calculateNextMonth end-of-month behavior (non-leap and leap)", "[DateTimeCalculator]") {
    // Non-leap year: Jan 31, 2021 -> Feb 28, 2021
    time_t ev_jan31_2021 = make_utc(2021, 1, 31, 10, 0, 0);
    EventEntry ev1{};
    ev1.event = ev_jan31_2021;
    ev1.period = EP_MONTH;
    ev1.period_count = 1;

    time_t cur_feb1_2021 = make_utc(2021, 2, 1, 0, 0, 0);
    time_t next_feb = DateTimeCalculator::calculateNextMonth(ev1, cur_feb1_2021);
    struct tm tn = get_tm_utc(next_feb);
    REQUIRE(tn.tm_mon == 1); // February
    REQUIRE(tn.tm_mday == 28);
    REQUIRE(tn.tm_hour == 10);

    // Leap year: Jan 31, 2020 -> Feb 29, 2020
    time_t ev_jan31_2020 = make_utc(2020, 1, 31, 8, 30, 0);
    EventEntry ev2{};
    ev2.event = ev_jan31_2020;
    ev2.period = EP_MONTH;
    ev2.period_count = 1;

    time_t cur_feb1_2020 = make_utc(2020, 2, 1, 0, 0, 0);
    time_t next_feb_2020 = DateTimeCalculator::calculateNextMonth(ev2, cur_feb1_2020);
    struct tm tn2 = get_tm_utc(next_feb_2020);
    REQUIRE(tn2.tm_mon == 1); // February
    REQUIRE(tn2.tm_mday == 29);
    REQUIRE(tn2.tm_hour == 8);
}

TEST_CASE("calculateStepBack returns expected previous month end for end-of-month events", "[DateTimeCalculator]") {
    // Event on Jan 31, stepping back from March 31 should return Feb 28 (non-leap year)
    time_t ev_jan31 = make_utc(2021, 1, 31, 10, 0, 0);
    EventEntry ev{};
    ev.event = ev_jan31;
    ev.period = EP_MONTH;
    ev.period_count = 1;

    time_t future_mar31 = make_utc(2021, 3, 31, 10, 0, 0);
    time_t stepped = DateTimeCalculator::calculateStepBack(ev, future_mar31);
    struct tm ts = get_tm_utc(stepped);
    REQUIRE(ts.tm_mon == 1); // February
    REQUIRE(ts.tm_mday == 28);
    REQUIRE(ts.tm_hour == 10);
}
