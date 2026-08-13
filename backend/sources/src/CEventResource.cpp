//
// Created by artem on 29.04.26.
//
#include <QVBoxLayout>
#include <unistd.h>
#include "../include/CEventResource.h"
#include "../include/CQueue.h"
#include "../include/CTestEventsTimer.h"
#include "../../../gui/res-widgets/ceventlistwidget.h"
#include "../../db-models/CEventsDBModel.h"
#include "../include/CEventsSyncTimer.h"
#include "backend/db-models/db_conn.h"

extern SProgCfg   g_progCfg;

extern uint32_t id_admin;
extern std::vector<EventEntry>  entriesToUpdate; // это отслеживание событий.
extern SSyncEvents syncEventsData;

CEventResource::CEventResource(QFrame *placeholder, CAbstractConnection *connection, CQueue *queue)
    : m_placeholder(placeholder)
    , m_connection(connection)
    , m_gui(new CEventListWidget())
    , m_db_model(new CEventsDBModel(m_connection))
    , m_queue(queue)
    , m_eventsScheduler(new CTestEventsTimer(m_queue, g_progCfg.eventsTrackingPeriod_ms))
    , m_syncEvents(new CEventsSyncTimer(m_queue, g_progCfg.publicNodesSyncPeriod_ms, m_connection->database_kind()))
{
    m_gui->setCallbackAddEntry([&](EventEntry &entry){ return this->addItem(entry);});
    m_gui->setCallbackRemoveEntry([&](std::vector<EventEntry>& items){ return this->removeItems(items);});
    m_gui->setCallbackUpdateEntry([&](EventEntry & entry){return this->updateItem(entry);});

    m_eventsScheduler->setEntries(&m_events, m_guardMutex);
    std::lock_guard lock(syncEventsData.sourceLock);
    syncEventsData.localData = &m_publicData;
}

CEventResource::~CEventResource()
{
    {
        std::lock_guard lock(m_guardMutex);
        m_events.clear();
        entriesToUpdate.clear();
    }

    delete m_eventsScheduler;
    delete m_syncEvents;

    m_db_model->finalDelete();

    delete m_gui;
    delete m_db_model;
}

void CEventResource::clear()
{
    hideWidget();
}

void CEventResource::putWidget()
{
    if (m_isShown)
    {
        return;
    }

    if (!m_placeholder->layout())
    {
        m_placeholder->setLayout(new QVBoxLayout());
        m_placeholder->layout()->setContentsMargins(0, 0, 0, 0);
    }

    m_gui->setParent(m_placeholder);
    m_placeholder->layout()->addWidget(m_gui);
    m_gui->show();
    m_isShown = true;
}

void CEventResource::hideWidget()
{
    if (!m_isShown)
    {
        return;
    }

    if (m_gui->parent() == m_placeholder)
    {
        m_placeholder->layout()->removeWidget(m_gui);
        m_gui->setParent(nullptr);
        m_gui->hide();
    }
    m_isShown = false;
}

void CEventResource::prefetchData(uint32_t userId)
{
    m_publicData.clear();


    if (userId == id_admin)
    {
        m_eventsScheduler->stop();
    }
    else
    {
        m_eventsScheduler->start();
    }

    std::vector<EventEntry> items;

    // protect and fulfill
    {
        std::lock_guard lock(syncEventsData.sourceLock);
        m_events = m_db_model->prefetchUsersEvents(0);

        if (!m_events.empty())
        {
            for (auto &item : m_events)
            {
                m_publicData[item.id_parent].push_back(item);
            }
        }

        if (userId > id_admin)
        {
            items = m_db_model->prefetchUsersEvents(userId);
            if (!items.empty())
            {
                for (auto &item : items)
                {
                    m_userData[item.id_parent].push_back(item);
                }
                m_events.insert(m_events.end(), items.begin(), items.end());
            }
        }
    }

    m_gui->setUserID(userId, id_admin);
}

bool CEventResource::addItem(EventEntry &textItem)
{
    std::lock_guard lock(m_guardMutex);
    //EventEntry item = textItem;
    textItem.id_parent = m_parentNode.id;
    bool res = m_db_model->addEvent(textItem);
    if (res)
    {
        m_events.push_back(textItem);
        if (m_parentNode.is_public)
            m_publicData[m_parentNode.id].push_back(m_events.back());
        else
            m_userData[m_parentNode.id].push_back(m_events.back());
        m_gui->eventModel()->addEntry(textItem);
    }
    return res;
}

bool CEventResource::removeItems(std::vector<EventEntry> &itemsToRemove)
{
    if (itemsToRemove.empty())
        return false;

    std::lock_guard lock(m_guardMutex);

    auto removeItem = [](std::vector<EventEntry> &array, const EventEntry &entry)
    {
        auto iter = std::find(array.begin(), array.end(), entry);
        if (iter != array.end()) {
            array.erase(iter);
        }
    };

    bool res = m_db_model->removeEvents(itemsToRemove);

    auto &map = m_parentNode.is_public ? m_publicData : m_userData;
    for (auto &item : itemsToRemove)
    {
        removeItem(map[m_parentNode.id], item);
        removeItem(m_events, item);
    }

    return res;
}

bool CEventResource::updateItem(EventEntry &item)
{
    std::lock_guard lock(m_guardMutex);
    item.id_parent = m_parentNode.id;
    bool res = m_db_model->updateEvent(item);

    if (res)
    {
        for (auto &evt : m_events)
        {
            if (evt.id == item.id)
            {
                evt = item;
                break;
            }
        }
        auto *map = m_parentNode.is_public ? &m_publicData : &m_userData;
        for (auto &pair : *map)
        {
            if (pair.second.empty())
            {
                continue;
            }

            auto it = std::find_if(pair.second.begin(), pair.second.end(),
                                   [item](EventEntry & event){return event.id == item.id; });
            if (it != pair.second.end())
            {
                *it = item;
            }
        }
    }

    return res;
}

void CEventResource::showNode(const SNode &node)
{
    if (!m_isShown)
    {
        putWidget();
    }

    std::vector<EventEntry> array;

    if (node.is_public)
    {
        array = m_publicData[node.id];
    }
    else
    {
        array = m_userData[node.id];
    }

    m_parentNode = node;
    m_gui->setCurrentNode(array, node);
}

void CEventResource::moveItemsToNode(uint32_t idNode, const std::vector<uint32_t> &Items)
{
    // make unique (items contains multiple ID equal as selected QModelIndex in the row)
    std::vector<uint32_t> items;
    std::unordered_set<uint32_t> uniqueItems(Items.begin(), Items.end());
    items.insert(items.end(), uniqueItems.begin(), uniqueItems.end());

    std::lock_guard lock(m_guardMutex);

    // helpers lambdas
    auto findEvent = [this](uint32_t id) -> EventEntry*
    {
        auto it = std::find_if(m_events.begin(), m_events.end(),
                               [id](EventEntry& e) { return e.id == id; });
        return it != m_events.end() ? &*it : nullptr;
    };

    auto removeFromStorage = [this](uint32_t parentId, uint32_t eventId)
    {
        auto it = m_publicData.find(parentId);
        if (it != m_publicData.end())
        {
            auto& vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                                     [eventId](EventEntry &e) { return e.id == eventId; }), vec.end());
        }
    };

    if (m_db_model->changeEventsParent(idNode, items))
    {
        for (auto &id : items)
        {
            EventEntry* event = findEvent(id);
            if (event)
            {
                event->id_parent = idNode;
                m_publicData[idNode].push_back(*event);
                removeFromStorage(m_parentNode.id, event->id);
            }
        }
    }
    showNode(m_parentNode);
}

void CEventResource::checkPendingTasks()
{
    std::vector<EventEntry> to_show;

    {
        std::lock_guard lock(m_guardMutex);
        to_show = entriesToUpdate;
        entriesToUpdate.clear();
    }

    if (!to_show.empty())
    {
        for (auto &item : to_show)
        {
            m_db_model->updateEvent(item);
        }
        m_gui->showPendingEvents(to_show);
    }

    std::lock_guard contentLock(syncEventsData.sourceLock);
    if (m_isShown && m_parentNode.is_public)
    {
        if (syncEventsData.is_updated)
        {
            syncEventsData.is_updated = false;
            showNode(m_parentNode);
        }
    }
}
