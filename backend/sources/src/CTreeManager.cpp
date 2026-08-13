//
// Created by artem on 29.04.26.
//

#include "../include/CTreeManager.h"
#include "../../../gui/models/ctreemodel.h"
#include "../../db-models/ctreedbmodel.h"
#include "../include/CEventResource.h"
#include "../include/CTextResource.h"
#include "../../db-models/db_conn.h"
#include "../../../gui/mainForm/CMyTreeView.h"
#include "../include/CTreeSynchronizer.h"

extern uint32_t id_admin;
extern SSyncTree sync_tree;
extern SProgCfg   g_progCfg;

CTreeManager::CTreeManager(CMyTreeView *view, QFrame *contentFrame)
    : m_view(view)
    , m_resourceContainer(contentFrame)
{
    m_db_connection = CAbstractConnection::createDatabaseInstance(g_progCfg.db_type, &sdb_connection);

    auto threadCount = std::thread::hardware_concurrency();
    if (threadCount <= 6)
    {
        threadCount = 6;
    }
    m_threadPool = new CThreadPool(threadCount);

    m_queue = new CQueue(m_threadPool);
    m_treeSync = new CTreeSynchronizer(m_queue, 5000, m_db_connection->database_kind());

    m_treeDBModel = new CTreeDBModel(m_db_connection);
    m_resources[ResourceType::TEXT]  = new CTextResource(m_resourceContainer, m_db_connection, m_queue);
    m_resources[ResourceType::DATES] = new CEventResource(m_resourceContainer, m_db_connection, m_queue);

    m_guiTreeModel = dynamic_cast<CTreeModel*>(view->model());
    m_guiTreeModel->setRenameCallback([this](SNode*node){this->updateNode(node);});

    m_view->setCallbackTextEntriesDropped([&](uint32_t targetId, const std::vector<uint32_t> &entriesId)
    {
        textEntriesDropped(targetId, entriesId);
    });
    m_view->setCallbackEventEntriesDropped([&](uint32_t targetId, const std::vector<uint32_t> & entries)
    {
        eventsDropped(targetId, entries);
    });
    m_view->setCallbackInnerMove([&](uint32_t targetId, const std::vector<uint32_t> & nodes)
    {
        nodesDropped(targetId, nodes);
    });

    loadTree(0);

    std::lock_guard lock(sync_tree.protectionLock);
    sync_tree.publicNodes = &m_publicStorage;
}

CTreeManager::~CTreeManager()
{
    delete m_treeDBModel;

    for (auto & resource : m_resources)
    {
        delete resource;
    }
    delete m_treeSync;
    delete m_queue;
    // строго после ресурсов (клиентов
    delete m_threadPool;

    m_db_connection->Delete();
}

void CTreeManager::loadTree(uint32_t id_user)
{
    // load public tree
    m_allNodes = m_treeDBModel->get_nodes();
    m_publicStorage.clear();
    for (auto &item : m_allNodes)
    {
        m_publicStorage.emplace(item.id, item);
    }

    if (id_user > id_admin)
    {
        m_privateStorage.clear();
        auto privateArr = m_treeDBModel->get_nodes(id_user);
        for (auto &item : privateArr)
        {
            m_privateStorage.emplace(item.id, item);
        }
        m_allNodes.insert(m_allNodes.end(), privateArr.begin(), privateArr.end());
    }

    m_guiTreeModel->resetModel(m_allNodes);
}

void CTreeManager::authorize(const CUser &user)
{
    m_user = user;

    loadTree(user.get_id());

    for (auto & resource : m_resources)
    {
        resource->prefetchData(m_user.get_id());
    }

    m_guiTreeModel->setUser(m_user.get_id());
    m_view->setUserId(m_user.get_id());
}

void CTreeManager::logout()
{
    for (auto & resource : m_resources)
    {
        resource->clear();
        resource->prefetchData(0);
    }

    if (m_active)
    {
        m_active->hideWidget();
    }
    m_active = nullptr;

    m_privateStorage.clear();
    m_allNodes.clear();

    for (auto &item : m_publicStorage)
    {
        m_allNodes.push_back(item.second);
    }


    m_user = {};
    m_user.set_id(0);

    authorize(m_user);
    m_guiTreeModel->resetModel(m_allNodes);

}

bool CTreeManager::addSiblingNode(const QModelIndex &srcNode)
{
    if (!srcNode.isValid() || !srcNode.parent().isValid())
    {
        return false;
    }

    auto *srcItem = static_cast<TreeItem*>(srcNode.internalPointer());
    if (!srcItem || !srcItem->get_node())
    {
        return false;
    }

    SNode node;
    node.title = "New node";
    node.user_id = m_user.get_id();
    node.parent_id = srcItem->parentItem() ? srcItem->parentItem()->get_node()->id : 0;
    node.res_type = srcItem->get_node()->res_type;
    node.is_admin = false;
    node.is_public = srcItem->get_node()->is_public;
    node.is_editable = true;
    node.is_container = true;

    auto res = m_treeDBModel->addNode(node);
    if (res)
    {
        m_guiTreeModel->addNode(node);
        if (node.is_public)
        {
            m_publicStorage[node.id] = node;
        }
        else
        {
            m_privateStorage[node.id] = node;
        }
    }

    return res;
}

bool CTreeManager::addNestedNode(const QModelIndex &srcNode)
{
    if (!srcNode.isValid())
    {
        return false;
    }

    auto *srcItem = static_cast<TreeItem*>(srcNode.internalPointer());
    if (!srcItem || !srcItem->get_node())
    {
        return false;
    }

    SNode node;
    node.title = "New node";
    node.user_id = m_user.get_id();
    node.parent_id = srcItem->get_node()->id;
    node.res_type = srcItem->get_node()->res_type;
    node.is_admin = false;
    node.is_public = srcItem->get_node()->is_public;
    node.is_editable = true;
    node.is_container = true;

    auto res = m_treeDBModel->addNode(node);
    if (res)
    {
        m_guiTreeModel->addNode(node);
        if (node.is_public)
        {
            m_publicStorage[node.parent_id] = node;
        }
        else
        {
            m_privateStorage[node.parent_id] = node;
        }
    }

    return res;
}

void CTreeManager::removeNode(const QModelIndex &index)
{
    if (!index.isValid() || (!static_cast<TreeItem*>(index.internalPointer())))
    {
        return;
    }

    auto *item = static_cast<TreeItem*>(index.internalPointer());
    if (!item)
    {
        return;
    }
    SNode *node = item->get_node();

    bool res = m_treeDBModel->removeNode(node->id);
    if (res)
    {
        {
            if (node->is_public)
            {
                std::lock_guard lock(sync_tree.protectionLock);
                auto it = m_publicStorage.find(node->id);
                m_publicStorage.erase(it);
            }
            else
            {
                auto iter = m_privateStorage.find(node->id);
                m_privateStorage.erase(iter);
            }
        }
        m_guiTreeModel->removeNode(*node);
    }
}

void CTreeManager::updateNode(SNode *node)
{
    if (!node)
    {
        return;
    }

    m_treeDBModel->updateNode(*node);
}

void CTreeManager::rebuildPresenter()
{
    m_guiTreeModel->resetModel(m_allNodes);
    m_view->expandAll();
}

void CTreeManager::nodeClicked(const QModelIndex &current)
{
    if (!current.isValid())
    {
        return;
    }

    auto *item = static_cast<TreeItem*>(current.internalPointer());
    SNode *node = item->get_node();
    currentNode = node;

    auto res = m_resources[node->res_type];
    if (res != m_active)
    {
        if (m_active)
        {
            m_active->clear();
            m_active->hideWidget();
        }
        m_active = res;
    }

    m_active->showNode(*node);
}

void CTreeManager::textEntriesDropped(uint32_t targetId, const std::vector<uint32_t> &array)
{
    m_resources[TEXT]->moveItemsToNode(targetId, array);
}

void CTreeManager::eventsDropped(uint32_t targetId, const std::vector<uint32_t> &array)
{
    m_resources[DATES]->moveItemsToNode(targetId, array);
}

void CTreeManager::checkPendingTasks()
{
    {
        std::lock_guard lock(sync_tree.protectionLock);
        if (sync_tree.needUpdate)
        {
            sync_tree.needUpdate = false;
            m_allNodes.clear();
            for (auto & item : m_publicStorage)
            {
                m_allNodes.push_back(item.second);
            }
            for (auto & item : m_privateStorage)
            {
                m_allNodes.push_back(item.second);
            }
            m_guiTreeModel->resetModel(m_allNodes);

            if (currentNode)
            {
                auto node = *currentNode;

                if (m_active)
                {
                    if (std::find_if(m_allNodes.begin(), m_allNodes.end(), [node](const SNode &item)
                    { return item.id == node.id; }) != m_allNodes.end())
                    {
                        m_active->showNode(node);
                    }
                    else
                    {
                        m_active->showNode(m_allNodes.front());
                    }
                }
                currentNode = nullptr;
            }
        }
    }

    for (auto &resource : m_resources)
    {
        resource->checkPendingTasks();
    }
}

void CTreeManager::nodesDropped(uint32_t targetId, const std::vector<uint32_t> &nodes)
{
    std::vector<SNode> nodesToUpdate;

    auto iter = std::find_if(m_allNodes.begin(), m_allNodes.end(), [targetId](const SNode &item){return item.id == targetId;});
    if (iter == m_allNodes.end())
    {
        return;
    }
    SNode &targetNode = *iter;

    for (auto & id : nodes)
    {
        auto it = std::find_if(m_allNodes.begin(), m_allNodes.end(), [id](const SNode &item){return item.id == id;});
        if (it == m_allNodes.end())
        {
            continue;
        }
        nodesToUpdate.push_back(*it);
    }

    if (!nodesToUpdate.empty())
    {
        for (auto &node : nodesToUpdate)
        {
            node.parent_id = targetId;
            m_treeDBModel->updateNode(node);
        }
    }
}

CAbstractConnection *CTreeManager::connection()
{
    return m_db_connection;
}
