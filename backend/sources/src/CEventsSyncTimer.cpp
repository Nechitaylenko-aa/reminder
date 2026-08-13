//
// Created by artem on 13.05.26.
//

#include "../include/CEventsSyncTimer.h"
#include "../../db-models/db_conn.h"

SSyncEvents syncEventsData;

CEventsSyncTimer::CEventsSyncTimer(CQueue *queue, uint32_t period_ms, E_DB_TYPE db_type)
    : CTimer(queue, period_ms)
{
    m_connection = CAbstractConnection::createDatabaseInstance(db_type, &sdb_connection);
    m_eventsDatabase = new CEventsDBModel(m_connection);
    m_eventsNodes = new CTrackedEvents(m_connection);
}

CEventsSyncTimer::~CEventsSyncTimer()
{
    while (m_processing)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    m_queue->removeTasks(this);

    {
        std::lock_guard lock(syncEventsData.sourceLock);
        syncEventsData.localData = nullptr;
    }

    delete m_eventsNodes;
    delete m_eventsDatabase;
    delete m_connection;
}

void CEventsSyncTimer::onTick()
{
    std::lock_guard lock(syncEventsData.sourceLock);

    if (!syncEventsData.localData || syncEventsData.localData->empty())
    {
        return;
    }

    m_processing = true;

    std::vector<uint32_t> trackedNodes = m_eventsNodes->trackedDatesNodes();
    if (trackedNodes.empty())
    {
        m_processing = false;
        return;
    }

    std::map<uint32_t, std::vector<EventEntry>> freshData;

    for (auto & id_node : trackedNodes)
    {
        std::vector<EventEntry> array = m_eventsDatabase->getEventsByNode(id_node);
        if (!array.empty())
        {
            freshData[id_node] = std::move(array);
        }
    }

    if (!freshData.empty())
    {
        if (freshData != *(syncEventsData.localData))
        {
            *(syncEventsData.localData) = std::move(freshData);
            syncEventsData.is_updated = true;
        }
    }

    m_processing = false;
}
