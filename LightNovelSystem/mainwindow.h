#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include<QDateTime>
#include<QModelIndex>
#include<QStandardItemModel>
#include <QMainWindow>

class NovelData;
class QStandardItem;
class QTableView;
class UserAccount;

QT_BEGIN_NAMESPACE
namespace Ui {class MainWindow;}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT     //重要的一个宏，把C++类升级成Qt元对象系统

public:
    explicit MainWindow(NovelData &data,const QString &account,QWidget *parent = nullptr);
    ~MainWindow() override;

    //Codex老师出得馊主意（给人看的，给程序看的）编号跟着 item 走，不靠行号推算
    static constexpr int ArticleIdRole=Qt::UserRole+1;
    static constexpr int ReportIdRole=Qt::UserRole+2;
private slots:
    void on_editPublicSearch_textChanged(const QString &text);
    void on_comboReviewStatus_currentIndexChanged(int index);
    void on_comboReportStatus_currentIndexChanged(int index);

    void on_tablePublic_doubleClicked(const QModelIndex &index);
    void on_btnRead_clicked();

    void on_btnNewArticle_clicked();
    void on_btnEditArticle_clicked();
    void on_btnPreviewMine_clicked();

    void on_btnLike_clicked();

    void on_btnReport_clicked();

    void on_btnReviewRead_clicked();
    void on_btnApprove_clicked();
    void on_btnReject_clicked();
    void on_btnWithdraw_clicked();

    void on_btnReportRead_clicked();
    void on_btnHandleReport_clicked();
    void on_btnWithdrawByReport_clicked();

    void on_btnAddWord_clicked();
    void on_btnDeleteWord_clicked();

    void on_actionSave_triggered();
    void on_actionLogout_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    //新添加的功能
    void on_comboCleanStatus_currentIndexChanged(int index);
    void on_editCleanBefore_dateChanged(const QDate &date);
    void on_btnCleanDelete_clicked();

    void on_editCleanAuthor_textChanged(const QString &text);


private:
    void configurePagesForRole();                                    //按角色布置页面
    void configureTable(QTableView *view, QStandardItemModel *model);//四张表通用配置
    void refreshAllViews();                                          //总刷新
    void refreshPublicTable();                                       //广场刷新
    void refreshMineTable();                                         //我的文章刷新
    void refreshReviewTable();                                       //文章审核刷新
    void refreshCleanTable();
    void refreshTopLikedTable();                                     //点赞排行刷新
    void refreshReportTable();                                       //举报管理刷新
    void refreshWordList();                                          //刷新违禁词代码
    void updateStatusBar();                                          //刷新状态栏
    //以上这些函数全部用于刷新

    int selectedArticleId(QTableView *view) const;                   //从指定表格中选取行中文章编号
    int selectedReportId() const;                                    //从举报列表中选中举报编号
    const UserAccount *currentUser() const;                          //返回当前用户的指针

    //以下是造格子的函数
    QStandardItem *makeTextItem(const QString &text) const;          //文字格
    QStandardItem *makeIntItem(int value) const;                     //数字格
    QStandardItem *makeDateItem(const QDateTime &dateTime) const;    //日期格

    Ui::MainWindow *ui;                                              //界面操作器
    NovelData &m_data;                                               //数据引用
    QString m_account;                                               //当前登录的账号
    QStandardItemModel *m_publicModel;                               //文章广场表
    QStandardItemModel *m_mineModel;                                 //我的文章表
    QStandardItemModel *m_reviewModel;                               //审核表
    QStandardItemModel *m_reportModel;                               //举报表
    QStandardItemModel *m_cleanModel;
    QStandardItemModel *m_topModel;                                //点赞排行表

    void openArticle(int articleId);
};

#endif // MAINWINDOW_H
