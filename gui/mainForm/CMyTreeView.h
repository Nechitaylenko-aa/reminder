//
// Created by artem on 02.05.26.
//

#ifndef REMINDER_CMYTREEVIEW_H
#define REMINDER_CMYTREEVIEW_H

#include <QTreeView>
#include "gui/models/ctreemodel.h"

constexpr uint32_t admin_id = 1;

/** @brief created to handle Drag&Drop */
class CMyTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit CMyTreeView(QWidget *parent = nullptr);
    ~CMyTreeView() override;

    void   setUserId(uint32_t userId);
    void   setCallbackTextEntriesDropped(std::function<void(uint32_t targetId, const std::vector<uint32_t>)> handler);
    void   setCallbackEventEntriesDropped(std::function<void(uint32_t targetId, const std::vector<uint32_t>)> handler);
    void   setCallbackInnerMove(std::function<void(uint32_t, const std::vector<uint32_t>)> handler);
protected:

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;



private: // members
    uint32_t m_user_id{0};
    std::function<void(uint32_t, const std::vector<uint32_t>)> m_cbTextEntriesDropped;
    std::function<void(uint32_t, const std::vector<uint32_t>)> m_cbEventsDropped;
    std::function<void(uint32_t, const std::vector<uint32_t>)> m_cbInnerMove;

private: // methods
    bool  canDropItem(const QModelIndex &index);

    void  dropLocal(QDropEvent *event);
    void  dropTextEntries(QDropEvent *event);
    void  dropEventEntries(QDropEvent *event);

    bool isDescendantOf(TreeItem *possibleChild, TreeItem *draggedItem);
};


#endif //REMINDER_CMYTREEVIEW_H
