#include "ctextdatamodel.h"
#include <QIcon>
#include <QMimeData>
#include <QIODevice>

CTextDataModel::CTextDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_header << "Описание" << "Значение";
    m_icons << ":/24/images/24/List.png" << ":/24/images/24/box-label.png";
}

QVariant CTextDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return {};

    if (role != Qt::DecorationRole && role != Qt::DisplayRole)
        return {};

    if (role == Qt::DecorationRole)
    {
        return QIcon(m_icons.at(section));
    }
    return m_header.at(section);
}

int CTextDataModel::rowCount(const QModelIndex &parent) const
{
    return m_entries.size();
}

int CTextDataModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant CTextDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (role != Qt::DisplayRole && role != Qt::UserRole && role != Qt::DecorationRole)
    {
        return {};
    }

    if (role == Qt::UserRole)
    {
        return m_entries.at(index.row()).id;
    }

    auto entry = m_entries.at(index.row());

    if (index.column() == 0)
    {
        if (role == Qt::DisplayRole)
        {
            return m_entries.at(index.row()).desc.c_str();
        }
        else
        {
            return getIcon(entry);
        }
    }

    if (entry.type == TextEntryType::SECRET)
    {
        if (!entry.showSecret)
            return "*";
    }
    return entry.data.c_str();
}

bool CTextDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, Qt::DisplayRole) != value)
    {
        auto &entry = m_entries.at(index.row());

        if (index.column() == 0)
        {
            entry.desc = value.toString().toStdString();
        }

        if (index.column() == 1)
        {
            entry.data = value.toString().toStdString();
        }
        emit entryChanged(entry);
        return true;
    }
    return false;
}

Qt::ItemFlags CTextDataModel::flags(const QModelIndex &index) const
{
    auto textEntry = static_cast<TextEntry*>(index.internalPointer());

    if (!index.isValid() || !textEntry)
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags =  Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (m_user_id == m_ownerId)
    {
        flags |= Qt::ItemIsEditable  | Qt::ItemIsDragEnabled;
    }

    return flags;
}

QStringList CTextDataModel::mimeTypes() const
{
    QStringList types = QAbstractTableModel::mimeTypes();
    types << "application/x-text-internal";
    return types;
}

QMimeData *CTextDataModel::mimeData(const QModelIndexList &indexes) const
{
    auto *mimeData = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QList<uint32_t> rows;
    for (const QModelIndex &index : indexes)
    {
        if (index.isValid() && !rows.contains(index.row()))
        {
            auto entry = static_cast<TextEntry*>(index.internalPointer());
            if (entry)
                rows.append(entry->id);
        }
    }

    stream << rows.size();
    for (auto &id : rows)
    {
        stream << id;
    }

    mimeData->setData("application/x-text-internal", data);//  to retrieve: QByteArray rawData = event->mimeData()->data("application/x-textEntry-data");
    return mimeData;
}

bool CTextDataModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!m_entry)
        return false;

    beginInsertRows(parent, row, row + count - 1);

    m_entries.push_back(*m_entry);
    m_entry = nullptr;

    endInsertRows();
    return true;
}

bool CTextDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    auto removeItem = [&](const TextEntry &item, std::vector<TextEntry> *array)->void
    {
        auto it = std::find_if(array->begin(), array->end(),
                               [item](const TextEntry & entry){return item.id == entry.id;});
        if (it != array->end())
        {
            array->erase(it);
        }
    };

    beginRemoveRows(parent, row, row + count - 1);

    removeItem(*m_entry, &m_entries);
    m_entry = nullptr;

    endRemoveRows();
    return true;
}


void CTextDataModel::resetModel(std::vector<TextEntry> &entries, uint32_t ownerId, uint32_t userId)
{
    beginResetModel();
    m_user_id = userId;
    m_ownerId = ownerId;
    m_entries = entries;

    endResetModel();
}

void CTextDataModel::addEntry(const TextEntry &entry)
{
    m_entry = const_cast<TextEntry*>(&entry);
    insertRows(static_cast<int>(m_entries.size()), 1);
}

void CTextDataModel::removeEntries(std::vector<TextEntry> &entries)
{
    auto findIndex = [&](const TextEntry &item, std::vector<TextEntry> *array) -> uint32_t
    {
        auto it = std::find_if(array->begin(), array->end(),
                               [&](const TextEntry &e) { return e.id == item.id; });
        return it != array->end() ? std::distance(array->begin(), it) : UINT32_MAX;
    };

    for (auto &item : entries)
    {
        m_entry = &item;
        auto row = findIndex(item, &m_entries);
        assert(row < UINT32_MAX);
        removeRows(static_cast<int>(row), 1);
    }
}

TextEntry *CTextDataModel::getEntry(const QModelIndex &index)
{
    if (index.row() >= m_entries.size())
        return nullptr;
    return &m_entries.at(index.row());
}

QModelIndex CTextDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return {};
    }

    const TextEntry* entry = &m_entries.at(row);
    return createIndex(row, column, entry);
}

QIcon CTextDataModel::getIcon(const TextEntry &entry)
{
    switch (entry.type)
    {
        case TextEntryType::SECRET:
            return QIcon(":/24/images/24/eye.png");
        case TextEntryType::FILE_SYSTEM:
            return QIcon(":/16/images/16/add_node_16.png");
        case TextEntryType::URI_LINK:
            return QIcon(":/24/images/24/broken-link.png");
        case TextEntryType::PLAIN_TEXT:
            return QIcon(":/16/images/16/book.png");
    }
}

bool CTextDataModel::moveRows(int targetRow, const QList<QPersistentModelIndex> &rowsToMove)
{
    if (rowsToMove.isEmpty() || targetRow < 0)
        return false;

    QList<int> rowsToMoveIndices;
    for (const QPersistentModelIndex &idx : rowsToMove)
    {
        if (idx.isValid())
            rowsToMoveIndices.append(idx.row());
    }

    std::sort(rowsToMoveIndices.begin(), rowsToMoveIndices.end());

    std::vector<TextEntry> movedEntries;
    movedEntries.reserve(rowsToMoveIndices.size());

    targetRow -= rowsToMoveIndices.count();

    // Удаляем с конца чтобы не ломать индексы
    for (int64_t i = rowsToMoveIndices.size() - 1; i >= 0; --i)
    {
        int row = rowsToMoveIndices[i];
        movedEntries.insert(movedEntries.begin(), std::move(m_entries[row]));
        m_entries.erase(m_entries.begin() + row);
    }

    if (targetRow < 0)
        targetRow = 0;
    if (targetRow > static_cast<int>(m_entries.size()))
        targetRow = static_cast<int>(m_entries.size());

    m_entries.insert(
            m_entries.begin() + targetRow,
            std::make_move_iterator(movedEntries.begin()),
            std::make_move_iterator(movedEntries.end())
    );

    return true;
}

TextEntry *CTextDataModel::getEntry(uint16_t row)
{
    if (row >= m_entries.size())
        return nullptr;
    return &m_entries.at(row);
}

void CTextDataModel::setUserId(const uint32_t &idUser)
{
    m_user_id = idUser;
}
