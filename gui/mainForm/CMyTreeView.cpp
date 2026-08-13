//
// Created by artem on 02.05.26.
//

#include "CMyTreeView.h"
#include <QMouseEvent>
#include <QApplication>
#include <QMimeData>

CMyTreeView::CMyTreeView(QWidget *parent) : QTreeView(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    // Заводские настройки оформления
    setDragDropOverwriteMode(false);
    setDefaultDropAction(Qt::MoveAction);
}

CMyTreeView::~CMyTreeView()
= default;

void CMyTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void CMyTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    if (canDropItem(this->indexAt(event->position().toPoint())))
    {
        event->acceptProposedAction();
    }
}

void CMyTreeView::dropEvent(QDropEvent *event)
{
    if (event->source() == this)
    {
        dropLocal(event);
        return;
    }
    if (event->mimeData()->hasFormat("application/x-text-internal"))
    {
        dropTextEntries(event);
        return;
    }

    if (event->mimeData()->hasFormat("application/x-event-internal"))
    {
        dropEventEntries(event);
        return;
    }
}

bool CMyTreeView::canDropItem(const QModelIndex &index)
{
    // not logged in
    if (m_user_id == 0)
    {
        return false;
    }

    // standard tests ----------------------------------------------------------------------
    if (!index.isValid())
    {
        return false;
    }
    auto targetItem = static_cast<TreeItem*>(index.internalPointer());
    if (!targetItem || !targetItem->get_node())
    {
        return false;
    }

    const SNode* targetNode = targetItem->get_node();
    if (!targetNode->is_container)
    {
        return false;
    }

    QModelIndexList selected = selectionModel()->selectedIndexes();
    if (selected.empty())
    {
        return false;
    }
    auto draggedItem = static_cast<TreeItem*>(selected.at(0).internalPointer());
    if (!draggedItem || !draggedItem->get_node())
    {
        return false;
    }

    if (draggedItem->parentItem() == targetItem)
    {
        return false;
    }

    // check user is logged-in and resource is him -------------------------------------------
    // don't drop to user area if admin
    if ((m_user_id == admin_id && targetNode->user_id > admin_id) || (m_user_id == admin_id && !targetNode->is_public) )
    {
        return false;
    }

    if (draggedItem->get_node()->user_id != m_user_id && m_user_id > admin_id)
    {
        // qDebug() << "User not admin and target not users";
        return false;
    }

    // check resource type compatible --------------------------------------------------------
    if (draggedItem->get_node()->res_type != targetNode->res_type)
    {
        //qDebug() << "target node resource differ from dragged node";
        return false;
    }


    if (targetItem == draggedItem || isDescendantOf(targetItem, draggedItem))
    {
        // qDebug() << "Don't try to drop parent node to the child node";
        return false;
    }

    return true;
}

void CMyTreeView::setUserId(uint32_t userId)
{
    m_user_id = userId;
}

bool CMyTreeView::isDescendantOf(TreeItem *possibleChild, TreeItem *ancestor)
{
    if (!possibleChild || !ancestor)
    {
        return false;
    }

    if (possibleChild == ancestor)
    {
        return true;
    }

    for (int i = 0; i < ancestor->childCount(); i++)
    {
        TreeItem *child = ancestor->child(i);
        if (child == possibleChild || isDescendantOf(possibleChild, child))
        {
            return true;
        }
    }

    return false;
}

void CMyTreeView::dropLocal(QDropEvent *event)
{
    auto selection = this->selectionModel()->selectedIndexes();
    if (selection.empty())
    {
        return;
    }
    auto draggedIndex = selection.at(0);
    auto targetIndex = this->indexAt(event->position().toPoint());
    if (!targetIndex.isValid() || !draggedIndex.isValid())
    {
        return;
    }

    SNode *movingNode = static_cast<TreeItem*>(draggedIndex.internalPointer())->get_node();
    SNode *targetNode = static_cast<TreeItem*>(targetIndex.internalPointer())->get_node();
    movingNode->parent_id = targetNode->id;

    auto *treeModel = dynamic_cast<CTreeModel*>(this->model());
    treeModel->moveNode(*movingNode);

    if (m_cbInnerMove)
    {
        m_cbInnerMove(targetNode->id, {movingNode->id});
    }
}

void CMyTreeView::dropTextEntries(QDropEvent *event)
{
    QByteArray rawData = event->mimeData()->data("application/x-textEntry-data");
    QDataStream stream(&rawData, QIODevice::ReadOnly);

    QModelIndex targetIndex = indexAt(event->position().toPoint());
    auto treeItem = static_cast<TreeItem*>(targetIndex.internalPointer());
    if (!treeItem || !treeItem->get_node())
    {
        return;
    }

    uint32_t targetId = treeItem->get_node()->id;

    qsizetype rowsCount;
    stream >> rowsCount;
    std::vector<uint32_t> textEntriesID;
    for (int64_t i = 0; i < rowsCount; ++i)
    {
        uint32_t  idTextEntry;
        stream >> idTextEntry;
        textEntriesID.push_back(idTextEntry);
    }
    if (textEntriesID.empty())
        return;

    if (m_cbTextEntriesDropped)
        m_cbTextEntriesDropped(targetId, textEntriesID);
    else
        qDebug() << "Don't forget to setup callback for CMyTreeView::dropTextEntries";
}

void CMyTreeView::dropEventEntries(QDropEvent *event)
{
    QByteArray rawData = event->mimeData()->data("application/x-event-internal");
    QDataStream stream(&rawData, QIODevice::ReadOnly);

    QModelIndex targetIndex = indexAt(event->position().toPoint());
    auto treeItem = static_cast<TreeItem*>(targetIndex.internalPointer());
    if (!treeItem || !treeItem->get_node())
    {
        return;
    }

    uint32_t targetId = treeItem->get_node()->id;

    qsizetype rowsCount;
    stream >> rowsCount;
    std::vector<uint32_t> textEntriesID;
    for (int64_t i = 0; i < rowsCount; ++i)
    {
        uint32_t  idTextEntry;
        stream >> idTextEntry;
        textEntriesID.push_back(idTextEntry);
    }
    if (textEntriesID.empty())
        return;

    if (m_cbEventsDropped)
        m_cbEventsDropped(targetId, textEntriesID);
    else
        qDebug() << "Don't forget to setup callback for CMyTreeView::dropTextEntries";
}

void CMyTreeView::setCallbackTextEntriesDropped(std::function<void(uint32_t , std::vector<uint32_t>)> handler)
{
    m_cbTextEntriesDropped = std::move(handler);
}

void CMyTreeView::setCallbackEventEntriesDropped(std::function<void(uint32_t, const std::vector<uint32_t>)> handler)
{
    m_cbEventsDropped = std::move(handler);
}

void CMyTreeView::setCallbackInnerMove(std::function<void(uint32_t, const std::vector<uint32_t>)> handler)
{
    m_cbInnerMove = std::move(handler);
}
