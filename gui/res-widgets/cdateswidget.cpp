#include "cdateswidget.h"
#include "ui_cdateswidget.h"
#include <QPushButton>

CEventWidget::CEventWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CEventWidget)
{
    ui->setupUi(this);

    auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    connect(okButton, &QPushButton::clicked, this, &CEventWidget::slotSaveClose);

    //connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CEventWidget::slotSaveClose);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &CEventWidget::slotClose);
}

CEventWidget::~CEventWidget()
{
    delete ui;
}

void CEventWidget::closeEvent(QCloseEvent *event)
{
    this->reject();
}

void CEventWidget::setEvent(EventEntry &event)
{
    m_event = &event;
    eventToGui();
}

void CEventWidget::slotSaveClose()
{
    guiToEvent();
    this->accept();
}

void CEventWidget::slotClose()
{
    this->reject();
}

void CEventWidget::guiToEvent()
{
    if (!m_event)
    {
        return;
    }

    if (!ui->leEventDescript->text().isEmpty())
    {
        m_event->description = ui->leEventDescript->text().toStdString();
    }
    m_event->period = (EventPeriod)ui->cbPeriodType->currentIndex();
    m_event->period_count = ui->sbPeriodAmount->value();
    m_event->trigger = (EventTrigger)ui->cbTrigger->currentIndex();
    m_event->event = ui->dateEdit->dateTime().toSecsSinceEpoch();
    m_event->type = (EventType)ui->cbEventDimension->currentIndex();
}

void CEventWidget::eventToGui()
{
    // fill combo boxes
    ui->cbEventDimension->clear();
    ui->cbPeriodType->clear();
    ui->cbTrigger->clear();

    //
    for (int i = 0; i < EvT_COUNT; i++)
    {
        ui->cbEventDimension->addItem(eventTypeNames[i].c_str());
    }
    ui->cbEventDimension->setCurrentIndex(m_event->type);

    for (int i = 0; i < EP_COUNT; i++)
    {
        ui->cbPeriodType->addItem(periodText[i].c_str());
    }
    ui->cbPeriodType->setCurrentIndex(m_event->period);

    for (int i = 0; i < ET_COUNT; i++)
    {
        ui->cbTrigger->addItem(triggerName[i].c_str());
    }
    ui->cbTrigger->setCurrentIndex(m_event->trigger);

    ui->sbPeriodAmount->setValue(m_event->period_count);
    ui->dateEdit->setDateTime(QDateTime::fromSecsSinceEpoch(m_event->event));
    ui->leEventDescript->setText(m_event->description.c_str());
}
