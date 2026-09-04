#ifndef REPORT_H
#define REPORT_H

#include<QDateTime>
#include<QJsonObject>
#include<QString>

enum class ReportStatus
{
    Pending,Handled
};

QString reportStatusToKey(ReportStatus status);
QString reportStatusText(ReportStatus status);

class Report
{
public:
    int m_id=0;                                    //举报记录的编号
    int m_articleId=0;                             //举报文章的编号
    QString m_reporterAccount;                     //举报者的学号
    QString m_reason;                              //举报原因
    QString m_detail;                              //补充说明
    QDateTime m_createdAt;                         //提交时间
    ReportStatus m_status = ReportStatus::Pending; //处理状态


    bool isValid()const;                               //是否合法
    QJsonObject toJson() const;                        //写入Json
    static Report fromJson(const QJsonObject &object); //从Json中读取
};
#endif 
