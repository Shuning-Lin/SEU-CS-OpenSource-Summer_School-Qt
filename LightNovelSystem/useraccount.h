#ifndef USERACCOUNT_H
#define USERACCOUNT_H

#include<QJsonObject>
#include<QString>

enum class UserRole//定义是学生还是管理员（默认是学生）
{
    Student,Admin
};

QString userRoleToKey(UserRole role);//转给文件看
QString userRoleText(UserRole role);//转给人看

class UserAccount
{
public:
    QString m_account;                  //账号
    QString m_name;                     //姓名
    QString m_passwordHash;             //hash，到时候要记一下已创建的账号
    UserRole m_role=UserRole::Student;  //默认身份为学生（就是普通用户啦）

    bool verifyPassword(const QString &password) const;  //验证密码
    bool isValid() const;                                //数据是否完整
    QJsonObject toJson() const;                          //把成员打包成Json对象

    static UserAccount fromJson(const QJsonObject &object);  //从Json中还原
    static QString hashPassword(const QString &password);    //明文->hash
};

#endif // USERACCOUNT_H
