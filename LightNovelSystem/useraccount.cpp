#include "useraccount.h"

#include<QCryptographicHash>

QString userRoleToKey(UserRole role)
{
    if (role==UserRole::Admin)
        return QStringLiteral("admin");
    return QStringLiteral("student");
}

QString userRoleText(UserRole role)
{
    if(role==UserRole::Admin)
        return QStringLiteral("管理员");
    return QStringLiteral("学生");
}

QString UserAccount::hashPassword(const QString &password)
{
    return QString::fromLatin1(QCryptographicHash::hash(password.toUtf8(),QCryptographicHash::Sha256).toHex());
    //return password;
    //这个就值得仔细注释一下了！
    //1.password.toUtf8()  字面意思；
    //2.SHA256著名的摘要算法（不可逆），后续如果怕忘记密码，这里可以改成原样输出；
    //3.toHex()二进制变文本
    //4.fromLatin1()字节变成QString
}

bool UserAccount::verifyPassword(const QString &password) const
{
    return m_passwordHash==hashPassword(password);
}

bool UserAccount::isValid() const
{
    return !m_account.trimmed().isEmpty() and !m_name.trimmed().isEmpty() and !m_passwordHash.isEmpty();
}

QJsonObject UserAccount::toJson() const
{
    QJsonObject object;
    object.insert("account",m_account);
    object.insert("name",m_name);
    object.insert("passwordHash",m_passwordHash);
    object.insert("role",userRoleToKey(m_role));
    return object;
}

UserAccount UserAccount::fromJson(const QJsonObject &object)
{
    UserAccount user;
    user.m_account = object.value("account").toString();
    user.m_name = object.value("name").toString();
    user.m_passwordHash = object.value("passwordHash").toString();
    user.m_role = object.value("role").toString() == QStringLiteral("admin")? UserRole::Admin: UserRole::Student;
    return user;
}