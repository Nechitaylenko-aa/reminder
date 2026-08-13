#include "ceventdatamodel.h"
#include <QIcon>
#include <QDateTime>
#include <qmimedata.h>
#include <QIODevice>
#include "../../backend/sources/include/DateTimeCalculator.h"

CEventDataModel::CEventDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_header << "Описание"
             << "Ед. периода"
             << "Колич. ед. пер."
             << "Следующий раз"
             << "Триггер"
             << "Приоритет"
             << "Отслеживание";
    m_icons.resize(7);


    getColumnValue[0] = [&](const EventEntry *entry){return getDescription(entry);};
    getColumnValue[1] = [&](const EventEntry *entry){return getPeriodUnit(entry);};
    getColumnValue[2] = [&](const EventEntry* entry) { return getUnitCount(entry); };
    getColumnValue[3] = [&](const EventEntry* entry) { return getNextTime(entry); };
    getColumnValue[4] = [&](const EventEntry* entry) { return getTrigger(entry); };
    getColumnValue[5] = [&](const EventEntry* entry) { return getPriority(entry); };
    getColumnValue[6] = [&](const EventEntry* entry) { return getIsTracking(entry); };
}

QVariant CEventDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
    {
        return {};
    }

    if (role == Qt::DisplayRole)
    {
        return m_header.at(section);
    }

    if (role == Qt::DecorationRole)
    {
        return QIcon(m_icons.at(section));
    }

    return {};
}

int CEventDataModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        return m_entries.size();
    }
    return 0;
}

int CEventDataModel::columnCount(const QModelIndex &parent) const
{
    return m_header.count();
}

Qt::ItemFlags CEventDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags =  Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (m_user_id == m_ownerId)
    {
        flags |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
    }

    return flags;
}

QStringList CEventDataModel::mimeTypes() const
{
    QStringList types = QAbstractTableModel::mimeTypes();
    types << "application/x-event-internal";
    return types;
}

QMimeData *CEventDataModel::mimeData(const QModelIndexList &indexes) const
{
    auto *mimeData = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QList<uint32_t> rows;
    for (const QModelIndex &index : indexes)
    {
        if (index.isValid() && !rows.contains(index.row()))
        {
            auto entry = static_cast<EventEntry*>(index.internalPointer());
            if (entry)
                rows.append(entry->id);
        }
    }

    stream << rows.size();
    for (auto &id : rows)
    {
        stream << id;
    }

    mimeData->setData("application/x-event-internal", data);
    return mimeData;
}

QModelIndex CEventDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return {};
    }

    const EventEntry* entry = &m_entries.at(row);
    return createIndex(row, column, entry);
}

QVariant CEventDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (role != Qt::UserRole && role != Qt::DisplayRole && role != Qt::DecorationRole)
    {
        return {};
    }

    const EventEntry &entry = m_entries.at(index.row());

    if (role == Qt::UserRole)
    {
        return entry.id;
    }

    if (role == Qt::DisplayRole)
    {
        return getColumnValue[index.column()](&entry);
    }
    return getIcon(index.row(), index.column());
}

bool CEventDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, Qt::EditRole) != value)
    {
        auto &entry = m_entries.at(index.row());

        //updateEntry(value, entry, index.column());

        emit entryChanged(entry);
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

bool CEventDataModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!m_entry)
        return false;

    beginInsertRows(parent, row, row + count - 1);

    m_entries.push_back(*m_entry);
    m_entry = nullptr;

    endInsertRows();
    return true;
}

bool CEventDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    auto removeItem = [&](const EventEntry &item, std::vector<EventEntry> *array)->void
    {
        auto it = std::find_if(array->begin(), array->end(),
                               [item](const EventEntry & entry){return item.id == entry.id;});
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

void CEventDataModel::resetModel(std::vector<EventEntry> &entries, uint32_t ownerId)
{
    beginResetModel();

    m_ownerId = ownerId;
    m_entries = entries;

    endResetModel();
}

void CEventDataModel::addEntry(const EventEntry &entry)
{
    m_entry = const_cast<EventEntry*>(&entry);
    insertRows(static_cast<int>(m_entries.size()), 1);
}

void CEventDataModel::removeEntries(std::vector<EventEntry> &entries)
{
    auto findIndex = [&](const EventEntry &item, std::vector<EventEntry> *array) -> uint32_t
    {
        auto it = std::find_if(array->begin(), array->end(),
                               [&](const EventEntry &e) { return e.id == item.id; });
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

EventEntry *CEventDataModel::getEntry(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return nullptr;
    }

    if (index.row() >= m_entries.size())
    {
        return nullptr;
    }

    return &m_entries.at(index.row());
}

EventEntry *CEventDataModel::getEntry(uint16_t row)
{
    if (row >= m_entries.size())
        return nullptr;
    return &m_entries.at(row);
}

void CEventDataModel::setUserId(const uint32_t &idUser)
{
    m_user_id = idUser;
}

bool CEventDataModel::moveRows(int targetRow, const QList<QPersistentModelIndex> &rowsToMove)
{
    if (rowsToMove.isEmpty() || targetRow < 0)
    {
        return false;
    }

    QList<int> rowsToMoveIndices;
    for (const QPersistentModelIndex &idx : rowsToMove)
    {
        if (idx.isValid())
        {
            rowsToMoveIndices.append(idx.row());
        }
    }

    std::sort(rowsToMoveIndices.begin(), rowsToMoveIndices.end());

    std::vector<EventEntry> movedEntries;

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
    {
        targetRow = 0;
    }
    if (targetRow > static_cast<int>(m_entries.size()))
    {
        targetRow = static_cast<int>(m_entries.size());
    }

    m_entries.insert(
            m_entries.begin() + targetRow,
            std::make_move_iterator(movedEntries.begin()),
            std::make_move_iterator(movedEntries.end())
    );

    return true;
}

uint32_t CEventDataModel::getOwnerId() const
{
    return m_ownerId;
}

uint32_t CEventDataModel::getUserId() const
{
    return m_user_id;
}

QVariant CEventDataModel::getDescription(const EventEntry *entry)
{
    return entry->description.c_str();
}

QVariant CEventDataModel::getPeriodUnit(const EventEntry *entry)
{
    return periodText[entry->period].c_str();
}

QVariant CEventDataModel::getUnitCount(const EventEntry *entry)
{
    return entry->period_count;
}

QVariant CEventDataModel::getNextTime(const EventEntry *entry)
{
    time_t now = time(nullptr);
    now = DateTimeCalculator::calculateNext(*entry, now);
    now = DateTimeCalculator::applyTrigger(now, entry->trigger);
    auto dt = QDateTime::fromSecsSinceEpoch(now);

    switch (entry->type)
    {
        case ET_DATE:
            return dt.date().toString();
        case ET_TIME:
            return dt.time().toString();
        case ET_DATE_TIME:
            return dt.toString();
        default:
            return {};
    }
}

QVariant CEventDataModel::getTrigger(const EventEntry *entry)
{
    return triggerName[entry->trigger].c_str();
}

QVariant CEventDataModel::getPriority(const EventEntry *entry)
{
    return priorityName[entry->priority].c_str();
}

QVariant CEventDataModel::getIsTracking(const EventEntry *entry)
{
    return entry->isEnabled ? "Отслеживается" : "Не отслеживается";
}

QIcon CEventDataModel::getIcon(int row, int col) const
{
    if (row >= m_entries.size())
    {
        return {};
    }

    auto entry = m_entries.at(row);
    if (col == 5)
    {
        return priorityIcon[entry.priority];
    }
    return {};
}

void CEventDataModel::updateEntry(const QVariant &value, EventEntry &entry, int column)
{
    switch (column)
    {
        case 0:
            entry.description = value.toString().toStdString();
            break;
        case 1:
            entry.period = (EventPeriod)value.toInt();
            break;
        case 2:
            entry.period_count = value.toInt();
            break;
        case 4:
            entry.trigger = (EventTrigger)value.toInt();
            break;
        case 5:
            entry.priority = (EventPriority)value.toInt();
            break;
        case 6:
            entry.isEnabled = value.toBool();
            break;
        default:
            return;
    }
}
