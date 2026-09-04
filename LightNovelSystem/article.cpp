#include "article.h"

#include <QJsonArray>

QString articleStatusToKey(ArticleStatus status)
{
    switch(status)
    {
        case ArticleStatus::Published:
            return QStringLiteral("published");
        case ArticleStatus::Rejected:
            return QStringLiteral("rejected");
        case ArticleStatus::Withdrawn:
            return QStringLiteral("withdrawn");
        case ArticleStatus::Pending:
        default:
            return QStringLiteral("pending");
    }
}

QString articleStatusText(ArticleStatus status)
{
    switch (status)
     {
        case ArticleStatus::Published:
            return QStringLiteral("已公开");
        case ArticleStatus::Rejected:
            return QStringLiteral("已驳回");
        case ArticleStatus::Withdrawn:
            return QStringLiteral("已撤回");
        case ArticleStatus::Pending:
        default:
            return QStringLiteral("待审核");
    }
}

bool Article::isValid() const
{
    return m_id > 0
           and !m_title.trimmed().isEmpty()
           and !m_content.trimmed().isEmpty()
           and !m_authorAccount.isEmpty()
           and !m_authorName.isEmpty()
           and m_createdAt.isValid()
           and m_updatedAt.isValid();
}

QJsonObject Article::toJson() const
{
    QJsonObject object;
    object.insert("id", m_id);
    object.insert("title", m_title);
    object.insert("content", m_content);
    object.insert("authorAccount", m_authorAccount);
    object.insert("authorName", m_authorName);
    object.insert("status", articleStatusToKey(m_status));
    object.insert("createdAt", m_createdAt.toString(Qt::ISODate));
    object.insert("updatedAt", m_updatedAt.toString(Qt::ISODate));
    object.insert("likedBy", QJsonArray::fromStringList(m_likedBy));
    return object;
}

Article Article::fromJson(const QJsonObject &object)
{
    Article article;
    article.m_id = object.value("id").toInt();
    article.m_title = object.value("title").toString();
    article.m_content = object.value("content").toString();
    article.m_authorAccount = object.value("authorAccount").toString();
    article.m_authorName = object.value("authorName").toString();

    //因为文本不可能直接转化为类，所以这里要特殊处理，还挺麻烦的
    const QString status = object.value("status").toString();
    if (status == QStringLiteral("published"))
        article.m_status = ArticleStatus::Published;
    else if (status == QStringLiteral("rejected"))
        article.m_status = ArticleStatus::Rejected;
    else if (status == QStringLiteral("withdrawn"))
        article.m_status = ArticleStatus::Withdrawn;
    else
        article.m_status = ArticleStatus::Pending;

    article.m_createdAt=QDateTime::fromString(object.value("createdAt").toString(),Qt::ISODate);
    article.m_updatedAt = QDateTime::fromString(object.value("updatedAt").toString(), Qt::ISODate);

    const QJsonArray likedArray = object.value("likedBy").toArray();
    for (const QJsonValue &value : likedArray)
    {
        const QString account = value.toString();
        if (!account.isEmpty() && !article.m_likedBy.contains(account))
            article.m_likedBy.append(account);
    }
    return article;
}


