//
// Created by artem on 04.05.26.
//

#ifndef REMINDER_BUTTONDELEGATE_H
#define REMINDER_BUTTONDELEGATE_H

#include <QApplication>
#include <QTableView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include "backend/sources/include/base-types.h"


class ButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ButtonDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

public: signals:
    void fileClicked(int row);
    void folderClicked(int row);
    void urlPressed(int row);

private:
    QPixmap fileIcon;
    QPixmap folderIcon;
    QPixmap urlIcon;

    QRect m_fileRect;
    QRect m_folderRect;
    QRect m_eyeRect;
    QRect m_urlRect;

    void getRects(QRect &eyeRect, QRect &fileRect, QRect &folderRect, QRect &execRect, TextEntry *text_entry,
                  const QStyleOptionViewItem &option) const;
};


#endif //REMINDER_BUTTONDELEGATE_H
