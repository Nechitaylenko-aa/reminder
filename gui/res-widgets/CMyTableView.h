//
// Created by artem on 02.05.26.
//

#ifndef REMINDER_CMYTABLEVIEW_H
#define REMINDER_CMYTABLEVIEW_H

#include <QTableView>
#include "backend/sources/include/base-types.h"

class CTextDataModel;
class CEventDataModel;

class CMyTableView : public QTableView
{
    Q_OBJECT
public:
    explicit CMyTableView(QWidget *parent = nullptr);
    ~CMyTableView() override;
    void setType(ResourceType type);
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
private:
    ResourceType m_type;
    CTextDataModel * m_textModel{nullptr};
    CEventDataModel * m_eventModel{nullptr};
};


#endif //REMINDER_CMYTABLEVIEW_H
