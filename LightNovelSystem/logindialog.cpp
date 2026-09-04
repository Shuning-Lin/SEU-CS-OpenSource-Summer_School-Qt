#include "logindialog.h"
#include "ui_logindialog.h"

#include "noveldata.h"
#include "registerdialog.h"

#include <QLineEdit>
#include<QMessageBox>

LoginDialog::LoginDialog(NovelData &data,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    ,m_data(data)
{
    ui->setupUi(this);
    ui->editAccount->setFocus();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

QString LoginDialog::loggedInAccount() const
{
    return m_loggedInAccount;
}

void LoginDialog::accept()
{
    const QString account=ui->editAccount->text().trimmed();
    const QString password=ui->editPassword->text();

    if (account.isEmpty() or password.isEmpty()) 
    {
        QMessageBox::warning(this, QStringLiteral("输入错误"), QStringLiteral("账号和密码不能为空。"));
        return;
    }

    QString error;
    if(!m_data.checkLogin(account,password,&error))
    {
        QMessageBox::warning(this,QStringLiteral("登录失败"),error);
        ui->editPassword->clear();
        ui->editPassword->setFocus();
        return;
    }
    m_loggedInAccount=account;
    QDialog::accept();
}

void LoginDialog::on_btnRegister_clicked()
{
    RegisterDialog dialog(m_data,this);
    if(dialog.exec()!=QDialog::Accepted)
        return;

    ui->editAccount->setText(dialog.registeredAccount());
    ui->editPassword->clear();       //提前一步清除
    ui->editPassword->setFocus();    //让光标重新集中到这里
}

void LoginDialog::on_checkShowPassword_toggled(bool checked)
{
    ui->editPassword->setEchoMode(checked ? QLineEdit::Normal:QLineEdit::Password);
}
