//
// Created by artem on 28.04.26.
//

#ifndef REMINDER_CTEXTRESOURCE_H
#define REMINDER_CTEXTRESOURCE_H

#include <QFrame>
#include "IResource.h"
#include "backend/db-models/CTextDBModel.h"


struct TextEntry;
class CTextWidget;
class CThreadPool;
class CTextSyncTimer;
class CQueue;

class CTextResource : public IResource
{

public:
    CTextResource(QFrame * placeholder, CAbstractConnection * connection, CQueue *queue);
    ~CTextResource() override;

    void clear() override;

    // put widget to the placeholder (if actual). May be to hide it to private and do it via displayNode
    void putWidget() override;
    void hideWidget() override;

    void prefetchData(uint32_t userId) override;

    void    moveItemsToNode(uint32_t idNode, const std::vector<uint32_t> & items) override;
    bool    addItem(TextEntry & textItem);
    bool    removeItems(std::vector<TextEntry> &itemsToRemove);
    void    showNode(const SNode &node) override;
    void    checkPendingTasks() override;

private:
    std::map<uint32_t, TextEntry> m_storage;
    CTextWidget * m_gui;
    QFrame      * m_placeholder;
    CAbstractConnection * m_connection;
    bool             m_isShown{false};
    CTextDBModel   * m_db_model;
    SNode       m_parentNode{};
    uint32_t    m_idUser{0};
    CTextSyncTimer * m_synchronizer;

    std::map<uint32_t, std::vector<TextEntry>>  m_publicData;
    std::map<uint32_t, std::vector<TextEntry>>  m_userData;
    SNode   m_currentNode;


private: // methods
    void    setEntrySecret(TextEntry & entry);
    void    setEntryPath(TextEntry & entry);
    void    setEntryUrl(TextEntry & entry);
    void    setEntryPlain(TextEntry & entry);
    bool    entryChanged(const TextEntry &entry);

};

#endif //REMINDER_CTEXTRESOURCE_H
