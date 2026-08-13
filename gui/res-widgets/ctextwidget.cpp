#include "ctextwidget.h"
#include "ui_ctextwidget.h"
#include "gui/delegates/ButtonDelegate.h"
#include "gui/delegates/CSimpleEditDelegate.h"
#include <QPushButton>
#include <QMenu>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>

CTextWidget::CTextWidget(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::CTextWidget)
{
    ui->setupUi(this);

    m_model = new CTextDataModel();
    connect(m_model, &CTextDataModel::entryChanged, this, &CTextWidget::slotEntryChanged);

    // setup data model
    CMyTableView * view = ui->textView;
    view->setModel(m_model);
    view->setType(ResourceType::TEXT);

    view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(view, &CMyTableView::customContextMenuRequested, this, &CTextWidget::slotContextTableMenu);

    // view style of the buttoned column via delegate
    auto buttonedDelegate = new ButtonDelegate(view);
    auto simpleEditDelegate = new CSimpleEditDelegate(view);
    view->setItemDelegateForColumn(0, simpleEditDelegate);
    view->setItemDelegateForColumn(1, buttonedDelegate);

    // нажатия на быстрых кнопках в записях
    connect(buttonedDelegate, &ButtonDelegate::fileClicked, this, &CTextWidget::slotFileDialog);
    connect(buttonedDelegate, &ButtonDelegate::folderClicked, this, &CTextWidget::slotFolderDialog);
    connect(buttonedDelegate, &ButtonDelegate::urlPressed, this, &CTextWidget::slotGotoUrl);


    // selection behaviour
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // columns alignment
    auto header = view->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::Stretch);

    // connect pushButtons with action code
    auto btAdd = ui->tbAdd;
    connect(btAdd, &QPushButton::released, this, &CTextWidget::slotAddEntry);

    auto btRemove = ui->tbRemove;
    connect(btRemove, &QPushButton::released, this, &CTextWidget::slotRemoveEntry);
}

CTextWidget::~CTextWidget()
{
    delete m_model;
    delete ui;
}

CMyTableView *CTextWidget::table_view()
{
    return ui->textView;
}

CTextDataModel *CTextWidget::model()
{
    return m_model;
}

void CTextWidget::setCallbackAddEntry(std::function<bool(TextEntry &)> handler)
{
    m_cbAddEntry = std::move(handler);
}

void CTextWidget::setCallbackRemoveEntry(std::function<bool(std::vector<TextEntry> &)> handler)
{
    m_cbRemoveEntry = std::move(handler);
}

void CTextWidget::setCallbackMoveEntries(std::function<bool(std::vector<TextEntry> &, uint32_t)> handler)
{
    m_cbMoveEntries = std::move(handler);
}

void CTextWidget::setCallbackUpdateEntry(std::function<bool(const TextEntry &)> handler)
{
    m_cbUpdateEntry = std::move(handler);
}

void CTextWidget::slotAddEntry()
{
    if (m_cbAddEntry)
    {
        TextEntry entry;
        entry.desc = "New res";
        entry.data = "Some data";

        if (m_cbAddEntry(entry))
        {
            m_model->addEntry(entry);
        }
    }
}

void CTextWidget::slotRemoveEntry()
{
    QItemSelectionModel* selectModel = ui->textView->selectionModel();
    QModelIndexList selectedRows = selectModel->selectedRows();
    if (selectedRows.empty())
    {
        return;
    }

    std::vector<TextEntry> array;
    array.reserve(selectedRows.size());

    for (const QModelIndex &index : selectedRows)
    {
        TextEntry *entry = m_model->getEntry(index);
        if (entry)
        {
            array.push_back(*entry);
        }
    }

    if (m_cbRemoveEntry)
    {
        if (!m_cbRemoveEntry(array))
        {
            qDebug() << "Ups... Can't remove items";
            return;
        }
        m_model->removeEntries(array);
    }

}

void CTextWidget::slotContextTableMenu(const QPoint &pos)
{
    QMenu menu;

    QModelIndex index = ui->textView->indexAt(pos);
    if (!index.isValid())
    {
        qDebug() << "CMyTableView::slotContextMenu index failed";
        return;
    }

    auto *entry = static_cast<TextEntry*>(index.internalPointer());
    if (!entry)
    {
        qDebug() << "CMyTableView::slotContextMenu entry failed";
        return;
    }

    bool no_access = !(m_loggedInUser == m_node.user_id && m_loggedInUser > 0);

    auto actSetSecret = new QAction(QIcon(":/24/images/24/eye.png"), "Секретные данные", &menu);
    connect(actSetSecret, &QAction::triggered, [&,entry](){this->slotSetSecretEntry(*entry);});
    if (entry->type == TextEntryType::SECRET || no_access)
        actSetSecret->setEnabled(false);

    auto actSetPath = new QAction(QIcon(":/16/images/16/add_node_16.png"), "Путь локальной файловой системы", &menu);
    connect(actSetPath, &QAction::triggered, [&,entry](){this->slotPathEntry(*entry);});
    if (entry->type == TextEntryType::FILE_SYSTEM || no_access)
        actSetPath->setEnabled(false);

    auto actSetUrl = new QAction(QIcon(":/48/images/48/198.png"), "Ссылка", &menu);
    connect(actSetUrl, &QAction::triggered, [&,entry](){this->slotUrlEntry(*entry);});
    if (entry->type == TextEntryType::URI_LINK || no_access)
        actSetUrl->setEnabled(false);

    auto actSetPlain = new QAction(QIcon(":/24/images/24/file_open.png"), "Обычный текст", &menu);
    connect(actSetPlain, &QAction::triggered, [&,entry](){this->slotSetPlainTextEntry(*entry);});
    if (entry->type == TextEntryType::PLAIN_TEXT || no_access)
        actSetPlain->setEnabled(false);

    menu.addSection("Сделать запись типа:");
    menu.addAction(actSetSecret);
    menu.addAction(actSetPath);
    menu.addAction(actSetUrl);
    menu.addAction(actSetPlain);

    menu.exec(mapToGlobal(pos));
}

void CTextWidget::slotSetSecretEntry(TextEntry &entry)
{
    if (m_cbEntrySecret)
        m_cbEntrySecret(entry);
}

void CTextWidget::slotPathEntry(TextEntry &entry)
{
    if (m_cbEntryPath)
        m_cbEntryPath(entry);
}

void CTextWidget::slotUrlEntry(TextEntry &entry)
{
    if (m_cbEntryUrl)
        m_cbEntryUrl(entry);
}

void CTextWidget::slotSetPlainTextEntry(TextEntry &entry)
{
    if (m_cbEntryPlain)
        m_cbEntryPlain(entry);
}

void CTextWidget::setCallbackEntrySecret(std::function<void(TextEntry &)> handler)
{
    m_cbEntrySecret = std::move(handler);
}

void CTextWidget::setCallbackEntryPath(std::function<void(TextEntry &)> handler)
{
    m_cbEntryPath = std::move(handler);
}

void CTextWidget::setCallbackEntryUrl(std::function<void(TextEntry &)> handler)
{
    m_cbEntryUrl = std::move(handler);
}

void CTextWidget::setCallbackEntryPlain(std::function<void(TextEntry &)> handler)
{
    m_cbEntryPlain = std::move(handler);
}

void CTextWidget::slotEntryChanged(const TextEntry &entry)
{
    if (entry.data.empty() || entry.desc.empty())
    {
        return;
    }
    if (m_cbUpdateEntry)
        m_cbUpdateEntry(entry);
}

void CTextWidget::slotFileDialog(int row)
{
    auto entry = m_model->getEntry(row);
    if (!entry)
    {
        return;
    }

    QString startPath = QDir::homePath();

    if (!entry->data.empty())
    {
        QFileInfo fileInfo(QString::fromStdString(entry->data));
        startPath = fileInfo.absolutePath();
    }

    QString file = QFileDialog::getOpenFileName(this, "Выбрать файл", startPath);
    if (file.isEmpty())
        return;

    bool no_access = !(m_loggedInUser == m_node.user_id && m_loggedInUser > 0);

    if (!no_access)
    {
        entry->data = file.toStdString();
        if (m_cbUpdateEntry)
        {
            m_cbUpdateEntry(*entry);
        }
    }
}

void CTextWidget::slotFolderDialog(int row)
{
    auto entry = m_model->getEntry(row);
    if (!entry)
    {
        return;
    }

    QString startFolder = QDir::homePath();
    if (!entry->data.empty())
    {
        startFolder = QString::fromStdString(entry->data);
    }

    bool no_access = !(m_loggedInUser == m_node.user_id && m_loggedInUser > 0);

    QString folder = QFileDialog::getExistingDirectory(this, "Выберите папку", startFolder);
    if (!folder.isEmpty())
    {
        if (!no_access)
        {
            entry->data = folder.toStdString();
            if (m_cbUpdateEntry)
            {
                m_cbUpdateEntry(*entry);
            }
        }
    }
}

void CTextWidget::slotGotoUrl(int row)
{
    auto entry = m_model->getEntry(row);
    if (!entry || entry->data.empty())
    {
        return;
    }
    QUrl url = QUrl(QString::fromStdString(entry->data));
    QDesktopServices::openUrl(url);
}

void CTextWidget::setCurrentNode(std::vector<TextEntry> & entries, const SNode &node, uint32_t loggedUserId)
{
    m_node = node;
    m_loggedInUser = loggedUserId;
    m_model->resetModel(entries, node.user_id, loggedUserId);
    updateButtons();
}

void CTextWidget::updateButtons()
{
    bool isEnabledAdd = false;
    bool isEnabledDel = false;
    bool isEnabledExec= false;

    if (m_node.user_id == m_loggedInUser && m_node.is_editable)
    {
        isEnabledAdd = true;
        isEnabledDel = true;
        isEnabledExec = true;
    }

    ui->tbAdd->setEnabled(isEnabledAdd);
    ui->tbRemove->setEnabled(isEnabledDel);
    ui->tbExec->setEnabled(isEnabledExec);
}
