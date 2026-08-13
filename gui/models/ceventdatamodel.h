#ifndef CEVENTDATAMODEL_H
#define CEVENTDATAMODEL_H

#include <QAbstractTableModel>
#include <QIcon>
#include "backend/sources/include/base-types.h"



class CEventDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CEventDataModel(QObject *parent = nullptr);

    // Header:
    [[nodiscard]] QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] QStringList mimeTypes() const override;
    [[nodiscard]] QMimeData* mimeData(const QModelIndexList &indexes) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool  setData(const QModelIndex &index, const QVariant &value, int role) override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool moveRows(int targetRow, const QList<QPersistentModelIndex> &rowsToMove);

    void resetModel(std::vector<EventEntry> &entries, uint32_t ownerId);
    void addEntry(const EventEntry &entry);
    void removeEntries(std::vector<EventEntry> &entries);
    EventEntry* getEntry(const QModelIndex &index);
    EventEntry* getEntry(uint16_t row);

    void  setUserId(const uint32_t & idUser);
    [[nodiscard]] uint32_t    getOwnerId() const;
    [[nodiscard]] uint32_t    getUserId() const;

signals:
    void entryChanged(EventEntry &entry);

private:

    QIcon priorityIcon[3]
        {
            QIcon(":/16/images/16/flag_blue.png"),
            QIcon(":/16/images/16/flag_orange.png"),
            QIcon(":/16/images/16/flag_red.png")
        };

    uint32_t  m_ownerId;
    uint32_t  m_user_id;
    std::vector<EventEntry> m_entries;
    QStringList m_header;
    QStringList m_icons;
    EventEntry  * m_entry{nullptr};

    std::function<QVariant(const EventEntry *)>  getColumnValue[7];

    static QVariant  getDescription(const EventEntry *entry);
    static QVariant  getPeriodUnit(const EventEntry *entry);
    static QVariant  getUnitCount(const EventEntry *entry);
    static QVariant  getNextTime(const EventEntry *entry);
    static QVariant  getTrigger(const EventEntry *entry);
    static QVariant  getPriority(const EventEntry *entry);
    static QVariant  getIsTracking(const EventEntry *entry);
    [[nodiscard]] QIcon     getIcon(int row, int col) const;

    static void updateEntry(const QVariant &value, EventEntry &entry, int column);

};

#endif // CEVENTDATAMODEL_H
