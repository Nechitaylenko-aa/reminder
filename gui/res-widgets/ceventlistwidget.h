#ifndef CEVENTLISTWIDGET_H
#define CEVENTLISTWIDGET_H

#include <QFrame>
#include "gui/models/ceventdatamodel.h"
#include "backend/sources/include/base-types.h"

namespace Ui {
class CEventListWidget;
}

class CEventListWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CEventListWidget(QWidget *parent = nullptr);
    ~CEventListWidget() override;

    void setCallbackAddEntry(std::function<bool(EventEntry &)> handler);
    void setCallbackRemoveEntry(std::function<bool(std::vector<EventEntry> &)> handler);
    void setCallbackUpdateEntry(std::function<bool(EventEntry&)> handler);
    CEventDataModel * eventModel();
    void showPendingEvents(const std::vector<EventEntry> & events);
    void setCurrentNode(std::vector<EventEntry> & entries, const SNode & node);
    void setUserID(uint32_t userId, uint32_t id_admin);
private slots:
    void slotAddEntry();
    void slotRemoveEntries();
    void slotUpdateEntry(EventEntry & entry);
    void slotContextMenu(const QPoint &pos);
    void slotAddSpecEntry();
    void slotEditEntry(const QModelIndex &index);

private:
    Ui::CEventListWidget *ui;
    CEventDataModel * m_model;
    SNode   m_node;

    std::function<bool(EventEntry &)> m_cbAddEvent;
    std::function<bool(std::vector<EventEntry> &)> m_cbRemoveEvents;
    std::function<bool(EventEntry&)> m_cbUpdateEvent;
    uint32_t  m_loggedInUser{0};
    uint32_t  m_id_admin{0};

private:
    void  updateButtons();

};

#endif // CEVENTLISTWIDGET_H
