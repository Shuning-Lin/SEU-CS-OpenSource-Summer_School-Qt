#ifndef ARTICLEEDITDIALOG_H
#define ARTICLEEDITDIALOG_H

#include <QDialog>

namespace Ui 
{
class ArticleEditDialog;
}

class ArticleEditDialog : public QDialog
{
    Q_OBJECT   //还是一样，升级

public:
    explicit ArticleEditDialog(QWidget *parent = nullptr);
    ~ArticleEditDialog() override;

    void setArticle(const QString &article,const QString &content);   //打开对话框前，把已有文章内容预填入输入框
    QString title() const;                                            //拿回标题输入框里文字
    QString content() const;                                          //拿回正文输入框里的文字

public slots:
  void accept() override;

private slots:
   void on_plainContent_textChanged();

private:
    Ui::ArticleEditDialog *ui;
};

#endif // ARTICLEEDITDIALOG_H
