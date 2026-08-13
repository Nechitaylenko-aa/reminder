//
// Created by artem on 14.05.26.
//

#include "../include/CTreeSynchronizer.h"
#include "../../db-models/CTrackedTexts.h"
#include "../../db-models/CtrackedEvents.h"
#include "../../db-models/db_conn.h"
#include "backend/sources/include/CTreeManager.h"
#include "../../db-models/ctreedbmodel.h"

SSyncTree sync_tree;

CTreeSynchronizer::CTreeSynchronizer(CQueue *queue, uint32_t period_ms, E_DB_TYPE db_type)
    : CTimer(queue, period_ms)
{
    m_connection = CAbstractConnection::createDatabaseInstance(db_type, &sdb_connection);
    m_trackedTexts = new CTrackedTexts(m_connection);
    m_trackedEvents = new CTrackedEvents(m_connection);
    m_treeDBModel = new CTreeDBModel(m_connection);
}

CTreeSynchronizer::~CTreeSynchronizer()
{
    m_queue->removeTasks(this);

    while (m_processing)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    {
        std::lock_guard lock(sync_tree.protectionLock);
        sync_tree.publicNodes = nullptr;
    }

    delete m_trackedTexts;
    delete m_trackedEvents;
    delete m_treeDBModel;
    delete m_connection;
}

void CTreeSynchronizer::onTick()
{
    std::lock_guard lock(sync_tree.protectionLock);

    if (!sync_tree.publicNodes || sync_tree.publicNodes->empty())
    {
        return;
    }

    m_processing = true;

    std::map<uint32_t, SNode>   serverSide;
    auto trackedNodes = m_trackedEvents->trackedDatesNodes();
    auto textsTracked = m_trackedTexts->trackedTextNodes();
    trackedNodes.insert(trackedNodes.end(), textsTracked.begin(), textsTracked.end());

    if (trackedNodes.empty())
    {
        m_processing = false;
        return;
    }

    auto treeNodes = m_treeDBModel->get_nodes();
    for (auto &node : treeNodes)
    {
        serverSide.emplace(node.id, node);
    }

    if (serverSide != *(sync_tree.publicNodes))
    {
        sync_tree.needUpdate = true;
        *(sync_tree.publicNodes) = std::move(serverSide);
    }
    m_processing = false;
}
