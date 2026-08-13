//
// Created by artem on 05.05.26.
//

#ifndef REMINDER_CSIMPLEEDITDELEGATE_H
#define REMINDER_CSIMPLEEDITDELEGATE_H

#include <QStyledItemDelegate>

class CSimpleEditDelegate : public QStyledItemDelegate
{
public:
    explicit CSimpleEditDelegate(QWidget* parent = nullptr);
    ~CSimpleEditDelegate() override;

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;
protected:


private:


private:

};


#endif //REMINDER_CSIMPLEEDITDELEGATE_H
