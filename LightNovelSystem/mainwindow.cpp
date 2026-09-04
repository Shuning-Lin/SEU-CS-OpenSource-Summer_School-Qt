#include "mainwindow.h"        //所有函数的定义
#include "ui_mainwindow.h"     //uic生成的界面类

#include "articleeditdialog.h" //修改文章的对话框
#include "articlereaddialog.h" //阅读文章的对话框
#include "noveldata.h"         //所有业务的调用
#include "reportdialog.h"      //举报的对话框

#include <QAbstractItemView>   //表格行为的设置
#include <QApplication>        //细节的格式
#include <QHeaderView>         //隐藏行头
#include <QListWidgetItem>     //违禁词列表
#include <QMessageBox>         //弹窗
#include <QStandardItem>       //格子
#include <QTableView>          //表格类型
#include <algorithm>           //std::sort（点赞排行）

MainWindow::MainWindow(NovelData &data,const QString &account,QWidget *parent)
: QMainWindow(parent)
,ui(new Ui::MainWindow)
,m_data(data)
,m_account(account)
, m_publicModel(new QStandardItemModel(this))
, m_mineModel(new QStandardItemModel(this))
, m_reviewModel(new QStandardItemModel(this))
, m_reportModel(new QStandardItemModel(this))
,m_cleanModel(new QStandardItemModel(this))
,m_topModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    configureTable(ui->tablePublic,m_publicModel);
    configureTable(ui->tableMine,m_mineModel);
    configureTable(ui->tableReview,m_reviewModel);
    configureTable(ui->tableReports,m_reportModel);
    configureTable(ui->tableCleanup, m_cleanModel);
    configureTable(ui->tableTopLiked, m_topModel);


    ui->editCleanBefore->setDate(QDate::currentDate());
    configurePagesForRole();     //角色可见性的设置
    refreshAllViews();
}

MainWindow::~MainWindow()
{
    delete ui;
}

const UserAccount *MainWindow::currentUser() const
{
    return m_data.findUser(m_account);
}

void MainWindow::configureTable(QTableView *view,QStandardItemModel *model)//参数分别是要配置的表格和要绑定的模型
{
    view->setModel(model);                                     //表格&模型的绑定  
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);  //禁止编辑
    view->setSelectionBehavior(QAbstractItemView::SelectRows); //点一下，整行选中
    view->setSelectionMode(QAbstractItemView::SingleSelection);//单选
    view->setAlternatingRowColors(true);                       //交替行色
    view->setSortingEnabled(true);                             //允许点击表头进行排序
    view->verticalHeader()->setVisible(false);                 // 隐藏行号列
    model->setSortRole(Qt::UserRole);                          //排序按照隐藏值（小心文本）
}

void MainWindow::configurePagesForRole()
{
    const UserAccount *user=currentUser();
    if(!user)
    {
        QMessageBox::critical(this, QStringLiteral("账号错误"),QStringLiteral("当前登录账号不存在。"));
        return;
    }

    const bool admin=(user->m_role ==UserRole::Admin);    //这里按照Codex老师的建议，加了个const
    //本来想写序号0，1，2，3，4的，但是Codex老师建议还是采用indexOf
    ui->tabWidget->setTabVisible(
        ui->tabWidget->indexOf(ui->tabMine), !admin);     //管理员没有自己的文章
    ui->tabWidget->setTabVisible(                         
        ui->tabWidget->indexOf(ui->tabReview), admin);    //管理员才能审核
    ui->tabWidget->setTabVisible(
        ui->tabWidget->indexOf(ui->tabReports), admin);   //管理员才能看到举报
    ui->tabWidget->setTabVisible(
        ui->tabWidget->indexOf(ui->tabWords), admin);     //管理员才能设置违禁词
    
    ui->tabWidget->setTabVisible(
        ui->tabWidget->indexOf(ui->tabCleanup),admin);    //管理员才能彻底删除文章底层数据

    ui->btnLike->setEnabled(!admin);
    ui->btnReport->setEnabled(!admin);                    //Codex老师这里的建议我觉得不合理，不过后续再改吧
    ui->labelCurrentUser->setText(QString("当前用户：%1 (%2,%3)").arg(user->m_name,user->m_account,userRoleText(user->m_role)));
    ui->tabWidget->setCurrentWidget(ui->tabPublic);
}

//创建表格和稳定编号
QStandardItem *MainWindow::makeTextItem(const QString &text) const
{
    auto *item = new QStandardItem(text);     //创造一个格子
    item->setData(text,Qt::UserRole);         //往排序抽屉里面存值
    item->setEditable(false);                 //锁死编辑
    item->setTextAlignment(Qt::AlignCenter);  //居中放置
    return item;                              //返回该格子的指针
}
//反正和上面如出一辙吧
QStandardItem *MainWindow::makeIntItem(int value) const
{
    auto *item = new QStandardItem(QString::number(value));
    item->setData(value, Qt::UserRole);
    item->setEditable(false);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}

QStandardItem *MainWindow::makeDateItem(const QDateTime &dateTime) const
{
    auto *item = new QStandardItem(dateTime.toString(QStringLiteral("yyyy-MM-dd HH:mm")));
    item->setData(dateTime, Qt::UserRole);
    item->setEditable(false);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}

int MainWindow::selectedArticleId(QTableView *view) const
{                                                       //当前选中的那行的索引
    const QModelIndex current =view->currentIndex();    //Codex老师建议加一个const
    if(!current.isValid())                              //防止根本没有选中
        return -1;
    
    const QModelIndex idIndex=view->model()->index(current.row(),0);   //选中指定的格子
    return idIndex.data(ArticleIdRole).toInt();                        //挑出“暗格”里的编号 
}

//和上面差不多
int MainWindow::selectedReportId() const
{
    const QModelIndex current =ui->tableReports->currentIndex();    //当用户点击某一行的时候，这一行就是currentIndex
    if(!current.isValid())
        return -1;

    const QModelIndex idIndex=m_reportModel->index(current.row(),0);
    return idIndex.data(ReportIdRole).toInt();
}
//开始重构文章广场

void MainWindow::refreshPublicTable()
{
    ui->tablePublic->setSortingEnabled(false);  //临时关掉排序
    m_publicModel->clear();                     //清空模型里的所有内容
    m_publicModel->setHorizontalHeaderLabels    //设置5个标题
    (
        {
            QStringLiteral("编号"),
            QStringLiteral("标题"),
            QStringLiteral("作者"),
            QStringLiteral("更新时间"),
            QStringLiteral("点赞数")
        }
    );
    //总之,Codex建议加const的地方我都会加上，我觉得Codex老师工程能力是很强的
    const QString key = ui->editPublicSearch->text().trimmed();
    int row=0;
    for(const Article &article:m_data.articles())
    {
        if(article.m_status!=ArticleStatus::Published)
           continue;

        const bool matches=key.isEmpty() or article.m_title.contains(key,Qt::CaseInsensitive)
                           or article.m_authorName.contains(key,Qt::CaseInsensitive);
        if(!matches)
            continue;

        QStandardItem *idItem=makeIntItem(article.m_id);
        idItem->setData(article.m_id,ArticleIdRole);

        m_publicModel->setItem(row,0,idItem);
        m_publicModel->setItem(row,1,makeTextItem(article.m_title));
        m_publicModel->setItem(row,2,makeTextItem(article.m_authorName));
        m_publicModel->setItem(row,3,makeDateItem(article.m_updatedAt));
        m_publicModel->setItem(row,4,makeIntItem(article.m_likedBy.size()));
        row++;
    }

    ui->tablePublic->hideColumn(0);                                                 //隐藏编号列
    ui->tablePublic->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//铺满整张表格
    ui->tablePublic->setSortingEnabled(true);                                       //重新开启排序
    ui->tablePublic->sortByColumn(3,Qt::DescendingOrder);                           //按更新时间排序一次
    ui->labelPublicCount->setText(QStringLiteral("共计%1篇").arg(row));              //显示查到了多少篇轻小说
}
//搜索时重建显示模型，不会删除 NovelData 中的文章。编号列被隐藏，但数据仍存在。
void MainWindow::on_editPublicSearch_textChanged(const QString &text)
{
    Q_UNUSED(text)          // 参数不用，刷新函数自己去读搜索框
    refreshPublicTable();   // 用户每敲一个字 -> 表格重建 -> 即时过滤
}

//开始重构“我的文章”
void MainWindow::refreshMineTable()
{
    ui->tableMine->setSortingEnabled(false);    //这里不加会有鬼畜的效果哟，到时候可以演示一下
    m_mineModel->clear();
    m_mineModel->setHorizontalHeaderLabels
    (
        {
            QStringLiteral("编号"), 
            QStringLiteral("标题"),
            QStringLiteral("状态"), 
            QStringLiteral("更新时间"),
            QStringLiteral("点赞数")
        }
    );
    int row=0;
    for(const Article &article:m_data.articles())
    {
        if(article.m_authorAccount==m_account)
        {
            QStandardItem *idItem=makeIntItem(article.m_id);    
            idItem->setData(article.m_id,ArticleIdRole);
            //这两句话，其实article.m_id流进了三个地方0，256，257,Codex老师让我们这样存一定有她的道理，先继续吧
            m_mineModel->setItem(row,0,idItem);
            m_mineModel->setItem(row,1,makeTextItem(article.m_title));
            m_mineModel->setItem(row,2,makeTextItem(articleStatusText(article.m_status)));
            m_mineModel->setItem(row,3,makeDateItem(article.m_updatedAt));
            m_mineModel->setItem(row,4,makeIntItem(article.m_likedBy.size()));
            row++;
        }
    }
    ui->tableMine->hideColumn(0);
    ui->tableMine->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableMine->setSortingEnabled(true);
    ui->tableMine->sortByColumn(3,Qt::DescendingOrder);
}

//重构管理员审核表
void MainWindow::refreshReviewTable()
{
    ui->tableReview->setSortingEnabled(false);
    m_reviewModel->clear();
    m_reviewModel->setHorizontalHeaderLabels
    (
        {
            QStringLiteral("编号"), QStringLiteral("标题"),
            QStringLiteral("作者"), QStringLiteral("状态"),
            QStringLiteral("更新时间"), QStringLiteral("点赞数")
        }
    );

    const int filter = ui->comboReviewStatus->currentIndex();   //这里返回在下拉框中的第几项
    int row = 0;                                                //计数器
    for (const Article &article : m_data.articles()) 
    {
        bool matches = filter == 0;                             //默认选“全部”的选项
        if (filter == 1)
            matches =(article.m_status == ArticleStatus::Pending);
        else if (filter == 2)
            matches =(article.m_status == ArticleStatus::Published);
        else if (filter == 3)
            matches = (article.m_status == ArticleStatus::Rejected);
        else if (filter == 4)
            matches = (article.m_status == ArticleStatus::Withdrawn);

        if (!matches)                                           //不符合筛选条件的文章就没有必要往下走了
            continue;

        QStandardItem *idItem = makeIntItem(article.m_id);      //同
        idItem->setData(article.m_id, ArticleIdRole);           //上
        m_reviewModel->setItem(row, 0, idItem);
        m_reviewModel->setItem(row, 1, makeTextItem(article.m_title));
        m_reviewModel->setItem(row, 2,makeTextItem(article.m_authorName));
        m_reviewModel->setItem(row, 3, makeTextItem(articleStatusText(article.m_status)));
        m_reviewModel->setItem(row, 4,makeDateItem(article.m_updatedAt));
        m_reviewModel->setItem(row, 5,makeIntItem(article.m_likedBy.size()));
        row++;
    }

    ui->tableReview->hideColumn(0);
    ui->tableReview->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch);
    ui->tableReview->setSortingEnabled(true);
    ui->tableReview->sortByColumn(4, Qt::DescendingOrder);
}

void MainWindow::on_comboReviewStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index)         //Codex老师稳定发挥
    refreshReviewTable();   //比如处理完一份举报？
}

void MainWindow::on_comboCleanStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    refreshCleanTable();
}

void MainWindow::on_editCleanBefore_dateChanged(const QDate& date)
{
    on_comboCleanStatus_currentIndexChanged(0);
}

void MainWindow::on_btnCleanDelete_clicked()
{
    const int articleId = selectedArticleId(ui->tableCleanup);
    if(articleId<0)
    {
        QMessageBox::information(this,QString("提示"),QString("请先选择一篇文章。"));
        return;
    }
    if(QMessageBox::question(this,QStringLiteral("确认删除"),QStringLiteral("你确认要永久删除吗？"))!=QMessageBox::Yes)
    {
        return;
    }
    const Article *article=m_data.findArticle(articleId);
    QString error;
    if(!m_data.deleteArticle(articleId,m_account,&error))
    {
        QMessageBox::warning(this,QStringLiteral("删除失败"),error);
    }
    refreshAllViews();
}

void MainWindow::refreshReportTable()
{
    ui->tableReports->setSortingEnabled(false);
    m_reportModel->clear();
    m_reportModel->setHorizontalHeaderLabels(
        {QStringLiteral("举报编号"), QStringLiteral("文章编号"),
         QStringLiteral("文章标题"), QStringLiteral("举报人"),
         QStringLiteral("原因"), QStringLiteral("补充说明"),
         QStringLiteral("举报时间"), QStringLiteral("状态")});

    const int filter = ui->comboReportStatus->currentIndex();
    int row = 0;
    for (const Report &report : m_data.reports())
     {
        const bool matches =
            filter == 2
            || (filter == 0 && report.m_status == ReportStatus::Pending)
            || (filter == 1 && report.m_status == ReportStatus::Handled);

        if (!matches)
            continue;

        const Article *article = m_data.findArticle(report.m_articleId);

        const QString title = article? article->m_title: QStringLiteral("文章不存在");
        //一下这样是行不通的
        /*
        if(article)
            const QString title=article->m_title;
        else
            const QString title=QStringLiteral("文章不存在");
        */

        QStandardItem *reportIdItem = makeIntItem(report.m_id);
        reportIdItem->setData(report.m_id, ReportIdRole);
        m_reportModel->setItem(row, 0, reportIdItem);
        m_reportModel->setItem(row, 1,makeIntItem(report.m_articleId));
        m_reportModel->setItem(row, 2, makeTextItem(title));
        m_reportModel->setItem(row, 3, makeTextItem(report.m_reporterAccount));
        m_reportModel->setItem(row, 4, makeTextItem(report.m_reason));
        m_reportModel->setItem(row, 5, makeTextItem(report.m_detail));
        m_reportModel->setItem(row, 6,makeDateItem(report.m_createdAt));
        m_reportModel->setItem(row, 7, makeTextItem(reportStatusText(report.m_status)));
        row++;
    }

    ui->tableReports->hideColumn(0);
    ui->tableReports->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableReports->setSortingEnabled(true);
    ui->tableReports->sortByColumn(6, Qt::DescendingOrder);
}

//关于行号的解释
void MainWindow::refreshWordList()
{
    ui->listWords->clear();                          //清空列表
    ui->listWords->addItems(m_data.forbiddenWords());//填入全部违禁词
    ui->listWords->sortItems(Qt::AscendingOrder);    //按字母升序
}

void MainWindow::on_comboReportStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    refreshReportTable();
}
void MainWindow::refreshCleanTable()
{
    ui->tableCleanup->setSortingEnabled(false);
    m_cleanModel->clear();
    m_cleanModel->setHorizontalHeaderLabels(
    {QStringLiteral("编号"), QStringLiteral("标题"),
    QStringLiteral("作者"), QStringLiteral("状态"),
    QStringLiteral("更新时间"),QStringLiteral("点赞数")});
    
    const int filter1=ui->comboCleanStatus->currentIndex();
    const QDate filter2=ui->editCleanBefore->date();
    const QString filter3=ui->editCleanAuthor->text().trimmed();  //还好老师多要的功能不难

    int row=0;
    for(const Article &article:m_data.articles())
    {
        bool matches = (filter1 == 0 and article.m_updatedAt.date()<filter2) and article.m_authorName.contains(filter3);                             //默认选“全部”的选项
        if (filter1 == 1)
            matches =(article.m_status == ArticleStatus::Pending) and article.m_updatedAt.date() < filter2 and article.m_authorName.contains(filter3);
        else if (filter1 == 2)
            matches =(article.m_status == ArticleStatus::Published) and article.m_updatedAt.date() < filter2 and article.m_authorName.contains(filter3);
        else if (filter1 == 3)
            matches = (article.m_status == ArticleStatus::Rejected) and article.m_updatedAt.date() < filter2 and article.m_authorName.contains(filter3);
        else if (filter1 == 4)
            matches = (article.m_status == ArticleStatus::Withdrawn) and article.m_updatedAt.date() < filter2 and  article.m_authorName.contains(filter3);
        

        if(!matches)
        {
            continue;
        }
        QStandardItem *idItem = makeIntItem(article.m_id);      //同
        idItem->setData(article.m_id, ArticleIdRole);           //上
        m_cleanModel->setItem(row, 0, idItem);
        m_cleanModel->setItem(row, 1, makeTextItem(article.m_title));
        m_cleanModel->setItem(row, 2,makeTextItem(article.m_authorName));
        m_cleanModel->setItem(row, 3, makeTextItem(articleStatusText(article.m_status)));
        m_cleanModel->setItem(row, 4,makeDateItem(article.m_updatedAt));
        m_cleanModel->setItem(row, 5,makeIntItem(article.m_likedBy.size()));
        row++;
    }
    ui->tableCleanup->hideColumn(0);
    ui->tableCleanup->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableCleanup->setSortingEnabled(true);
    ui->tableCleanup->sortByColumn(4, Qt::DescendingOrder);
}

//点赞排行（前三名）
void MainWindow::refreshTopLikedTable()
{
    ui->tableTopLiked->setSortingEnabled(false);   // 固定前三，关闭交互排序
    m_topModel->clear();
    m_topModel->setHorizontalHeaderLabels(
        {QStringLiteral("编号"), QStringLiteral("标题"), QStringLiteral("作者"),
         QStringLiteral("点赞数"), QStringLiteral("更新时间")});

    // ① 收集已公开文章（指针，避免复制）
    QVector<const Article *> candidates;
    for (const Article &article : m_data.articles()) {
        if (article.m_status == ArticleStatus::Published)
            candidates.append(&article);
    }

    // ② 按点赞数降序；同票 → 编号小者靠前（并列处理）
    std::sort(candidates.begin(), candidates.end(),
              [](const Article *a, const Article *b) {
                  if (a->m_likedBy.size() != b->m_likedBy.size())
                      return a->m_likedBy.size() > b->m_likedBy.size();
                  return a->m_id < b->m_id;
              });

    // ③ 取前三名
    const int topCount = qMin(3, candidates.size());
    for (int row = 0; row < topCount; ++row) {
        const Article *article = candidates.at(row);
        QStandardItem *idItem = makeIntItem(article->m_id);
        idItem->setData(article->m_id, ArticleIdRole);
        m_topModel->setItem(row, 0, idItem);
        m_topModel->setItem(row, 1, makeTextItem(article->m_title));
        m_topModel->setItem(row, 2, makeTextItem(article->m_authorName));
        m_topModel->setItem(row, 3, makeIntItem(article->m_likedBy.size()));
        m_topModel->setItem(row, 4, makeDateItem(article->m_updatedAt));
    }

    ui->tableTopLiked->hideColumn(0);
    ui->tableTopLiked->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//刷新状态栏
void MainWindow::updateStatusBar()
{
    int published=0;
    int pendingArticles=0;
    int pendingReports=0;

    for(const Article &article:m_data.articles())
    {
        if(article.m_status==ArticleStatus::Pending)
            pendingArticles++;
        else if(article.m_status==ArticleStatus::Published)
            published++;
    }
    for (const Report &report : m_data.reports())
    {
        if (report.m_status == ReportStatus::Pending)
            ++pendingReports;
    }
    statusBar()->showMessage(                   //到时候看看这个是刷在哪里的
        QStringLiteral("文章总数：%1  已公开：%2  待审核：%3  待处理举报：%4")
        .arg(m_data.articles().size())
        .arg(published)
        .arg(pendingArticles)
        .arg(pendingReports));
}
//统一刷新
void MainWindow::refreshAllViews()
{
    refreshPublicTable();
    refreshMineTable();
    refreshReviewTable();
    refreshReportTable();
    refreshWordList();
    refreshCleanTable();
    refreshTopLikedTable();
    updateStatusBar();
}
//补充两个槽函数
void MainWindow::openArticle(int articleId)
{
    const Article *article=m_data.findArticle(articleId);
    if(!article)
    { 
        QMessageBox::warning(this,QStringLiteral("操作失败咯"),QStringLiteral("找不到选中的文章。"));
        return;
    }

    ArticleReadDialog dialog(this);
    dialog.setArticle(*article);
    dialog.exec();
}

void MainWindow::on_btnRead_clicked()
{
    const int articleId=selectedArticleId(ui->tablePublic);
    if(articleId<0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一篇文章。"));
        return;
    }
    openArticle(articleId);
}

void MainWindow::on_tablePublic_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    on_btnRead_clicked();
}

void MainWindow::on_btnNewArticle_clicked()
{
    ArticleEditDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("新建文章"));
    if(dialog.exec()!=QDialog::Accepted)
       return;

    QString error;
    int newId=0;
    if(!m_data.addArticle(m_account,dialog.title(),dialog.content(),&newId,&error))
    {
        QMessageBox::warning(this,QStringLiteral("提交失败"),error);
        return;
    }

    QMessageBox::information(this,QStringLiteral("提交成功"),QStringLiteral("文章编号 %1 已进入待审核列表。").arg(newId));
    refreshAllViews();
}

void MainWindow::on_btnEditArticle_clicked()
{
    const int articleId = selectedArticleId(ui->tableMine);
    Article *article = m_data.findArticle(articleId);
    if (!article) //理论上压根不可能出现这种情况，但是Codex老师建议加那就加吧...
    {
        QMessageBox::information(this, QStringLiteral("提示"),QStringLiteral("请先选择自己的文章。"));
        return;
    }

    ArticleEditDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("修改文章"));
    dialog.setArticle(article->m_title,article->m_content);
    if(dialog.exec()!=QDialog::Accepted)
        return;

    QString error;
    if(!m_data.updateArticle(articleId,m_account,dialog.title(),dialog.content(),&error))
    {
        QMessageBox::warning(this,QStringLiteral("修改失败"),error);
        return;
    }

    QMessageBox::information(this, QStringLiteral("修改成功"),QStringLiteral("文章已重新进入待审核状态。"));
    refreshAllViews();
}

void MainWindow::on_btnPreviewMine_clicked()
{
    const int articleId=selectedArticleId(ui->tableMine);
    if(articleId<0)   //Codex老师的防伪人操作
    {
        QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("请选择一篇文章"));
        return;
    }
    openArticle(articleId);
}
//学生页面只显示自己的文章，但 updateArticle() 仍会再次核对作者账号。
//界面过滤不能代替数据层权限

//点赞与取消点赞
void MainWindow::on_btnLike_clicked()
{
    const int articleId=selectedArticleId(ui->tablePublic);
    if(articleId<0)
    {
        QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("请先选择一篇文章"));
        return;
    }
    bool Liked=false;
    QString error;
    if(!m_data.toggleLike(articleId,m_account,&Liked,&error))
    {
        QMessageBox::warning(this,QStringLiteral("操作失败"),error);
        return;
    }
    statusBar()->showMessage(Liked? QStringLiteral("点赞成功"):QStringLiteral("取消点赞"),3000);//暂停3喵
    refreshPublicTable();    //文章广场
    refreshMineTable();      //我的文章
    refreshReviewTable();    //文章审核
    refreshCleanTable();
    refreshTopLikedTable();
    //后期测试的时候可以试试这个 refreshAllViews()
}

void MainWindow::on_editCleanAuthor_textChanged(const QString &text)
{
    Q_UNUSED(text)         
    refreshCleanTable();    
}

void MainWindow::on_btnReport_clicked()
{
    const int articleId = selectedArticleId(ui->tablePublic);
    const Article *article = m_data.findArticle(articleId);
    if(!article)
    {
        QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("请先选择一篇文章。"));
    }
    ReportDialog dialog(this);
    dialog.setArticleTitle(article->m_title);
    if(dialog.exec()!=QDialog::Accepted)  //关闭了之后就没有必有继续操作了
        return;
    QString error;
    if(!m_data.addReport(articleId,m_account,dialog.reason(),dialog.detail(),&error))
    {
        QMessageBox::warning(this,QStringLiteral("举报失败"),error);
        return;
    }

    QMessageBox::information(this,QStringLiteral("举报成功"),QStringLiteral("你的举报已经提交给了管理员"));
    refreshReportTable();
    updateStatusBar();
}

//审核的预览、通过、驳回、撤回

void MainWindow::on_btnReviewRead_clicked()   
{
    const int articleId = selectedArticleId(ui->tableReview);
    if (articleId < 0) 
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一篇文章。"));
        return;
    }
    openArticle(articleId);
}

void MainWindow::on_btnApprove_clicked()
{
    const int articleId = selectedArticleId(ui->tableReview);
    if (articleId < 0)
     {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择待审核文章。"));
        return;
    }

    QString error;
    if (!m_data.approveArticle(articleId, m_account, &error)) 
    { 
        QMessageBox::warning(this, QStringLiteral("审核失败"), error);
        return;
    }
    QMessageBox::information(this, QStringLiteral("审核完成"),QStringLiteral("文章已公开。"));
    refreshAllViews();   //其实只有违禁词管理不需要刷新
}

void MainWindow::on_btnReject_clicked()
{
    const int articleId = selectedArticleId(ui->tableReview);
    if (articleId < 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择待审核文章。"));
        return;
    }

    if(QMessageBox::question(this,QStringLiteral("确认驳回"),QStringLiteral("Are you sure?"))!=QMessageBox::Yes)
    {
        return;    //取消了操作
    }

    QString error;
    if (!m_data.rejectArticle(articleId, m_account, &error))
    {
        QMessageBox::warning(this, QStringLiteral("驳回失败"), error);
        return;
    }
    refreshAllViews();
}

void MainWindow::on_btnWithdraw_clicked()     //撤回直接是从文章广场上撤回（可能之前审核通过了，但是管理员反悔了）
{
    const int articleId = selectedArticleId(ui->tableReview);
    if (articleId < 0) 
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择已公开文章。"));
        return;
    }

    if (QMessageBox::question(this, QStringLiteral("确认撤回"),QStringLiteral("撤回后文章将立即从文章广场消失，是否继续？"))!= QMessageBox::Yes) 
    {
        return;
    }

    QString error;
    if(!m_data.withdrawArticle(articleId,m_account,&error))  //我觉得好像是不是有点问题，算了，后续测试的时候再说吧
    {
        QMessageBox::warning(this,QStringLiteral("撤回失败"),error);
    }
    refreshAllViews();
}

void MainWindow::on_btnReportRead_clicked()
{
    const int reportId = selectedReportId();
    const Report *report = m_data.findReport(reportId);
    if (!report) 
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一条举报。"));
        return;
    }
    openArticle(report->m_articleId);
}

void MainWindow::on_btnHandleReport_clicked()
{
    const int reportId = selectedReportId();
    if (reportId < 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择待处理举报。"));
        return;
    }

    QString error;
    if (!m_data.handleReport(reportId, m_account, false, &error))
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), error);
        return;
    }
    refreshAllViews();
}

void MainWindow::on_btnWithdrawByReport_clicked()
{
    const int reportId = selectedReportId();
    if (reportId < 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择待处理举报。"));
        return;
    }

    if (QMessageBox::question(this, QStringLiteral("确认撤回"),QStringLiteral("确定撤回被举报文章，并把该举报标记为已处理吗？"))!= QMessageBox::Yes) 
    {
        return;    //终止操作
    }

    QString error;
    if (!m_data.handleReport(reportId, m_account, true, &error))
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), error);
        return;
    }
    refreshAllViews();
}

void MainWindow::on_btnAddWord_clicked()
{
    const QString word =ui->editNewWord->text().trimmed();
    QString error;
    if (!m_data.addForbiddenWord(word, m_account, &error)) 
    {
        QMessageBox::warning(this, QStringLiteral("添加失败"), error);
        return;
    }
    ui->editNewWord->clear();
    refreshWordList();
}

void MainWindow::on_btnDeleteWord_clicked()
{
    QListWidgetItem *item=ui->listWords->currentItem();
    if (!item)                                          //学到了，在工程上，删除操作还是非常谨慎的
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一个违禁词。"));
        return;
    }

    if(QMessageBox::question(this,QStringLiteral("确认删除"),QStringLiteral("确定删除违禁词“%1吗”(小心BL文乱入!!)").arg(item->text()))!=QMessageBox::Yes)
    {
        return;
    }
    QString error;
    if (!m_data.removeForbiddenWord(item->text(), m_account, &error))
    {
        QMessageBox::warning(this, QStringLiteral("删除失败"), error);
        return;
    }
    refreshWordList();
}

void MainWindow::on_actionSave_triggered()
{
    QString error;
    if (!m_data.save(&error))
    {
        QMessageBox::warning(this, QStringLiteral("保存失败"), error);
        return;
    }
    statusBar()->showMessage(QStringLiteral("数据已保存"), 3000);
}

void MainWindow::on_actionLogout_triggered()
{
    if (QMessageBox::question(this, QStringLiteral("注销"),QStringLiteral("程序将退出,是否继续？"))== QMessageBox::Yes)
    {
        qApp->quit();
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,QStringLiteral("关于"),QStringLiteral("本系统为：石蕗高中轻小说系统，目前该系统版本：V1.0\n\n"
        "这里特别感谢石蕗高中学生会提供的支持，显然，当前的系统比较粗糙，不过相信在马剃天爱星会长的带领下，系统将会被不断完善（bushi）").arg(m_data.dataFilePath()));
}