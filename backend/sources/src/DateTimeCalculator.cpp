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

// Helper: compute last day of month for the given year/month (year is tm_year, month is 0-based tm_mon)
static int last_day_of_month_by_ym(int tm_year, int tm_mon)
{
    struct tm t{};
    t.tm_year = tm_year;
    // build first day of next month
    int next_mon = tm_mon + 1;
    int next_year = tm_year;
    if (next_mon > 11) { next_mon -= 12; next_year += 1; }
    t.tm_mon = next_mon;
    t.tm_mday = 1;
    t.tm_hour = 12; // midday to avoid DST issues
    t.tm_isdst = -1;

    time_t next_first = safe_mktime(t);
    if (next_first == (time_t)-1) {
        // fallback: brute force from 31 down to 28
        for (int d = 31; d >= 28; --d) {
            struct tm tf{};
            tf.tm_year = tm_year;
            tf.tm_mon = tm_mon;
            tf.tm_mday = d;
            tf.tm_hour = 12;
            tf.tm_isdst = -1;
            time_t cand = safe_mktime(tf);
            if (cand != (time_t)-1) {
                struct tm r = DateTimeCalculator::safeLocaltime(cand);
                if (r.tm_mon == tm_mon) return r.tm_mday;
            }
        }
        return 28;
    }

    time_t last = next_first - 24 * 3600;
    struct tm last_tm = DateTimeCalculator::safeLocaltime(last);
    return last_tm.tm_mday;
}

// Helper: compute desired year/month after adding months (months may be negative)
static void add_months_to_ym(int base_year, int base_mon, int months_to_add, int &out_year, int &out_mon)
{
    int total = base_mon + months_to_add;
    out_year = base_year + (total / 12);
    out_mon = total % 12;
    if (out_mon < 0) { out_mon += 12; out_year -= 1; }
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

        int desired_year, desired_mon;
        add_months_to_ym(base.tm_year, base.tm_mon, months_accumulated, desired_year, desired_mon);

        // compute last day first and pick use_day = min(target_day, last)
        int last = last_day_of_month_by_ym(desired_year, desired_mon);
        int use_day = target_day < last ? target_day : last;

        // build tm for desired month/day, preserve original hour/min/sec
        struct tm tm_event{};
        tm_event.tm_year = desired_year;
        tm_event.tm_mon = desired_mon;
        tm_event.tm_mday = use_day;
        tm_event.tm_hour = base.tm_hour;
        tm_event.tm_min  = base.tm_min;
        tm_event.tm_sec  = base.tm_sec;
        tm_event.tm_isdst = -1;


        time_t cand = safe_mktime(tm_event);
        if (cand == (time_t)-1) {
            // try last day explicitly
            tm_event.tm_mday = last;
            tm_event.tm_isdst = -1;
            cand = safe_mktime(tm_event);
            if (cand == (time_t)-1) break; // give up for safety
        }

        struct tm cand_tm = safeLocaltime(cand);
        if (cand_tm.tm_mon != desired_mon || cand_tm.tm_mday != use_day) {
            tm_event.tm_mday = last;
            tm_event.tm_isdst = -1;
            cand = safe_mktime(tm_event);
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
            int last = last_day_of_month_by_ym(adj.tm_year, adj.tm_mon);
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

    // store desired year/month/day before normalization
    struct tm desired = t;

    switch (event.period)
    {
        case EP_MINUTE: t.tm_min -= event.period_count; break;
        case EP_HOURLY: t.tm_hour -= event.period_count; break;
        case EP_DAILY:  t.tm_mday -= event.period_count; break;
        case EP_WEEKLY: t.tm_mday -= (event.period_count * 7); break;
        case EP_MONTH:  desired.tm_mon -= event.period_count; t.tm_mon -= event.period_count; break;
        case EP_YEAR:   desired.tm_year -= event.period_count; t.tm_year -= event.period_count; break;
        default: return futureTime;
    }

    t.tm_isdst = -1;
    time_t result = safe_mktime(t);

    if ((event.period == EP_MONTH || event.period == EP_YEAR) && result != (time_t)-1) {
        struct tm res_tm = safeLocaltime(result);
        // compute desired year/month
        int desired_year = desired.tm_year;
        int desired_mon = desired.tm_mon;
        if (res_tm.tm_mon != desired_mon || res_tm.tm_mday != desired.tm_mday) {
            int last = last_day_of_month_by_ym(desired_year, desired_mon);
            struct tm adj = desired;
            adj.tm_mday = last;
            adj.tm_hour = desired.tm_hour; // preserve original hour
            adj.tm_isdst = -1;
            time_t cand = safe_mktime(adj);
            if (cand != (time_t)-1) result = cand;
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
