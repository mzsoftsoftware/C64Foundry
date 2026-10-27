#include "IECAsciiPrinterRowWidget.h"
#include "ui_IECAsciiPrinterRowWidget.h"

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

void IECDeviceRowWidget::setOperationState(IECOperationStatusWidget::State state, const QString& message)
{
    ui->widget_OperationStatus->setState(state, message);
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

void IECDeviceRowWidget::setBusy(bool busy)
{
    if (m_bBusy == busy)
        return;

    m_bBusy = busy;

    ui->toolButton_Power->setEnabled(!busy);
    ui->toolButton_Settings->setEnabled(!busy);
    ui->toolButton_Remove->setEnabled(!busy);
}

void IECDeviceRowWidget::triggerActivity(int index)
{
    ui->widget_ActivityLED->triggerActivity(index);
}

void IECDeviceRowWidget::on_toolButton_Power_toggled(bool checked)
{
    if (m_bBusy)
        return;

    {
        QSignalBlocker blocker(ui->toolButton_Power);
        ui->toolButton_Power->setChecked(!checked);
    }

    emit enabledChanged(m_deviceId, checked);
}
void IECDeviceRowWidget::on_toolButton_Settings_clicked()
{
    if (m_bBusy)
        return;

    emit settingsRequested(m_deviceId);
}

void IECDeviceRowWidget::on_toolButton_Remove_clicked()
{
    if (m_bBusy)
        return;

    emit removeRequested(m_deviceId);
}
