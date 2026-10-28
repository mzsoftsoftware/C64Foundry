#include "IECFoundryDiskRowWidget.h"
#include "ui_IECFoundryDiskRowWidget.h"

#include <QDialog>
#include <QSignalBlocker>

#include "C64/IEC/IECFoundryDiskConfiguration.h"
#include "Emulator/EmulatorController.h"
#include "Emulator/EmulatorResponse.h"
#include "Gui/IEC/IECFoundryDiskSettingsDialog.h"


IECFoundryDiskRowWidget::IECFoundryDiskRowWidget(QWidget* parent)
    : IECDeviceRowWidget(parent)
    , ui(new Ui::IECFoundryDiskRowWidget)
{
    ui->setupUi(this);

    ui->widget_ActivityLED->setLEDCount(2);
}

IECFoundryDiskRowWidget::~IECFoundryDiskRowWidget()
{
    delete ui;
}

void IECFoundryDiskRowWidget::onControllerSet()
{
    // Hier kommen später laufwerksspezifische Controller-Signale hinzu.
}

void IECFoundryDiskRowWidget::applyConfiguration(const IECDeviceConfiguration& configuration)
{
    if (configuration.type() != IECDeviceType::FoundryDisk)
        return;

    const IECFoundryDiskConfiguration* ptrDiskConfiguration = dynamic_cast<const IECFoundryDiskConfiguration*>(&configuration);
    if (ptrDiskConfiguration == nullptr)
        return;

    setPowerButtonChecked(ptrDiskConfiguration->enabled());
    setDeviceNumberLabel(ui->label_DeviceNumber, ptrDiskConfiguration->deviceNumber());
    ui->label_DeviceType->setText(tr("Foundry Disk"));
}

void IECFoundryDiskRowWidget::setPowerButtonChecked(bool checked)
{
    const QSignalBlocker blocker(ui->toolButton_Power);
    ui->toolButton_Power->setChecked(checked);
}

void IECFoundryDiskRowWidget::setControlsEnabled(bool enabled)
{
    ui->toolButton_Power->setEnabled(enabled);
    ui->toolButton_Disk->setEnabled(enabled);
    ui->toolButton_Eject->setEnabled(enabled);
    ui->toolButton_DiskName->setEnabled(enabled);
    ui->toolButton_Settings->setEnabled(enabled);
    ui->toolButton_Remove->setEnabled(enabled);
}

void IECFoundryDiskRowWidget::setOperationState(
    IECOperationStatusWidget::State state,
    const QString& message)
{
    ui->widget_OperationStatus->setState(state, message);
}

void IECFoundryDiskRowWidget::on_toolButton_Power_toggled(bool checked)
{
    requestPowerChange(checked);
}

void IECFoundryDiskRowWidget::on_toolButton_Disk_clicked()
{
    // Einlegen eines Disk-Images wird im nächsten Schritt implementiert.
}

void IECFoundryDiskRowWidget::on_toolButton_Eject_clicked()
{
    // Auswerfen einer Disk wird im nächsten Schritt implementiert.
}

void IECFoundryDiskRowWidget::on_toolButton_DiskName_clicked()
{
    // Das Directory-Popup wird im nächsten Schritt implementiert.
}

void IECFoundryDiskRowWidget::on_toolButton_Settings_clicked()
{
    requestSettings();
}

void IECFoundryDiskRowWidget::on_toolButton_Remove_clicked()
{
    requestRemove();
}

void IECFoundryDiskRowWidget::openSettingsDialog(
    const EmulatorResponseIEC* ptrResponse)
{
    showSettingsDialog(ptrResponse);
}

void IECFoundryDiskRowWidget::showSettingsDialog(
    const EmulatorResponseIEC* ptrResponse)
{
    if (ptrResponse == nullptr || configuration() == nullptr)
    {
        setOperationState(IECOperationStatusWidget::State::Failed,
                          tr("Invalid IEC configuration"));
        return;
    }

    const IECFoundryDiskConfiguration* ptrDiskConfiguration =
        dynamic_cast<const IECFoundryDiskConfiguration*>(configuration());

    if (ptrDiskConfiguration == nullptr)
    {
        setOperationState(IECOperationStatusWidget::State::Failed,
                          tr("Invalid Foundry Disk configuration"));
        return;
    }

    QList<quint8> availableDeviceNumbers =
        ptrResponse->supportedDeviceNumbers().value(IECDeviceType::FoundryDisk);

    const quint8 currentDeviceNumber = ptrDiskConfiguration->deviceNumber();

    if (!availableDeviceNumbers.contains(currentDeviceNumber))
        availableDeviceNumbers.append(currentDeviceNumber);

    IECFoundryDiskSettingsDialog dialog(this);
    dialog.setConfiguration(*ptrDiskConfiguration, availableDeviceNumbers);

    if (dialog.exec() != QDialog::Accepted)
    {
        setOperationState(IECOperationStatusWidget::State::Idle);
        return;
    }

    const IECFoundryDiskConfiguration updatedConfiguration =
        dialog.configuration();

    requestSetConfiguration(updatedConfiguration);
}
