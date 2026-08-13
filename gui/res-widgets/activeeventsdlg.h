#ifndef ACTIVEEVENTSDLG_H
#define ACTIVEEVENTSDLG_H

#include <QDialog>
#include "../models/cactiveeventsmodel.h"
#include "backend/sources/include/base-types.h"

namespace Ui {
class ActiveEventsDlg;
}

class CMyTableView;

class ActiveEventsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ActiveEventsDlg(QWidget *parent = nullptr);
    ~ActiveEventsDlg() override;

    void  setActiveEvents(const std::vector<EventEntry> &items);


private:
    Ui::ActiveEventsDlg *ui;
    CActiveEventsModel  * m_model;
};

#endif // ACTIVEEVENTSDLG_H
