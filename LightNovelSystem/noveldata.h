#ifndef NOVELDATA_H
#define NOVELDATA_H

#include "article.h"
#include "report.h"
#include "useraccount.h"

#include<QStringList>
#include<QVector>

class NovelData
{
public:
    NovelData();

    bool load(QString *error=nullptr);   //从文件读取数据->内存
    bool save(QString *error=nullptr) const;   //内存数据->写回文件
    QString dataFilePath() const;        //问一下文件在哪里

    //以下这四个函数是针对私有数据成员访问而专门设置的
    const QVector<UserAccount> &users() const;    //读取用户
    const QVector<Article> &articles() const;     //读取文章
    const QVector<Report> &reports() const;       //读取举报
    const QStringList &forbiddenWords() const;    //读取违禁词

    //3个业务，两个版本
    UserAccount *findUser(const QString &account);             //按账号查找用户（可改）
    const UserAccount *findUser(const QString &account) const; //按账号查找用户（只读）
    Article *findArticle(int articleId);                       //按编号查找文章（可改） 
    const Article *findArticle(int articleId) const;           //按编号查找文章（只读）
    Report *findReport(int reportId);                          //按编号查找举报（可改）
    const Report *findReport(int reportId) const;              //按编号查找举报（只读）

    bool checkLogin(const QString &account, const QString &password,QString *error = nullptr) const;                     //登录（只读，不改变数据）
    bool registerStudent(const QString &studentId, const QString &name,const QString &password,QString *error = nullptr);//注册，写数据，要保存

    QString findForbiddenWord(const QString &title,  const QString &content) const;//可以在标题和内容中找到违禁词

    bool addArticle(const QString &authorAccount,const QString &title, const QString &content,int *newArticleId = nullptr, QString *error = nullptr);//添加文章
    bool updateArticle(int articleId, const QString &authorAccount,const QString &title, const QString &content,QString *error = nullptr);           //就该文章
    bool toggleLike(int articleId, const QString &studentAccount,bool *nowLiked = nullptr,QString *error = nullptr);                                 //点赞or取消点赞
    bool addReport(int articleId, const QString &reporterAccount,const QString &reason, const QString &detail,QString *error = nullptr);             //举报

    bool approveArticle(int articleId, const QString &adminAccount,QString *error = nullptr);                      //审核通过（有二次检查哦）
    bool rejectArticle(int articleId, const QString &adminAccount,QString *error = nullptr);                       //驳回文章
    bool withdrawArticle(int articleId, const QString &adminAccount,QString *error = nullptr);                     //撤回文章
    bool handleReport(int reportId, const QString &adminAccount,bool withdrawArticleToo,QString *error = nullptr); //处理举报

    bool deleteArticle(int articleId,const QString &adminAccount,QString *error);

    bool addForbiddenWord(const QString &word, const QString &adminAccount,QString *error = nullptr);     //添加违禁词
    bool removeForbiddenWord(const QString &word,const QString &adminAccount,QString *error = nullptr);   //移除违禁词
private:
    bool isAdmin(const QString &account) const;  //是否为管理员（马剃天爱星）
    bool ensureDefaultAdmin();                   //系统有管理员吗

    QString m_filePath;
    QVector<UserAccount> m_users;
    QVector<Article> m_articles;
    QVector<Report> m_reports;
    QStringList m_forbiddenWords;

    int m_nextArticleId=1;
    int m_nextReportId=1;
};

#endif // NOVELDATA_H
