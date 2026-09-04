#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

class NovelData;       //前置声明

namespace Ui 
{
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT            //升级
public:
    explicit RegisterDialog(NovelData &data,QWidget *parent = nullptr);  //提供了一份data的数据
    ~RegisterDialog() override;

    QString registeredAccount() const;

public slots:                  //来了奥，这是Qt关键字，下面的函数是槽
    void accept() override;    //重写QDialog自带的accpt()

private:
    Ui::RegisterDialog *ui;    //页面操作器(管理界面)
    NovelData &m_data;         //数据引用中心
    QString m_registerAccount; //跨窗口对话
};

#endif // REGISTERDIALOG_H
