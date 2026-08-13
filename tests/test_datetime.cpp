#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "DateTimeCalculator.h"
#include "base-types.h"

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
