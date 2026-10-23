#pragma once

#include <QDialog>
#include <QList>

#include "C64/IEC/IECAsciiPrinterConfiguration.h"

namespace Ui {
class IECAsciiPrinterSettingsDialog;
}

class IECAsciiPrinterSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IECAsciiPrinterSettingsDialog(QWidget* parent = nullptr);
    virtual ~IECAsciiPrinterSettingsDialog();

    void setConfiguration(const IECAsciiPrinterConfiguration& configuration, const QList<quint8>& availableDeviceNumbers);
    IECAsciiPrinterConfiguration configuration() const;

private:
    Ui::IECAsciiPrinterSettingsDialog* ui;
    IECAsciiPrinterConfiguration m_configuration;
};
