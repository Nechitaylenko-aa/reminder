//
// Created by artem on 10.05.26.
//

#include "../include/DateTimeCalculator.h"
#include <ctime>
#include <cerrno>
#include <iostream>

struct tm DateTimeCalculator::safeLocaltime(time_t time) {
    struct tm result{};
#ifdef _WIN32
    localtime_s(&result, &time);
#else
    localtime_r(&time, &result);
#endif
    return result;
}

// Helper: safe mktime wrapper that checks for errors
static time_t safe_mktime(struct tm &t)
{
    errno = 0;
    time_t r = mktime(&t);
    if (r == (time_t)-1) {
        // mktime may legitimately return -1 for some edge timestamps,
        // but in our usage we treat -1 as an error to avoid infinite loops.
        return (time_t)-1;
    }
    return r;
}

// Helper: compute last day of month for the month represented by 't'
static int last_day_of_month(struct tm t)
{
    // Normalize to first day of next month and subtract one day
    t.tm_mday = 1;
    t.tm_mon += 1;
    t.tm_isdst = -1;
    time_t next_first = safe_mktime(t);
    if (next_first == (time_t)-1) {
        // fallback: try brute-force days 31..28
        for (int d = 31; d >= 28; --d) {
            t.tm_mday = d;
            t.tm_mon -= 1; // restore to original month
            t.tm_isdst = -1;
            time_t cand = safe_mktime(t);
            if (cand != (time_t)-1) {
                struct tm r = DateTimeCalculator::safeLocaltime(cand);
                if (r.tm_mon == t.tm_mon) return r.tm_mday;
            }
            t.tm_mon += 1; // keep trying
        }
        return 28; // very conservative fallback
    }

    time_t last = next_first - 24 * 3600; // subtract one day in seconds
    struct tm last_tm = DateTimeCalculator::safeLocaltime(last);
    return last_tm.tm_mday;
}


time_t DateTimeCalculator::calculateNextMinute(const EventEntry& event, time_t current)
{
    int64_t duration = int64_t(60) * int64_t(event.period_count);
    if (duration <= 0) return event.event;

    time_t elapsed = current - event.event;
    time_t periods = elapsed / duration;
    return event.event + (periods + 1) * duration;
}

time_t DateTimeCalculator::calculateNextHour(const EventEntry& event, time_t current)
{
    int64_t duration = int64_t(3600) * int64_t(event.period_count);
    if (duration <= 0) return event.event;

    time_t elapsed = current - event.event;
    time_t periods = elapsed / duration;
    return event.event + (periods + 1) * duration;
}

time_t DateTimeCalculator::calculateNextDay(const EventEntry& event, time_t current)
{
    int64_t duration = int64_t(86400) * int64_t(event.period_count);
    if (duration <= 0) return event.event;

    time_t elapsed = current - event.event;
    time_t periods = elapsed / duration;
    return event.event + (periods + 1) * duration;
}

time_t DateTimeCalculator::calculateNextWeek(const EventEntry& event, time_t current)
{
    int64_t duration = int64_t(604800) * int64_t(event.period_count);
    if (duration <= 0) return event.event;

    time_t elapsed = current - event.event;
    time_t periods = elapsed / duration;
    return event.event + (periods + 1) * duration;
}

time_t DateTimeCalculator::calculateNextMonth(const EventEntry& event, time_t current)
{
    if (event.period_count == 0) return event.event;

    struct tm base = safeLocaltime(event.event);
    int target_day = base.tm_mday;
    int months_accumulated = 0;

    time_t result_ts = (time_t)-1;
    const int max_iterations = 1200; // safety limit
    int iter = 0;

    while (true) {
        months_accumulated += event.period_count;
        struct tm tm_event = safeLocaltime(event.event);
        tm_event.tm_mon += months_accumulated;
        tm_event.tm_isdst = -1;

        time_t cand = safe_mktime(tm_event);
        if (cand == (time_t)-1) break; // error — bail out

        struct tm cand_tm = safeLocaltime(cand);
        if (cand_tm.tm_mday != target_day) {
            // choose last day of target month
            int last = last_day_of_month(cand_tm);
            cand_tm.tm_mday = last;
            cand_tm.tm_isdst = -1;
            cand = safe_mktime(cand_tm);
            if (cand == (time_t)-1) break;
        }

        result_ts = cand;
        if (result_ts > current) break;

        ++iter;
        if (iter > max_iterations) break;
        if (event.period_count == 0) break;
    }

    if (result_ts == (time_t)-1) return event.event;
    return result_ts;
}

time_t DateTimeCalculator::calculateNextYear(const EventEntry& event, time_t current)
{
    if (event.period_count == 0) return event.event;

    struct tm tm_event = safeLocaltime(event.event);
    int years_to_add = event.period_count;

    const int max_iterations = 1000;
    int iter = 0;

    while (true) {
        tm_event.tm_year += years_to_add;
        time_t newTime = safe_mktime(tm_event);
        if (newTime == (time_t)-1) {
            // try to adjust day to last valid day of month (e.g., Feb 29 -> Feb 28)
            struct tm adj = tm_event;
            int last = last_day_of_month(adj);
            adj.tm_mday = last;
            adj.tm_isdst = -1;
            newTime = safe_mktime(adj);
            if (newTime == (time_t)-1) break;
            tm_event = safeLocaltime(newTime);
        }

        if (newTime > current) return newTime;

        ++iter;
        if (iter > max_iterations) break;
    }

    return event.event;
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
    time_t result = safe_mktime(t);
    if (result == (time_t)-1) {
        // If month/year adjustments caused invalid day (e.g., Feb 30), set to last day of month
        if (event.period == EP_MONTH || event.period == EP_YEAR)
        {
            struct tm orig = safeLocaltime(event.event);
            struct tm adj = t;
            int last = last_day_of_month(adj);
            if (adj.tm_mday != orig.tm_mday && last > 0) {
                adj.tm_mday = last;
                adj.tm_isdst = -1;
                result = safe_mktime(adj);
            }
        }
    }

    if (result == (time_t)-1) return futureTime;
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
        int tries = 0;
        while ((t.tm_wday == 0 || t.tm_wday == 6) && tries < 7)
        {
            t.tm_mday -= 1;
            t.tm_isdst = -1;
            timestamp = safe_mktime(t);
            if (timestamp == (time_t)-1) break;
            t = safeLocaltime(timestamp);
            ++tries;
        }
    }
    else if (trigger == ET_AFTER)
    {
        int tries = 0;
        while ((t.tm_wday == 0 || t.tm_wday == 6) && tries < 7)
        {
            t.tm_mday += 1;
            t.tm_isdst = -1;
            timestamp = safe_mktime(t);
            if (timestamp == (time_t)-1) break;
            t = safeLocaltime(timestamp);
            ++tries;
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
