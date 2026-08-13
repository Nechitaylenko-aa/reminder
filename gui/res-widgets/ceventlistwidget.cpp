#include "ceventlistwidget.h"
#include "ui_ceventlistwidget.h"
#include <QPushButton>
#include <QMenu>
#include "cdateswidget.h"
#include "../delegates/ceventdelegate.h"
#include "activeeventsdlg.h"

CEventListWidget::CEventListWidget(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::CEventListWidget)
    , m_model(new CEventDataModel())
{
    ui->setupUi(this);

    CMyTableView * view = ui->tvEventTable;
    view->setModel(m_model);
    view->setType(ResourceType::DATES);

    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    connect(m_model, &CEventDataModel::entryChanged, this, &CEventListWidget::slotUpdateEntry);
    connect(ui->tbAddEvent, &QPushButton::released, this, &CEventListWidget::slotAddEntry);
    connect(ui->tbRemoveEvent, &QPushButton::released, this, &CEventListWidget::slotRemoveEntries);
    connect(view, &CMyTableView::customContextMenuRequested, this, &CEventListWidget::slotContextMenu);

    connect(m_model, &QAbstractTableModel::dataChanged, view,
            [&, view](){view->horizontalHeader()->update();});
    connect(m_model, &QAbstractTableModel::rowsInserted, view,
            [&, view](){view->horizontalHeader()->update();});
    connect(m_model, &QAbstractTableModel::rowsRemoved, view,
            [&, view](){view->horizontalHeader()->update();});
    connect(m_model, &QAbstractTableModel::modelReset, view,
            [&, view](){view->horizontalHeader()->update();});

    auto header = view->horizontalHeader();
    for (int i = 0; i < header->count(); i++)
    {
        header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }

    header->setStretchLastSection(true);

    auto evtDelegate = new EventEditorDelegate(view);
    view->setItemDelegate(evtDelegate);
}

CEventListWidget::~CEventListWidget()
{
    delete ui;
}

void CEventListWidget::setCallbackAddEntry(std::function<bool(EventEntry &)> handler)
{
    m_cbAddEvent = std::move(handler);
}

void CEventListWidget::setCallbackRemoveEntry(std::function<bool(std::vector<EventEntry> &)> handler)
{
    m_cbRemoveEvents = std::move(handler);
}

void CEventListWidget::setCallbackUpdateEntry(std::function<bool(EventEntry &)> handler)
{
    m_cbUpdateEvent = std::move(handler);
}

CEventDataModel *CEventListWidget::eventModel()
{
    return m_model;
}

void CEventListWidget::showPendingEvents(const std::vector<EventEntry> &events)
{
    if (events.empty())
    {
        return;
    }

    auto activeItemsDlg = new ActiveEventsDlg(nullptr);

    activeItemsDlg->setActiveEvents(events);
    activeItemsDlg->exec();

    delete activeItemsDlg;
}

void CEventListWidget::slotAddEntry()
{
    EventEntry event;
    event.event = time(nullptr); // что бы не сильно менять
    event.description = "Новое событие";
    event.isEnabled = false; // не отслеживаем, а то не отредактируем

    if (m_cbAddEvent)
    {
        m_cbAddEvent(event);
    }
}

void CEventListWidget::slotRemoveEntries()
{
    std::vector<EventEntry> toRemove;

    auto selected = ui->tvEventTable->selectionModel()->selectedRows(1);
    if (selected.empty())
    {
        return;
    }

    for (auto &item : selected)
    {
        auto evt = static_cast<EventEntry*>(item.internalPointer());
        if (!evt)
        {
            continue;
        }

        toRemove.push_back(*evt);
    }

    if (m_cbRemoveEvents)
    {
        if (m_cbRemoveEvents(toRemove))
        {
            m_model->removeEntries(toRemove);
        }
    }
}

void CEventListWidget::slotUpdateEntry(EventEntry &entry)
{
    if (m_cbUpdateEvent)
    {
        m_cbUpdateEvent(entry);
    }
}

void CEventListWidget::slotContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    auto actNewEvent = new QAction(QIcon(":/24/images/24/dialog-more.png"), "Добавить событие", &menu);
    auto actRemoveEvt = new QAction(QIcon(":/16/images/16/dialog-cancel-4.png"), "Удалить событие", &menu);
    auto actEditEvt = new QAction(QIcon(":/48/images/48/configure-2.png"), "Редактировать событие...", &menu);

    auto selected = ui->tvEventTable->selectionModel()->selectedRows();
    std::set<QModelIndex> selectedSet(selected.begin(), selected.end());

    bool empty = selectedSet.empty();
    bool single = selectedSet.size() == 1;

    actNewEvent->setEnabled(m_loggedInUser == m_node.user_id && m_loggedInUser > 0);
    actEditEvt->setEnabled(single);
    actRemoveEvt->setEnabled(!empty);

    connect(actNewEvent, &QAction::triggered, this, &CEventListWidget::slotAddSpecEntry);
    connect(actRemoveEvt, &QAction::triggered, this, &CEventListWidget::slotRemoveEntries);
    if (!empty)
    {
        connect(actEditEvt, &QAction::triggered, [&, selectedSet](){ slotEditEntry(*selectedSet.begin());});
    }


    menu.addAction(actNewEvent);
    menu.addAction(actEditEvt);
    menu.addAction(actRemoveEvt);

    menu.exec(mapToGlobal(pos));
}

void CEventListWidget::slotAddSpecEntry()
{
    EventEntry newEntry;
    auto dlg = new CEventWidget(nullptr);
    dlg->setEvent(newEntry);

    if (!dlg->exec())
    {
        return;
    }

    if (m_cbAddEvent)
    {
        m_cbAddEvent(newEntry);
    }

    delete dlg;
}

void CEventListWidget::slotEditEntry(const QModelIndex &index)
{
    auto *newEntry = static_cast<EventEntry*>(index.internalPointer());
    if (!newEntry)
    {
        return;
    }
    auto dlg = new CEventWidget(nullptr);
    dlg->setEvent(*newEntry);

    if (!dlg->exec())
    {
        return;
    }

    if (m_cbUpdateEvent)
    {
        m_cbUpdateEvent(*newEntry);
    }
    delete dlg;
}

void CEventListWidget::setCurrentNode(std::vector<EventEntry> & entries, const SNode & node)
{
    m_node = node;
    m_model->resetModel(entries, node.user_id);
    updateButtons();
}

void CEventListWidget::updateButtons()
{
    bool isEnabledAdd = false;
    bool isEnabledDel = false;
    bool isEnabledExec= false;

    if (m_node.user_id == m_loggedInUser && m_node.is_editable)
    {
        isEnabledAdd = true;
        isEnabledDel = true;
        isEnabledExec = true;
    }
    //if (m_node.user_id == 0 && m_node.is_public == false)

    ui->tbAddEvent->setEnabled(isEnabledAdd);
    ui->tbRemoveEvent->setEnabled(isEnabledDel);
}

void CEventListWidget::setUserID(uint32_t userId, uint32_t id_admin)
{
    m_loggedInUser = userId;
    m_id_admin = id_admin;
    m_model->setUserId(userId);
}
