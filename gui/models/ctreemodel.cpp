#include "ctreemodel.h"

//-----------------------------------------------------------------------------------------------------------------
// TreeItem. Contains system Node
//-----------------------------------------------------------------------------------------------------------------
TreeItem::TreeItem(const SNode &node, TreeItem *parentItem)
        : m_parentItem(parentItem)
        , m_node(new SNode(node))
{
    if (parentItem)
    {
        parentItem->appendChild(this);
    }

    switch (node.res_type)
    {
        case ResourceType::TEXT:
            m_icon = QIcon(":/16/images/16/book.png");//:/16/images/16/document.png
            break;
        case ResourceType::DATES:
            m_icon = QIcon(":/16/images/16/calendar_date.png");
            break;
        default:
            break;
    }
}

TreeItem::TreeItem(QString rootName, TreeItem *parentItem)
        : m_parentItem(parentItem)
        , m_rootName(std::move(rootName))
        , m_node(nullptr)
{
    if (parentItem)
    {
        parentItem->appendChild(this);
    }
}

TreeItem::~TreeItem()
{
    clearChildren();
    delete m_node;
}

SNode *TreeItem::get_node()
{
    return m_node;
}

void TreeItem::appendChild(TreeItem *child)
{
    if (child)
    {
        m_childItems.push_back(child);
        child->m_parentItem = this;
    }
}

TreeItem *TreeItem::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int TreeItem::childCount() const
{
    return m_childItems.count();
}

int TreeItem::columnCount() const
{
    return 1;
}

QVariant TreeItem::data(int column) const
{
    if (column > 0)
        return {};
    return m_node ? QVariant(m_node->title.c_str()) : QVariant(m_rootName);
}

int TreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

TreeItem *TreeItem::parentItem()
{
    return m_parentItem;
}

void TreeItem::removeChild(int idx)
{
    if (m_childItems.empty())
        return;
    if (idx < 0 || idx >= m_childItems.size())
        return;

    //delete m_childItems.at(idx);  // Важно: удаляем объект
    m_childItems.removeAt(idx);
}

void TreeItem::clearChildren()
{
    for (auto &item : m_childItems)
    {
        delete item;
    }
    m_childItems.clear();
}

QIcon TreeItem::getIcon()
{
    return m_icon;
}

void TreeItem::setIcon(const QString &string)
{
    m_icon = QIcon(string);
}


//-----------------------------------------------------------------------------------------------------------------
// CTreeModel
//-----------------------------------------------------------------------------------------------------------------

CTreeModel::CTreeModel(QObject *parent)
        : QAbstractItemModel(parent)
        , m_root(new TreeItem("Root")){}

CTreeModel::~CTreeModel()
{
    delete m_root;
}

QVariant CTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
    {
        return "Nodes";
    }
    return {};
}

QModelIndex CTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);

    return {};
}

QModelIndex CTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_root)
        return {};

    return createIndex(parentItem->row(), 0, parentItem);
}

int CTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (!parent.isValid())
        parentItem = m_root;

    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int CTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant CTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (role != Qt::DisplayRole && role != Qt::DecorationRole && role != Qt::UserRole)
        return {};

    auto *item = static_cast<TreeItem*>(index.internalPointer());

    if (role == Qt::DisplayRole)
        return item->data(index.column());
    if (role == Qt::UserRole)
        return item->get_node()->id;

    if (item->parentItem() == m_root)
    {
        if (item->get_node()->is_public)
            return QIcon(":/48/images/48/kdmconfig-2.png");
        return QIcon(":/48/images/48/identity.png");
    }
    return item->getIcon();
}

Qt::ItemFlags CTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (!index.parent().isValid())
    {
        return Qt::ItemIsEnabled;
    }

    auto *treeItem = static_cast<TreeItem*>(index.internalPointer());
    SNode* node = treeItem->get_node();

    if (!node)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (user_id == 0)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    // if node is not belongs user
    if (user_id != node->user_id)
    {
        // but if target node is container
        if (node->is_container)
        {

            if (user_id == 1 && node->is_public)
            {
                // and admin try to drop
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
            }
            if (user_id > 1 && !node->is_public)
            {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
            }
        }
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
}

void CTreeModel::resetModel(std::vector<SNode> &nodes)
{
    beginResetModel();

    // Очищаем старое дерево
    delete m_root;
    m_root = new TreeItem("Root");

    // Строим карту для быстрого поиска родительских элементов
    std::map<uint32_t, TreeItem*> nodeMap;


    for (auto &node : nodes)
    {
        auto *item = new TreeItem(node);
        nodeMap[node.id] = item;
    }

    for (auto &node : nodes)
    {
        TreeItem *item = nodeMap[node.id];
        TreeItem *parentItem = nullptr;

        if (node.parent_id != 0 && nodeMap.find(node.parent_id) != nodeMap.end())
        {
            parentItem = nodeMap[node.parent_id];
        }
        else
        {
            parentItem = m_root;
        }

        parentItem->appendChild(item);
    }

    endResetModel();
}

void CTreeModel::addNode(const SNode &node)
{
    TreeItem *parentItem = m_root;

    if (node.parent_id != 0)
    {
        QModelIndexList items = match(index(0, 0), Qt::UserRole, node.parent_id, 1, Qt::MatchRecursive);
        if (!items.isEmpty())
        {
            parentItem = static_cast<TreeItem*>(items.first().internalPointer());
        }
    }


    int row = parentItem->childCount();
    beginInsertRows(parentItem == m_root ? QModelIndex() : createIndex(parentItem->row(), 0, parentItem),
                    row, row);

    new TreeItem(node, parentItem);

    endInsertRows();
}

void CTreeModel::removeNode(const SNode &node)
{
    QModelIndexList items = match(index(0, 0), Qt::UserRole, node.id, 1, Qt::MatchRecursive);

    if (!items.isEmpty())
    {
        QModelIndex idx = items.first();
        auto *item = static_cast<TreeItem*>(idx.internalPointer());
        TreeItem *parentItem = item->parentItem();

        int row = item->row();

        beginRemoveRows(idx.parent(), row, row);
        parentItem->removeChild(row);
        endRemoveRows();
    }
}

void CTreeModel::moveNode(const SNode &node)
{
    removeNode(node);
    addNode(node);
}

bool CTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid())
    {
        auto treeItem = static_cast<TreeItem*>(index.internalPointer());
        if (treeItem && treeItem->get_node())
        {
            SNode *node = treeItem->get_node();
            node->title = value.toString().toStdString();
            if (m_callbackRename)
            {
                m_callbackRename(node);
            }
            return true;
        }
    }
    return false;
}

void CTreeModel::setUser(const uint32_t &userId)
{
    user_id = userId;
}

void CTreeModel::setRenameCallback(std::function<void(SNode *)> handler)
{
    m_callbackRename = std::move(handler);
}
/*
bool CTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    return QAbstractItemModel::insertRows(row, count, parent);
}

bool CTreeModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    return QAbstractItemModel::insertColumns(column, count, parent);
}

bool CTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    return QAbstractItemModel::removeRows(row, count, parent);
}

bool CTreeModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    return QAbstractItemModel::removeColumns(column, count, parent);
}*/
