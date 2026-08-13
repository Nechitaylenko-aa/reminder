//
// Created by artem on 04.05.26.
//

#include <QToolTip>
#include "ButtonDelegate.h"
#include "gui/models/ctextdatamodel.h"
#include <QLineEdit>

ButtonDelegate::ButtonDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    fileIcon = QPixmap(":/24/images/24/open-folder.png");
    folderIcon = QPixmap(":/48/images/48/104.png");
    urlIcon = QPixmap(":/48/images/48/198.png");
}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() != 1)
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    auto * entry = static_cast<TextEntry*>(index.internalPointer());
    if (!entry)
    {
        return;
    }

    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_HasFocus;

    // рисуем стандартную ячейку с текстом
    QStyledItemDelegate::paint(painter, opt, index);

    // поверх рисуем кнопки как наложение
    painter->save();

    QRect eyeRect, editRect, deleteRect, execRect;

    getRects(eyeRect, editRect, deleteRect, execRect, entry, option);

    auto * mutableThis = const_cast<ButtonDelegate*>(this);

    mutableThis->m_fileRect = editRect;
    mutableThis->m_folderRect = deleteRect;
    mutableThis->m_eyeRect = eyeRect;
    mutableThis->m_urlRect = execRect;

    // painter->setPen(Qt::NoPen);

    if (entry->type == TextEntryType::SECRET)
    {
        painter->setBrush(QColor(240, 240, 240, 200));
        painter->drawRoundedRect(eyeRect, 3, 3);
        painter->setPen(Qt::black);
        painter->drawText(eyeRect, Qt::AlignCenter, "👁");
    }
    else if (entry->type == TextEntryType::FILE_SYSTEM)
    {
        painter->drawPixmap(editRect, fileIcon);
        painter->drawPixmap(deleteRect, folderIcon);
    }
    else if(entry->type == TextEntryType::URI_LINK)
    {
        painter->drawPixmap(execRect, urlIcon);
    }


    painter->restore();
}

void ButtonDelegate::getRects(QRect &eyeRect, QRect &fileRect, QRect &folderRect, QRect &execRect,
                              TextEntry *text_entry, const QStyleOptionViewItem &option) const
{
    int buttonWidth = 20;
    int buttonHeight = 20;
    int spacing = 5;
    int rightMargin = 8;

    int startX = option.rect.right() - (buttonWidth + spacing + rightMargin);
    int y = option.rect.top() + (option.rect.height() - buttonHeight) / 2;

    switch (text_entry->type)
    {
        case TextEntryType::PLAIN_TEXT:
            return;
        case TextEntryType::SECRET:
            eyeRect = QRect(startX, y, buttonWidth, buttonHeight);
            break;
        case TextEntryType::URI_LINK:
            execRect = QRect(startX, y, buttonWidth, buttonHeight);
            break;
        case TextEntryType::FILE_SYSTEM:
            startX = option.rect.right() - (buttonWidth * 2 + spacing + rightMargin);
            fileRect = QRect(startX, y, buttonWidth, buttonHeight);
            folderRect = QRect(startX +  + (buttonWidth + spacing), y, buttonWidth, buttonHeight);
    }
}


bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    if (index.column() != 1)
        return false;

    if (event->type() == QEvent::MouseButtonRelease)
    {
        TextEntry *entry = static_cast<TextEntry*>(index.internalPointer());

        auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (m_fileRect.contains(mouseEvent->pos()))
        {
            emit fileClicked(index.row());
            return true;
        }

        if (m_folderRect.contains(mouseEvent->pos()))
        {
            emit folderClicked(index.row());
            return true;
        }

        if (m_eyeRect.contains(mouseEvent->pos()))
        {
            entry->showSecret = !entry->showSecret;
            return true;
        }

        if (m_urlRect.contains(mouseEvent->pos()))
        {
            emit urlPressed(index.row());
            return true;
        }
    }

    return false;
}

bool ButtonDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option,
                               const QModelIndex &index)
{
    if (event->type() != QEvent::ToolTip || index.column() != 1)
    {
        return QAbstractItemDelegate::helpEvent(event, view, option, index);
    }

    auto * entry = static_cast<TextEntry*>(index.internalPointer());
    if (!entry)
    {
        return QAbstractItemDelegate::helpEvent(event, view, option, index);
    }

    QPoint pos = event->pos();

    switch (entry->type)
    {
        case TextEntryType::SECRET:
            if (m_eyeRect.contains(pos))
            {
                QToolTip::showText(event->globalPos(),
                                   "👁️ Показать/скрыть подробности");
                return true;
            }
        case TextEntryType::URI_LINK:
             if (m_urlRect.contains(pos))
             {
                 QToolTip::showText(event->globalPos(),
                                    "🔗️ Перейти по ссылке");
                 return true;
             }
        case TextEntryType::FILE_SYSTEM:
            if (m_fileRect.contains(pos))
            {
                QToolTip::showText(event->globalPos(),
                                    "🗁 Открыть диалог выбора файла");
                return true;
            }
            if (m_folderRect.contains(pos))
            {
                QToolTip::showText(event->globalPos(),
                                    "📂 Открыть диалог выбора папки");
                return true;
            }
            break;
        default:
            break;
    }
    return QAbstractItemDelegate::helpEvent(event, view, option, index);
}

QWidget *
ButtonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLineEdit* editor = new QLineEdit(parent);
    return editor;
}

void ButtonDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::DisplayRole).toString();
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit)
    {
        lineEdit->setText(value);
    }
}

void ButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit)
    {
        if (lineEdit->text().isEmpty())
        {
            return;
        }
        model->setData(index, lineEdit->text(), Qt::EditRole);
    }

}

void ButtonDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
