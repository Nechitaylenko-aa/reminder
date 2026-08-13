//
// Created by artem on 12.05.26.
//

#ifndef REMINDER_CTEXTSYNCTIMER_H
#define REMINDER_CTEXTSYNCTIMER_H

#include "CTimer.h"



class CTextDBModel;
class CTrackedTexts;
class CAbstractConnection;

/** @brief Синхронизация общедоступного дерева текстового ресурса */
class CTextSyncTimer : public CTimer
{
public:
    CTextSyncTimer(CQueue* queue, uint32_t period_ms, E_DB_TYPE db_type);
    ~CTextSyncTimer() override;

protected:
    void  onTick() override;

private: // members
    CTextDBModel        * m_textDatabase;
    CTrackedTexts       * m_textNodes;
    CAbstractConnection * m_connection;

private: // methods



};


#endif //REMINDER_CTEXTSYNCTIMER_H
