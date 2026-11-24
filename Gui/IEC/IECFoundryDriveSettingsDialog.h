#pragma once

#include <QDialog>
#include <QList>

#include "C64/IEC/IECFoundryDriveConfiguration.h"

namespace Ui {
class IECFoundryDriveSettingsDialog;
}


class IECFoundryDriveSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IECFoundryDriveSettingsDialog(QWidget* parent = nullptr);
    virtual ~IECFoundryDriveSettingsDialog();

    void setConfiguration(const IECFoundryDriveConfiguration& configuration);
    IECFoundryDriveConfiguration configuration() const;

private slots:
    void on_spinBox_StartDevice_valueChanged(int value);
    void on_spinBox_SlotCount_valueChanged(int value);
    void onSlotDeviceNumberChanged(int index);

private:
    void updateStartDeviceMaximum();
    void updateReservedRangeLabel();
    void updateDeviceNumberAvailability();

    void addSlotRow();
    void removeSlotRow();

private:
    Ui::IECFoundryDriveSettingsDialog* ui;
    IECFoundryDriveConfiguration m_configuration;
};
