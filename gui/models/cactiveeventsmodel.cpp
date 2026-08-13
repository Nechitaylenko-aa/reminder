#include "cactiveeventsmodel.h"
#include <QIcon>
#include <QDateTime>
#include "../../backend/sources/include/DateTimeCalculator.h"

CActiveEventsModel::CActiveEventsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_header << "Описание"
             << "Периодичность"
             << "Следующий раз"
             << "Отслеживание";
    m_icons.resize(m_header.size());
}

QVariant CActiveEventsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DecorationRole)
        {
            return QIcon(m_icons.at(section));
        }
        if (role == Qt::DisplayRole)
        {
            return m_header.at(section);
        }
        return {};
    }

    if (role == Qt::DisplayRole)
    {
        return QString::number(section + 1);
    }
    return {};
}


int CActiveEventsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_items.size());
}

int CActiveEventsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_header.size());
}

QVariant CActiveEventsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto &item = m_items.at(index.row());
    std::string txt;
    time_t  next, now = time(nullptr);
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case 0:
                return item.description.c_str();
            case 1:
                txt = std::to_string(item.period_count) + " " + periodText[item.period];
                return txt.c_str();
            case 2:
                next = DateTimeCalculator::calculateNext(item, now);
                next = DateTimeCalculator::applyTrigger(next, item.trigger);
                switch (item.period)
                {
                    case EventPeriod::EP_NONE:
                        return "-";
                    default:
                        switch (item.type)
                        {
                            case EventType::ET_DATE:
                                return QDateTime::fromSecsSinceEpoch(next).date().toString();
                            case EventType::ET_DATE_TIME:
                                return QDateTime::fromSecsSinceEpoch(next).toString();
                            case EventType::ET_TIME:
                                return QDateTime::fromSecsSinceEpoch(next).time().toString();
                            default:
                                return {};
                        }
                }
            case 3:
                if (item.isEnabled)
                    return "Следим внимательно";
                return "Не отслеживается";
            default:
                return {};
        }
    }
    if (role == Qt::DecorationRole)
    {
        if (index.column() == 3)
        {
            if (item.isEnabled)
                return QIcon(":/24/images/24/eye.png");
            return QIcon(":/48/images/48/Remove_48x48.png");
        }
    }
    if (role == Qt::UserRole)
    {
        return item.id;
    }
    return {};
}

bool CActiveEventsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags CActiveEventsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool CActiveEventsModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return true;
}

bool CActiveEventsModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}

bool CActiveEventsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
    return true;
}

bool CActiveEventsModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}

void CActiveEventsModel::setEventsData(const std::vector<EventEntry> &items)
{
    beginResetModel();

    m_items.clear();
    m_items.insert(m_items.end(), items.begin(), items.end());

    endResetModel();
}
