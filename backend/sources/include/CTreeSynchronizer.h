//
// Created by artem on 14.05.26.
//

#ifndef REMINDER_CTREESYNCHRONIZER_H
#define REMINDER_CTREESYNCHRONIZER_H

#include "CTimer.h"

class CTrackedTexts;
class CTrackedEvents;
class CAbstractConnection;
class CTreeDBModel;

class CTreeSynchronizer : public CTimer
{
public:
    CTreeSynchronizer(CQueue* queue, uint32_t period_ms, E_DB_TYPE db_type);
    ~CTreeSynchronizer() override;
protected:
    void  onTick() override;
private:
    CTrackedTexts       * m_trackedTexts;
    CTrackedEvents      * m_trackedEvents;
    CAbstractConnection * m_connection;
    CTreeDBModel        * m_treeDBModel;
};


#endif //REMINDER_CTREESYNCHRONIZER_H
