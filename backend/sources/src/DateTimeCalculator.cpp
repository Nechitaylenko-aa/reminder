//
// Created by artem on 10.05.26.
//

#include "../include/DateTimeCalculator.h"
#include <ctime>

struct tm DateTimeCalculator::safeLocaltime(time_t time) {
    struct tm result{};
    localtime_r(&time, &result);
    return result;
}

time_t DateTimeCalculator::calculateNextMinute(const EventEntry& event, time_t current)
{
    uint32_t duration = 60 * event.period_count;
    if (duration == 0) return event.event;

    time_t elapsed = current - event.event;
    time_t periods = elapsed / duration;
    return event.event + (periods + 1) * duration;
}

time_t DateTimeCalculator::calculateNextHour(const EventEntry& event, time_t current)
{
    uint32_t duration = 3600 * event.period_count;
    if (duration == 0) return event.event;

    time_t elapsed = current - event.event;
    time_t periods = elapsed / duration;
    return event.event + (periods + 1) * duration;
}

time_t DateTimeCalculator::calculateNextDay(const EventEntry& event, time_t current)
{
    uint32_t duration = 86400 * event.period_count;
    if (duration == 0) return event.event;

    time_t elapsed = current - event.event;
    time_t periods = elapsed / duration;
    return event.event + (periods + 1) * duration;
}

time_t DateTimeCalculator::calculateNextWeek(const EventEntry& event, time_t current)
{
    uint32_t duration = 604800 * event.period_count;
    if (duration == 0) return event.event;

    time_t elapsed = current - event.event;
    time_t periods = elapsed / duration;
    return event.event + (periods + 1) * duration;
}

time_t DateTimeCalculator::calculateNextMonth(const EventEntry& event, time_t current)
{
    struct tm tm_event = safeLocaltime(event.event);
    int target_day = tm_event.tm_mday;
    int months_accumulated = 0;

    tm_event.tm_isdst = -1;
    time_t result_ts = mktime(&tm_event);

    while (result_ts <= current) {
        months_accumulated += event.period_count;
        tm_event = safeLocaltime(event.event);
        tm_event.tm_mon += months_accumulated;

        result_ts = mktime(&tm_event);

        if (tm_event.tm_mday != target_day) {
            tm_event.tm_mday = 0;
            result_ts = mktime(&tm_event);
        }

        if (event.period_count == 0) break;
    }

    return result_ts;
}

time_t DateTimeCalculator::calculateNextYear(const EventEntry& event, time_t current)
{
    struct tm tm_event = safeLocaltime(event.event);
    int years_to_add = event.period_count;
    tm_event.tm_year += years_to_add;

    time_t newTime = mktime(&tm_event);

    while (newTime <= current)
    {
        tm_event.tm_year += years_to_add;
        newTime = mktime(&tm_event);
    }

    return newTime;
}

time_t DateTimeCalculator::calculateStepBack(const EventEntry& event, time_t futureTime)
{
    struct tm t = safeLocaltime(futureTime);

    switch (event.period)
    {
        case EP_MINUTE: t.tm_min -= event.period_count; break;
        case EP_HOURLY: t.tm_hour -= event.period_count; break;
        case EP_DAILY:  t.tm_mday -= event.period_count; break;
        case EP_WEEKLY: t.tm_mday -= (event.period_count * 7); break;
        case EP_MONTH:  t.tm_mon -= event.period_count; break;
        case EP_YEAR:   t.tm_year -= event.period_count; break;
        default: return futureTime;
    }

    t.tm_isdst = -1;
    time_t result = mktime(&t);

    if (event.period == EP_MONTH || event.period == EP_YEAR)
    {
        struct tm orig = safeLocaltime(event.event);
        if (t.tm_mday != orig.tm_mday)
        {
            t.tm_mday = 0;
            result = mktime(&t);
        }
    }

    return result;
}

bool DateTimeCalculator::isWeekend(time_t time)
{
    struct tm t = safeLocaltime(time);
    return (t.tm_wday == 0 || t.tm_wday == 6); // 0 - воскресенье, 6 - суббота
}

time_t DateTimeCalculator::applyTrigger(time_t timestamp, EventTrigger trigger)
{
    if (trigger == ET_STRICTLY || !isWeekend(timestamp))
    {
        return timestamp;
    }

    struct tm t = safeLocaltime(timestamp);

    if (trigger == ET_BEFORE)
    {
        while (t.tm_wday == 0 || t.tm_wday == 6)
        {
            t.tm_mday -= 1;
            t.tm_isdst = -1;
            timestamp = mktime(&t);
            t = safeLocaltime(timestamp);
        }
    }
    else if (trigger == ET_AFTER)
    {
        while (t.tm_wday == 0 || t.tm_wday == 6)
        {
            t.tm_mday += 1;
            t.tm_isdst = -1;
            timestamp = mktime(&t);
            t = safeLocaltime(timestamp);
        }
    }

    return timestamp;
}

time_t DateTimeCalculator::calculateNext(const EventEntry& event, time_t current)
{
    switch (event.period)
    {
        case EP_MINUTE: return calculateNextMinute(event, current);
        case EP_HOURLY: return calculateNextHour(event, current);
        case EP_DAILY:  return calculateNextDay(event, current);
        case EP_WEEKLY: return calculateNextWeek(event, current);
        case EP_MONTH:  return calculateNextMonth(event, current);
        case EP_YEAR:   return calculateNextYear(event, current);
        case EP_NONE:
        default:
            return event.event;
    }
}
