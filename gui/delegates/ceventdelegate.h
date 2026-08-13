#ifndef CEVENTDELEGATE_H
#define CEVENTDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QSpinBox>
#include <QComboBox>
#include <QDateTimeEdit>

// EventEditorDelegate.h
class EventEditorDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
    explicit EventEditorDelegate(QObject* parent = nullptr);

    // Создание редактора в зависимости от колонки
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    // Установка данных из модели в редактор
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    // Сохранение данных из редактора в модель
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;

    // Обновление размеров редактора
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

private:
    // Вспомогательные методы для каждого типа редактора
    void setEditorDataForLineEdit(QLineEdit* editor, const QModelIndex& index) const;
    void setEditorDataForSpinBox(QSpinBox* editor, const QModelIndex& index) const;
    void setEditorDataForComboBox(QComboBox* editor, const QModelIndex& index) const;
    void setEditorDataForDateTimeEdit(QDateTimeEdit* editor, const QModelIndex& index) const;

    void setModelDataForLineEdit(QLineEdit* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    void setModelDataForSpinBox(QSpinBox* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    void setModelDataForComboBox(QComboBox* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    void setModelDataForDateTimeEdit(QDateTimeEdit* editor, QAbstractItemModel* model, const QModelIndex& index) const;
};

#endif // CEVENTDELEGATE_H
