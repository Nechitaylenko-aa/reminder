//
// Created by artem on 13.05.26.
//

#ifndef REMINDER_CEVENTSSYNCTIMER_H
#define REMINDER_CEVENTSSYNCTIMER_H

#include <CAbstractConnection.h>
#include "CTimer.h"
#include "backend/db-models/CtrackedEvents.h"
#include "backend/db-models/CEventsDBModel.h"

class CEventsSyncTimer : public CTimer
{
public:
    CEventsSyncTimer(CQueue* queue, uint32_t period_ms, E_DB_TYPE db_type);
    ~CEventsSyncTimer() override;

protected:
    void  onTick() override;

private:
    CEventsDBModel * m_eventsDatabase;
    CTrackedEvents* m_eventsNodes;
    CAbstractConnection * m_connection;

};


#endif //REMINDER_CEVENTSSYNCTIMER_H
