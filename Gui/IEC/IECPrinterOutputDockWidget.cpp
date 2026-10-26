#include "IECPrinterOutputDockWidget.h"
#include "ui_IECPrinterOutputDockWidget.h"

IECPrinterOutputDockWidget::IECPrinterOutputDockWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::IECPrinterOutputDockWidget)
{
    ui->setupUi(this);
}

IECPrinterOutputDockWidget::~IECPrinterOutputDockWidget()
{
    delete ui;
}
