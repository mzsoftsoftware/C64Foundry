#pragma once

#include <QDialog>
#include <QList>

#include "C64/IEC/IECFoundryDiskConfiguration.h"

namespace Ui {
class IECFoundryDriveSettingsDialog;
}


class IECFoundryDriveSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IECFoundryDriveSettingsDialog(QWidget* parent = nullptr);
    virtual ~IECFoundryDriveSettingsDialog();

    void setConfiguration(const IECFoundryDiskConfiguration& configuration, const QList<quint8>& availableDeviceNumbers);
    IECFoundryDiskConfiguration configuration() const;

private:
    Ui::IECFoundryDriveSettingsDialog* ui;
    IECFoundryDiskConfiguration m_configuration;
};
