//
// Created by artem on 12.05.26.
//

#include "../include/CTextSyncTimer.h"
#include "../../db-models/CTextDBModel.h"
#include "../../db-models/CTrackedTexts.h"
#include "../../db-models/db_conn.h"

SSyncText   syncTextData;

CTextSyncTimer::CTextSyncTimer(CQueue *queue, uint32_t period_ms, E_DB_TYPE db_type) : CTimer(queue, period_ms)
{
    m_connection = CAbstractConnection::createDatabaseInstance(db_type, &sdb_connection);
    m_textDatabase = new CTextDBModel(m_connection);
    m_textNodes = new CTrackedTexts(m_connection);
}

CTextSyncTimer::~CTextSyncTimer()
{
    m_queue->removeTasks(this);

    while (m_processing)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    {
        std::lock_guard lock(syncTextData.sourceLock);
        syncTextData.localData = nullptr;
    }

    delete m_textNodes;
    delete m_textDatabase;
    m_connection->Delete();
}

bool entriesEqual(const TextEntry& a, const TextEntry& b) {
    return a.id == b.id &&
           a.id_parent == b.id_parent &&
           a.type == b.type &&
           a.desc == b.desc &&
           a.data == b.data &&
           a.is_removed == b.is_removed;
}

void CTextSyncTimer::onTick()
{
    std::lock_guard localLock(syncTextData.sourceLock);

    if (!syncTextData.localData || syncTextData.localData->empty())
    {
        return;
    }

    m_processing = true;

    std::vector<uint32_t> trackedNodes = m_textNodes->trackedTextNodes();

    std::map<uint32_t, std::vector<TextEntry>> publicContent;

    if (!trackedNodes.empty())
    {
        for (auto & id_node : trackedNodes)
        {
            auto query = m_textDatabase->textDataByParent(id_node);
            if (!query.empty())
            {
                publicContent[id_node] = query;
            }
        }

        if (!publicContent.empty())
        {
            auto& local = *syncTextData.localData;
            auto& fresh = publicContent;

            // Сравниваем
            if (local != fresh)
            {
                std::lock_guard lock(syncTextData.lock);  // защищаем запись
                local = std::move(fresh);
                syncTextData.content_changed = true;
            }
        }
    }

    m_processing = false;
}
