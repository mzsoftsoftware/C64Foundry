#include "IECAsciiPrinterRowWidget.h"
#include "ui_IECAsciiPrinterRowWidget.h"

#include <QDialog>
#include <QSignalBlocker>

#include "Emulator/EmulatorController.h"
#include "C64/IEC/IECSystemResponse.h"
#include "Gui/IEC/IECAsciiPrinterSettingsDialog.h"


IECAsciiPrinterRowWidget::IECAsciiPrinterRowWidget(QWidget* parent)
    : IECDeviceRowWidget(parent)
    , ui(new Ui::IECFoudnryDriveRowWidget)
{
    ui->setupUi(this);
}


IECAsciiPrinterRowWidget::~IECAsciiPrinterRowWidget()
{
    delete ui;
}


void IECAsciiPrinterRowWidget::onControllerSet()
{
    connect(m_ptrController, &EmulatorController::iecDeviceEventsChanged,
            this, &IECAsciiPrinterRowWidget::onIECDeviceEventsChanged);
}


void IECAsciiPrinterRowWidget::applyConfiguration(
    const IECDeviceConfiguration& configuration)
{
    if (configuration.type() != IECDeviceType::PrinterASCII)
        return;

    const IECAsciiPrinterConfiguration* ptrPrinterConfiguration =
        dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

    if (ptrPrinterConfiguration == nullptr)
        return;

    setPowerButtonChecked(ptrPrinterConfiguration->enabled());
    setDeviceNumberLabel(ui->label_DeviceNumber,
                         ptrPrinterConfiguration->deviceNumber());

    ui->label_DeviceType->setText(tr("ASCII Printer"));
}


void IECAsciiPrinterRowWidget::setPowerButtonChecked(bool checked)
{
    const QSignalBlocker blocker(ui->toolButton_Power);
    ui->toolButton_Power->setChecked(checked);
}


void IECAsciiPrinterRowWidget::setControlsEnabled(bool enabled)
{
    ui->toolButton_Power->setEnabled(enabled);
    ui->toolButton_Settings->setEnabled(enabled);
    ui->toolButton_Remove->setEnabled(enabled);
}


void IECAsciiPrinterRowWidget::setOperationState(
    IECOperationStatusWidget::State state,
    const QString& message)
{
    ui->widget_OperationStatus->setState(state, message);
}


void IECAsciiPrinterRowWidget::triggerActivity(int index)
{
    ui->widget_ActivityLED->triggerActivity(index);
}


void IECAsciiPrinterRowWidget::on_toolButton_Power_toggled(bool checked)
{
    requestPowerChange(checked);
}


void IECAsciiPrinterRowWidget::on_toolButton_Settings_clicked()
{
    requestSettings();
}


void IECAsciiPrinterRowWidget::on_toolButton_Remove_clicked()
{
    requestRemove();
}


void IECAsciiPrinterRowWidget::openSettingsDialog(
    const IECSystemGetConfigurationsResponse* ptrResponse)
{
    showSettingsDialog(ptrResponse);
}


void IECAsciiPrinterRowWidget::showSettingsDialog(
    const IECSystemGetConfigurationsResponse* ptrResponse)
{
    if (ptrResponse == nullptr || configuration() == nullptr)
    {
        setOperationState(IECOperationStatusWidget::State::Failed,
                          tr("Invalid IEC configuration"));
        return;
    }

    const IECAsciiPrinterConfiguration* ptrPrinterConfiguration =
        dynamic_cast<const IECAsciiPrinterConfiguration*>(configuration());

    if (ptrPrinterConfiguration == nullptr)
    {
        setOperationState(IECOperationStatusWidget::State::Failed,
                          tr("Invalid ASCII printer configuration"));
        return;
    }

    QList<quint8> availableDeviceNumbers =
        ptrResponse->supportedDeviceNumbers().value(IECDeviceType::PrinterASCII);

    const quint8 currentDeviceNumber =
        ptrPrinterConfiguration->deviceNumber();

    for (const QSharedPointer<IECDeviceConfiguration>& ptrOtherConfiguration :
         ptrResponse->configurations())
    {
        if (ptrOtherConfiguration.isNull() ||
            ptrOtherConfiguration->id() == ptrPrinterConfiguration->id())
        {
            continue;
        }

        const IECAsciiPrinterConfiguration* ptrOtherPrinter =
            dynamic_cast<const IECAsciiPrinterConfiguration*>(ptrOtherConfiguration.data());

        if (ptrOtherPrinter != nullptr)
            availableDeviceNumbers.removeAll(ptrOtherPrinter->deviceNumber());
    }

    if (!availableDeviceNumbers.contains(currentDeviceNumber))
        availableDeviceNumbers.append(currentDeviceNumber);

    IECAsciiPrinterSettingsDialog dialog(this);
    dialog.setConfiguration(*ptrPrinterConfiguration, availableDeviceNumbers);

    if (dialog.exec() != QDialog::Accepted)
    {
        setOperationState(IECOperationStatusWidget::State::Idle);
        return;
    }

    const IECAsciiPrinterConfiguration updatedConfiguration =
        dialog.configuration();

    requestSetConfiguration(updatedConfiguration);
}


void IECAsciiPrinterRowWidget::onIECDeviceEventsChanged(IECDeviceEventList events)
{
    for (const IECDeviceEventPtr& ptrEvent : events)
    {
        if (ptrEvent.isNull() || ptrEvent->deviceId != deviceId())
            continue;

        const IECPrinterOutputEvent* ptrPrinterEvent =
            dynamic_cast<const IECPrinterOutputEvent*>(ptrEvent.data());

        if (ptrPrinterEvent == nullptr)
            continue;

        triggerActivity();
        break;
    }
}
