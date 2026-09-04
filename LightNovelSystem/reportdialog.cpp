#include "reportdialog.h"
#include "ui_reportdialog.h"

#include<QMessageBox>

ReportDialog::ReportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReportDialog)
{
    ui->setupUi(this);
    on_plainDetail_textChanged();    //手动触发一次“0/200”
}

ReportDialog::~ReportDialog()
{
    delete ui;
}

void ReportDialog::setArticleTitle(const QString &title)
{
    ui->labelArticleTitle->setText(QStringLiteral("举报文章：《%1》").arg(title));
}

QString ReportDialog::reason() const
{
    return ui->comboReason->currentText().trimmed();
}

QString ReportDialog::detail() const
{
    return ui->plainDetail->toPlainText().trimmed();   //多行文本框内部是文档模型（不是简单存一个 QString）
}

void ReportDialog::accept()
{
    if (reason().isEmpty()) 
    {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("请选择举报原因。"));
        return;
    }
    if (detail().size() > 200) 
    {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("补充说明不能超过 200 个字符。"));
        ui->plainDetail->setFocus();
        return;
    }
    QDialog::accept();
}

void ReportDialog::on_plainDetail_textChanged()
{
    ui->labelDetailCount->setText(QStringLiteral("%1 / 200").arg(ui->plainDetail->toPlainText().size()));
}
