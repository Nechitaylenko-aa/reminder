#include "ceventdelegate.h"
#include "backend/sources/include/base-types.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QDateTimeEdit>

EventEditorDelegate::EventEditorDelegate(QObject* parent)
        : QStyledItemDelegate(parent)
{}

QWidget* EventEditorDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const
{
    switch (index.column()) {
        case 0: // Описание
            return new QLineEdit(parent);
        case 1: // Период
            return new QComboBox(parent);
        case 2:
            return new QSpinBox(parent);
        case 3: // Следующий раз
            return nullptr;
        case 4: // Триггер
        case 5: // Приоритет
        case 6: // Отслеживание
            return new QComboBox(parent);
        default:
            return nullptr;//QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void EventEditorDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (!editor || !index.isValid()) return;
    //EventEntry * entry = static_cast<EventEntry*>(index.internalPointer());

    switch (index.column()) {
        case 0:
            setEditorDataForLineEdit(qobject_cast<QLineEdit*>(editor), index);
            break;
        case 1:
            setEditorDataForComboBox(qobject_cast<QComboBox*>(editor), index);
            break;
        case 2:
            setEditorDataForSpinBox(qobject_cast<QSpinBox*>(editor), index);
            break;
        case 3:
            return;
        case 4:
        case 5:
        case 6:
            setEditorDataForComboBox(qobject_cast<QComboBox*>(editor), index);
            break;
    }
}

void EventEditorDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                       const QModelIndex& index) const
{
    if (!editor || !index.isValid()) return;

    switch (index.column()) {
        case 0:
            setModelDataForLineEdit(qobject_cast<QLineEdit*>(editor), model, index);
            break;
        case 2:
            setModelDataForSpinBox(qobject_cast<QSpinBox*>(editor), model, index);
            break;
        case 1: case 4: case 5: case 6:
            setModelDataForComboBox(qobject_cast<QComboBox*>(editor), model, index);
            break;
    }
}

void EventEditorDelegate::setEditorDataForComboBox(QComboBox* editor, const QModelIndex& index) const
{
    if (!editor) return;

    editor->clear();

    switch (index.column())
    {
        case 1:
            editor->addItems({"разовое",
                              "минута",
                              "час",
                              "сутки",
                              "неделя",
                              "месяц",
                              "год"});
            break;
        case 4: // Триггер
            editor->addItems({"строго",
                              "перед",
                              "после"});
            break;
        case 5: // Приоритет
            editor->addItems({"обычное",
                              "важное",
                              "критическое"});
            break;
        case 6: // Отслеживание
            editor->addItems({"Отслеживается", "Не отслеживается"});
            break;
    }

    // Устанавливаем текущее значение
    QString currentText = index.data(Qt::DisplayRole).toString();
    int idx = editor->findText(currentText);
    if (idx >= 0) {
        editor->setCurrentIndex(idx);
    }
}

void EventEditorDelegate::setModelDataForComboBox(QComboBox* editor, QAbstractItemModel* model,
                                                  const QModelIndex& index) const
{
    if (!editor) return;

    //model->setData(index, editor->currentText(), Qt::EditRole);
    auto event = static_cast<EventEntry*>(index.internalPointer());

    switch (index.column())
    {
        case 1:
            event->period = (EventPeriod)editor->currentIndex();
            break;
        case 4:
            event->trigger = (EventTrigger)editor->currentIndex();
            break;
        case 5:
            event->priority = (EventPriority)editor->currentIndex();
            break;
        case 6:
            event->isEnabled = !(bool)editor->currentIndex();
            break;
    }
    model->setData(index, editor->currentText(), Qt::EditRole);
}

void EventEditorDelegate::setEditorDataForLineEdit(QLineEdit *editor, const QModelIndex &index) const
{
    if (!editor) return;
    auto event = static_cast<EventEntry*>(index.internalPointer());
    editor->setText(event->description.c_str());
}

void EventEditorDelegate::setEditorDataForSpinBox(QSpinBox *editor, const QModelIndex &index) const
{
    if (!editor) return;
    auto event = static_cast<EventEntry*>(index.internalPointer());
    if (index.column() == 2)
    {
        editor->setMinimum(1);
        editor->setValue(event->period_count);
    }

}

void EventEditorDelegate::setEditorDataForDateTimeEdit(QDateTimeEdit *editor, const QModelIndex &index) const
{
    if (!editor) return;

}

void EventEditorDelegate::setModelDataForLineEdit(QLineEdit *editor, QAbstractItemModel *model,
                                                  const QModelIndex &index) const
{
    if (!editor) return;
    auto event = static_cast<EventEntry*>(index.internalPointer());
    if (!editor->text().isEmpty())
    {
        event->description = editor->text().toStdString();
    }

    model->setData(index, editor->text());
}

void
EventEditorDelegate::setModelDataForSpinBox(QSpinBox *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!editor) return;
    auto event = static_cast<EventEntry*>(index.internalPointer());
    event->period_count = editor->value();
    model->setData(index, editor->value());
}

void EventEditorDelegate::setModelDataForDateTimeEdit(QDateTimeEdit *editor, QAbstractItemModel *model,
                                                      const QModelIndex &index) const
{
    if (!editor) return;
}

void EventEditorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}
