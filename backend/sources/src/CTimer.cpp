//
// Created by artem on 07.05.26.
//

#include "../include/CTimer.h"

CTimer::CTimer(CQueue *queue, uint32_t period_ms)
        : m_queue(queue), m_period(period_ms)
{
    m_queue->submit(this, [this](){ this->onTick();}, m_period);
}


CTimer::~CTimer()
{
    m_queue->removeTasks(this);
}

void CTimer::stop()
{
    m_queue->removeTasks(this);
}

void CTimer::start()
{
    m_queue->submit(this, [this](){ this->onTick();}, m_period);
}
