#include "report.h"

//代码风格突变，不用怀疑，就是让agent代劳的
//实际上Report、Article、UserAccount结构几乎一摸一样，所以这里就偷了一点点小懒了

QString reportStatusToKey(ReportStatus status)
{
    return status == ReportStatus::Handled
               ? QStringLiteral("handled")
               : QStringLiteral("pending");
}

QString reportStatusText(ReportStatus status)
{
    return status == ReportStatus::Handled
               ? QStringLiteral("已处理")
               : QStringLiteral("待处理");
}

bool Report::isValid() const
{
    return m_id > 0
           && m_articleId > 0
           && !m_reporterAccount.isEmpty()
           && !m_reason.trimmed().isEmpty()
           && m_createdAt.isValid();
}

QJsonObject Report::toJson() const
{
    QJsonObject object;
    object.insert("id", m_id);
    object.insert("articleId", m_articleId);
    object.insert("reporterAccount", m_reporterAccount);
    object.insert("reason", m_reason);
    object.insert("detail", m_detail);
    object.insert("createdAt", m_createdAt.toString(Qt::ISODate));
    object.insert("status", reportStatusToKey(m_status));
    return object;
}

Report Report::fromJson(const QJsonObject &object)
{
    Report report;
    report.m_id = object.value("id").toInt();
    report.m_articleId = object.value("articleId").toInt();
    report.m_reporterAccount = object.value("reporterAccount").toString();
    report.m_reason = object.value("reason").toString();
    report.m_detail = object.value("detail").toString();
    report.m_createdAt = QDateTime::fromString(
        object.value("createdAt").toString(), Qt::ISODate);
    report.m_status = object.value("status").toString()
                              == QStringLiteral("handled")
                          ? ReportStatus::Handled
                          : ReportStatus::Pending;
    return report;
}
