#ifndef ARTICLE_H
#define ARTICLE_H

#include<QDateTime>
#include<QJsonObject>
#include<QString>
#include<QStringList>

enum class ArticleStatus
{
    Pending,Published,Rejected,Withdrawn
};

QString articleStatusToKey(ArticleStatus status);   //写入Json文件
QString articleStatusText(ArticleStatus status);    //给人看的


class Article
{
public:
    int m_id=0;                                      //文章的唯一编号   
    QString m_title;                                 //标题
    QString m_content;                               //正文
    QString m_authorAccount;                         //学号
    QString m_authorName;                            //姓名
    ArticleStatus m_status=ArticleStatus::Pending;   //文章状态
    QDateTime m_createdAt;                           //创建时间
    QDateTime m_updatedAt;                           //最后修改时间
    QStringList m_likedBy;                           //点赞人的学号列表

    bool isValid() const;                               //文章检验
    QJsonObject toJson() const;                         //打包存档
    static Article fromJson(const QJsonObject &object); //加载还原
};

#endif // ARTICLE_H
