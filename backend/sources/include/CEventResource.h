//
// Created by artem on 29.04.26.
//

#ifndef REMINDER_CEVENTRESOURCE_H
#define REMINDER_CEVENTRESOURCE_H

#include <QFrame>

#include "IResource.h"
#include "backend/db-models/units/CUser.h"

class CEventListWidget;
class CEventWidget;
class CEventsDBModel;
class CThreadPool;
class CTestEventsTimer;
struct EventEntry;
class  CEventsSyncTimer;
class CQueue;

class CEventResource : public IResource
{
public:
    CEventResource(QFrame * placeholder, CAbstractConnection * connection, CQueue *queue);
    ~CEventResource() override;

    // --- GUI ------------------------------------------------------------------
    void clear() override;

    // put widget to the placeholder (if actual). May be to hide it to private and do it via displayNode
    void putWidget() override;
    void hideWidget() override;

    // --- Data Management ------------------------------------------------------
    void prefetchData(uint32_t userId) override;

    // проверить обработанные в потоке данные
    void    checkPendingTasks() override;

    bool    addItem(EventEntry & textItem);
    bool    removeItems(std::vector<EventEntry> &itemsToRemove);
    bool    updateItem(EventEntry& item);
    void    showNode(const SNode &node) override;
    void    moveItemsToNode(uint32_t idNode, const std::vector<uint32_t> & items) override;



private:
    std::map<uint32_t, std::vector<EventEntry>> m_publicData;
    std::map<uint32_t, std::vector<EventEntry>> m_userData;
    CEventListWidget * m_gui;
    QFrame           * m_placeholder;
    CAbstractConnection * m_connection;
    bool               m_isShown{false};
    CEventsDBModel   * m_db_model;
    SNode              m_parentNode{};
    CUser              m_user{};
    CQueue      * m_queue;
    CTestEventsTimer * m_eventsScheduler;
    CEventsSyncTimer  * m_syncEvents;
    std::mutex  m_guardMutex;
    std::vector<EventEntry>   m_events;
private:


};

#endif //REMINDER_CEVENTRESOURCE_H
