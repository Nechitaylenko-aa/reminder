//
// Created by artem on 02.05.26.
//

#include <QMenu>
#include "CMyTableView.h"
#include "backend/sources/include/base-types.h"
#include <QDragEnterEvent>
#include "../models/ctextdatamodel.h"
#include "../models/ceventdatamodel.h"

CMyTableView::CMyTableView(QWidget *parent)
    : QTableView(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    setDragDropMode(QAbstractItemView::InternalMove);

    setEditTriggers(QAbstractItemView::DoubleClicked |
                    QAbstractItemView::EditKeyPressed);
}

CMyTableView::~CMyTableView()
= default;

void CMyTableView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() == this)
    {
        if (m_type == ResourceType::TEXT)
        {
            if (!m_textModel)
            {
                m_textModel = dynamic_cast<CTextDataModel*>(model());
            }

            if (m_textModel->getUserId() == m_textModel->getOwnerId())
            {
                event->acceptProposedAction();
            }
            else
                event->ignore();
        }
        else
        {
            if (!m_eventModel)
            {
                m_eventModel = dynamic_cast<CEventDataModel*>(model());
            }
            if (m_eventModel->getUserId() == m_eventModel->getOwnerId())
            {
                event->acceptProposedAction();
            }
            else
                event->ignore();
        }
    }
    else
    {
        event->ignore();
    }
    QAbstractItemView::dragMoveEvent(event);
}

void CMyTableView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() != this)
    {
        event->ignore();
        return;
    }

    auto targetIndex = this->indexAt(event->position().toPoint());
    if (!targetIndex.isValid())
    {
        event->acceptProposedAction();
        return;
    }

    auto selectModel = this->selectionModel();
    auto selection = selectModel->selectedRows();

    auto min = std::min(selection.first().row(), selection.last().row());
    auto max = std::max(selection.first().row(), selection.last().row());
    if (targetIndex.row() >= min && targetIndex.row() <= max)
    {
        event->ignore();
        return;
    }

    event->acceptProposedAction();
}

void CMyTableView::dropEvent(QDropEvent *event)
{

    //auto *model = dynamic_cast<CTextDataModel*>(this->model());
    auto model = this->model();

    QModelIndexList selectedRows = this->selectionModel()->selectedRows();
    if (event->source() != this || !model || selectedRows.isEmpty())
    {
        event->ignore();
        return;
    }

    QModelIndex targetIndex = this->indexAt(event->position().toPoint());
    int targetRow = targetIndex.row();
    if (!targetIndex.isValid())
    {
        targetRow = model->rowCount();
    }

    QList<QPersistentModelIndex> persistentIndexes;
    for (const QModelIndex &idx : selectedRows)
    {
        persistentIndexes.append(QPersistentModelIndex(idx));
    }

    // по убыванию
    auto pred = [](const QPersistentModelIndex &a, const QPersistentModelIndex &b)->bool
    {
        return a.row() > b.row();
    };
    std::sort(persistentIndexes.begin(), persistentIndexes.end(), pred);


    int selectMin = persistentIndexes.last().row();
    int selectMax = persistentIndexes.first().row();

    // Не дропать между выбранными строками
    if (targetRow >= selectMin && targetRow <= selectMax)
    {
        event->ignore();
        return;
    }

    bool res;
    if (m_type == ResourceType::TEXT)
        res = dynamic_cast<CTextDataModel*>(this->model())->moveRows(targetRow, persistentIndexes);
    else
        res = dynamic_cast<CEventDataModel*>(this->model())->moveRows(targetRow, persistentIndexes);

    if (!res)
    {
        event->ignore();
            return;
    }

    this->selectionModel()->clear();

    event->accept();
}

void CMyTableView::setType(ResourceType type)
{
    m_type = type;
}
