#include "logindialog.h"
#include "ui_logindialog.h"

void makePwdRed(bool isRed);

LoginDialog::LoginDialog(CUser *user, CAbstractConnection * connection, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_user(user)
    , m_connection(connection)
{
    ui->setupUi(this);
    if (!m_user)
    {
        return;
    }

    connect(ui->lePasswd, &QLineEdit::textChanged, this, &LoginDialog::slot_pwdChanged);
    m_usersDBModel = new CUserModelDB(m_connection);
}

LoginDialog::~LoginDialog()
{
    delete ui;
    delete m_usersDBModel;
}

void LoginDialog::slot_pwdChanged(const QString &pwd)
{
    if (!m_user)
    {
        return;
    }

    if (ui->leLogin->text().isEmpty())
    {
        makePwdRed();
        return;
    }
    std::string login = ui->leLogin->text().toStdString();
    std::string passwd= pwd.toStdString();
    CUser user = m_usersDBModel->login(login.c_str(), passwd.c_str());
    if (user.get_id() == 0)
    {
        makePwdRed();
        return;
    }
    makePwdRed(false);
    *(m_user) = user;
}

void LoginDialog::makePwdRed(bool isRed)
{
    if (isRed)
    {
        ui->lePasswd->setStyleSheet("QLineEdit { border: 1px solid red; }");
    }
    else
    {
        ui->lePasswd->setStyleSheet("");
    }
}
