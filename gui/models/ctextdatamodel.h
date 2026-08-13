#ifndef CTEXTDATAMODEL_H
#define CTEXTDATAMODEL_H

#include <QAbstractTableModel>
#include <QIcon>
#include "backend/sources/include/base-types.h"

class CTextDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CTextDataModel(QObject *parent = nullptr);

    // Header:
    [[nodiscard]] QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] QStringList mimeTypes() const override;
    [[nodiscard]] QMimeData* mimeData(const QModelIndexList &indexes) const override;    

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool moveRows(int targetRow, const QList<QPersistentModelIndex> &rowsToMove);

    void resetModel(std::vector<TextEntry> &entries, uint32_t ownerId, uint32_t userId);
    void addEntry(const TextEntry &entry);
    void removeEntries(std::vector<TextEntry> &entries);
    TextEntry* getEntry(const QModelIndex &index);
    TextEntry* getEntry(uint16_t row);

    void  setUserId(const uint32_t & idUser);
    uint32_t    getOwnerId() const { return m_ownerId; }
    uint32_t    getUserId() const { return m_user_id; }

signals:
    void  entryChanged(const TextEntry &entry);

private: // members
    std::vector<TextEntry> m_entries;
    QStringList      m_header;
    QStringList      m_icons;
    TextEntry       * m_entry{nullptr};
    uint32_t         m_user_id{0};
    uint32_t         m_ownerId{UINT32_MAX};

private: // methods
    static QIcon  getIcon(const TextEntry &entry) ;

};

#endif // CTEXTDATAMODEL_H
