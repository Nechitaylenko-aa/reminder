#ifndef CDATESWIDGET_H
#define CDATESWIDGET_H

#include <QDialog>
#include "backend/sources/include/base-types.h"

namespace Ui {
class CEventWidget;
}

class CEventWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CEventWidget(QWidget *parent = nullptr);
    ~CEventWidget() override;

    void closeEvent(QCloseEvent* event) override;

    void  setEvent(EventEntry & event);
private slots:
    void slotSaveClose();
    void slotClose();

private:
    Ui::CEventWidget *ui;

    EventEntry *m_event{nullptr};
    void guiToEvent();
    void eventToGui();
};

#endif // CDATESWIDGET_H
