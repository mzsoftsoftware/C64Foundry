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

void IECFoundryDriveSettingsDialog::on_spinBox_StartDevice_valueChanged(int value)
{
    const int slotCount = ui->spinBox_SlotCount->value();
    const int lastDeviceNumber = startDeviceNumber + slotCount - 1;

    for (int slotIndex = 0; slotIndex < slotCount; ++slotIndex)
    {
        QComboBox* comboBox = qobject_cast<QComboBox*>(ui->tableWidget_Slots->cellWidget(slotIndex, 0));
        if (comboBox == nullptr)
        {
            continue;
        }

        const int previousDeviceNumber = comboBox->currentData().toInt();
        comboBox->clear();
        for (int deviceNumber = startDeviceNumber; deviceNumber <= lastDeviceNumber; ++deviceNumber)
        {
            comboBox->addItem(QString("#%1").arg(deviceNumber), deviceNumber);
        }
        int selectedIndex = comboBox->findData(previousDeviceNumber);
        if (selectedIndex < 0)
        {
            selectedIndex = slotIndex;
        }
        comboBox->setCurrentIndex(selectedIndex);
    }

    updateReservedRangeLabel();
    updateDeviceNumberAvailability();
}
void IECFoundryDriveSettingsDialog::on_spinBox_SlotCount_valueChanged(int value)
{
    while (ui->tableWidget_Slots->rowCount() < slotCount)
    {
        addSlotRow();
    }

    while (ui->tableWidget_Slots->rowCount() > slotCount)
    {
        removeSlotRow();
    }

    updateStartDeviceMaximum();
    updateReservedRangeLabel();
    updateDeviceNumberAvailability();
}


void IECFoundryDriveSettingsDialog::onSlotDeviceNumberChanged(int index)
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(sender());

    if (comboBox == nullptr || index < 0)
    {
        return;
    }

    updateDeviceNumberAvailability();
}

void IECFoundryDriveSettingsDialog::updateStartDeviceMaximum()
{
    const int slotCount = ui->spinBox_SlotCount->value();
    const int maximumStartDeviceNumber = 30 - slotCount + 1;

    ui->spinBox_StartDevice->setMaximum(maximumStartDeviceNumber);
}
void IECFoundryDriveSettingsDialog::updateReservedRangeLabel()
{
    const int startDeviceNumber = ui->spinBox_StartDevice->value();
    const int endDeviceNumber = startDeviceNumber + ui->spinBox_SlotCount->value() - 1;
    ui->label_ReservedRangeValue->setText(QStringLiteral("#%1 – #%2").arg(startDeviceNumber, 2, 10, QLatin1Char('0')).arg(endDeviceNumber, 2, 10, QLatin1Char('0')));
}

void IECFoundryDriveSettingsDialog::updateDeviceNumberAvailability()
{
    for (int slotIndex = 0; slotIndex < ui->tableWidget_Slots->rowCount(); ++slotIndex)
    {
        QComboBox* comboBox = qobject_cast<QComboBox*>(ui->tableWidget_Slots->cellWidget(slotIndex, 0));
        if (comboBox == nullptr)
        {
            continue;
        }

        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(comboBox->model());
        if (model == nullptr)
        {
            continue;
        }

        for (int itemIndex = 0; itemIndex < comboBox->count(); ++itemIndex)
        {
            const int deviceNumber = comboBox->itemData(itemIndex).toInt();
            bool available = true;

            for (int otherSlotIndex = 0; otherSlotIndex < ui->tableWidget_Slots->rowCount(); ++otherSlotIndex)
            {
                if (otherSlotIndex == slotIndex)
                {
                    continue;
                }

                QComboBox* otherComboBox = qobject_cast<QComboBox*>(ui->tableWidget_Slots->cellWidget(otherSlotIndex, 0));
                if (otherComboBox != nullptr && otherComboBox->currentData().toInt() == deviceNumber)
                {
                    available = false;
                    break;
                }
            }

            model->item(itemIndex)->setEnabled(available || comboBox->currentData().toInt() == deviceNumber);
        }
    }
}

void IECFoundryDriveSettingsDialog::addSlotRow()
{
    const int slotIndex = ui->tableWidget_Slots->rowCount();

    ui->tableWidget_Slots->insertRow(slotIndex);

    QTableWidgetItem* slotItem = new QTableWidgetItem(QString::number(slotIndex + 1));
    ui->tableWidget_Slots->setVerticalHeaderItem(slotIndex, slotItem);

    QComboBox* comboBox = new QComboBox(ui->tableWidget_Slots);

    const int firstDeviceNumber = ui->spinBox_StartDevice->value();
    const int lastDeviceNumber = firstDeviceNumber + ui->spinBox_SlotCount->value() - 1;

    for (int deviceNumber = firstDeviceNumber; deviceNumber <= lastDeviceNumber; ++deviceNumber)
    {
        comboBox->addItem(QString("#%1").arg(deviceNumber), deviceNumber);
    }

    comboBox->setCurrentIndex(slotIndex);

    connect(comboBox, &QComboBox::activated, this, &IECFoundryDriveSettingsDialog::onSlotDeviceNumberChanged);

    ui->tableWidget_Slots->setCellWidget(slotIndex, 0, comboBox);

    QCheckBox* checkBox = new QCheckBox(tr("Save Current Image on Exit"), ui->tableWidget_Slots);

    ui->tableWidget_Slots->setCellWidget(slotIndex, 1, checkBox);
    ui->tableWidget_Slots->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget_Slots->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void IECFoundryDriveSettingsDialog::removeSlotRow()
{
    const int lastRow = ui->tableWidget_Slots->rowCount() - 1;

    if (lastRow >= 0)
    {
        ui->tableWidget_Slots->removeRow(lastRow);
    }
}