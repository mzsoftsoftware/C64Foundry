#pragma once

#include <QDialog>
#include <QList>

#include "C64/IEC/IECFoundryDiskConfiguration.h"

namespace Ui {
class IECFoundryDiskSettingsDialog;
}


class IECFoundryDiskSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IECFoundryDiskSettingsDialog(QWidget* parent = nullptr);
    virtual ~IECFoundryDiskSettingsDialog();

    void setConfiguration(const IECFoundryDiskConfiguration& configuration, const QList<quint8>& availableDeviceNumbers);
    IECFoundryDiskConfiguration configuration() const;

private:
    Ui::IECFoundryDiskSettingsDialog* ui;
    IECFoundryDiskConfiguration m_configuration;
};
