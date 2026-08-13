//
// Created by artem on 29.04.26.
//
#include <QVBoxLayout>
#include "../include/CTextResource.h"
#include "../include/CTextSyncTimer.h"
#include "../../../gui/res-widgets/ctextwidget.h"
#include "../../../gui/res-widgets/CMyTableView.h"
#include "backend/db-models/db_conn.h"

#include <QHeaderView>


extern uint32_t id_admin;
extern SSyncText   syncTextData;
extern SProgCfg   g_progCfg;

CTextResource::CTextResource(QFrame *placeholder, CAbstractConnection *connection, CQueue *queue)
    : m_placeholder(placeholder)
    , m_connection(connection)
    , m_db_model(new CTextDBModel(m_connection))
    , m_gui(new CTextWidget())
    , m_synchronizer(new CTextSyncTimer(queue, g_progCfg.publicNodesSyncPeriod_ms, m_connection->database_kind()))
{

    m_gui->setCallbackEntrySecret([&](TextEntry & entry){ this->setEntrySecret(entry);});
    m_gui->setCallbackEntryPath([&](TextEntry & entry){ this->setEntryPath(entry);});
    m_gui->setCallbackEntryUrl([&](TextEntry & entry){this->setEntryUrl(entry);});
    m_gui->setCallbackEntryPlain([&](TextEntry & entry){this->setEntryPlain(entry);});

    m_gui->setCallbackAddEntry([&](TextEntry &entry){ return this->addItem(entry);});
    m_gui->setCallbackRemoveEntry([&](std::vector<TextEntry>& items){ return this->removeItems(items);});
    m_gui->setCallbackUpdateEntry([&](const TextEntry & entry){return this->entryChanged(entry);});

    std::lock_guard lock(syncTextData.sourceLock);
    syncTextData.localData = &m_publicData;
}

CTextResource::~CTextResource()
{
    m_db_model->finalDelete();
    delete m_synchronizer;
    delete m_db_model;
    delete m_gui;
}

void CTextResource::clear()
{
    hideWidget();
}

void CTextResource::putWidget()
{
    if (m_isShown)
        return;

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

void CTextResource::hideWidget()
{
    if (!m_isShown)
        return;

    if (m_gui->parent() == m_placeholder)
    {
        m_placeholder->layout()->removeWidget(m_gui);
        m_gui->setParent(nullptr);
        m_gui->hide();
    }
    m_isShown = false;
}

void CTextResource::prefetchData(uint32_t userId)
{
    m_idUser = userId;
    m_gui->model()->setUserId(m_idUser);

    // no need to synchronize for admin
    if (userId == id_admin)
    {
        m_synchronizer->stop();
    } else
    {
        m_synchronizer->start();
    }

    std::vector<TextEntry> items;
    // protect end fulfill
    {
        std::lock_guard lock(syncTextData.sourceLock);

        m_publicData.clear();
        items = m_db_model->prefetchData(0);

        if (!items.empty())
        {
            for (auto &item: items)
            {
                m_publicData[item.id_parent].push_back(item);
            }
        }
    }

    if (userId == 0)
    {
        return;
    }

    m_userData.clear();

    items = m_db_model->prefetchData(userId);
    if (!items.empty())
    {
        for (auto &item : items)
        {
            m_userData[item.id_parent].push_back(item);
        }
    }

}

bool CTextResource::addItem(TextEntry &textItem)
{
    textItem.type = TextEntryType::PLAIN_TEXT;
    textItem.id_parent = m_parentNode.id;

    bool res = m_db_model->addEntry(textItem);
    if (res)
    {
        std::vector<TextEntry> &array = m_parentNode.is_public ? m_publicData[m_parentNode.id] : m_userData[m_parentNode.id];
        array.push_back(textItem);
    }

    return res;
}

bool CTextResource::removeItems(std::vector<TextEntry> &itemsToRemove)
{
    bool res = false;
    for (auto &item : itemsToRemove)
    {
        res = m_db_model->removeEntry(item.id);
        if (res)
        {
            std::vector<TextEntry> &array = m_parentNode.is_public ? m_publicData[m_parentNode.id] : m_userData[m_parentNode.id];
            array.erase(std::remove_if(array.begin(), array.end(),
                                       [&item](const TextEntry &entry) { return entry.id == item.id; }), array.end());
        }
    }
    return res;
}

void CTextResource::showNode(const SNode &node)
{
    m_parentNode = node;
    if (!m_isShown)
    {
        putWidget();
    }

    std::vector<TextEntry> array;
    if (node.is_public)
    {
        array = m_publicData[node.id];
    } else
    {
        array = m_userData[node.id];
    }

    m_gui->setCurrentNode(array, node, m_idUser);
}

void CTextResource::setEntrySecret(TextEntry &entry)
{
    entry.type = TextEntryType::SECRET;
    m_db_model->updateEntry(entry);
}

void CTextResource::setEntryPath(TextEntry &entry)
{
    entry.type = TextEntryType::FILE_SYSTEM;
    m_db_model->updateEntry(entry);
}

void CTextResource::setEntryUrl(TextEntry &entry)
{
    entry.type = TextEntryType::URI_LINK;
    m_db_model->updateEntry(entry);
}

void CTextResource::setEntryPlain(TextEntry &entry)
{
    entry.type = TextEntryType::PLAIN_TEXT;
    m_db_model->updateEntry(entry);
}

void CTextResource::moveItemsToNode(uint32_t idNode, const std::vector<uint32_t> &items)
{
    m_db_model->changeParent(idNode, items);
    showNode(m_parentNode);
}

bool CTextResource::entryChanged(const TextEntry &entry)
{
    bool res = m_db_model->updateEntry(entry);
    if (res)
    {
        auto &map = m_parentNode.is_public ? m_publicData : m_userData;
        auto & array = map[m_parentNode.id];
        auto it = std::find_if(array.begin(), array.end(), [entry](TextEntry& item){ return entry.id == item.id;});
        if (it != array.end())
        {
            *it = entry;
        }
    }
    return res;
}

void CTextResource::checkPendingTasks()
{
    std::lock_guard lock(syncTextData.lock);
    if (syncTextData.content_changed)
    {
        syncTextData.content_changed = false;
        if (m_isShown)
        {
            auto node = m_parentNode;
            showNode(node);
        }
    }
}
