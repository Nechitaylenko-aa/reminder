//
// Created by artem on 07.05.26.
//

#include "../include/CTestEventsTimer.h"
#include "../include/DateTimeCalculator.h"

std::vector<EventEntry>  entriesToUpdate;

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
        time_t realTriggerTime = DateTimeCalculator::applyTrigger(lastActual, item.trigger);

        bool datesEqual = false;
        if (item.type == EventType::ET_DATE)
        {
            // check if event was or active
            datesEqual = isDateLessEqual(lastActual, cTime);
        }

        if ((realTriggerTime <= cTime || datesEqual) && realTriggerTime > item.was_shown)
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
        struct tm tm_buf{};  // свой буфер на стеке
        struct tm* tm_now = localtime(&time);
        if (!tm_now) return 0;

        tm_buf = *tm_now;  // копируем данные
        tm_buf.tm_hour = 0;
        tm_buf.tm_min = 0;
        tm_buf.tm_sec = 0;

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
