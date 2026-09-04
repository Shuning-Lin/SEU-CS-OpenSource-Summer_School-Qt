#include "mainwindow.h"
#include "logindialog.h"
#include "noveldata.h"

#include <QApplication>
#include <QCoreApplication>
#include<QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);//这里argc是参数个数，argv保存传入的参数
    QCoreApplication::setOrganizationName(QStringLiteral("TsuwabukiHighSchool"));
    QCoreApplication::setApplicationName(QStringLiteral("LightNovelSystem"));

    NovelData data;           //把所有数据导入内存
    QString error;            //错误信息
    if(!data.load(&error))  //基本上不会发生，总之是Codex老师让我这么做的
    {
        QMessageBox::critical(nullptr, QStringLiteral("启动失败"),
                              error + QStringLiteral("\n\n数据文件：")
                              + data.dataFilePath());
        return 0;
    }
    
    LoginDialog login(data);
    if(login.exec()!=QDialog::Accepted)   //阻塞式弹出登录框，模态运行，程序执行到这里卡住，把控制权转交个登录对话框，用户再对话框里点按钮、输账号，直到窗口关闭才放回。
        return 0;

    MainWindow window(data, login.loggedInAccount());
    window.setWindowTitle(QStringLiteral("已登录：%1 %2").arg(login.loggedInAccount()).arg(data.findUser(login.loggedInAccount())->m_name));
    window.show();
    return app.exec();
}
