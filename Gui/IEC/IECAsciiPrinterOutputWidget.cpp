#include "IECAsciiPrinterOutputWidget.h"
#include "ui_IECAsciiPrinterOutputWidget.h"

#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QTextCursor>


IECAsciiPrinterOutputWidget::IECAsciiPrinterOutputWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECAsciiPrinterOutputWidget)
{
    ui->setupUi(this);

    ui->plainTextEdit_Paper->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

IECAsciiPrinterOutputWidget::~IECAsciiPrinterOutputWidget()
{
    delete ui;
}

void IECAsciiPrinterOutputWidget::setPrinterName(const QString& name)
{
    ui->label_PrinterName->setText(name);
}

void IECAsciiPrinterOutputWidget::setOutput(const QByteArray& output)
{
    ui->plainTextEdit_Paper->setPlainText(QString::fromLatin1(output));
    ui->plainTextEdit_Paper->moveCursor(QTextCursor::End);
}
