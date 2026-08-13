#include "activeeventsdlg.h"
#include "ui_activeeventsdlg.h"
#include "CMyTableView.h"

ActiveEventsDlg::ActiveEventsDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActiveEventsDlg)
{
    ui->setupUi(this);
    auto view = ui->tableView;
    m_model = new CActiveEventsModel(view);
    view->setModel(m_model);

    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    auto header = view->horizontalHeader();
    for (int i = 0; i < header->count(); i++)
    {
        header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }

    header->setStretchLastSection(true);
}

ActiveEventsDlg::~ActiveEventsDlg()
{
    delete ui;
}

void ActiveEventsDlg::setActiveEvents(const std::vector<EventEntry> &items)
{
    m_model->setEventsData(items);
}
