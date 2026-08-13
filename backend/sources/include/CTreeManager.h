//
// Created by artem on 29.04.26.
//

#ifndef REMINDER_CTREEMANAGER_H
#define REMINDER_CTREEMANAGER_H


#include <QTreeView>
#include "base-types.h"
#include "backend/db-models/units/CUser.h"
#include "CQueue.h"

struct SSyncTree
{
    bool needUpdate{false};
    std::mutex  protectionLock;
    std::map<uint32_t, SNode> * publicNodes{nullptr};
};

class CAbstractConnection;
class CTreeModel;
class CTreeDBModel;
class IResource;
class CMyTreeView;
class CThreadPool;
class CTreeSynchronizer;

class CTreeManager  {
public:
    CTreeManager(CMyTreeView* view, QFrame* contentFrame);
    ~CTreeManager();

    void authorize(const CUser &user);
    void logout();

    void nodeClicked(const QModelIndex &index);

    bool addSiblingNode(const QModelIndex &srcNode);
    bool addNestedNode(const QModelIndex &srcNode);
    void removeNode(const QModelIndex &index);

    void  checkPendingTasks();

    CAbstractConnection * connection();

private:
    // Наши данные
    std::vector<SNode> m_allNodes;

    std::map<uint32_t, SNode> m_publicStorage;
    std::map<uint32_t, SNode> m_privateStorage;

    IResource   * m_resources[ResourceType::RT_COUNT]{};
    IResource   * m_active{nullptr};
    CMyTreeView   * m_view;
    QFrame      * m_resourceContainer;
    CAbstractConnection * m_db_connection;
    CTreeModel  * m_guiTreeModel;
    CTreeDBModel* m_treeDBModel;
    CUser         m_user;
    CThreadPool * m_threadPool;
    CQueue      * m_queue;
    SNode * currentNode{nullptr};
    CTreeSynchronizer   *m_treeSync;

private: // methods
    void rebuildPresenter();
    void loadTree(uint32_t id_user);
    void updateNode(SNode *node);
    void textEntriesDropped(uint32_t targetId, const std::vector<uint32_t> &array);
    void eventsDropped(uint32_t targetId, const std::vector<uint32_t> &array);
    void nodesDropped(uint32_t targetId, const std::vector<uint32_t> &nodes);
};

#endif //REMINDER_CTREEMANAGER_H
