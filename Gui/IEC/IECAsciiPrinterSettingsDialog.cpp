#include "IECAsciiPrinterSettingsDialog.h"
#include "ui_IECAsciiPrinterSettingsDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVariant>


IECAsciiPrinterSettingsDialog::IECAsciiPrinterSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IECAsciiPrinterSettingsDialog)
{
    ui->setupUi(this);
}

IECAsciiPrinterSettingsDialog::~IECAsciiPrinterSettingsDialog()
{
    delete ui;
}

void IECAsciiPrinterSettingsDialog::setConfiguration(const IECAsciiPrinterConfiguration& configuration, const QList<quint8>& availableDeviceNumbers)
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

IECAsciiPrinterConfiguration IECAsciiPrinterSettingsDialog::configuration() const
{
    IECAsciiPrinterConfiguration result = m_configuration;

    if (ui->comboBox_DeviceNumber->currentIndex() >= 0)
        result.setDeviceNumber(static_cast<quint8>(ui->comboBox_DeviceNumber->currentData().toUInt()));

    return result;
}
