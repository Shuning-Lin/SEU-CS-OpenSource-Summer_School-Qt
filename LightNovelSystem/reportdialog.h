#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>

namespace Ui 
{
class ReportDialog;
}

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportDialog(QWidget *parent = nullptr);
    ~ReportDialog() override;

    void setArticleTitle(const QString &title);   //把被举报的文章标题填入对话框
    QString reason() const;                       //获取举报原因
    QString detail() const;                       //获取补充的原因
    
public slots:
    void accept() override;
private slots:
    void on_plainDetail_textChanged();
private:
    Ui::ReportDialog *ui;
};

#endif // REPORTDIALOG_H
