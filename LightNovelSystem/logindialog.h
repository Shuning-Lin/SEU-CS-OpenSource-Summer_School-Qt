#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class NovelData;

namespace Ui 
{
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT            //升级

public:
    explicit LoginDialog(NovelData &data,QWidget *parent = nullptr);
    ~LoginDialog() override;

    QString loggedInAccount() const;      //把存好的账号交出去

public slots:
    void accept() override;               //登录按钮内在真正的逻辑

private slots:
    void on_btnRegister_clicked();                   //自动槽   
    void on_checkShowPassword_toggled(bool checked); //自动槽

private:
    Ui::LoginDialog *ui;                  //界面操作器
    NovelData &m_data;                    //数据引用
    QString m_loggedInAccount;            //登录成功的账号
};

#endif // LOGINDIALOG_H
