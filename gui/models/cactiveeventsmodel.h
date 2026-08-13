#ifndef CACTIVEEVENTSMODEL_H
#define CACTIVEEVENTSMODEL_H

#include <QAbstractTableModel>
#include "backend/sources/include/base-types.h"

class CActiveEventsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CActiveEventsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    void setEventsData(const std::vector<EventEntry> & items);

private:
    std::vector<EventEntry> m_items;
    QStringList  m_header;
    QStringList  m_icons;
};

#endif // CACTIVEEVENTSMODEL_H
