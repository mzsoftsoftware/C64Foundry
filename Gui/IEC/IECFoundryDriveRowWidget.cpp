#include "IECFoundryDriveRowWidget.h"
#include "ui_IECFoundryDriveRowWidget.h"

#include <QSignalBlocker>

#include "C64/IEC/IECFoundryDriveConfiguration.h"
#include "C64/IEC/IECSystemResponse.h"


IECFoundryDriveRowWidget::IECFoundryDriveRowWidget(QWidget* parent)
    : IECDeviceRowWidget(parent)
    , ui(new Ui::IECFoundryDriveRowWidget)
{
    ui->setupUi(this);
}


IECFoundryDriveRowWidget::~IECFoundryDriveRowWidget()
{
    delete ui;
}


void IECFoundryDriveRowWidget::applyConfiguration(
    const IECDeviceConfiguration& configuration)
{
    if (configuration.type() != IECDeviceType::FoundryDrive)
        return;

    const IECFoundryDriveConfiguration* ptrDriveConfiguration =
        dynamic_cast<const IECFoundryDriveConfiguration*>(&configuration);

    if (ptrDriveConfiguration == nullptr)
        return;

    setPowerButtonChecked(ptrDriveConfiguration->enabled());

    ui->label_DeviceNumber->setText(
        QStringLiteral("#%1–#%2")
            .arg(ptrDriveConfiguration->startDeviceNumber(), 2, 10, QChar('0'))
            .arg(ptrDriveConfiguration->endDeviceNumber(), 2, 10, QChar('0')));

    ui->label_DeviceType->setText(tr("Foundry Drive"));
}


void IECFoundryDriveRowWidget::setPowerButtonChecked(bool checked)
{
    const QSignalBlocker blocker(ui->toolButton_Power);
    ui->toolButton_Power->setChecked(checked);
}


void IECFoundryDriveRowWidget::setControlsEnabled(bool enabled)
{
    ui->toolButton_Power->setEnabled(enabled);
    ui->toolButton_Settings->setEnabled(enabled);
    ui->toolButton_Remove->setEnabled(enabled);
}


void IECFoundryDriveRowWidget::setOperationState(
    IECOperationStatusWidget::State state,
    const QString& message)
{
    ui->widget_OperationStatus->setState(state, message);
}


void IECFoundryDriveRowWidget::on_toolButton_Power_toggled(bool checked)
{
    requestPowerChange(checked);
}


void IECFoundryDriveRowWidget::on_toolButton_Settings_clicked()
{
    requestSettings();
}


void IECFoundryDriveRowWidget::on_toolButton_Remove_clicked()
{
    requestRemove();
}


void IECFoundryDriveRowWidget::openSettingsDialog(
    const IECSystemGetConfigurationsResponse* ptrResponse)
{
    Q_UNUSED(ptrResponse)

    if (configuration() == nullptr)
    {
        setOperationState(IECOperationStatusWidget::State::Failed, tr("Invalid IEC configuration"));
        return;
    }

    const IECFoundryDriveConfiguration* ptrDriveConfiguration = dynamic_cast<const IECFoundryDriveConfiguration*>(configuration());

    if (ptrDriveConfiguration == nullptr)
    {
        setOperationState(IECOperationStatusWidget::State::Failed, tr("Invalid Foundry Drive configuration"));
        return;
    }

    IECFoundryDriveSettingsDialog dialog(this);
    dialog.setConfiguration(*ptrDriveConfiguration);

    if (dialog.exec() != QDialog::Accepted)
    {
        setOperationState(IECOperationStatusWidget::State::Idle);
        return;
    }

    const IECFoundryDriveConfiguration updatedConfiguration = dialog.configuration();
    requestSetConfiguration(updatedConfiguration);
}
