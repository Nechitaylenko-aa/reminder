#include "mainwindow.h"
#include "backend/db-models/db_conn.h"
#include "ui_mainwindow.h"
#include "../delegates/CSimpleEditDelegate.h"
#include "../login/logindialog.h"
#include <qmessagebox.h>

extern SProgCfg   g_progCfg;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setSizes({10000, 20000});

    auto treeModel = new CTreeModel(ui->treeView);
    auto simpleDelegate = new CSimpleEditDelegate(ui->treeView);
    ui->treeView->setModel(treeModel);
    ui->treeView->setItemDelegate(simpleDelegate);
    ui->treeView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    m_treeManager = new CTreeManager(ui->treeView, ui->placeholder);

    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::slot_treeClicked);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::slot_treeContextMenu);
    connect(ui->tbLogin, &QAction::triggered, this, &MainWindow::slot_loginPressed);
    connect(ui->actLogout, &QAction::triggered, this, &MainWindow::slot_logoutPressed);

    m_user.set_id(0);
    m_treeManager->authorize(m_user);

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::slot_checkResourcesTasks);
    m_timer.start(g_progCfg.pendingResTimeout_ms);
}

MainWindow::~MainWindow()
{
    delete m_treeManager;
    delete ui;
}

void MainWindow::treeContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);

    TreeItem *item = nullptr;

    if (index.isValid())
    {
        item = static_cast<TreeItem*>(index.internalPointer());
    }

    if (!item)
    {
        return;
    }

    QMenu menu;

    auto actAddSibling = new QAction(QIcon(""), "Создать одноуровневую ветку", &menu);
    connect(actAddSibling, &QAction::triggered, this, &MainWindow::slot_addSiblingNode);

    auto actAddChild   = new QAction(QIcon(""), "Создать дочернюю ветку", &menu);
    connect(actAddChild, &QAction::triggered, this, &MainWindow::slot_addNestedNode);

    auto actRemoveNode = new QAction(QIcon(""), "Удалить содержимое рекурсивно", &menu);
    connect(actRemoveNode, &QAction::triggered, this, &MainWindow::slot_removeNode);

    //auto node = item->get_node();


    actAddSibling->setEnabled(actEbabled.addSibling);
    actAddChild->setEnabled(actEbabled.addNested);
    actRemoveNode->setEnabled(actEbabled.canRemove);

    menu.addAction(actAddSibling);
    menu.addAction(actAddChild);
    menu.addAction(actRemoveNode);

    menu.exec(ui->treeView->viewport()->mapToGlobal(pos));
}

void MainWindow::slot_treeClicked(const QModelIndex &index)
{
    updateButtons(index);
    m_treeManager->nodeClicked(index);
}

void MainWindow::slot_treeContextMenu(const QPoint &pos)
{
    updateButtons(ui->treeView->indexAt(pos));
    this->treeContextMenu(pos);
}

void MainWindow::updateButtons(const QModelIndex &index)
{
    auto *treeItem = static_cast<TreeItem*>(index.internalPointer());
    bool enabledAddNested{false}, enabledAddSibling{false}, canRemove{false};

    if (treeItem && treeItem->get_node() && m_user.get_id() > 0)
    {
        auto node = treeItem->get_node();

        if (m_user.is_admin())
        {
            if (node->user_id != m_user.get_id() && node->is_public && node->is_container)
            {
                enabledAddNested = true;
            }

            if (node->user_id == m_user.get_id())
            {
                enabledAddNested = true;
                enabledAddSibling = true;
                canRemove = true;
            }
        }
        else
        {
            if (m_user.get_id() == node->user_id)
            {
                canRemove = true;
                enabledAddNested = true;
                enabledAddSibling = true;
            }

            if ((node->user_id == 0 && node->is_container && !node->is_public))
            {
                enabledAddNested = true;
            }
        }
    }

    actEbabled.addNested = enabledAddNested;
    actEbabled.addSibling = enabledAddSibling;
    actEbabled.canRemove = canRemove;

    ui->tbAddNested->setEnabled(enabledAddNested);
    ui->tbAddSibling->setEnabled(enabledAddSibling);
    ui->tbRemoveSelected->setEnabled(canRemove);
}

void MainWindow::slot_addSiblingNode()
{
    QModelIndex index = getSelection();
    if (index.isValid())
    {
        m_treeManager->addSiblingNode(index);
    }
}

void MainWindow::slot_addNestedNode()
{
    QModelIndex index = getSelection();
    if (index.isValid())
    {
        m_treeManager->addNestedNode(index);
    }
}

void MainWindow::slot_removeNode()
{
    QModelIndex index = getSelection();
    if (index.isValid())
    {
        m_treeManager->removeNode(index);
    }
}

QModelIndex MainWindow::getSelection()
{
    auto selection = ui->treeView->selectionModel()->selectedIndexes();
    if (selection.empty())
        return {};

    return selection.at(0);
}

void MainWindow::slot_checkResourcesTasks()
{
    m_treeManager->checkPendingTasks();
}

void MainWindow::slot_loginPressed()
{
    if (m_user.get_id() > 0)
    {
        QMessageBox::information(this, "Внимание", "Вам следует выйти из учётки текущего пользователя, прежде чем пытаться войти в другую учетную запись.");
        return;
    }

    auto *dlg = new LoginDialog(&m_user, m_treeManager->connection());
    if (dlg->exec())
    {
        if (m_user.get_id() > 0)
        {
            this->setWindowTitle(QString::fromStdString(m_user.get_login()) + " Reminder");
            m_treeManager->authorize(m_user);
        }
    }
    delete dlg;
}

void MainWindow::slot_logoutPressed()
{
    if (m_user.get_id() > 0)
    {
        m_user = {};
        m_treeManager->authorize(m_user);
        this->setWindowTitle("Reminder");
        updateButtons({});
    }
}
