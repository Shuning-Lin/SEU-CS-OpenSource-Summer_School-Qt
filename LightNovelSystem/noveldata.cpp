#include "noveldata.h"

//服了，头文件真多。。。
#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>

//这里也可以用老一辈的static写法
namespace 
{
void setError(QString *error, const QString &message)    //用到的时候注意一下
{
    if (error)
        *error = message;
}

bool containsCaseInsensitive(const QStringList &list, const QString &text)//忽略大小写的查找，检查是否为已经添加过的违禁词
{
    for (const QString &item : list)
    {
        if (item.compare(text, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

bool validArticleStatusKey(const QString &key)   //检查文章是否出于一个合法的状态
{
    return key == QStringLiteral("pending") or key == QStringLiteral("published") or key == QStringLiteral("rejected") or key == QStringLiteral("withdrawn");
}

bool validReportStatusKey(const QString &key)    //检查举报是否出于一个合法的状态
{
    return key == QStringLiteral("pending") or key == QStringLiteral("handled");
}
} 
////////////////////////

NovelData::NovelData()
{
    // 提交版把数据保存在 exe 同目录，整个文件夹复制到其他电脑后仍可读写。
    // 不依赖当前工作目录，双击程序或通过快捷方式启动时使用同一份数据。
    m_filePath=QDir(QCoreApplication::applicationDirPath())
                   .filePath(QStringLiteral("novel_data.json"));
}

//以下五个函数没有含金量
QString NovelData::dataFilePath() const
{
    return m_filePath;
}

const QVector<UserAccount> &NovelData::users() const
{
    return m_users;
}

const QVector<Article> &NovelData::articles() const
{
    return m_articles;
}

const QVector<Report> &NovelData::reports() const
{
    return m_reports;
}

const QStringList &NovelData::forbiddenWords() const
{
    return m_forbiddenWords;
}
/////////////
UserAccount *NovelData::findUser(const QString &account) 
{
    for(UserAccount &user:m_users)
    {
        if(user.m_account==account)
            return &user;
    }
    return nullptr;
}

const UserAccount *NovelData::findUser(const QString &account) const
{
    for (const UserAccount &user : m_users) 
    {
        if (user.m_account == account)
            return &user;
    }
    return nullptr;
}

Article *NovelData::findArticle(int articleId)
{
    for (Article &article : m_articles) {
        if (article.m_id == articleId)
            return &article;
    }
    return nullptr;
}

const Article *NovelData::findArticle(int articleId) const
{
    for (const Article &article : m_articles) {
        if (article.m_id == articleId)
            return &article;
    }
    return nullptr;
}

Report *NovelData::findReport(int reportId)
{
    for (Report &report : m_reports) {
        if (report.m_id == reportId)
            return &report;
    }
    return nullptr;
}

const Report *NovelData::findReport(int reportId) const
{
    for (const Report &report : m_reports) {
        if (report.m_id == reportId)
            return &report;
    }
    return nullptr;
}

bool NovelData::isAdmin(const QString &account) const
{
    const UserAccount *user=findUser(account);
    return user and user->m_role==UserRole::Admin;
}

bool NovelData::ensureDefaultAdmin()
{
    for (const UserAccount &user : m_users) {
        if (user.m_role == UserRole::Admin)
            return false;
    }
    UserAccount admin;
    admin.m_account=QStringLiteral("admin");
    admin.m_name=QString("马剃天爱星");
    admin.m_passwordHash=UserAccount::hashPassword(QStringLiteral("0314"));//另一位我喜欢角色的生日
    admin.m_role=UserRole::Admin;
    m_users.append(admin);
    return true;
}

//呼呼，接下来将进入最难的环节
//23.完整读取JSON
bool NovelData::load(QString *error)
{
    QFile file(m_filePath);
    if(!file.exists())      //如果文件第一次运行
    {
        m_users.clear();
        m_articles.clear();
        m_reports.clear();
        m_forbiddenWords={QStringLiteral("违禁示例词"),QStringLiteral("不良示例词")};
        m_nextArticleId=1;
        m_nextReportId=1;
        ensureDefaultAdmin();   //巧妙利用已有的函数进行构建（狗头）
        return save(error);
    }

    if(!file.open(QIODevice::ReadOnly))
    {
        setError(error,QStringLiteral("无法读取数据文件：%1").arg(file.errorString()));
        return false;
    }

    QJsonParseError parseError;        //JSON的解析报告
    const QJsonDocument document=QJsonDocument::fromJson(file.readAll(),&parseError);
    file.close();

    if(parseError.error!=QJsonParseError::NoError or !document.isObject())
    {
        setError(error,QStringLiteral("JSON格式错误：%1").arg(parseError.errorString()));
        return false;
    }

    const QJsonObject root=document.object();
    if(!root.value("users").isArray() or !root.value("articles").isArray() or !root.value("reports").isArray() or !root.value("forbiddenWords").isArray())
    {
        setError(error, QStringLiteral("数据文件缺少必要数组。"));
        return false;
    }

    QVector<UserAccount> loadedUsers;
    QVector<Article> loadedArticles;
    QVector<Report> loadedReports;
    QStringList loadedWords;

    //其实不得不说Codex的指导是在是太严谨了
    QSet<QString> accountSet;
    const QJsonArray userArray=root.value("users").toArray();
    for(int i=0;i<userArray.size();i++)
    {
        if(!userArray.at(i).isObject())
        {
            setError(error,QStringLiteral("第%1个账号不是对象。").arg(i+1));
            return false;
        }
        //我真的受不了了，是在是太太太严谨了，连后台篡改身份信息出错都给考虑到了。。。
        const QJsonObject object =userArray.at(i).toObject();
        const QString roleKey=object.value("role").toString();
        if(roleKey!=QStringLiteral("student") and roleKey!=QStringLiteral("admin"))
        {
            setError(error,QStringLiteral("第 %1 个账号角色非法").arg(i+1));
            return false;
        }

        UserAccount user =UserAccount::fromJson(object);
        if(!user.isValid() or accountSet.contains(user.m_account))
        {
            setError(error,QStringLiteral("第 %1 个账号非法或者重复。").arg(i+1));
            return false;
        }
        accountSet.insert(user.m_account);
        loadedUsers.append(user);
    }

    const auto loadedUserByAccount=[&loadedUsers](const QString &account)->const UserAccount*
    {
        for(const UserAccount &user:loadedUsers)
        {
            if(user.m_account==account)
                return &user;
        }
        return nullptr;
    };

    QSet<int> articleIdSet;
    int maxArticleId=0;
    const QJsonArray articleArray=root.value("articles").toArray();
    for(int i=0;i<articleArray.size();i++)
    {
        if (!articleArray.at(i).isObject())
        {
            setError(error, QStringLiteral("第 %1 篇文章不是对象。").arg(i + 1));
            return false;
        }
        
        const QJsonObject object = articleArray.at(i).toObject();
        if (!validArticleStatusKey(object.value("status").toString())) 
        {
            setError(error, QStringLiteral("第 %1 篇文章状态非法。").arg(i + 1));
            return false;
        }

        Article article=Article::fromJson(object);
        const UserAccount *author=loadedUserByAccount(article.m_authorAccount);
        bool likedAccountsValid=true;
        for(const QString &account:article.m_likedBy)
        {
            const UserAccount *likedUser=loadedUserByAccount(account);
            if(!likedUser or likedUser->m_role!=UserRole::Student)
            {
                likedAccountsValid=false;
                break;
            }
        }
        if (!article.isValid()
            or article.m_title.size() > 50
            or article.m_content.size() > 20000
            or articleIdSet.contains(article.m_id)
            or !author 
            or author->m_role != UserRole::Student
            or article.m_authorName != author->m_name
            or !likedAccountsValid)
        {
            setError(error,QStringLiteral("第 %1 篇文章数据非法。").arg(i + 1));
            return false;
        }

        articleIdSet.insert(article.m_id);
        maxArticleId=qMax(maxArticleId,article.m_id);
        loadedArticles.append(article);
    }
    
    QSet<int> reportIdSet;
    QSet<QString> reportPairSet;
    int maxReportId = 0;
    const QJsonArray reportArray = root.value("reports").toArray();
    for (int i = 0; i < reportArray.size(); ++i) 
    {
        if (!reportArray.at(i).isObject()) 
        {
            setError(error, QStringLiteral("第 %1 条举报不是对象。").arg(i + 1));
            return false;
        }

        const QJsonObject object = reportArray.at(i).toObject();
        if (!validReportStatusKey(object.value("status").toString())) 
        {
            setError(error, QStringLiteral("第 %1 条举报状态非法。").arg(i + 1));
            return false;
        }

        Report report = Report::fromJson(object);
        const UserAccount *reporter =loadedUserByAccount(report.m_reporterAccount);
        const QString reportPair =QString::number(report.m_articleId)+ QLatin1Char(':') + report.m_reporterAccount;
        if (!report.isValid()
            or report.m_detail.size() > 200
            or reportIdSet.contains(report.m_id)
            or reportPairSet.contains(reportPair)
            or !articleIdSet.contains(report.m_articleId)
            or !reporter 
            or reporter->m_role != UserRole::Student) 
            {
            setError(error, QStringLiteral("第 %1 条举报数据非法。").arg(i + 1));
            return false;
            }

        reportIdSet.insert(report.m_id);
        reportPairSet.insert(reportPair);
        maxReportId = qMax(maxReportId, report.m_id);
        loadedReports.append(report);
    }
    const QJsonArray wordArray = root.value("forbiddenWords").toArray();
    for (const QJsonValue &value : wordArray) 
    {
        const QString word = value.toString().trimmed();
        if (!word.isEmpty() && !containsCaseInsensitive(loadedWords, word))
            loadedWords.append(word);
    }
    bool hasAdmin = false;
    for (const UserAccount &user : loadedUsers) {
        if (user.m_role == UserRole::Admin) {
            hasAdmin = true;
            break;
        }
    }
    if (!hasAdmin && accountSet.contains(QStringLiteral("admin"))) {
        setError(error,
                 QStringLiteral("账号 admin 已被非管理员占用，无法补建管理员。"));
        return false;
    }

    m_users = loadedUsers;
    m_articles = loadedArticles;
    m_reports = loadedReports;
    m_forbiddenWords = loadedWords;
    m_nextArticleId=qMax(root.value("nextArticleId").toInt(1),maxArticleId+1);
    m_nextReportId=qMax(root.value("nextReportId").toInt(1),maxReportId+1);

    if(ensureDefaultAdmin())
        return save(error);
    return true;
}

bool NovelData::save(QString *error) const
{
    const QFileInfo fileInfo(m_filePath);
    QDir directory=fileInfo.dir();

    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) 
    {
        setError(error, QStringLiteral("无法创建数据目录：%1").arg(directory.absolutePath()));
        return false;
    }

    QJsonArray userArray;
    for (const UserAccount &user : m_users)
        userArray.append(user.toJson());

    QJsonArray articleArray;
    for (const Article &article : m_articles)
        articleArray.append(article.toJson());

    QJsonArray reportArray;
    for (const Report &report : m_reports)
        reportArray.append(report.toJson());

    QJsonObject root;
    root.insert("schemaVersion", 1);
    root.insert("nextArticleId", m_nextArticleId);
    root.insert("nextReportId", m_nextReportId);
    root.insert("users", userArray);
    root.insert("articles", articleArray);
    root.insert("reports", reportArray);
    root.insert("forbiddenWords",QJsonArray::fromStringList(m_forbiddenWords));

   QSaveFile file(m_filePath);     //安全写入文件
   if(!file.open(QIODevice::WriteOnly))
   {
    setError(error,QStringLiteral("无法写入数据文件：%1").arg(file.errorString()));
    return false;
   }
   
   const QByteArray bytes =QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size() || !file.commit())                          //我是被逼的，Codex老师说这里有可能断电（呵呵）
    {
        setError(error, QStringLiteral("保存数据失败：%1") .arg(file.errorString()));
        return false;
    }
    return true;
}

//QSaveFile 会先写临时文件，成功后再替换旧文件，比直接覆盖更不容易在断电或写入失败时留下半份 JSON。
//用法仍与 QFile 很接近。

//25.注册于登录业务
bool NovelData::checkLogin(const QString &account,const QString &password,QString *error) const
{
    const UserAccount *user=findUser(account.trimmed());   //trimmed()去掉字符串首尾的空白
    if (!user or !user->verifyPassword(password))
    {
        setError(error, QStringLiteral("账号或密码错误。"));
        return false;
    }
    return true;
}
bool NovelData::registerStudent(const QString &studentId,const QString &name,const QString &password,QString *error)
{
    static const QRegularExpression idPattern( QStringLiteral("^[0-9]{9}$"));
    const QString id =studentId.trimmed();
    const QString realName=name.trimmed();
    if(!idPattern.match(id).hasMatch())
    {
        setError(error,QStringLiteral("学号必须为9位数字(比如213020001)"));
        return false;
    }
    if(realName.isEmpty() or realName.size()>20)
    {
        setError(error,QStringLiteral("姓名长度应为1-20个字符。"));
        return false;
    }
    if(password.size()<6 or password.size()>20)    //其实有意思的是，管理员的密码实际上并没有这种限制
    {
        setError(error,QStringLiteral("密码长度应为6-20个字符。"));
        return false;
    }
    if(findUser(id))
    {
        setError(error,QStringLiteral("该学号已经被注册。"));
        return false;
    }

    UserAccount user;
    user.m_account = id;
    user.m_name = realName;
    user.m_passwordHash = UserAccount::hashPassword(password);
    user.m_role = UserRole::Student;
    m_users.append(user);

    if(!save(error))
    {
        m_users.removeLast();
        return false;
    }
    return true;
}
//即使注册对话框已经检查过输入，数据层仍再检查一次。界面隐藏或输入框限制不能替代真正的
//业务校验。

//26.违禁词的检查、投稿和修改
QString NovelData::findForbiddenWord(const QString &title,const QString &content) const
{
    const QString allText = title +QLatin1Char('\n')+content;
    for(const QString &rawWord:m_forbiddenWords)
    {
        const QString word =rawWord.trimmed();
        if(!word.isEmpty() and allText.contains(word,Qt::CaseInsensitive))    //这里要小心空字符串陷阱,还有不区分大小写
        {
            return word;
        }
    }
    return QString();
}

bool NovelData::addArticle(const QString &authorAccount,const QString &title,const QString &content,int *newArticleId,QString *error)
{
   const UserAccount *author=findUser(authorAccount); 
   if (!author || author->m_role != UserRole::Student)   //管理员不能投稿哟 
   {
        setError(error, QStringLiteral("只有学生账号可以投稿。"));
        return false;
   }

    const QString cleanTitle = title.trimmed();
    const QString cleanContent = content.trimmed();
    if (cleanTitle.isEmpty() or cleanTitle.size() > 50) 
    {
        setError(error, QStringLiteral("标题长度应为 1-50 个字符。"));
        return false;
    }
    if (cleanContent.isEmpty() or cleanContent.size() > 20000) 
    {
        setError(error, QStringLiteral("正文长度应为 1-20000 个字符。"));
        return false;
    }
    const QString matched = findForbiddenWord(cleanTitle, cleanContent);
    if(!matched.isEmpty())
    {
         setError(error,QStringLiteral("提交失败：文章命中违禁词“%1”。(V我50,我考虑一下解禁,咕咕嘎嘎)").arg(matched));
        return false;
    }

    Article article;
    article.m_id = m_nextArticleId++;
    article.m_title = cleanTitle;
    article.m_content = cleanContent;
    article.m_authorAccount = author->m_account;
    article.m_authorName = author->m_name;
    article.m_status = ArticleStatus::Pending;
    article.m_createdAt = QDateTime::currentDateTime();
    article.m_updatedAt = article.m_createdAt;
    m_articles.append(article);

    if(!save(error))            //Codex老师说要防断电
    {
        m_articles.removeLast();
        //千万要注意
        m_nextArticleId--;
        return false;
    }

    if(newArticleId)                //这里传的指针不可以是空指针！！不然程序会崩溃
        *newArticleId=article.m_id;
    return true;
}

bool NovelData::updateArticle(int articleId,const QString &authorAccount,const QString &title,const QString &content,QString *error)
{
    Article *article=findArticle(articleId);
    if (!article)
    {
        setError(error, QStringLiteral("找不到该文章。"));  //这个不太可能，但是仍然是Codex老师的建议
        return false;
    }
    if (article->m_authorAccount != authorAccount) {
        setError(error, QStringLiteral("只能修改自己创作的文章。"));
        return false;
    }
    const QString cleanTitle = title.trimmed();
    const QString cleanContent = content.trimmed();
    if (cleanTitle.isEmpty() or cleanTitle.size() > 50 
    or cleanContent.isEmpty() or cleanContent.size() > 20000)
    {
        setError(error, QStringLiteral("标题或正文长度不合法。"));
        return false;
    }

    const QString matched = findForbiddenWord(cleanTitle, cleanContent);
    if (!matched.isEmpty()) 
    {
        setError(error,QStringLiteral("修改失败：文章命中违禁词“%1”,但旧内容未改变。").arg(matched));
        return false;
    }

    const Article oldArticle = *article;     //你Codex老师的防断电操作又来啦
    article->m_title = cleanTitle;
    article->m_content = cleanContent;
    article->m_status = ArticleStatus::Pending;
    article->m_updatedAt = QDateTime::currentDateTime();

    if (!save(error))
    {
        *article = oldArticle;
        return false;
    }
    return true;
}
//注意修改流程先完成所有检查，最后才改原对象。命中违禁词时，已公开的旧文章仍保持原内容和原状态。
//27.点赞和举报
bool NovelData::toggleLike(int articleId,const QString &studentAccount,bool *nowLiked,QString *error)
{
    const UserAccount *student=findUser(studentAccount);
    Article *article=findArticle(articleId);
    if(!student or student->m_role!=UserRole::Student)     //可怜的管理员也不能点赞
    {
        setError(error, QStringLiteral("只有学生账号可以点赞。"));
        return false;
    }
    if (!article or article->m_status != ArticleStatus::Published)   //你Codex老师大手又又又发力了
    {
        setError(error, QStringLiteral("只能点赞已公开文章。"));
        return false;
    }
    const Article oldArticle = *article;
    const bool alreadyLiked = article->m_likedBy.contains(studentAccount);
    if (alreadyLiked)
        article->m_likedBy.removeAll(studentAccount);
    else
        article->m_likedBy.append(studentAccount);

    if (!save(error)) 
    {
        *article = oldArticle;
        return false;
    }
    if (nowLiked)
        *nowLiked = !alreadyLiked;    
    return true;
}

bool NovelData::addReport(int articleId,const QString &reporterAccount,const QString &reason,const QString &detail,QString *error)
{
    const UserAccount *student=findUser(reporterAccount);
    const Article *article=findArticle(articleId);
    if (!student or student->m_role != UserRole::Student) 
    {
        setError(error, QStringLiteral("只有学生账号可以举报。"));
        return false;
    }
    //Codex老师稳定发挥
    if (!article or article->m_status != ArticleStatus::Published) 
    {
        setError(error, QStringLiteral("只能举报已公开文章。"));
        return false;
    }
    for(const Report &report:m_reports)
    {
        if (report.m_articleId == articleId and report.m_reporterAccount == reporterAccount) 
        {
            setError(error, QStringLiteral("你已经举报过这篇文章。"));
            return false;
        }
    }
    const QString cleanReason = reason.trimmed();
    const QString cleanDetail = detail.trimmed();
    if (cleanReason.isEmpty() or cleanDetail.size() > 200)      //这里其实我有点不满意，后续再看看吧
    {
        setError(error, QStringLiteral("举报原因或补充说明不合法。"));
        return false;
    }

    Report report;
    report.m_id = m_nextReportId++;
    report.m_articleId = articleId;
    report.m_reporterAccount = reporterAccount;
    report.m_reason = cleanReason;
    report.m_detail = cleanDetail;
    report.m_createdAt = QDateTime::currentDateTime();
    report.m_status = ReportStatus::Pending;
    m_reports.append(report);

    if(!save(error))
    {
        m_reports.removeLast();
        m_nextReportId--;
        return false;
    }
    return true;
}
//一次点赞操作在“已点赞”和“未点赞”之间切换，所以同一学生不会重复计数。
//举报则限制为同一学生对同一文章只能提交一次

//28.管理员审核、撤回和处理举报
bool NovelData::approveArticle(int articleId,const QString &adminAccount,QString *error)
{
    if (!isAdmin(adminAccount))     //我觉得不可能发生，但是Codex老师说加
    {
        setError(error, QStringLiteral("只有管理员可以审核文章。"));
        return false;
    }

    Article *article=findArticle(articleId);
    if (!article || article->m_status != ArticleStatus::Pending) 
    {
        setError(error, QStringLiteral("只有待审核文章可以通过。"));
        return false;
    }
    
    const QString matched = findForbiddenWord(article->m_title,article->m_content);
    if (!matched.isEmpty())
    {
        setError(error,QStringLiteral("不能通过：文章命中当前违禁词“%1”。").arg(matched));
        return false;
    }
    const ArticleStatus oldStatus = article->m_status;
    article->m_status = ArticleStatus::Published;
    if (!save(error))
    {
        article->m_status = oldStatus;
        return false;
    }
    return true;
}

bool NovelData::rejectArticle(int articleId,const QString &adminAccount,QString *error)
{
    if (!isAdmin(adminAccount))
    {
        setError(error, QStringLiteral("只有管理员可以驳回文章。"));
        return false;
    }

    Article *article = findArticle(articleId);
    if (!article or article->m_status != ArticleStatus::Pending)
    {
        setError(error, QStringLiteral("只有待审核文章可以驳回。"));
        return false;
    }

    article->m_status = ArticleStatus::Rejected;
    if (!save(error))
    {
        article->m_status = ArticleStatus::Pending;
        return false;
    }
    return true;
}

//下面这个函数的逻辑基本上和上面这个如出一辙
bool NovelData::withdrawArticle(int articleId,const QString &adminAccount,QString *error)
{
    if (!isAdmin(adminAccount)) 
    {
        setError(error, QStringLiteral("只有管理员可以撤回文章。"));
        return false;
    }

    Article *article = findArticle(articleId);
    if (!article or article->m_status != ArticleStatus::Published) 
    {
        setError(error, QStringLiteral("只有已公开文章可以撤回。"));
        return false;
    }

    article->m_status = ArticleStatus::Withdrawn;
    if (!save(error))
    {
        article->m_status = ArticleStatus::Published;
        return false;
    }
    return true;
}

bool NovelData::handleReport(int reportId,const QString &adminAccount,bool withdrawArticleToo,QString *error)
{
    if (!isAdmin(adminAccount)) 
    {
        setError(error, QStringLiteral("只有管理员可以处理举报。"));   //实则非管理员根本看不到举报界面。。。
        return false;
    }

    Report *report = findReport(reportId);
    if (!report or report->m_status != ReportStatus::Pending)
    {
        setError(error, QStringLiteral("只能处理待处理举报。"));   //这里我有点想法
        return false;
    }

    Article *article = findArticle(report->m_articleId);
    if (!article) 
    {
        setError(error, QStringLiteral("找不到被举报文章。"));
        return false;
    }

    if (withdrawArticleToo and article->m_status != ArticleStatus::Published)
     {
        setError(error, QStringLiteral("文章已不是公开状态，无需再次撤回。"));
        return false;
    }

    const ReportStatus oldReportStatus = report->m_status;
    const ArticleStatus oldArticleStatus = article->m_status;
    report->m_status = ReportStatus::Handled;
    if (withdrawArticleToo)
        article->m_status = ArticleStatus::Withdrawn;

    if (!save(error)) 
    {
        report->m_status = oldReportStatus;
        article->m_status = oldArticleStatus;
        return false;
    }
    return true;
}

bool NovelData::deleteArticle(int articleId,const QString &adminAccount,QString *error)
{
    if (!isAdmin(adminAccount)) 
    {
        setError(error,QStringLiteral("只有管理员可以删除文章"));
        return false;
    }
    Article* article=findArticle(articleId);
    if(!article)
    {
        setError(error,QStringLiteral("找不到该文章"));
        return false;
    }
    for (int i = m_reports.size() - 1; i >= 0; --i)      // 从后往前删举报
    {
        if (m_reports.at(i).m_articleId == articleId)
            m_reports.removeAt(i);
    }
    const int index=int(article-m_articles.data());
    m_articles.takeAt(index);
    if(!save(error))
        return false;
    return true;
}


//举报本身不会自动删除或撤回文章，
//避免一个学生点击举报就直接影响所有读者。最终决定由管理员完成。

//29.管理违禁词
bool NovelData::addForbiddenWord(const QString &word,const QString &adminAccount,QString *error)
{
    if (!isAdmin(adminAccount)) 
    {
        setError(error, QStringLiteral("只有管理员可以设置违禁词。"));
        return false;
    }
    const QString cleanWord = word.trimmed();
    if (cleanWord.isEmpty()) 
    {
        setError(error, QStringLiteral("违禁词不能为空。"));
        return false;
    }
    if (cleanWord.size() > 30)
    {
        setError(error, QStringLiteral("单个违禁词不能超过 30 个字符。"));
        return false;
    }
    if (containsCaseInsensitive(m_forbiddenWords, cleanWord)) 
    {
        setError(error, QStringLiteral("该违禁词已经存在。"));
        return false;
    }
    m_forbiddenWords.append(cleanWord);
    if (!save(error)) 
    {
        m_forbiddenWords.removeLast();
        return false;
    }
    return true;
}

bool NovelData::removeForbiddenWord(const QString &word,const QString &adminAccount,QString *error)
{
    if (!isAdmin(adminAccount))
    {
        setError(error, QStringLiteral("只有管理员可以删除违禁词。"));  //其实普通用户根本看不到。。。
        return false;
    }
    int foundIndex=-1;
    for(int i=0;i<m_forbiddenWords.size();i++)
    {
        if(m_forbiddenWords.at(i).compare(word,Qt::CaseInsensitive)==0)
        {
            foundIndex=i;
            break;
        }
    }
    if(foundIndex==-1)
    {
        setError(error,QStringLiteral("找不到该违禁词。"));
        return false;
    }
    const QString removed =m_forbiddenWords.takeAt(foundIndex);
    if(!save(error))
    {
        m_forbiddenWords.insert(foundIndex,removed);//又是一个小巧思,如果有时间的话可以把foundIndex化成word试试
        return false;
    }
    return true;
}
