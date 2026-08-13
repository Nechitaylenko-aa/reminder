//
// Created by artem on 07.05.26.
//

#ifndef REMINDER_CTIMER_H
#define REMINDER_CTIMER_H

#include "CQueue.h"
#include "base-types.h"


struct SSyncText
{
    std::mutex  lock;

    bool content_changed{false};

    std::mutex  sourceLock;
    std::map<uint32_t, std::vector<TextEntry>>  * localData{nullptr};
};

struct SSyncEvents
{
    std::mutex  sourceLock;
    bool treeStructureChanged{false};
    bool is_updated{false};
    std::map<uint32_t, std::vector<EventEntry>>  * localData{nullptr};
};


/** @brief base class for timer with thread inside */
class CTimer {
public:

    virtual ~CTimer();

    void stop();
    void start();

protected:
    CTimer(CQueue* queue, uint32_t period_ms);
    virtual void onTick() = 0;
    std::atomic_bool m_processing{false};


protected:
    CQueue          * m_queue;

private:

    uint32_t         m_period;

};


#endif //REMINDER_CTIMER_H
