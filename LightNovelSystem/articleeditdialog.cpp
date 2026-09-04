#include "articleeditdialog.h"
#include "ui_articleeditdialog.h"

#include <QMessageBox>

ArticleEditDialog::ArticleEditDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ArticleEditDialog)
{
    ui->setupUi(this);
    on_plainContent_textChanged();     //这里稍微注意一下
}

ArticleEditDialog::~ArticleEditDialog()
{
    delete ui;
}

void ArticleEditDialog::setArticle(const QString &title,const QString &content)
{
    ui->editTitle->setText(title);
    ui->plainContent->setPlainText(content);
}

QString ArticleEditDialog::title() const
{
    return ui->editTitle->text().trimmed();
}

QString ArticleEditDialog::content() const
{
    return ui->plainContent->toPlainText().trimmed();
}

void ArticleEditDialog::accept()
{
    if (title().isEmpty() or title().size() > 50) 
    {
        QMessageBox::warning(this, QStringLiteral("输入错误"), QStringLiteral("标题长度应为 1-50 个字符。"));
        ui->editTitle->setFocus();
        return;
    }
    if (content().isEmpty() or content().size() > 20000) 
    {
        QMessageBox::warning(this, QStringLiteral("输入错误"),QStringLiteral("正文长度应为 1-20000 个字符。"));
        ui->plainContent->setFocus();
        return;
    }
    QDialog::accept();
}

void ArticleEditDialog::on_plainContent_textChanged()
{
    ui->labelCharCount->setText(QStringLiteral("%1 / 20000").arg(ui->plainContent->toPlainText().size()));
}

//ps:对话框只检查空值和长度，真正的违禁词检查由其他函数完成
