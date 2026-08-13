#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "../../backend/sources/include/CTreeManager.h"
#include "../../backend/db-models/units/CUser.h"



QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void slot_treeClicked(const QModelIndex &index);
    void slot_treeContextMenu(const QPoint &pos);
    void slot_addSiblingNode();
    void slot_addNestedNode();
    void slot_removeNode();
    void slot_checkResourcesTasks();
    void slot_loginPressed();
    void slot_logoutPressed();

private: // members
    Ui::MainWindow * ui;
    CTreeManager   * m_treeManager;
    CUser   m_user{}; //!< logged in user
    QTimer  m_timer;

private: // methods
    // on tree context menu
    void treeContextMenu(const QPoint &pos);
    // after user click on the tree
    void updateButtons(const QModelIndex &index);

    QModelIndex getSelection();

    struct SActEnabled
    {
        bool addSibling{false};
        bool addNested{false};
        bool canRemove{false};
    };

    SActEnabled actEbabled;


};
#endif // MAINWINDOW_H
