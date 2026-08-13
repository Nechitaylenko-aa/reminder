//
// Created by artem on 07.05.26.
//

#include "../include/CTestEventsTimer.h"
#include "../include/DateTimeCalculator.h"

std::vector<EventEntry>  entriesToUpdate;

// Default hour (local time) at which a DATE-only event is considered triggered
static constexpr int DEFAULT_DATE_TRIGGER_HOUR = 8;

CTestEventsTimer::CTestEventsTimer(CQueue *queue, uint32_t period_ms) : CTimer(queue, period_ms)
{}

CTestEventsTimer::~CTestEventsTimer()
{
    m_queue->removeTasks(this);

    while (m_processing)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    {
        std::lock_guard lock(*m_mutex);
        m_entries = nullptr;
    }
}

void CTestEventsTimer::onTick()
{
    if (!m_mutex || !m_entries || m_entries->empty()) return;

    std::lock_guard lock(*m_mutex);
    m_processing = true;
    time_t cTime = time(nullptr);


    for (auto &item : *m_entries)
    {
        if (!item.isEnabled || item.is_removed) continue;

        time_t futureTime = DateTimeCalculator::calculateNext(item, cTime);
        time_t lastActual = DateTimeCalculator::calculateStepBack(item, futureTime);

        // default real trigger time is the computed lastActual adjusted by trigger
        time_t realTriggerTime = DateTimeCalculator::applyTrigger(lastActual, item.trigger);

        bool dateReached = false;

        if (item.type == EventType::ET_DATE)
        {
            // Build local tm for event date and set trigger hour (local)
            struct tm ev_tm = DateTimeCalculator::safeLocaltime(item.event);

            // Preserve date, set time to business trigger hour (DEFAULT_DATE_TRIGGER_HOUR:08:00)
            ev_tm.tm_hour = DEFAULT_DATE_TRIGGER_HOUR;
            ev_tm.tm_min = 0;
            ev_tm.tm_sec = 0;
            ev_tm.tm_isdst = -1; // let mktime decide DST rules for that local time

            time_t date_trigger = mktime(&ev_tm);

            // apply weekend/ET_BEFORE/ET_AFTER adjustments on that date-local time
            date_trigger = DateTimeCalculator::applyTrigger(date_trigger, item.trigger);

            realTriggerTime = date_trigger;

            // calendar-based check (year/month/day) as additional safety
            struct tm ev = DateTimeCalculator::safeLocaltime(item.event);
            struct tm now = DateTimeCalculator::safeLocaltime(cTime);
            if (now.tm_year > ev.tm_year ||
                (now.tm_year == ev.tm_year && now.tm_mon > ev.tm_mon) ||
                (now.tm_year == ev.tm_year && now.tm_mon == ev.tm_mon && now.tm_mday >= ev.tm_mday)) {
                dateReached = true;
            }
        }

        if ((realTriggerTime <= cTime || dateReached) && realTriggerTime > item.was_shown)
        {
            item.was_shown = realTriggerTime;
            entriesToUpdate.push_back(item);
        }
    }
    m_processing = false;
}

bool CTestEventsTimer::isDateLessEqual(time_t event, time_t now)
{
    auto getDate = [&](time_t time)->time_t
    {
        // Use thread-safe safeLocaltime to avoid static localtime and set tm to midnight
        struct tm tm_buf = DateTimeCalculator::safeLocaltime(time);
        tm_buf.tm_hour = 0;
        tm_buf.tm_min = 0;
        tm_buf.tm_sec = 0;
        tm_buf.tm_isdst = -1; // let mktime determine DST

        return mktime(&tm_buf);
    };

    auto evtDate = getDate(event);
    auto nowDate = getDate(now);

    return nowDate >= evtDate;
}

void CTestEventsTimer::setEntries(std::vector<EventEntry> *entries, std::mutex &guard_mutex)
{
    m_mutex = &guard_mutex;
    m_entries = entries;
}
