#include "IECFoundryDiskSettingsDialog.h"
#include "ui_IECFoundryDiskSettingsDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVariant>


IECFoundryDiskSettingsDialog::IECFoundryDiskSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IECFoundryDiskSettingsDialog)
{
    ui->setupUi(this);
}

IECFoundryDiskSettingsDialog::~IECFoundryDiskSettingsDialog()
{
    delete ui;
}

void IECFoundryDiskSettingsDialog::setConfiguration(const IECFoundryDiskConfiguration& configuration, const QList<quint8>& availableDeviceNumbers)
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

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->comboBox_DeviceNumber->count() > 0);
}

IECFoundryDiskConfiguration IECFoundryDiskSettingsDialog::configuration() const
{
    IECFoundryDiskConfiguration result = m_configuration;

    if (ui->comboBox_DeviceNumber->currentIndex() >= 0)
        result.setDeviceNumber(static_cast<quint8>(ui->comboBox_DeviceNumber->currentData().toUInt()));

    return result;
}
