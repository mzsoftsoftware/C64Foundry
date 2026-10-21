#include "IECDeviceRowWidget.h"
#include "ui_IECDeviceRowWidget.h"

#include <QSignalBlocker>

IECDeviceRowWidget::IECDeviceRowWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECDeviceRowWidget)
{
    ui->setupUi(this);
}

IECDeviceRowWidget::~IECDeviceRowWidget()
{
    delete ui;
}

void IECDeviceRowWidget::setConfiguration(const IECDeviceConfiguration& configuration)
{
    m_deviceId = configuration.id();

    QSignalBlocker blocker(ui->toolButton_Power);

    ui->toolButton_Power->setChecked(configuration.enabled());
    ui->label_DeviceNumber->setText(QStringLiteral("#%1").arg(configuration.deviceNumber()));

    switch (configuration.type())
    {
    case IECDeviceType::PrinterASCII:
        ui->label_DeviceType->setText(tr("ASCII Printer"));
        break;
    }
}

IECDeviceConfiguration::DeviceId IECDeviceRowWidget::deviceId() const
{
    return m_deviceId;
}

void IECDeviceRowWidget::on_toolButton_Power_toggled(bool checked)
{
    if (ui->widget_OperationStatus->state() == IECOperationStatusWidget::State::Pending)
        return;

    {
        QSignalBlocker blocker(ui->toolButton_Power);
        ui->toolButton_Power->setChecked(!checked);
    }

    emit enabledChanged(m_deviceId, checked);
}

void IECDeviceRowWidget::on_pushButton_Settings_clicked()
{
    emit settingsRequested(m_deviceId);
}

void IECDeviceRowWidget::on_pushButton_Remove_clicked()
{
    emit removeRequested(m_deviceId);
}