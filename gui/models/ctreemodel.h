#ifndef CTREEMODEL_H
#define CTREEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>


///-----------------------------------------------------------------------------------------------------------------<br>
/// TreeItem. Contains system Node <br>
///-----------------------------------------------------------------------------------------------------------------<br>

#include "../../backend/sources/include/base-types.h"


class TreeItem
{
public:
    explicit TreeItem(const SNode & node, TreeItem *parentItem = nullptr);
    explicit TreeItem(QString  rootName, TreeItem * parentItem = nullptr);
    ~TreeItem();

    SNode * get_node();

    void appendChild(TreeItem *child);
    void removeChild(int idx);
    void clearChildren();
    QIcon getIcon();
    void setIcon(const QString &string);

    TreeItem *child(int row);
    [[nodiscard]] int childCount() const;
    [[nodiscard]] int columnCount() const;
    [[nodiscard]] QVariant data(int column) const;
    [[nodiscard]] int row() const;
    TreeItem *parentItem();

private:
    QVector<TreeItem*> m_childItems;
    TreeItem *m_parentItem{nullptr};
    SNode * m_node{nullptr};  // Владеющий
    QString m_rootName;
    QIcon   m_icon;
};


///-----------------------------------------------------------------------------------------------------------------<br>
/// CTreeModel   <br>
///-----------------------------------------------------------------------------------------------------------------<br>


class CTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CTreeModel(QObject *parent = nullptr);
    ~CTreeModel() override;
    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // bool setHeaderData(int section,
    //                   Qt::Orientation orientation,
    //                   const QVariant &value,
    //                   int role = Qt::EditRole) override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /*
    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    */

    void  resetModel(std::vector<SNode> &nodes);
    void  addNode(const SNode & node);
    void  removeNode(const SNode &node);
    void  moveNode(const SNode& node);
    void  setUser(const uint32_t & userId);
    void setRenameCallback(std::function<void(SNode*)> handler);

private: // members
    std::vector<SNode>     m_nodes;
    TreeItem            * m_root;
    uint32_t              user_id{0};
    std::function<void(SNode *)>  m_callbackRename;

private: // methods
};

#endif // CTREEMODEL_H
