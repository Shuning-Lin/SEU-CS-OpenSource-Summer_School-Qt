#include "articlereaddialog.h"
#include "ui_articlereaddialog.h"

#include "article.h"

ArticleReadDialog::ArticleReadDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ArticleReadDialog)
{
    ui->setupUi(this);
    ui->plainContent->setReadOnly(true);    //字面意思，把正文框设置为“只读”————用户能看，能选，能滚动但是不能修改
}

ArticleReadDialog::~ArticleReadDialog()
{
    delete ui;
}

void ArticleReadDialog::setArticle(const Article &article)
{
    ui->labelTitle->setText(article.m_title);
    ui->labelMeta->setText(QStringLiteral("作者：%1(%2)   更新时间：%3    点赞：%4     状态：%5")
    .arg(article.m_authorName).arg(article.m_authorAccount).arg(article.m_updatedAt.toString("yyyy-MM-dd HH:mm")).arg(article.m_likedBy.size()).arg(articleStatusText(article.m_status)));
    ui->plainContent->setPlainText(article.m_content);
}