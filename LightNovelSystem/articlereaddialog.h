#ifndef ARTICLEREADDIALOG_H
#define ARTICLEREADDIALOG_H

#include <QDialog>

class Article;

namespace Ui 
{
class ArticleReadDialog;
}

class ArticleReadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ArticleReadDialog(QWidget *parent = nullptr);
    ~ArticleReadDialog() override;

    void setArticle(const Article &article);   //把一篇文章的所有信息停入显示框

private:
    Ui::ArticleReadDialog *ui;
};

#endif // ARTICLEREADDIALOG_H
