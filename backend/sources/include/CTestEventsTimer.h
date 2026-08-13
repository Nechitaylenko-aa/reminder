//
// Created by artem on 07.05.26.
//

#ifndef REMINDER_CTESTEVENTSTIMER_H
#define REMINDER_CTESTEVENTSTIMER_H

#include "CTimer.h"
#include "base-types.h"

class CTestEventsTimer : public CTimer
{
public:
    CTestEventsTimer(CQueue *queue, uint32_t period_ms);
    ~CTestEventsTimer() override;
    void setEntries(std::vector<EventEntry> *entries, std::mutex &guard_mutex);

protected:
    void onTick() override;
    std::mutex * m_mutex{nullptr};
    std::vector<EventEntry> * m_entries{nullptr};

private: // members

private: // methods

    static bool isDateLessEqual(time_t event, time_t now);
};


#endif //REMINDER_CTESTEVENTSTIMER_H
