//
// Created by artem on 05.05.26.
//

#include "CSimpleEditDelegate.h"
#include <QLineEdit>
#include <QMessageBox>

CSimpleEditDelegate::CSimpleEditDelegate(QWidget *parent)
{}

CSimpleEditDelegate::~CSimpleEditDelegate()
= default;

QWidget *
CSimpleEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLineEdit* editor = new QLineEdit(parent);
    return editor;
}

void CSimpleEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::DisplayRole).toString();
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit)
        lineEdit->setText(value);
}

void CSimpleEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit->text().isEmpty())
    {
        QMessageBox::information(dynamic_cast<QWidget*>(this->parent()), "Неприемлемое исправление",
                                 "Пустое описание или значение неприемлемы");
        return;
    }

    model->setData(index, lineEdit->text(), Qt::EditRole);
}

void CSimpleEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
