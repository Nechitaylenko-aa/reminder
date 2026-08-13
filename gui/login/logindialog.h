#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "../../backend/db-models/CUserModelDB.h"


class CAbstractConnection;

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(CUser *user, CAbstractConnection * connection, QWidget *parent = nullptr);
    ~LoginDialog() override;
protected:


private:
    Ui::LoginDialog *ui;
    CUser  * m_user{nullptr};
    CUserModelDB * m_usersDBModel;
    CAbstractConnection * m_connection;

private slots:
    void slot_pwdChanged(const QString & pwd);
    void makePwdRed(bool isRed = true);
};

#endif // LOGINDIALOG_H
