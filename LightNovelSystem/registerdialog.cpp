#include "registerdialog.h"
#include "ui_registerdialog.h"

#include "noveldata.h"

#include<QMessageBox>
#include<QRegularExpression>

RegisterDialog::RegisterDialog(NovelData &data,QWidget *parent)
    : QDialog(parent)               //给父类传参
    , ui(new Ui::RegisterDialog)    //创建界面对象
    ,m_data(data)                   //引用成员绑定
{
    ui->setupUi(this);              //把界面封装进this
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

QString RegisterDialog::registeredAccount() const    //这里确实有点没有懂
{
    return m_registerAccount;
}

void RegisterDialog::accept()
{
    const QString name=ui->editName->text().trimmed();
    const QString studentId=ui->editStudentId->text().trimmed();
    const QString password=ui->editPassword->text();
    const QString passwordAgain=ui->editPasswordAgain->text();

    static const QRegularExpression idPattern(QStringLiteral("^[0-9]{9}"));

   if (name.isEmpty() or name.size() > 20) 
   {
        QMessageBox::warning(this, QStringLiteral("输入错误"),QStringLiteral("姓名长度应为 1-20 个字符。"));
        ui->editName->setFocus();
        return;
    }
    if (!idPattern.match(studentId).hasMatch())    //这个我稍微设置得有点特殊
    {
        QMessageBox::warning(this, QStringLiteral("输入错误"),QStringLiteral("学号必须是 9 位数字。"));
        ui->editStudentId->setFocus();
        return;
    }
    if (password.size() < 6 or password.size() > 20) 
    {
        QMessageBox::warning(this, QStringLiteral("输入错误"),QStringLiteral("密码长度应为 6-20 个字符。"));
        ui->editPassword->setFocus();
        return;
    }
    if (password != passwordAgain)
    {
        QMessageBox::warning(this, QStringLiteral("输入错误"), QStringLiteral("两次输入的密码不一致。"));
        ui->editPasswordAgain->clear();
        ui->editPasswordAgain->setFocus();
        return; 
    }

    QString error;
    if(!m_data.registerStudent(studentId,name,password,&error))
    {
        QMessageBox::warning(this,QStringLiteral("注册失败"),error);
        return;
    }

    m_registerAccount=studentId;     
    QMessageBox::information(this,QStringLiteral("注册成功"),QStringLiteral("请使用学号和密码登录。"));
    QDialog::accept();     //关闭窗口并返回成功结果
}
