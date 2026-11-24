#include "IECFoundryDriveSettingsDialog.h"
#include "ui_IECFoundryDriveSettingsDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVariant>


IECFoundryDriveSettingsDialog::IECFoundryDriveSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IECFoundryDriveSettingsDialog)
{
    ui->setupUi(this);
}

IECFoundryDriveSettingsDialog::~IECFoundryDriveSettingsDialog()
{
    delete ui;
}

void IECFoundryDriveSettingsDialog::setConfiguration(const IECFoundryDiskConfiguration& configuration, const QList<quint8>& availableDeviceNumbers)
{
    m_configuration = configuration;

    ui->comboBox_DeviceNumber->clear();
    for (quint8 deviceNumber : availableDeviceNumbers)
    {
        ui->comboBox_DeviceNumber->addItem(QStringLiteral("#%1").arg(deviceNumber), static_cast<int>(deviceNumber));
    }
    const int index = ui->comboBox_DeviceNumber->findData(static_cast<int>(configuration.deviceNumber()));
    if (index >= 0)
        ui->comboBox_DeviceNumber->setCurrentIndex(index);

    ui->checkBox_SaveCurrentDiskOnExit->setChecked(configuration.saveCurrentDiskOnExit());

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->comboBox_DeviceNumber->count() > 0);
}

IECFoundryDiskConfiguration IECFoundryDriveSettingsDialog::configuration() const
{
    IECFoundryDiskConfiguration result = m_configuration;

    if (ui->comboBox_DeviceNumber->currentIndex() >= 0)
        result.setDeviceNumber(static_cast<quint8>(ui->comboBox_DeviceNumber->currentData().toUInt()));

    result.setSaveCurrentDiskOnExit(ui->checkBox_SaveCurrentDiskOnExit->isChecked());

    return result;
}
