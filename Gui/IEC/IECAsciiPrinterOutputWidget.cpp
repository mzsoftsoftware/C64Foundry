#include "IECAsciiPrinterOutputWidget.h"
#include "ui_IECAsciiPrinterOutputWidget.h"

IECAsciiPrinterOutputWidget::IECAsciiPrinterOutputWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::IECAsciiPrinterOutputWidget)
{
    ui->setupUi(this);
}

IECAsciiPrinterOutputWidget::~IECAsciiPrinterOutputWidget()
{
    delete ui;
}
