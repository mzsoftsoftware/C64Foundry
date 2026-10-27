#include "IECFoundryDiskRowWidget.h"
#include "ui_IECFoundryDiskRowWidget.h"

IECFoundryDiskRowWidget::IECFoundryDiskRowWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::IECFoundryDiskRowWidget)
{
    ui->setupUi(this);
}

IECFoundryDiskRowWidget::~IECFoundryDiskRowWidget()
{
    delete ui;
}
